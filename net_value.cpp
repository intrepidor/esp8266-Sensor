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
	r += "  ssid       :: SSID of the connected AP" + eol;
	r += "  id         :: Device ID number" + eol;
	r += "  name       :: Device Name" + eol;
	r += "  thingspeak :: Last Thingspeak http return value" + eol;
	return r;
}

String WebPrintInfo(String eol) {
	// Describe Webserver access
	String r = "Configure Device: http://" + localIPstr() + "/config" + eol;
	r += "Show Measured Data: http://" + localIPstr() + eol;
	r += "Show Device Status: http://" + localIPstr() + "/status" + eol;
	r += "Show current values in csv format: http://" + localIPstr() + "/csv" + eol;
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
		if (sarg == "ssid") {
			found = true;
			value = WiFi.SSID();
		}
		if (sarg == "api") {
			found = true;
			value = String(dinfo.getcThingspeakApikey());
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
		if (!found) {
			value = "Unknown command.<br><br>Use one of the following:<br>" + getReadAPIString("<br>");
		}
	}

	if (server.hasArg("reset")) {
		ESP.reset();
	}

	file += value;
	file += String("</body></html>"); // comma for easier web page scraping
	server.send(200, "text/html", file);
}
