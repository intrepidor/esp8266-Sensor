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
//const char sHTTP_ENDLABEL_BR[] = "></label> <br>";
const char sHTTP_ENDLABELQ_BR[] = "\"></label><br>";
const char sHTTP_ENDLABELQ[] = "\"></label>";
//const char sHTTP_ENDBRACEQ[] = "\">";
const char sHTTP_CLOSE_AND_VALUE[] = "\"  value=\"";
const char sHTTP_END[] = "</body></html>";
// ## Header
const char sHTTP_BR[] = "<br>";
const char sHTTP_DIVSTART[] = "<div class=\"base ";
const char sHTTP_DIVSTART_CLOSE[] = "\">";
const char sHTTP_DIVBASE[] = "<div class=\"sensorblock\">";
const char sHTTP_DIVEND[] = "</div>";
const char sHTTP_TOP[] =
		"<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">"
				"<html><head><title>Environment Sensor</title>";
const char sHTTP_CSS[] = "<STYLE type=\"text/css\">"
		"label{"
		"  font-weight:bold;"
		"  height:18px;"
		"  color:midnightblue;"
		"}"
		".base{"
		"  -webkit-font-smoothing:antialiased;"
		"  font-family:\"Arial\";"
		"  font-size:15px;"
		"  border-style:solid;"
		"  border-width:0px;"
		"  border-radius:10px;"
		"  padding:10px;"
		"  margin-bottom:5px;"
		"}"
		".sensorblock{"
		"  -webkit-font-smoothing:antialiased;"
		"  font-family:\"Arial\";"
		"  font-size:15px;"
		"  border-style:solid;"
		"  border-width:1px;"
		"  border-radius:10px;"
		"  padding:5px;"
		"  margin-bottom:5px;"
		"  background-color:gray"
		"  color:black;"
		"  float:left;"
		"}"
		".device{"
		"  background-color:#edd9c0;"
		"}"
		".thingspeak{"
		"  background-color:#c9d8c5;"
		"}"
		".port{"
		"  background-color:#a8b6bf;"
		"}"
		".links{"
		"  background-color:#c9c9c9;"
		"  float:left;"
		"}"
		".basebutton{"
		"  font-family:\"Arial\";"
		"  font-size:16px;"
		"  font-weight:bold;"
		"  text-align:center;"
		"  text-decoration:none;"
		"  padding-left:10px;"
		"  padding-right:10px;"
		"  margin-left:4px;"
		"  text-align:center;"
		"  border:none;"
		"  height:50px;"
		"  width:80px;"
		"  display:flex;"
		"  flex-direction:column;"
		"  justify-content:center;"
		"  color:white;"
		"}"
		".savediv{"
		"  float:right;"
		"}"
		".savebutton{"
		"  background-color:#4CAF50;"
		"}"
		".hrefbutton{"
		"  background-color:DarkCyan;"
		"  float:left;"
		"}"
		".defaultbutton{"
		"background-color:darksalmon;"
		"  float:left;"
		"  color:black;"
		"}"
		".rebootbutton {"
		"background-color:chocolate;"
		"  float:left;"
		"  color:black;"
		"}"
		".hrefbutton:hover{"
		"  background:cyan;"
		"  color:blue;"
		"  text-decoration:none;"
		"}"
		".savebutton:hover{"
		"  background:green;"
		"  color:black;"
		"  text-decoration:none;"
		"}"
		".defaultbutton:hover{"
		"  background:red;"
		"  color:white;"
		"  text-decoration:none;"
		"}"
		".rebootbutton:hover{"
		"  background:crimson;"
		"  color:white;"
		"  text-decoration:none;"
		"}"
		".showdatacontainer{"
		"  display:flex;"
		"  flex-direction:column;"
		"  flex-wrap:nowrap;"
		"  flex-flow:center;"
		"}"
		".field{"
		"  height:15px;"
		"  background-color:lightgray;"
		"  color:darkgreen;"
		"  margin-top:-2px;"
		"  border-radius:2px;"
		"}"
		".fieldshort{"
		"  width:80px;"
		"}"
		".fieldmedium{"
		"  width:170px;"
		"}"
		".fieldlong {"
		"  width:60%;"
		"}"
		".newcheckbox{"
		"  display:inline-block;"
		"  width:15px;"
		"  height:15px;"
		"  margin:-1px 4px 0 4px;"
		"  vertical-align:middle;"
		"}"
		".newradio{"
		"  display:inline-block;"
		"  width:15px;"
		"  height:15px;"
		"  margin:-1px 0px 0 9px;"
		"  vertical-align:middle;"
		"}"
		"</STYLE>";
const char sHTTP_START_BODY[] = "</head><body>";
//
// ## Device Name
const char sHTTP_DEVICE_NAME[] = ""
		"<label>Device name: "
		"<input type=\"text\" class=\"field fieldmedium\" name=\"name\" value=\"";
// <print user assigned name>
// <ENDLABELQ>
//
// ## Device ID
const char sHTTP_DEVICE_ID[] = ""
		"<label>Device ID: "
		"<input type=\"text\" class=\"field fieldmedium\" name=\"deviceid\" value=\"";
// <print user assigned name>
// <ENDLABELQ>
//
// ## Thingspeak
const char sHTTP_TS_ENABLE[] = ""
		"<input type=\"checkbox\" name=\"ts_enable\" class=\"newcheckbox\" value=\"tsenable\" ";
// <print "checked" or blank>
// <ENDLABEL>
const char sHTTP_TS_URL[] = ""
		"<label>URL (optional): "
		"<input type=\"text\" class=\"field fieldlong\" name=\"tsurl\" value=\"";
// print current apikey
// <ENDLABELQ>
const char sHTTP_TS_CHANNEL[] = ""
		"<label>Channel (optional): "
		"<input type=\"text\" class=\"field fieldshort\" name=\"tschannel\" value=\"";
// print current apikey
// <ENDLABELQ>
const char sHTTP_TS_APIKEY[] = ""
		"<label>API Key: "
		"<input type=\"text\" class=\"field fieldmedium\" name=\"apikey\" value=\"";
// print current apikey
// <ENDLABELQ>
const char sHTTP_TS_IPADDR[] =
		"<label>IP address: <input type=\"text\" class=\"field fieldmedium\" name=\"ipaddr\" value=\"";
// print current ipaddress
// <ENDLABELQ>
//
// ## Port Configuration
const char sHTTP_PORT_NUMBER[] = "<label>Port#";
const char sHTTP_PORT_NAME[] = "<input type=\"text\" class=\"field fieldmedium\" name=\"port";
const char sHTTP_PORTADJ_NUMBER[] = "<label>";
const char sHTTP_PORTADJ_INPUT[] = " <input type=\"text\" class=\"field fieldshort\" name=\"adjport";
// -- Radio buttons for each port
const char sHTTP_PORT_RADIO_START[] = "<input type=\"radio\" class=\"newradio\" name=\"radport";
// print port number, e.g. 1, 2, 3, 4, ...
// <sHTTP_CLOSE_AND_VALUE>
// print sensor name, then print port number --- this creates a handle for the radio button
// <ENDBRACEQ>
// Print the sensor name to present to the user, same value as above, but without the port number
//
// ## Submit Buttons
// Misc
const char sHTTP_AHREF_END[] = "</a>";

//-----------------------------------------------------------------------------------
const char sHTTP_BUTTONS[] = "<div class=\"savediv\">"
		"<input type=\"submit\" class=\"basebutton savebutton\" name=\"submit\" value=\"SAVE\"></div>";
String getWebFooter(bool all) {
	String wf("<div>");
	String lin("\" href=\"http://" + localIPstr());
	const String a("<a class=\"basebutton");
	const String hre(" hrefbutton");
	const String reb(" rebootbutton");
	const String def(" defaultbutton");
	wf += a + hre + lin + "\">Show Data" + sHTTP_AHREF_END;
	wf += a + hre + lin + "/csv\">Show CSV" + sHTTP_AHREF_END;
	wf += a + hre + lin + "/config\">Configure" + sHTTP_AHREF_END;
	wf += a + hre + lin + "/status\">Status" + sHTTP_AHREF_END;
	wf += a + hre + lin + "/sensordebug\">Sensor Debug" + sHTTP_AHREF_END;
	if (all) {
		wf += a + def + lin + "/default_configuration\">Factory Defaults" + sHTTP_AHREF_END;
		wf += a + reb + lin + "/reboot\">Reboot" + sHTTP_AHREF_END;
		wf += sHTTP_BUTTONS;
	}
	wf += sHTTP_DIVEND;
	return wf;
}

//-----------------------------------------------------------------------------------
void sendHTML_Header(bool sendCSS) {
//	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
//	server.sendHeader("Pragma", "no-cache");
//	server.sendHeader("Expires", "-1");
//	server.send(200, "text/html", ""); 	// Empty content inhibits Content-length header so we have to close the socket ourselves.

	String r = String(sHTTP_TOP);
	if (sendCSS) {
		r += sHTTP_CSS;
	}
	r += sHTTP_START_BODY;
	server.sendContent(r);
}

//-----------------------------------------------------------------------------------
void config(void) {
	// If there was a submit, then process the changes
	ConfigurationChange();

	// Start of page
	String r("");
//	r = +sHTTP_TOP;
//	r += sHTTP_CSS;
//	r += sHTTP_START_BODY;
	sendHTML_Header(true);
	r += "<form action=\"config\" method=\"get\" name=\"Configuration\">";
	r += "<h2>Device Configuration :: " + ProgramInfo + "</h2>";
	server.sendContent(r);

	// Device Name
	r = sHTTP_DIVSTART + String("device") + sHTTP_DIVSTART_CLOSE;
	r += sHTTP_DEVICE_NAME + String(dinfo.getDeviceName()) + sHTTP_ENDLABELQ_BR;
	r += sHTTP_DEVICE_ID + String(dinfo.getDeviceID()) + sHTTP_ENDLABELQ;
	r += sHTTP_DIVEND;

	// Thingspeak
	r += sHTTP_DIVSTART + String("thingspeak") + sHTTP_DIVSTART_CLOSE;
	r += sHTTP_TS_ENABLE + String(dinfo.getEnableStr()) + "> Thingspeak Enable<br>";
	r += sHTTP_TS_APIKEY + dinfo.getThingspeakApikey() + sHTTP_ENDLABELQ_BR;
	r += sHTTP_TS_URL + dinfo.getThingspeakURL() + sHTTP_ENDLABELQ_BR;
	r += sHTTP_TS_CHANNEL + dinfo.getThingspeakChannel() + sHTTP_ENDLABELQ_BR;
	r += sHTTP_TS_IPADDR + dinfo.getThingspeakIpaddr() + sHTTP_ENDLABELQ + " Default=184.106.153.149";
	r += sHTTP_DIVEND;
	server.sendContent(r);

	// Ports
	for (int i = 0; i < dinfo.getPortMax(); i++) {
		// Port Name
		r = sHTTP_DIVSTART + String("port") + sHTTP_DIVSTART_CLOSE;
		r += sHTTP_PORT_NUMBER + String(i) + sHTTP_PORT_NAME + String(i) + sHTTP_CLOSE_AND_VALUE
				+ dinfo.getPortName(i) + sHTTP_ENDLABELQ;

		// Current Values
		for (int vindx = 0; vindx < getSensorValueCount(); vindx++) {
			r += " raw/val" + String(i) + String(vindx) + "= " + sensors[i]->getRawValue(vindx) + "/"
					+ sensors[i]->getValue(vindx) + String(" : ");
		}
		r += "<br>";
		// Port Adj Numeric Values
		for (int k = 0; k < dinfo.getPortAdjMax(); k++) {
			r += sHTTP_PORTADJ_NUMBER + /*String(k)+*/dinfo.getPortAdjName(i, k) + sHTTP_PORTADJ_INPUT
					+ String(i) + String(k) + sHTTP_CLOSE_AND_VALUE
					+ String(dinfo.getPortAdj(i, k), DECIMAL_PRECISION) + sHTTP_ENDLABELQ;
		}
		r += "<br>Port Mode: ";
		// Port radio buttons
		//lint -e{26,785} suppress since lint doesn't understand C++11
		for (int j = 0; j < static_cast<int>(sensorModule::END); j++) {
			r += sHTTP_PORT_RADIO_START + String(i) + sHTTP_CLOSE_AND_VALUE;
			r += String(sensorList[static_cast<int>(j)].name);
			r += "\" ";
			if (dinfo.getPortMode(i) == static_cast<sensorModule>(j)) {
				r += "checked";
			}
			r += ">" + String(sensorList[static_cast<int>(j)].name);
		}
		r += sHTTP_DIVEND;
		server.sendContent(r);
	}

	// Buttons and links then END of Page
	r = getWebFooter(true) + "</form>" + sHTTP_END;
	server.sendContent(r);
}

//-----------------------------------------------------------------------------------
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
		debug.println(DebugLevel::DEBUGMORE, "");
		debug.println(DebugLevel::DEBUGMORE, "##########################################################");
		dinfo.setThingspeakEnable(false);
		for (uint8_t i = 0; i < server.args(); i++) {
			String sarg = server.argName(i);
			String varg = server.arg(i);
			debug.print(DebugLevel::DEBUGMORE, "NAME=" + sarg + ", VALUE=" + varg);
			if (sarg == String("name")) {
				dinfo.setDeviceName(varg);
				debug.println(DebugLevel::DEBUGMORE, " ok name");
				found = true;
			}
			if (sarg == String("deviceid")) {
				dinfo.setDeviceID(atoi(varg.c_str()));
				debug.println(DebugLevel::DEBUGMORE, " ok deviceid");
				found = true;
			}
			if (sarg == String("ts_enable")) {
				dinfo.setThingspeakEnable(true);
				debug.println(DebugLevel::DEBUGMORE, " ok ts_enable");
				found = true;
			}
			if (sarg == "apikey") {
				dinfo.setThingspeakApikey(varg);
				debug.println(DebugLevel::DEBUGMORE, " ok apikey");
				found = true;
			}
			if (sarg == "ipaddr") {
				dinfo.setThingspeakIpaddr(varg);
				debug.println(DebugLevel::DEBUGMORE, " ok ipaddr");
				found = true;
			}
			if (sarg == "tsurl") {
				dinfo.setThingspeakURL(varg);
				debug.println(DebugLevel::DEBUGMORE, " ok tsurl");
				found = true;
			}
			if (sarg == "tschannel") {
				dinfo.setThingspeakChannel(varg);
				debug.println(DebugLevel::DEBUGMORE, " ok tschannel");
				found = true;
			}
			if (strncmp(sarg.c_str(), "port", 4) == 0) {
				found = true;
				char c = sarg.c_str()[4];
				int n = static_cast<int>(c) - static_cast<int>('0');
				debug.print(DebugLevel::DEBUGMORE, ", n=");
				debug.print(DebugLevel::DEBUGMORE, n);
				if (n >= 0 && n < dinfo.getPortMax()) {
					if (varg.length() > 0) {
						dinfo.setPortName(n, varg);
						debug.print(DebugLevel::DEBUGMORE,
								" ok, port[" + String(n) + "].name set to " + varg);
					}
					else {
						dinfo.setPortName(n, "");
						debug.println(DebugLevel::DEBUGMORE, " ok - cleared");
					}
				}
				else {
					debug.println(DebugLevel::DEBUGMORE,
							nl + "ERROR: Bug - Invalid port #(" + String(n) + "," + String(c)
									+ ") found in ConfigurationChange() - " + sarg);
				}
			}

			if (strncmp(sarg.c_str(), "adjport", 7) == 0) {
				found = true;
				char c1 = sarg.c_str()[7];
				char c2 = sarg.c_str()[8];
				int n1 = static_cast<int>(c1) - static_cast<int>('0');
				int n2 = static_cast<int>(c2) - static_cast<int>('0');
				debug.print(DebugLevel::DEBUGMORE, ", n1=" + String(n1));
				debug.print(DebugLevel::DEBUGMORE, ", n2=" + String(n2));
				if (n1 >= 0 && n1 < dinfo.getPortMax()) {
					double d = 0;
					if (varg.length() > 0) {
						debug.println(DebugLevel::DEBUGMORE, " ok, set");
						d = ::atof(varg.c_str());
					}
					else {
						debug.println(DebugLevel::DEBUGMORE, " ok, cleared");
					}
					dinfo.setPortAdj(n1, n2, d);
				}
				else {
					debug.println(DebugLevel::DEBUGMORE,
							nl + "ERROR: Bug - Invalid port #(" + String(n1) + "," + String(c1)
									+ ") found in ConfigurationChange() - " + sarg);
				}
			}

			if (strncmp(sarg.c_str(), "radport", 7) == 0) {
				found = true;
				char c1 = sarg.c_str()[7];
				int n1 = static_cast<int>(c1) - static_cast<int>('0');
				debug.print(DebugLevel::DEBUGMORE, ", n1=");
				debug.print(DebugLevel::DEBUGMORE, n1);
				if (n1 >= 0 && n1 < dinfo.getPortMax()) {
					bool found1 = false;
					if (varg.length() > 0) {
						//lint -e{26,785} suppress since lint doesn't understand C++11
						for (int j = 0; j < static_cast<int>(sensorModule::END); j++) {
							if (strcmp(varg.c_str(), sensorList[j].name) == 0) {
// FIXME there is a function that does the same as these next few lines
								sensorModule current = dinfo.getPortMode(n1);
								if (current != sensorList[j].id) {
									dinfo.setPortMode(n1, sensorList[j].id);
									need_reboot = true;
								}
								debug.print(DebugLevel::DEBUGMORE, nl + "Info: Setting mode to ");
								debug.print(DebugLevel::DEBUGMORE, static_cast<int>(sensorList[j].id));
								debug.print(DebugLevel::DEBUGMORE, "(");
								debug.print(DebugLevel::DEBUGMORE, sensorList[j].name);
								debug.print(DebugLevel::DEBUGMORE, "), Reboot needed: ");
								if (need_reboot) {
									debug.println(DebugLevel::DEBUGMORE, "yes");
								}
								else {
									debug.println(DebugLevel::DEBUGMORE, "no");
								}
								found1 = true;
								break;
							}
						}
						if (!found1) {
							debug.println(DebugLevel::DEBUGMORE, nl + "ERROR: unable to map mode: " + varg);
						}
					}
					else {
						debug.print(DebugLevel::DEBUGMORE, nl + "ERROR: Invalid varg mode: (" + varg);
						debug.println(DebugLevel::DEBUGMORE, ") found in ConfigurationChange() - " + sarg);
					}
				}
				else {
					debug.print(DebugLevel::DEBUGMORE, nl + "ERROR:Invalid port #(");
					debug.print(DebugLevel::DEBUGMORE, n1);
					debug.print(DebugLevel::DEBUGMORE, ",");
					debug.print(DebugLevel::DEBUGMORE, c1);
					debug.println(DebugLevel::DEBUGMORE, ") found in ConfigurationChange() - " + sarg);
				}
			}

			if (sarg == "reboot") {
				debug.println(DebugLevel::DEBUGMORE, ", reboot button pressed");
				found = true;
				// do the action
				ESP.reset();
			}

			if (sarg == "status") {
				debug.println(DebugLevel::DEBUGMORE, ", status button pressed");
				found = true;
				// do the action
				server.send(200, "text/plain", dinfo.databaseToString(",\n"));
			}

			if (!found) {
				debug.println(DebugLevel::DEBUGMORE, "");
			}
			found = false;
		}

		dinfo.saveDatabaseToEEPROM();
		if (need_reboot) {
			ESP.reset();
		}

		// Regardless if the calibration data changed or not, recopy it into the Sensors
		CopyCalibrationDataToSensors();
	}
	return server.args();
}
