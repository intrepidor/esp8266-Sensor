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
#include "thingspeak.h"
#include "debugprint.h"
#include "main.h"
#include "queue.h"
#include "deviceinfo.h"
#include "wdog.h"
#include "util.h"

extern String getThingspeakGET(void);
extern String getTCPStatusString(uint8_t s);

size_t thingspeak_update_counter = 0;
size_t thingspeak_error_counter = 0;
long thinkspeak_total_entries = 0; // this is filled in by the response from the REST update
unsigned long last_thingspeak_update_time_ms = 0;

String getsThingspeakInfo(String eol) {
	unsigned long next_update = dinfo.getThingspeakUpdatePeriodMS()
			- (millis() - last_thingspeak_update_time_ms);
	String s("== Thingspeak Information ==" + eol);
	s += "Enabled: " + dinfo.getThingspeakEnableString() + eol;
	s += "Update Period[sec]: " + String(dinfo.getThingspeakUpdatePeriodSeconds()) + eol;
	s += "Queue.recur[ms]" + String(myQueue.getTimeInterval("thingspeak")) + eol;
	s += "Next Update[sec]: " + String(next_update / MS_PER_SECOND) + eol;
	s += "URL: " + dinfo.getThingspeakURL() + " (future use)" + eol;
	s += "Write Key: " + dinfo.getThingspeakApikey() + eol;
	s += "User API Key: " + dinfo.getThingspeakUserApikey() + eol;
	s += "Channel: " + String(dinfo.getThingspeakChannel()) + " (future use)" + eol;
	s += "IP Address: " + dinfo.getThingspeakIpaddr() + eol;
	s += "Updates: " + String(thingspeak_update_counter) + eol;
	s += "Errors: " + String(thingspeak_error_counter) + eol;
	s += "Entries: " + String(thinkspeak_total_entries) + eol;
	return s;
}

String getsThingspeakChannelInfo(String eol) {
	String s("== Thingspeak Channel Info ==" + eol);
	int fld = 0;
	bool fields[MAX_THINGSPEAK_FIELD_COUNT] = { 0 };
	s += "Channel Name: " + dinfo.getTSChannelName() + eol;
	s += "Channel Desc: " + dinfo.getTSChannelDesc() + eol;
	s += "Extra Fields: " + String(dinfo.getTSFieldExtraMax()) + eol;
	for (int i = 0; i < dinfo.getTSFieldExtraMax(); i++) {
		fld = dinfo.getTSFieldExtraNumber(i);
		if (fld < 1) fld = 1;
		s += "  " + getNameByPosition(i + dinfo.getTSFieldPortMax()) + "= field" + String(fld);
		if (fields[fld - 1]) {
			s += " IGNORED AS DUPLICATE";
		}
		fields[fld - 1] = true;
		s += eol;
	}
	s += "Port Fields: " + String(dinfo.getTSFieldPortMax()) + eol;
	for (int i = 0; i < dinfo.getTSFieldPortMax(); i++) {
		fld = dinfo.getTSFieldPortNumber(i);
		if (fld < 1) fld = 1; // this avoid buffer overrun, but also creates a potential bug
		s += "  " + getNameByPosition(i) + "= field" + String(fld);
		if (fields[fld - 1]) {
			s += " IGNORED AS DUPLICATE";
		}
		fields[fld - 1] = true;
		s += eol;
	}
	s += "== Field Mapping to Position == " + eol;
	for (fld = 1; fld <= MAX_THINGSPEAK_FIELD_COUNT; fld++) {
		int _pos = getPositionByTSFieldNumber(fld);
		s += "  Field" + String(fld) + " = Position" + _pos + ", " + getNameByPosition(_pos) + eol;
	}
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

/* --------------------------------------------------------------------------------
 * Push the Channel Settings to the Thingspeak Server
 */
void ThingspeakPushChannelSettings(void) {
}

/* --------------------------------------------------------------------------------
 * Update the Thingspeak channel feed
 */
String getThingspeakGET(void) {
	String getStr = "/update?api_key=" + dinfo.getThingspeakApikey();
	for (int fld = 1; fld <= MAX_THINGSPEAK_FIELD_COUNT; fld++) {
		int pos = getPositionByTSFieldNumber(fld);
		getStr += "&field" + String(fld) + "=" + getValueByPosition(pos);
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

