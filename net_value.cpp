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
	Serial.print("Configure Device: http://");
	Serial.print(localIPstr());
	Serial.println("/config");

	Serial.print("Read Measured Data: http://");
	Serial.println(localIPstr());

	Serial.print("Read API: http://");
	Serial.print(localIPstr());
	Serial.println(uri_v);
	Serial.println("?read=api        :: read ThingSpeak API key");
	Serial.println("?read=rssi       :: read AP signal strength in dBm at connect time");
	Serial.println("?read=deviceid   :: read deviceid number");
	Serial.println("?read=name       :: read name of device");
	Serial.println("?read=thingspeak :: read thingspeak api codes");
	Serial.println("?read=csv        :: read summary of sensor data");
	Serial.println("?reset=0         :: reboot device");
	Serial.println("");
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
