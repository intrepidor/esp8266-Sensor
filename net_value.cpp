/*
 * net_value.cpp
 *
 *  Created on: Jan 30, 2016
 *      Author: Allan Inda
 */

#include "network.h"
#include "temperature.h"
#include "main.h"
#include "deviceinfo.h"
#include "net_value.h"
#include "util.h"

void WebPrintInfo(void) {
	// Describe Webserver access
	debug.print(DebugLevel::ALWAYS, "Configure Device: http://");
	debug.print(DebugLevel::ALWAYS, localIPstr());
	debug.println(DebugLevel::ALWAYS, "/config");

	debug.print(DebugLevel::ALWAYS, "Read Measured Data: http://");
	debug.println(DebugLevel::ALWAYS, localIPstr());

	debug.print(DebugLevel::ALWAYS, "Read API: http://");
	debug.print(DebugLevel::ALWAYS, localIPstr());
	debug.println(DebugLevel::ALWAYS, uri_v);
	debug.println(DebugLevel::ALWAYS, "?read=api        :: read ThingSpeak API key");
	debug.println(DebugLevel::ALWAYS, "?read=rssi       :: read AP signal strength in dBm at connect time");
	debug.println(DebugLevel::ALWAYS, "?read=deviceid   :: read deviceid number");
	debug.println(DebugLevel::ALWAYS, "?read=name       :: read name of device");
	debug.println(DebugLevel::ALWAYS, "?read=thingspeak :: read thingspeak api codes");
	debug.println(DebugLevel::ALWAYS, "?read=csv        :: read summary of sensor data");
	debug.println(DebugLevel::ALWAYS, "?reset=0         :: reboot device");
	debug.println(DebugLevel::ALWAYS, "");
}

void sendValue(void) {

	String file("<!DOCTYPE HTML><html><head></head><body>,");
	String value("");
// READ API
	bool arg = server.hasArg("read");
	if (arg) {
		String sarg = server.arg("read");
		if (sarg == "factory_default_ssid") {
			value = String(factory_default_ssid);
		}
		if (sarg == "api") {
			value = String(dinfo.getcThinkspeakApikey());
		}
		if (sarg == "rssi") {
			value = String(rssi);
		}
		if (sarg == "deviceid") {
			value = String(dinfo.getDeviceID());
		}
		if (sarg == "name") {
			value = String(dinfo.getDeviceName());
		}
		if (sarg == "thingspeak") {
			value = String(dinfo.getThingspeakStatus());
		}
		if (sarg == "csv") {
			value = String("");
			for (int i = 0; i < SENSOR_COUNT; i++) {
				if (sensors[i]) {
					for (int j = 0; j < getValueCount(); j++) {
						if (sensors[i]->getValueEnable(j)) {
							value += String("val") + String(i) + String(j) + "="
									+ String(sensors[i]->getValue(j)) + ",";
						}
					} // for j
					for (int j = 0; j < getCalCount(); j++) {
						if (sensors[i]->getCalEnable(j)) {
							value += String("cal") + String(i) + String(j) + "="
									+ String(sensors[i]->getCal(j)) + ",";
						}
					} // for j
				} // for i
			}
			value += "pir=" + String(PIRcount) + ",ts=";
			value += String(dinfo.getThingspeakStatus());
		}
	}

	if (server.hasArg("reset")) {
		reset();
	}

	file += value;
	file += String(",</body></html>");
	server.send(200, "text/html", file);
}
