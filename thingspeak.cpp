/*
 * thingspeak.cpp
 *
 *  Created on: May 30, 2016
 *      Author: allan
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "include/wl_definitions.h"
#include "debugprint.h"
#include "main.h"
#include "deviceinfo.h"

size_t thingspeak_update_counter = 0;
size_t thingspeak_error_counter = 0;
String getThingspeakGET(void);
String getTCPStatusString(uint8_t s);

void printThingspeakInfo(void) {
	debug.println(DebugLevel::ALWAYS, "== Thingspeak Information ==");
	debug.println(DebugLevel::ALWAYS, "Enabled: " + dinfo.getThingspeakEnableString());
	debug.println(DebugLevel::ALWAYS, "URL: " + dinfo.getThingspeakURL() + " (future use)");
	debug.println(DebugLevel::ALWAYS, "Write Key: " + dinfo.getThingspeakApikey());
	debug.println(DebugLevel::ALWAYS, "Channel: " + dinfo.getThingspeakChannel() + " (future use)");
	debug.println(DebugLevel::ALWAYS, "IP Address: " + dinfo.getThingspeakIpaddr());
	debug.println(DebugLevel::ALWAYS, "Updates: " + String(thingspeak_update_counter));
	debug.println(DebugLevel::ALWAYS, "Errors: " + String(thingspeak_error_counter));
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
	String getStr = "GET /update?api_key=" + dinfo.getThingspeakApikey();

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
			for (int j = 0; j < getValueCount() && nextfield <= MAX_THINGSPEAK_FIELD_COUNT; j++) {
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
			for (int k = 0; k < getValueCount() && nextfield <= MAX_THINGSPEAK_FIELD_COUNT; k++) {
				getStr += "&field" + String(nextfield++) + "=0";
			}
		}
	}
	return getStr;
}

void updateThingspeak(void) {
	WiFiClient client;
	unsigned int r;

	int connect_ret = client.connect(dinfo.getThingspeakIpaddr_c(), 80);

	if (!connect_ret) {
		debug.println(DebugLevel::ALWAYS,
				nl + "Connecting to " + dinfo.getThingspeakIpaddr() + ": "
						+ getTCPStatusString(client.status()) + " : FAILED");
		thingspeak_error_counter++;
	}
	else {
		debug.println(DebugLevel::DEBUG,
				nl + "Connecting to " + dinfo.getThingspeakIpaddr() + ": "
						+ getTCPStatusString(client.status()) + " : Success");

		String getStr = getThingspeakGET();
		debug.println(DebugLevel::DEBUG, "Sending -> " + getStr);

		r = client.print(
				getStr + " HTPP/1.1\r\nHost: " + dinfo.getThingspeakIpaddr()
						+ "\r\nConnection: close\r\n\r\n");

		optimistic_yield(50000); // 50,000 us, or 0.1 seconds

		// Read the response
		debug.println(DebugLevel::DEBUG, "        -> " + String(r) + " bytes sent");
		if (r == 0) {
			debug.println(DebugLevel::ALWAYS, "  Send FAILED");
			thingspeak_error_counter++;
		}
		else {
			thingspeak_update_counter++;
		}

		while (client.available()) {
			String line = client.readStringUntil('\r');
			debug.print(DebugLevel::DEBUG, ">> " + line);
		}
//		client.stop();
	}
}

