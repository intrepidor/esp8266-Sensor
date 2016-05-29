/*
 * net_value.cpp
 *
 *  Created on: Jan 30, 2016
 *      Author: Allan Inda
 */

#include "network.h"
#include "temperature.h"
#include "main.h"
#include "reset.h"
#include "deviceinfo.h"
#include "net_value.h"
#include "util.h"

String getReadAPIString(String eol) {
	String r = "Read API: http://" + localIPstr() + uri_v + "?read=X" + eol;
	r += "Where X is one of:" + eol;
	r += "  api        :: ThingSpeak API key" + eol;
	r += "  rssi       :: AP signal strength in dBm at connect time" + eol;
	r += "  id         :: Device ID number" + eol;
	r += "  name       :: Name of device" + eol;
	r += "  thingspeak :: Thingspeak api codes" + eol;
	r += "  csv        :: Summary of sensor data" + eol;
	r += "  status     :: Device status" + eol;
	return r;
}

String WebPrintInfo(String eol) {
	// Describe Webserver access
	String r = "Configure Device: http://" + localIPstr() + "/config" + eol;
	r += "Read Measured Data: http://" + localIPstr() + eol;
	r += "Reboot: http://" + localIPstr() + "/reboot" + eol;
	r += getReadAPIString(nl);
	return r;
}

void sendValue(void) {

	String file("<!DOCTYPE HTML><html><head></head><body>");
	String value("");
// READ API
	bool arg = server.hasArg("read");
	bool found = false;
	if (arg) {
		String sarg = server.arg("read");
		if (sarg == "factory_default_ssid") {
			found = true;
			value = String(factory_default_ssid);
		}
		if (sarg == "api") {
			found = true;
			value = String(dinfo.getcThinkspeakApikey());
		}
		if (sarg == "rssi") {
			found = true;
			value = String(rssi);
		}
		if (sarg == "id") {
			found = true;
			value = String(dinfo.getDeviceID());
		}
		if (sarg == "name") {
			found = true;
			value = String(dinfo.getDeviceName());
		}
		if (sarg == "thingspeak") {
			found = true;
			value = String(dinfo.getThingspeakStatus());
		}
		if (sarg == "status") {
			found = true;
			value = "," + dinfo.toString(",<br>");
		}
		if (sarg == "csv") {
			found = true;
			value = ",pir=" + String(PIRcount) + ",pirlast=" + String(PIRcountLast) + ",";
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
			value += "ts=" + String(dinfo.getThingspeakStatus()) + ",";
		}
		if (!found) {
			value = "Unknown command.<br><br>Use one of the following:<br>" + getReadAPIString("<br>");
		}
	}

	if (server.hasArg("reset")) {
		reset();
	}

	file += value;
	file += String("</body></html>"); // comma for easier web page scraping
	server.send(200, "text/html", file);
}
