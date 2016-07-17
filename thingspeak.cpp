/*
 * thingspeak.cpp
 *
 *  Created on: May 30, 2016
 *      Author: allan
 */

#include <Arduino.h>
#include <WString.h>
#include <ESP8266WiFi.h>
#include "include/wl_definitions.h"
#include "debugprint.h"
#include "main.h"
#include "queue.h"
#include "deviceinfo.h"
#include "wdog.h"

extern String getThingspeakGET(void);
extern String getTCPStatusString(uint8_t s);

size_t thingspeak_update_counter = 0;
size_t thingspeak_error_counter = 0;
long thinkspeak_total_entries = 0; // this is filled in by the response from the REST update

String getsThingspeakInfo(String eol) {
	String s("== Thingspeak Information ==" + eol);
	s += "Enabled: " + dinfo.getThingspeakEnableString() + eol;
	s += "Update Period[s]: " + String(dinfo.getThingspeakUpdatePeriodSeconds()) + " Queue.recur="
			+ String(myQueue.getTimeInterval("thingspeak")) + " ms" + eol;
	s += "Next Update: " + String(myQueue.getTimeTillRun("thingspeak")) + " millis()" + eol;
	s += "URL: " + dinfo.getThingspeakURL() + " (future use)" + eol;
	s += "Write Key: " + dinfo.getThingspeakApikey() + eol;
	s += "Channel: " + String(dinfo.getThingspeakChannel()) + " (future use)" + eol;
	s += "IP Address: " + dinfo.getThingspeakIpaddr() + eol;
	s += "Updates: " + String(thingspeak_update_counter) + eol;
	s += "Errors: " + String(thingspeak_error_counter) + eol;
	s += "Entries: " + String(thinkspeak_total_entries) + eol;
	return s;
}

String getTCPStatusString(uint8_t s) {
	String r = String("");
	switch (s) {
		case wl_tcp_state::CLOSED:
			r = "CLOSED";
			break;
		case wl_tcp_state::LISTEN:
			r = "LISTEN";
			break;
		case wl_tcp_state::SYN_SENT:
			r = "SYN_SENT";
			break;
		case wl_tcp_state::SYN_RCVD:
			r = "SYN_RCVD";
			break;
		case wl_tcp_state::ESTABLISHED:
			r = "ESTABLISHED";
			break;
		case wl_tcp_state::FIN_WAIT_1:
			r = "FIN_WAIT_1";
			break;
		case wl_tcp_state::FIN_WAIT_2:
			r = "FIN_WAIT_2";
			break;
		case wl_tcp_state::CLOSE_WAIT:
			r = "CLOSE_WAIT";
			break;
		case wl_tcp_state::CLOSING:
			r = "CLOSING";
			break;
		case wl_tcp_state::LAST_ACK:
			r = "LAST_ACK";
			break;
		case wl_tcp_state::TIME_WAIT:
			r = "TIME_WAIT";
			break;
		default:
			r = "unknown";
			break;
	}
	return r;
}

String getThingspeakGET(void) {
	int const MAX_THINGSPEAK_FIELD_COUNT = 8; // Thingspeak only accept 8 fields
	String getStr = "/update?api_key=" + dinfo.getThingspeakApikey();

	getStr += "&field1=" + String(PIRcount); // The built-in PIR sensor is always field1
// This code is written so the fields are static regardless of which sensor is used in
//    which port, or how many ports are present, or how many values are enabled per
//    a sensor. This is done so the Thingspeak channel configuration is not dependent
//    on the configuration of the device. Since Thingspeak only permits 8 channels,
//    and there are nine possible, 1 PIR + 2 per each of 4 sensors, the 4th sensor's
//    second value will not be sent to Thingspeak.
	int nextfield = 2;
	for (int i = 0; i < SENSOR_COUNT && nextfield <= MAX_THINGSPEAK_FIELD_COUNT; i++) {
		if (sensors[i]) {
			for (int j = 0; j < getSensorValueCount() && nextfield <= MAX_THINGSPEAK_FIELD_COUNT; j++) {
				getStr += "&field" + String(nextfield++) + "=";
				if (sensors[i]->getValueEnable(j)) {
					getStr += String(sensors[i]->getValue(j));
				}
				else {
					getStr += "0";
				}
			}
		}
		else { // sensor not present; assume zero for the values
			for (int k = 0; k < getSensorValueCount() && nextfield <= MAX_THINGSPEAK_FIELD_COUNT; k++) {
				getStr += "&field" + String(nextfield++) + "=0";
			}
		}
	}
	return getStr;
}

void updateThingspeak(void) {
	WiFiClient client;

	// Create the connection
	kickExternalWatchdog();
	if (!client.connect(dinfo.getThingspeakIpaddr_c(), 80)) {
		Serial.println(
				nl + "Connecting to " + dinfo.getThingspeakIpaddr() + ": "
						+ getTCPStatusString(client.status()) + " : FAILED");
		thingspeak_error_counter++;
	}
	else {
		kickExternalWatchdog();
		debug.println(DebugLevel::DEBUG,
				nl + "Connecting to " + dinfo.getThingspeakIpaddr() + ": "
						+ getTCPStatusString(client.status()) + " : Success");

		// Create the command
		String getStr = "GET " + getThingspeakGET() + "HTTP/1.1";

		// Send the command
		debug.println(DebugLevel::DEBUG, "Sending -> " + getStr);
		yield();
		kickExternalWatchdog();
		size_t r = client.println(getStr); // this takes about 1 second (no watchdogs during this time)
		kickExternalWatchdog();
		yield();
		r += client.println("Host: " + dinfo.getThingspeakIpaddr());
		r += client.println("Connection: close");
		r += client.println();

		// Read the response
		debug.println(DebugLevel::DEBUG, "        -> " + String(r) + " bytes sent");

		if (r == 0) {
			Serial.println(getStr + "  Send FAILED");
			thingspeak_error_counter++;
		}
		else if (client.available()) {
			String response = client.readString();
			debug.println(DebugLevel::DEBUG, "Response: " + response);
			if (isdigit(response.charAt(0))) {
				thinkspeak_total_entries = response.toInt();
				thingspeak_update_counter++;
			}
		}
	}
}

