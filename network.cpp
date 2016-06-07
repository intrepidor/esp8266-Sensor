/*
 * network.cpp
 *
 *  Created on: Jan 21, 2016
 *      Author: Allan Inda
 */

#include "network.h"
#include "temperature.h"
#include "main.h"
#include "reset.h"
#include "deviceinfo.h"
#include "net_config.h"
#include "net_value.h"
#include "thingspeak.h"

const char* const factory_default_ssid = "ssid";
ESP8266WebServer server(80);
int32_t rssi = 0;
String uri_v("/value");

void _WriteDefaultsToDatabase(void) {
	dinfo.writeDefaultsToDatabase();
}

void _EraseEEPROM(void) {
	dinfo.eraseEEPROM();
}

void WebInit(void) {
// There are problems with linting ESP8266WebServer, specifically the 'on' member function.
//lint --e{64}   Ignore error about type mismatch (pertains to server.on)
//lint --e{1025} Ignore error about no function matches invocation (pertains to server.on)
//lint --e{1703} Ignore error about arbitrarily selecting ::on()

// Local initialization. Once its business is done, there is no need to keep it around
	WiFiManager wifiManager; // FIXME modify library code so it flashes the RED LED in a pattern when in SoftAP mode
	// reset saved settings
	//wifiManager.resetSettings();

	// Configure a custom ip for portal, otherwise uses the default of 192.168.4.1
	//wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

	//fetches ssid and pass from eeprom and tries to connect
	//if it does not connect it starts an access point with the specified name
	//here  "AutoConnectAP"
	//and goes into a blocking loop awaiting configuration
	String APname("ESPsAP" + String(dinfo.getDeviceID()));
	wifiManager.autoConnect(APname.c_str());
	//or use this for auto generated name ESP + ChipID
	//wifiManager.autoConnect();

// Start WiFi
//  WiFi.begin(factory_default_ssid, factory_default_pass);
//  debug.print(DebugLevel::ALWAYS, "Searching for ");
//  debug.print(DebugLevel::ALWAYS, factory_default_ssid);
//
//  while (WiFi.status() != WL_CONNECTED) {
//    digitalWrite(BUILTIN_LED, BUILTIN_LED_OFF);
//    delay(250);
//    digitalWrite(BUILTIN_LED, BUILTIN_LED_ON);
//    delay(250);
//    debug.print(DebugLevel::ALWAYS, ".");
//  }

	debug.println(DebugLevel::ALWAYS, "");
	debug.println(DebugLevel::ALWAYS, "Connected to " + WiFi.SSID());
	debug.print(DebugLevel::ALWAYS, "IP address: ");
	debug.println(DebugLevel::ALWAYS, WiFi.localIP());

	uint8_t available_networks = static_cast<uint8_t>(WiFi.scanNetworks());
	for (uint8_t network = 0; network < available_networks; network++) {
		if (strcmp(WiFi.SSID(network).c_str(), WiFi.SSID().c_str()) == 0) {
			rssi = WiFi.RSSI(network);
			debug.println(DebugLevel::ALWAYS, "RSSI: " + String(rssi) + " dBm");
		}
	}

	if (MDNS.begin("esp8266")) {
		debug.println(DebugLevel::ALWAYS, "mDNS started");
	}

	// Configure webserver
	server.onNotFound([] () {
		String message = "File Not Found\n\n";
		message += "URI: ";
		message += server.uri();
		message += "\nMethod: ";
		message += (server.method() == HTTP_GET) ? "GET" : "POST";
		message += "\nArguments: ";
		message += server.args();
		message += "\n";
		for (uint8_t i = 0; i < server.args(); i++) {
			message += " " + server.argName(i);
			message += ": " + server.arg(i) + "\n";
		}
		message += "\n" + WebPrintInfo("\n");
		server.send(404, "text/plain", message);
	});

	server.on("/", []() {
		String response("");
		sendHTML_Header(true);
		response += "<h2>Show Data</h2>";
		response += "Count=" + String(count);
		response += "<br>PIR=" + String(PIRcount);
		response += "<br>PIRLast=" + String(PIRcountLast);
		response += "ThingspeakUpdates="+String(thingspeak_update_counter);
		response += "<br>Thingspeakerrors="+String(thingspeak_error_counter);
		response += "<br>Thinkspeakentries="+String(thinkspeak_total_entries);
		for (int i=0; i<SENSOR_COUNT; i++) {
			if (sensors[i]) {
				for (int j=0; j<getSensorValueCount(); j++) {
					if (sensors[i]->getValueEnable(j)) {
						response+="val" + String(i) + String(j) + ":" +
						String(sensors[i]->getValueName(j)) + "=" +
						String(sensors[i]->getValue(j)) + "<br>";
						response+="raw" + String(i) + String(j) + ":" +
						String(sensors[i]->getValueName(j)) + "=" +
						String(sensors[i]->getRawValue(j)) + "<br>";

					}
				}
			}
		}
		response += "<br>" + getWebFooter(false) + sHTTP_END;
		server.sendContent(response);
	});

	server.on("/status", []() {
		sendHTML_Header(true);
		String response("<h2>Show Status</h2><div style=\"width:99%\">");
		response += dinfo.databaseToString("<br>") +"</div>" + getWebFooter(false);
		server.sendContent(response);
	});

	server.on("/csv", []() {
		String response("");
		response = ",pir=" + String(PIRcount) + ",pirlast=" + String(PIRcountLast) + ",";
		for (int i = 0; i < SENSOR_COUNT; i++) {
			if (sensors[i]) {

				for (int j = 0; j < getSensorValueCount(); j++) {
					if (sensors[i]->getValueEnable(j)) {
						response += String("val") + String(i) + String(j) + "="
						+ String(sensors[i]->getRawValue(j)) + ",";
						response += String("val") + String(i) + String(j) + "="
						+ String(sensors[i]->getValue(j)) + ",";
					}
				}

				for (int j = 0; j < getSensorCalCount(); j++) {
					if (sensors[i]->getCalEnable(j)) {
						response += String("cal") + String(i) + String(j) + "="
						+ String(sensors[i]->getCal(j)) + ",";
					}
				}
			}
		}
		response += "ts=" + String(dinfo.getThingspeakStatus()) + ",";
		server.send(200, "text/plain", response);
	});

	server.on(uri_v.c_str(), sendValue);
	server.on("/config", config);
	server.on("/default_configuration", _WriteDefaultsToDatabase);
	server.on("/erase_eeprom", _EraseEEPROM);
	server.on("/reboot", reset);

	server.begin();
	debug.println(DebugLevel::ALWAYS, "Web server started");
}

void WebWorker(void) {
	server.handleClient();
}

