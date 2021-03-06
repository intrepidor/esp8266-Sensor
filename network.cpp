/*
 * network.cpp
 *
 *  Created on: Jan 21, 2016
 *      Author: Allan Inda
 */

#include "network.h"
#include "temperature.h"
#include "main.h"
#include "deviceinfo.h"
#include "net_config.h"
#include "net_value.h"
#include "thingspeak.h"
#include "debugprint.h"
#include "wdog.h"

const char* const factory_default_ssid = "ssid";
ESP8266WebServer server(80);
int32_t rssi = 0;
String uri_v("/value");

void _EraseEEPROM(void) {
	dinfo.eraseEEPROM();
}

void ESPreset(void) {
	ESP.reset();
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
//	while (1) {
//		ESP.wdtFeed();
//		yield();
//	}

	kickAllWatchdogs();
	//or use this for auto generated name ESP + ChipID
	//wifiManager.autoConnect();

// Start WiFi
//  WiFi.begin(factory_default_ssid, factory_default_pass);
//  Serial.print( "Searching for ");
//  Serial.print( factory_default_ssid);
//
//  while (WiFi.status() != WL_CONNECTED) {
//    digitalWrite(BUILTIN_LED, BUILTIN_LED_OFF);
//    delay(250);
//    digitalWrite(BUILTIN_LED, BUILTIN_LED_ON);
//    delay(250);
//    Serial.print( ".");
//  }

	Serial.println("");
	Serial.println("Connected to " + WiFi.SSID());
	Serial.print(F("IP address: "));
	Serial.println(WiFi.localIP());

	uint8_t available_networks = static_cast<uint8_t>(WiFi.scanNetworks());
	for (uint8_t network = 0; network < available_networks; network++) {
		if (strcmp(WiFi.SSID(network).c_str(), WiFi.SSID().c_str()) == 0) {
			rssi = WiFi.RSSI(network);
			Serial.println("RSSI: " + String(rssi) + " dBm");
		}
	}

	if (MDNS.begin("esp8266")) {
		Serial.println(F("mDNS started"));
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
		response += "<h2>Main Menu</h2>";
		response += "<br>" + getWebFooter(menu::Main) + "</body></html>";
		server.sendContent(response);
	});

	server.on("/showdata", []() {
		String response("");
		sendHTML_Header(true);
		response += "<h2>Show Data</h2>";
		response += "Count=" + String(count);
		response += "<br>PIR=" + String(PIRcount);
		response += "<br>PIRLast=" + String(PIRcountLast);
		response += "<br>RSSI="+String(WiFi.RSSI());
		response += "<br>ThingspeakUpdates="+String(thingspeak_update_counter);
		response += "<br>Thingspeakerrors="+String(thingspeak_error_counter);
		response += "<br>Thinkspeakentries="+String(thinkspeak_total_entries);
		response += "<br>";
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
		response += "<br>" + getWebFooter(menu::Utility) + "</body></html>";
		server.sendContent(response);
	});

	server.on("/status",
			[]() {
				sendHTML_Header(true);
				String response("<h2>Show Status</h2><div style=\"width:99%\">");
				response += getsDeviceInfo("<br>");
				response += getsThingspeakInfo("<br>");
				response += getsThingspeakChannelInfo("<br>");
				response += "== Database ==<br>"+dinfo.databaseToString("<br>") +"</div>" + getWebFooter(menu::Utility);
				server.sendContent(response);
			});

	server.on("/sensordebug", []() {
		sendHTML_Header(true);
		String response("<h2>Sensor Debug</h2><div style=\"width:99%\">");
		response += getsSensorInfo("<br>");
		response += "</div>" + getWebFooter(menu::Utility);
		server.sendContent(response);
	});

	server.on("/csv", []() {
		String response("count=");
		response = String(count) + ",pir=" + String(PIRcount);
		response += ",pirlast=" + String(PIRcountLast);
		response += ",rssi=" + String(WiFi.RSSI()) + ",";
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

	server.on("/utility", []() {
		sendHTML_Header(true);
		String response("<h1>Utility Menu</h1>");
		response += getWebFooter(menu::Utility);
		server.sendContent(response);
	});

	server.on("/admin", []() {
		sendHTML_Header(true);
		String response("<h1>Administration Menu</h1>");
		response += getWebFooter(menu::Admin);
		server.sendContent(response);
	});

	server.on("/factory_settings", []() {
		sendHTML_Header(true);
		String response("Configuration set to Factory defaults.");
		dinfo.writeDefaultsToDatabase();
		response += getWebFooter(menu::Admin);
		server.sendContent(response);
	});

	server.on("/erase_eeprom", []() {
		sendHTML_Header(true);
		String response("EEPROM Erased. Factory settings loaded.");
		_EraseEEPROM();
		response += getWebFooter(menu::Admin);
		server.sendContent(response);
	});

	server.on("/reboot", []() {
		sendHTML_Header(true);
		String response("Rebooting ... This may take a couple minutes.");
		response += getWebFooter(menu::Admin);
		server.sendContent(response);
		delay(3000); /* 3 second delay */
		ESPreset();
	});

	server.on("/pushtschannelsettings", []() {
		sendHTML_Header(true);
		String response("<h2>Push Channel Settings to Thingspeak</h2><div style=\"width:99%\">");
		response += HTMLifyNewlines(ThingspeakPushChannelSettings("<br>", true));
		response += "</div>" + getWebFooter(menu::Thingspeak);
		server.sendContent(response);
	});

	server.on(uri_v.c_str(), sendValue);
	server.on("/config", config);
	server.on("/tsconfig", tsconfig);

	kickAllWatchdogs();
	server.begin();
	Serial.println(F("Web server started"));
}

void WebWorker(void) {
	kickExternalWatchdog();
	server.handleClient();
}

