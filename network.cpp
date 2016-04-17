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

const char* const factory_default_ssid = "ssid";
ESP8266WebServer server(80);
int32_t rssi = 0;
String v("/value");

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
//  Serial.print("Searching for ");
//  Serial.print(factory_default_ssid);
//
//  while (WiFi.status() != WL_CONNECTED) {
//    digitalWrite(BUILTIN_LED, BUILTIN_LED_OFF);
//    delay(250);
//    digitalWrite(BUILTIN_LED, BUILTIN_LED_ON);
//    delay(250);
//    Serial.print(".");
//  }

	Serial.println("");
	Serial.print("Connected to ");
	Serial.println(factory_default_ssid);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	uint8_t available_networks = static_cast<uint8_t>(WiFi.scanNetworks());
	for (uint8_t network = 0; network < available_networks; network++) {
		if (strcmp(WiFi.SSID(network).c_str(), factory_default_ssid) == 0) {
			rssi = WiFi.RSSI(network);
			Serial.print("RSSI: ");
			Serial.print(String(rssi).c_str());
			Serial.println(" dBm");
		}
	}

	if (MDNS.begin("esp8266")) {
		Serial.println("mDNS started");
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
		server.send(404, "text/plain", message);
	});

	server.on("/", []() {
		String response(""); // FIXME status1.c_str());
			response += "";// FIXME status2 + status3;
			response += "\nCount=" + String(count);
			response += "\nHumidity#1=" + String(t1.getHumidity());
			response += "\nTemperature#1=" + String(t1.getTemperature());
			response += "\nTempOffset#1=" + String(t1.getCalOffset());
			response += "\nHeatIndex#1=" + String(t1.getHeatindex());
			response += "\nHumidity#2=" + String(t2.getHumidity());
			response += "\nTemperature#2=" + String(t2.getTemperature());
			response += "\nTempOffset#2=" + String(t2.getCalOffset());
			response += "\nHeatIndex#2=" + String(t2.getHeatindex());
			response += "\nMotion#1=" + String(PIRcount);
			response += "\n<a href=\"" + String(WiFi.localIP()) + String("/config\">config</a>");
			server.send(200, "text/plain", response);
		});

	server.on("/config", config);
	server.on(v.c_str(), sendValue);

	server.begin();
	Serial.println("Web server started");
}

void WebWorker(void) {
	server.handleClient();
}

void WebPrintInfo(void) {
	// Describe Webserver access
	Serial.print("Read Everything: http://");
	Serial.println(WiFi.localIP());

	Serial.print("Read API:   http://");
	Serial.print(WiFi.localIP());
	Serial.println(v);
	Serial.println("?read=1,2,3      :: read temperature, humidity, heatindex for Sensor #1");
	Serial.println("?read=4,5,6      :: read same for Sensor #2");
	Serial.println("?read=7          :: read Motion detector value");
	Serial.println("?read=api        :: read ThingSpeak API key");
	Serial.println("?read=rssi       :: read signal strength of AP at the time of connection in dBm");
	Serial.println("?read=deviceid   :: read deviceid number");
	Serial.println("?read=name       :: read name of device");
	Serial.println("?read=status     :: read status information for device");
	Serial.println("?read=thingspeak :: read thingspeak api codes");
	Serial.println("?read=sherbet    :: read summary of sensor data");
	Serial.println("?read=offset1    :: read offset1 value");
	Serial.println("?read=offset2    :: read offset1 value");
	Serial.println("?offset1=X       :: write offset1 to X");
	Serial.println("?offset2=X       :: write offset2 to X");
	Serial.println("?reset=0         :: reset device");
	Serial.println("");
}
