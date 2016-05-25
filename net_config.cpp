/*
 *
 *  Created on: Jan 30, 2016
 *      Author: Allan Inda
 */

#include <stdio.h>
#include <stdlib.h>
#include "network.h"
#include "util.h"
#include "main.h"
#include "deviceinfo.h"
#include "net_config.h"

extern int ConfigurationChange(void);

// Generic and reused statements
const char sHTTP_ENDLABEL_BR[] = "></label> <br>";
const char sHTTP_ENDLABELQ_BR[] = "\"></label><br>";
const char sHTTP_ENDLABELQ[] = "\"></label>";
//const char sHTTP_ENDBRACEQ[] = "\">";
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
const char sHTTP_TS_IPADDR[] =
		"<label>IP address: <input type=\"text\" name=\"ipaddr\" value=\"";
// print current ipaddress
// <ENDLABELQ>
//
// ## Port Configuration
const char sHTTP_PORT_HEADING[] = "<br>"; //"<p><b>Port Configuration</b></p>";
const char sHTTP_PORT_NUMBER[] = "<label>Port#";
const char sHTTP_PORT_NAME[] = "<input type=\"text\" name=\"port";
const char sHTTP_PORTADJ_NUMBER[] = "<label>Adj#";
const char sHTTP_PORTADJ_NAME[] = " <input type=\"text\" name=\"adjport";
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
		/*		"<input type=\"submit\" name=\"reboot\" value=\"reboot\">"*/
		"</form>";
const char sHTTP_AHREF_START[] = "<a href=\"http://";
const char sHTTP_AHREF_END[] = "</a>";

void config(void) {
	// If there was a submit, then process the changes
	int configChanges = ConfigurationChange();
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
	r = String(sHTTP_DEVICE_NAME) + String(dinfo.getDeviceName())
			+ String(sHTTP_ENDLABELQ_BR);
	// Thingspeak
	r += String(sHTTP_TS_ENABLE) + String(dinfo.getEnableStr())
			+ String(sHTTP_ENDLABEL_BR) + String(sHTTP_TS_APIKEY)
			+ String(dinfo.getcThinkspeakApikey()) + String(sHTTP_ENDLABELQ_BR)
			+ String(sHTTP_TS_IPADDR) + String(dinfo.getIpaddr())
			+ String(sHTTP_ENDLABELQ_BR);

	// Port Configuration Heading
	r += String(sHTTP_PORT_HEADING);
	// ....SEND
	server.sendContent(r);

	// Ports
	for (int i = 0; i < dinfo.getPortMax(); i++) {
		// Port Name and edit box
		if (i > 0) {
			r = String("<br>");
		}
		else {
			r = String("");
		}
		// Port Name
		r += String(sHTTP_PORT_NUMBER) + String(i) + String(sHTTP_PORT_NAME) + String(i)
				+ String(sHTTP_CLOSE_AND_VALUE) + String(dinfo.getPortName(i))
				+ String(sHTTP_ENDLABELQ_BR);
		// Port Adj Numeric Values
		for (int k = 0; k < dinfo.getPortAdjMax(); k++) {
			r += String(sHTTP_PORTADJ_NUMBER) + String(k) + String(sHTTP_PORTADJ_NAME)
					+ String(i) + String(k) + String(sHTTP_CLOSE_AND_VALUE)
					+ String(dinfo.getPortAdj(i, k), DECIMAL_PRECISION)
					+ String(sHTTP_ENDLABELQ);
		}
		r += String("<br>");
		// Port radio buttons
		//lint -e{26,785} suppress since lint doesn't understand C++11
		for (int j = 0; j < static_cast<int>(sensorModule::END); j++) {
			r += String(sHTTP_PORT_RADIO_START) + String(i)
					+ String(sHTTP_CLOSE_AND_VALUE);
			r += String(sensorList[static_cast<int>(j)].name);
			r += String("\" ");
			if (dinfo.getPortMode(i) == static_cast<sensorModule>(j)) {
				r += String("checked");
			}
			r += String(">") + String(sensorList[static_cast<int>(j)].name);
		}
		server.sendContent(r + String("<br>"));
	}

	// Buttons and links
	r = String(sHTTP_BUTTONS);
	r += String(sHTTP_AHREF_START) + String(localIPstr())
			+ String("\">Show current values<br>") + String(sHTTP_AHREF_END);
	r += String(sHTTP_AHREF_START) + String(localIPstr())
			+ String("/value?read=csv\">Show current values in csv format<br>")
			+ String(sHTTP_AHREF_END);
	r += String(sHTTP_AHREF_START) + String(localIPstr())
			+ String("/config\">Configure Device<br>") + String(sHTTP_AHREF_END);
	r += String(sHTTP_AHREF_START) + String(localIPstr())
			+ String("/value?read=status\">Show Device Status<br>")
			+ String(sHTTP_AHREF_END);
	r += String(sHTTP_AHREF_START) + String(localIPstr())
			+ String("/value?reset=0\">Reboot<br>") + String(sHTTP_AHREF_END);
	server.sendContent(r);

	// End of page
	unsigned long tdiff = millis() - t0;
	server.sendContent(
			String("<pre>time:") + String(tdiff) + String("ms\nConfig:")
					+ String(configChanges) + String("</pre>") + String(sHTTP_END));

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

	/* ITEM					NAME			VALUE
	 * ------------------   --------------  ---------------------------
	 * Device name: 		name			string
	 * Thingspeak enable: 	ts_enable		tsenable|tsdisable
	 * API Key: 			apikey			string
	 * IP Address:			ipaddr			string
	 * Port N Name:			portN			string
	 * Port N Adj M:		adjportNM		string (convert to float)
	 * Radio Port N:		radportN		[OFF|DHT11|DHT22|DS18b20|Sonar|Dust|Sound]N
	 */

	bool need_reboot = false;
	if (server.args() > 0) {
		bool found = false;
		if (debug_output) {
			Serial.println("");
			Serial.println("##########################################################");
		}
		dinfo.setEnable(false);
		for (uint8_t i = 0; i < server.args(); i++) {
			String sarg = server.argName(i);
			String varg = server.arg(i);
			if (debug_output) {
				Serial.print("NAME=");
				Serial.print(sarg);
				Serial.print("  VALUE=");
				Serial.print(varg);
			}
			if (sarg == String("name")) {
				dinfo.setDeviceName(varg);
				if (debug_output) Serial.println(" ok name");
				found = true;
			}
			if (sarg == String("ts_enable")) {
				dinfo.setEnable(true);
				if (debug_output) Serial.println(" ok ts_enable");
				found = true;
			}
			if (sarg == "apikey") {
				dinfo.setThingspeakApikey(varg);
				if (debug_output) Serial.println(" ok apikey");
				found = true;
			}
			if (sarg == "ipaddr") {
				dinfo.setIpaddr(varg);
				if (debug_output) Serial.println(" ok ipaddr");
				found = true;
			}

			if (strncmp(sarg.c_str(), "port", 4) == 0) {
				found = true;
				char c = sarg.c_str()[4];
				int n = static_cast<int>(c) - static_cast<int>('0');
				if (debug_output) {
					Serial.print(", n=");
					Serial.print(n);
				}
				if (n >= 0 && n < dinfo.getPortMax()) {
					if (varg.length() > 0) {
						dinfo.setPortName(n, varg);
						if (debug_output) {
							Serial.print(" ok, port[");
							Serial.print(n);
							Serial.print("].name set to ");
							Serial.println(varg.c_str());
						}
					}
					else {
						dinfo.setPortName(n, "");
						if (debug_output) Serial.println(" ok - cleared");
					}
				}
				else {
					if (debug_output) {
						Serial.print("\nERROR: Bug - Invalid port #(");
						Serial.print(n);
						Serial.print(",");
						Serial.print(c);
						Serial.print(") found in ConfigurationChange() - ");
						Serial.println(sarg.c_str());
					}
				}
			}

			if (strncmp(sarg.c_str(), "adjport", 7) == 0) {
				found = true;
				char c1 = sarg.c_str()[7];
				char c2 = sarg.c_str()[8];
				int n1 = static_cast<int>(c1) - static_cast<int>('0');
				int n2 = static_cast<int>(c2) - static_cast<int>('0');
				if (debug_output) {
					Serial.print(", n1=");
					Serial.print(n1);
					Serial.print(", n2=");
					Serial.print(n2);
				}
				if (n1 >= 0 && n1 < dinfo.getPortMax()) {
					double d = 0;
					if (varg.length() > 0) {
						if (debug_output) Serial.println(" ok, set");
						d = ::atof(varg.c_str());
					}
					else {
						if (debug_output) Serial.println(" ok, cleared");
					}
					dinfo.setPortAdj(n1, n2, d);
				}
				else {
					if (debug_output) {
						Serial.print("\nERROR: Bug - Invalid port #(");
						Serial.print(n1);
						Serial.print(",");
						Serial.print(c1);
						Serial.print(") found in ConfigurationChange() - ");
						Serial.println(sarg.c_str());
					}
				}
			}

			if (strncmp(sarg.c_str(), "radport", 7) == 0) {
				found = true;
				char c1 = sarg.c_str()[7];
				int n1 = static_cast<int>(c1) - static_cast<int>('0');
				if (debug_output) {
					Serial.print(", n1=");
					Serial.print(n1);
				}
				if (n1 >= 0 && n1 < dinfo.getPortMax()) {
					bool found1 = false;
					if (varg.length() > 0) {
						//lint -e{26,785} suppress since lint doesn't understand C++11
						for (int j = 0; j < static_cast<int>(sensorModule::END); j++) {
							if (strcmp(varg.c_str(), sensorList[j].name) == 0) {
								dinfo.setPortMode(n1, sensorList[j].id);
								need_reboot = true;
								if (debug_output) {
									Serial.print("\r\nInfo: Setting mode to ");
									Serial.print(static_cast<int>(sensorList[j].id));
									Serial.print("(");
									Serial.print(sensorList[j].name);
									Serial.println(")");
								}
								found1 = true;
								break;
							}
						}
						if (!found1) {
							Serial.print("\r\nERROR: unable to map mode: ");
							Serial.println(varg.c_str());
						}
					}
					else {
						if (debug_output) {
							Serial.print("\r\nERROR: But - Invalid varg mode: (");
							Serial.print(varg.c_str());
							Serial.print(") found in ConfigurationChange() - ");
							Serial.println(sarg.c_str());
						}
					}
				}
				else {
					if (debug_output) {
						Serial.print("\r\nERROR: Bug - Invalid port #(");
						Serial.print(n1);
						Serial.print(",");
						Serial.print(c1);
						Serial.print(") found in ConfigurationChange() - ");
						Serial.println(sarg.c_str());
					}
				}
			}

			if (sarg == "reboot") {
				if (debug_output) {
					Serial.println(", reboot");
				}
				found = true;
			}

			if (!found) {
				Serial.println("");
			}
			found = false;
		}

		dinfo.StoreConfigurationIntoEEPROM();
		if (need_reboot) {
			reset();
		}

	}
	return server.args();
}
