/*
 * net_config.cpp
 *
 *  Created on: Jan 30, 2016
 *      Author: Allan Inda
 */

#include "network.h"
#include "temperature.h"
#include "main.h"
#include "deviceinfo.h"
#include "net_config.h"

extern int ConfigurationChange(void);

t_sensor sensors[MAX_SENSOR] = { { "off", 0 }, { "DHT11", 1 }, { "DHT22", 2 }, { "DS18b20", 3 }, { "Sonar", 4 }, {
		"Dust", 5 }, { "Sound", 6 } };

// Generic and reused statements
const char sHTTP_ENDLABEL_BR[] = "></label> <br>";
const char sHTTP_ENDLABELQ_BR[] = "\"></label><br>";
const char sHTTP_ENDLABELQ[] = "\"></label>";
const char sHTTP_ENDBRACEQ[] = "\">";
const char sHTTP_CLOSE_AND_VALUE[] = "\"  value=\"";
const char sHTTP_END[] = "</body></html>";
// ## Header
const char sHTTP_TOP[] =
		"<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">"
				"<html>"
				"<head>"
				"<title>Environment Sensor Configuration</title>"
				"</head>"
				"<body>"
				"<h1>Environment Sensor Configuration</h1>"
				"<form action=\"config\" method=\"get\" name=\"Configuration\"> "; // extra character to make it word aligned
//
// ## Device Name
const char sHTTP_DEVICE_NAME[] = ""
		"<label>Device name: "
		"<input type=\"text\" name=\"name\" value=\"";
// <print user assigned name>
// <ENDLABELQ>
//
// ## Thingspeak
const char sHTTP_TS_ENABLE[] = ""
		"<br>" //<p><b>Thingspeak</b></p>"
		"<label>Thingspeak Enabled"
		"<input type=\"checkbox\" name=\"ts_enable\" value=\"tsenable\" ";
// <print "checked" or blank>
// <ENDLABEL>
const char sHTTP_TS_APIKEY[] = ""
		"<label>API Key: "
		"<input type=\"text\" name=\"apikey\" value=\"";
// print current apikey
// <ENDLABELQ>
const char sHTTP_TS_IPADDR[] = "<label>IP address: <input type=\"text\" name=\"ipaddr\" value=\"";
// print current ipaddress
// <ENDLABELQ>
//
// ## Port Configuration
const char sHTTP_PORT_HEADING[] = "<p><b>Port Configuration</b></p>";
const char sHTTP_PORT_NUMBER[] = "<label>Port#";
const char sHTTP_PORT_NAME[] = "<input type=\"text\" name=\"port";
const char sHTTP_PORTADJ_NUMBER[] = "<label>Adj#";
const char sHTTP_PORTADJ_NAME[] = " <input type=\"text\" name=\"portadj";
// -- Radio buttons for each port
const char sHTTP_PORT_RADIO_START[] = "<input type=\"radio\" name=\"radport";
// print port number, e.g. 1, 2, 3, 4, ...
// <sHTTP_CLOSE_AND_VALUE>
// print sensor name, then print port number --- this creates a handle for the radio button
// <ENDBRACEQ>
// Print the sensor name to present to the user, same value as above, but without the port number
//
// ## Submit Buttons
const char sHTTP_BUTTONS[] = ""
		"<br>"
		"<input type=\"submit\" name=\"submit\" value=\"submit\">"
		"<input type=\"submit\" name=\"reboot\" value=\"reboot\"></form>";

void config(void) {
	// If there was a submit, then process the changes
	int configChanges = 0;
//	configChanges = ConfigurationChange();
	unsigned long t0 = millis();

	// Show the current values and ask for updated input
	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");
	server.sendHeader("Expires", "-1");
	server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.

	// Start of page
	String r = String(sHTTP_TOP);
	server.sendContent(r);
	// Device Name
	r = String(String(sHTTP_DEVICE_NAME) + String(dinfo.getDeviceName()) + String(sHTTP_ENDLABELQ_BR));
	// Thingspeak
	r += String(
			String(sHTTP_TS_ENABLE) + String(dinfo.getEnableStr()) + String(sHTTP_ENDLABEL_BR) + String(sHTTP_TS_APIKEY)
					+ String(dinfo.getcThinkspeakApikey()) + String(sHTTP_ENDLABELQ_BR) + String(sHTTP_TS_IPADDR)
					+ String(dinfo.getIpaddr()) + String(sHTTP_ENDLABELQ_BR));

	// Port Configuration Heading
	r += String(sHTTP_PORT_HEADING);
	// ....SEND
	server.sendContent(r);

	// Ports
	for (int i = 0; i < dinfo.getPortMax(); i++) {
		// Port Name and edit box
		r = String(
				String(sHTTP_PORT_NUMBER) + String(i) + String(sHTTP_PORT_NAME) + String(i)
						+ String(sHTTP_CLOSE_AND_VALUE) + String(dinfo.getPortName(i)) + String(sHTTP_ENDLABELQ_BR));
		for (int k = 0; k < dinfo.getPortAdjMax(); k++) {
			r += String(
					String(sHTTP_PORTADJ_NUMBER) + String(k) + String(sHTTP_PORTADJ_NAME) + String(i) + String(k)
							+ String(sHTTP_CLOSE_AND_VALUE) + String(dinfo.getPortAdj(i, k)) + String(sHTTP_ENDLABELQ));
		}
		r += String("<br>");
		// Port radio buttons
		for (int j = 0; j < MAX_SENSOR; j++) {
			r += String(
					String(sHTTP_PORT_RADIO_START) + String(i) + String(sHTTP_CLOSE_AND_VALUE) + String(sensors[j].name)
							+ String(i) + String(sHTTP_ENDBRACEQ) + String(sensors[j].name));
		}
		server.sendContent(r + String("<br>"));
	}

	// End of page
	unsigned long tdiff = millis() - t0;

	server.sendContent(
			String(sHTTP_BUTTONS) + String("<pre>time:") + String(tdiff) + String("ms\nConfig:") + String(configChanges)
					+ String("</pre>") + String(sHTTP_END));

	// Stop client because ...
//	server.client().stop();
}

int ConfigurationChange(void) {
	/* Sample input
	 http://192.168.0.74/change?name=house&apikey=ABC&ipaddr=184.106.153.149&port0=&radport0=off0&port1=&radport1=DHT111&port2=&radport2=DHT222&port3=&radport3=DHT113&submit=submit
	 http://192.168.0.74/config?name=house&ts_enable=tsenable&apikey=api&ipaddr=1.2.3..4&port0=&port1=&port2=&port3=&submit=submit
	 http://192.168.0.74/config?name=house&apikey=api&ipaddr=1.2.3..4&port0=&port1=&port2=&port3=&submit=submit

	 http://192.168.0.74/config?name=_device_name_&ts_enable=tsenable&apikey=_api_key_&ipaddr=_ipaddr_&port0=&port1=&port2=&port3=&reboot=reboot
	 http://192.168.0.74/config?name=_Device_&apikey=_api_&ipaddr=_ipaddr_&port0=_port0_&radport0=off0&port1=&port2=&port3=&port4=&port5=&submit=submit

	 http://192.168.0.74/config?
	 name=_device_name_&
	 ts_enable=tsenable&
	 apikey=_api_key_&
	 ipaddr=_ipaddr_&
	 port0=_port0_&
	 radport0=off0&
	 port1=_port1_&
	 radport1=DHT111
	 &port2=_port2_&
	 radport2=DHT222&
	 port3=_port3_&
	 radport3=DS18b203&
	 submit=submit
	 http: //192.168.0.74/config?name=_name_&ts_enable=tsenable&apikey=_api_&ipaddr=_addr_&port0=&port1=&port2=_2_&port3=&port4=_4_&port5=_5_&submit=submit
	 */

	if (server.args() > 0) {
		dinfo.setEnable(false);
		for (uint8_t i = 0; i < server.args(); i++) {
			String sarg = server.argName(i);
			String varg = server.arg(i);
			Serial.print("NAME=");
			Serial.print(sarg);
			Serial.print("  VALUE=");
			Serial.println(varg);
			if (sarg == String("name")) {
				dinfo.setDeviceName(varg);
			}
			if (sarg == String("ts_enable")) {
				dinfo.setEnable(true);
			}
			if (sarg == "apikey") {
				dinfo.setThingspeakApikey(varg);
			}
			if (sarg == "ipaddr") {
				dinfo.setIpaddr(varg);
			}
			if (strncmp(sarg.c_str(), "port", 4) == 0) {
				char c = sarg.c_str()[4];
				int n = static_cast<int>(c - '0');
				if (n >= 0 && n < dinfo.getPortMax()) {
					if (varg.length() > 0) {
						dinfo.setPortName(n, varg);
					}
					else {
						dinfo.setPortName(n, "");
					}
				}
				else {
					Serial.println("Error: Bug - Invalid port# found in ConfigurationChange()");
				}
				//dinfo.setPortName(i, varg);
			}
			if (sarg == "radport0") {
				//Ports[i].setMode(varg);
			}
			if (sarg == "reboot") {
				// reboot device
			}
		}
		dinfo.StoreConfigurationIntoEEPROM();
	}
	return server.args();
}
