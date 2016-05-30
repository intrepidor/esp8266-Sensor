/*
 * thingspeak.cpp
 *
 *  Created on: May 30, 2016
 *      Author: allan
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "debugprint.h"
#include "main.h"
#include "deviceinfo.h"

void printThingspeakInfo(void) {
	debug.println(DebugLevel::ALWAYS, "== Thingspeak Configuration ==");
	debug.println(DebugLevel::ALWAYS, "Enable: " + dinfo.getThingspeakEnableString());
	debug.println(DebugLevel::ALWAYS, "URL: " + dinfo.getThingspeakURL());
	debug.println(DebugLevel::ALWAYS, "Write Key: " + dinfo.getThinkspeakApikey());
	debug.println(DebugLevel::ALWAYS, "Channel" + dinfo.getThingspeakChannel());
	debug.println(DebugLevel::ALWAYS, "IP Address" + dinfo.getThingspeakIpaddr());
}

void updateThingspeak(void) {
	WiFiClient client;
	int const MAX_THINGSPEAK_FIELD_COUNT = 8; // Thingspeak only accept 8 fields

	if (!client.connect(dinfo.getThingspeakURL().c_str(), 80)) {
		debug.println(DebugLevel::ALWAYS, "error: connection to " + dinfo.getThingspeakURL() + " failed");
	}
	else {
		// Send message to Thingspeak
		String getStr = "/update?api_key=";
		getStr += dinfo.getThinkspeakApikey();
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
		debug.println(DebugLevel::DEBUG, "Thinkspeak URL:" + getStr);
		client.print(
				String("GET ") + getStr + " HTTP/1.1\r\n" + "Host: " + dinfo.getThingspeakURL() + "\r\n"
						+ "Connection: close\r\n\r\n");
		delay(10);

		// Read the response
		dinfo.setThingspeakStatus(0);
		while (client.available()) {
			String line = client.readStringUntil('\r');
			if (line.startsWith("HTTP/1.1 200 OK")) {
				dinfo.setThingspeakStatus(dinfo.getThingspeakStatus() | 0x01);
				debug.println(DebugLevel::DEBUG, line);
			}
			if (line.startsWith("Status: 200 OK")) {
				dinfo.setThingspeakStatus(dinfo.getThingspeakStatus() | 0x02);
				debug.println(DebugLevel::DEBUG, line);
			}
			if (line.startsWith("error")) {
				dinfo.setThingspeakStatus(dinfo.getThingspeakStatus() | 0x04);
				debug.println(DebugLevel::DEBUG, line);
			}
		}
	}
}

