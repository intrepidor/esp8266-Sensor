/*
 *
 *  Created on: Jan 30, 2016
 *      Author: Allan Inda
 */

#include <stdio.h>
#include <stdlib.h>
#include "Queue.h"
#include "network.h"
#include "util.h"
#include "main.h"
#include "deviceinfo.h"
#include "net_config.h"
#include "thingspeak.h"

extern int ConfigurationChange(void);
extern String sHTML_INPUT(int size, String name, String value);
extern String getfieldHTMLcode(int selectedfield);
extern String sHTML_TSFieldExtra_INPUT(int xfield_index);

//////////////////////////////////////////////////////////////////////////////
// ICACHE_FLASH_ATTR strings
ICACHE_FLASH_ATTR const char sHTTP_TOP[] =
		"<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">"
				"<html><head><title>Environment Sensor</title>";
ICACHE_FLASH_ATTR const char sHTTP_CSS[] = "<STYLE type=\"text/css\">"
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
		"  font-size:15px;"
		"  font-weight:bold;"
		"  text-align:center;"
		"  text-decoration:none;"
		"  padding-left:5px;"
		"  padding-right:5px;"
		"  margin-left:2px;"
		"  text-align:center;"
		"  border:none;"
		"  height:50px;"
		"  width:60px;"
		"  display:flex;"
		"  flex-direction:column;"
		"  justify-content:center;"
		"  color:white;"
		"}"
		".savediv{"
//		"  float:right;"
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
		".fieldlong{"
		"  width:60%;"
		"}"
		".fieldmultiline{"
		"  background-color:lightgray;"
		"  color:darkgreen;"
		"  margin-top:-2px;"
		"  border-radius:2px;"
		"}"
		".newcheckbox{"
		"  display:inline-block;"
		"  width:15px;"
		"  height:15px;"
		"  margin:-1px 3px 0 3px;"
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
ICACHE_FLASH_ATTR const char sHTTP_START_BODY[] = "</head><body>";

//////////////////////////////////////////////////////////////////////////////
// Generic and reused statements
//const char sHTTP_ENDLABEL_BR[] = "></label> <br>";
const char sHTTP_ENDLABELQ_BR[] = "\"></label><br>";
const char sHTTP_ENDLABELQ[] = "\"></label>";
//const char sHTTP_ENDBRACEQ[] = "\">";
const char sHTTP_CLOSE_AND_VALUE[] = "\"  value=\"";
const char sHTTP_END[] = "</body></html>";
// ## Header
const char sHTTP_DIVSTART[] = "<div class=\"base ";
const char sHTTP_DIVSTART_CLOSE[] = "\">";
const char sHTTP_DIVEND[] = "</div>";
// ## Input Fields
const char sHTTP_INPUT_TEXT_LONG[] = "<input type=\"text\" class=\"field fieldlong\" name=\"";
const char sHTTP_INPUT_TEXT_MEDIUM[] = "<input type=\"text\" class=\"field fieldmedium\" name=\"";
const char sHTTP_INPUT_TEXT_SHORT[] = "<input type=\"text\" class=\"field fieldshort\" name=\"";
//
// ## Device Configuration Type -- used to determine which config web page is sending a response
// ##   This is placed at the top and passes values back to the webserver. The webserver looks
// ##   at the type returned so it knows how to process the rest of the page.
const char sHTTP_CONFIG_TYPE[] = "<input type=\"text\" name=\"configtype\" hidden readonly value=\"";
const char sHTTP_CONFIG_TYPE_DEVICE[] = "device\">";
const char sHTTP_CONFIG_TYPE_THINGSPEAK[] = "thingspeak\">";
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

const char sHTTP_FARHEN_ENABLE[] = ""
		"<input type=\"checkbox\" class=\"newcheckbox\" name=\"temp_farhen\" value=\"tempunitsenable\" ";
// <print "checked" or blank>
// <ENDLABEL>
// ## Thingspeak

const char sHTTP_TS_ENABLE[] = ""
		"<input type=\"checkbox\" class=\"newcheckbox\" name=\"ts_enable\" value=\"tsenable\" ";
// <print "checked" or blank>
// <ENDLABEL>

const char sHTTP_TS_URL[] = ""
		"<label>URL (optional): "
		"<input type=\"text\" class=\"field fieldlong\" name=\"tsurl\" value=\"";
// print current url
// <ENDLABELQ>

const char sHTTP_TS_CHANNEL[] = ""
		"<label>Channel (optional): "
		"<input type=\"text\" class=\"field fieldshort\" name=\"tschannel\" value=\"";
// print current channel
// <ENDLABELQ>

const char sHTTP_TS_PERIOD[] = ""
		"<label>Update Period (sec): "
		"<input type=\"text\" class=\"field fieldshort\" name=\"tsupdateperiod\" value=\"";
// print current update period
// <ENDLABELQ>

const char sHTTP_TS_APIKEY[] = ""
		"<label>API Write Key: "
		"<input type=\"text\" class=\"field fieldmedium\" name=\"apikey\" value=\"";
// print current apikey
// <ENDLABELQ>

const char sHTTP_TS_IPADDR[] =
		"<label>IP address: <input type=\"text\" class=\"field fieldmedium\" name=\"ipaddr\" value=\"";
// print current ipaddress
// <ENDLABELQ>

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
	wf += a + hre + lin + "/config\">Device Config" + sHTTP_AHREF_END;
	wf += a + hre + lin + "/tsconfig\">Thing Speak" + sHTTP_AHREF_END;
	wf += a + hre + lin + "/status\">Status" + sHTTP_AHREF_END;
	wf += a + hre + lin + "/sensordebug\">Sensor Debug" + sHTTP_AHREF_END;
	if (all) {
		wf += a + def + lin + "/factory_settings\">Factory Settings" + sHTTP_AHREF_END;
		wf += a + reb + lin + "/reboot\">Reboot" + sHTTP_AHREF_END;
		wf += sHTTP_BUTTONS;
	}
	wf += sHTTP_DIVEND;
	return wf;
}

//-----------------------------------------------------------------------------------
String sHTML_INPUT(int size, String name, String value) {
	String s("");
	switch (size) {
		case 0:
			s += sHTTP_INPUT_TEXT_SHORT;
			break;
		case 1:
			s += sHTTP_INPUT_TEXT_MEDIUM;
			break;
		default:
		case 2:
			s += sHTTP_INPUT_TEXT_LONG;
			break;
	}
	s += name + "\" value=\"" + value + "\"><br>";
	return s;
}

//-----------------------------------------------------------------------------------
void sendHTML_Header(bool sendCSS) {
//	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
//	server.sendHeader("Pragma", "no-cache");
//	server.sendHeader("Expires", "-1");
//	server.send(200, "text/html", ""); 	// Empty content inhibits Content-length header so we have to close the socket ourselves.

	server.sendContent_P(sHTTP_TOP);
	if (sendCSS) {
		server.sendContent_P(sHTTP_CSS);
	}
	server.sendContent_P(sHTTP_START_BODY);
}

//-----------------------------------------------------------------------------------
void config(void) {
	// If there was a submit, then process the changes
	ConfigurationChange();

	// Start of page
	String r("");
	sendHTML_Header(true);
	r += "<form action=\"config\" method=\"get\" name=\"Configuration\">";
	r += "<h2>Device Configuration :: " + ProgramInfo + "</h2>";
	server.sendContent(r);

	// Device Name
	r = sHTTP_DIVSTART + String("device") + sHTTP_DIVSTART_CLOSE;
	r += sHTTP_CONFIG_TYPE + String(sHTTP_CONFIG_TYPE_DEVICE);
	r += sHTTP_DEVICE_NAME + String(dinfo.getDeviceName()) + sHTTP_ENDLABELQ_BR;
	r += sHTTP_DEVICE_ID + String(dinfo.getDeviceID()) + sHTTP_ENDLABELQ;
	r += sHTTP_DIVSTART + String("temp_units") + sHTTP_DIVSTART_CLOSE;
	r += sHTTP_FARHEN_ENABLE + String(getCheckedStr(dinfo.isFahrenheit())) + "> Farhenheit Units<br>";
	r += sHTTP_DIVEND;

//	// Thingspeak
//	r += sHTTP_DIVSTART + String("thingspeak") + sHTTP_DIVSTART_CLOSE;
//	r += sHTTP_TS_ENABLE + String(dinfo.getThingspeakEnableStr()) + "> Thingspeak Enable<br>";
//	r += sHTTP_TS_PERIOD + String(dinfo.getThingspeakUpdatePeriodSeconds()) + sHTTP_ENDLABELQ_BR;
//	r += sHTTP_TS_APIKEY + dinfo.getThingspeakApikey() + sHTTP_ENDLABELQ_BR;
//	r += sHTTP_TS_URL + dinfo.getThingspeakURL() + sHTTP_ENDLABELQ_BR;
//	r += sHTTP_TS_CHANNEL + String(dinfo.getThingspeakChannel()) + sHTTP_ENDLABELQ_BR;
//	r += sHTTP_TS_IPADDR + dinfo.getThingspeakIpaddr() + sHTTP_ENDLABELQ + " Default=184.106.153.149";
//	r += sHTTP_DIVEND;
	server.sendContent(r);

	// Ports
	for (int i = 0; i < dinfo.getPortMax(); i++) {
		// Port Name
		r = sHTTP_DIVSTART + String("port") + sHTTP_DIVSTART_CLOSE;
		r += "<label>Port#" + String(i) + sHTTP_INPUT_TEXT_MEDIUM;
		//"<input type=\"text\" class=\"field fieldmedium\" name=\"port";
		r += "port" + String(i) + sHTTP_CLOSE_AND_VALUE + dinfo.getPortName(i) + sHTTP_ENDLABELQ;

		// Port Mode - drop-down menu
		r += "<br>Port Mode <select name=\"modemenu" + String(i) + "\">";
		//lint -e{26,785} suppress since lint doesn't understand C++11
		for (int mode = 0; mode < static_cast<int>(sensorModule::END); mode++) {
			r += "<option value=\"" + String(sensorList[static_cast<int>(mode)].name) + "\"";
			if (dinfo.getPortMode(i) == static_cast<sensorModule>(mode)) {
				r += "selected";
			}
			r += ">" + String(sensorList[static_cast<int>(mode)].name) + "</option>";
		}
		r += "</select><br>";

		// Port Adj Numeric Values
		for (int k = 0; k < dinfo.getPortAdjMax(); k++) {
			r += "<label>" + /*String(k)+*/dinfo.getPortAdjName(i, k);
			//r += " <input type=\"text\" class=\"field fieldshort\" name=\"";
			r += String(sHTTP_INPUT_TEXT_SHORT) + "adjport" + String(i) + String(k) + sHTTP_CLOSE_AND_VALUE
					+ String(dinfo.getPortAdj(i, k), DECIMAL_PRECISION) + sHTTP_ENDLABELQ + "<br>";
		}

//		// Port Mode - radio buttons
//		r += "<br>Port Mode: ";
//		//lint -e{26,785} suppress since lint doesn't understand C++11
//		for (int j = 0; j < static_cast<int>(sensorModule::END); j++) {
//			r += "<input type=\"radio\" class=\"newradio\" name=\"radport";
//			r += String(i) + sHTTP_CLOSE_AND_VALUE;
//			r += String(sensorList[static_cast<int>(j)].name);
//			r += "\" ";
//			if (dinfo.getPortMode(i) == static_cast<sensorModule>(j)) {
//				r += "checked";
//			}
//			r += ">" + String(sensorList[static_cast<int>(j)].name);
//		}

// Current Values
		for (int vindx = 0; vindx < getSensorValueCount(); vindx++) {
			r += " raw/val" + String(i) + String(vindx) + "= "
					+ sensors[i]->getRawValueWithoutUnitConversion(vindx) + "/"
					+ sensors[i]->getValueWithoutUnitConversion(vindx) + String("  ");
		}

// Done with Port content
		r += sHTTP_DIVEND;

		// send
		server.sendContent(r);
	}

// Buttons and links then END of Page
	r = getWebFooter(true) + "</form>" + sHTTP_END;
	server.sendContent(r);
}

//-----------------------------------------------------------------------------------
String getfieldHTMLcode(int selectedfield) {
	String s("");
	for (int field = 0; field <= MAX_THINGSPEAK_FIELD_COUNT; field++) {
		s += "<option value=\"";
		if (field == 0) {
			s += "None";
		}
		else {
			s += "field" + String(field);
		}
		s += "\"";
		if (field == selectedfield) {
			s += "selected";
		}
		if (field == 0) {
			s += ">None";
		}
		else {
			s += ">Field" + String(field);
		}
		s += "</option>";
	}
	s += "</select>";
	return s;
}

String sHTML_TSFieldExtra_INPUT(int xfield_index) {
	String s("<select name=\"xfldmenu");
	s += String(xfield_index) + "\">";
	s += getfieldHTMLcode(dinfo.getTSFieldExtraNumber(xfield_index));

	s += sHTTP_INPUT_TEXT_MEDIUM;
	s += "xfldname" + String(xfield_index) + "\" value=\"";
	s += dinfo.getTSFieldExtraName(xfield_index) + "\">";
	return s;
}

String sHTML_TSFieldPort_INPUT(int xfield_index) {
	String s("<select name=\"pfldmenu");
	s += String(xfield_index) + "\">";
	s += getfieldHTMLcode(dinfo.getTSFieldPortNumber(xfield_index));

	s += sHTTP_INPUT_TEXT_MEDIUM;
	s += "pfldname" + String(xfield_index) + "\" value=\"";
	s += dinfo.getTSFieldPortName(xfield_index) + "\">";
	return s;
}

//-----------------------------------------------------------------------------------
void tsconfig(void) {
	// If there was a submit, then process the changes
	ConfigurationChange();

	// Start of page
	String r("");
	sendHTML_Header(true);
	r += "<form action=\"tsconfig\" method=\"get\" name=\"Thingspeak Configuration\">";
	r += "<h2>Thingspeak Configuration :: " + ProgramInfo + "</h2>";
	server.sendContent(r);

	// Thingspeak
	r = sHTTP_DIVSTART + String("thingspeak") + sHTTP_DIVSTART_CLOSE;
	r += sHTTP_CONFIG_TYPE + String(sHTTP_CONFIG_TYPE_THINGSPEAK);
	r += sHTTP_TS_ENABLE + String(dinfo.getThingspeakEnableStr()) + "> Thingspeak Enable<br>";
	r += sHTTP_TS_PERIOD + String(dinfo.getThingspeakUpdatePeriodSeconds()) + sHTTP_ENDLABELQ_BR;
	r += sHTTP_TS_APIKEY + dinfo.getThingspeakApikey() + sHTTP_ENDLABELQ_BR;
	r += sHTTP_TS_URL + dinfo.getThingspeakURL() + sHTTP_ENDLABELQ_BR;
	r += sHTTP_TS_CHANNEL + String(dinfo.getThingspeakChannel()) + sHTTP_ENDLABELQ_BR;
	r += sHTTP_TS_IPADDR + dinfo.getThingspeakIpaddr() + sHTTP_ENDLABELQ + " Default=184.106.153.149";
	r += sHTTP_DIVEND;
	server.sendContent(r);

	// Channel Settings
	r = sHTTP_DIVSTART + String("thingspeak") + sHTTP_DIVSTART_CLOSE;
	r += "<label>Channel Name: " + String(sHTTP_INPUT_TEXT_MEDIUM) + "chname\" value=\"";
	r += dinfo.getTSChannelName() + sHTTP_ENDLABELQ_BR;
	r += "<label>Desc: " + String(sHTTP_INPUT_TEXT_LONG) + "chdesc\" value=\"";
	r += dinfo.getTSChannelDesc() + sHTTP_ENDLABELQ_BR;
	r += sHTTP_DIVEND;
	server.sendContent(r);

	// Device Special Fields
	r = sHTTP_DIVSTART + String("thingspeak") + sHTTP_DIVSTART_CLOSE;
	r += "<label>Non-Sensor Parameters</label>";
	r += "<br>PIR " + sHTML_TSFieldExtra_INPUT(0);
	r += "<br>RSSI " + sHTML_TSFieldExtra_INPUT(1);
	r += "<br>Uptime " + sHTML_TSFieldExtra_INPUT(2);
	r += "<br>";
	r += sHTTP_DIVEND;
	server.sendContent(r);

	// Port Fields
	r = sHTTP_DIVSTART + String("thingspeak") + sHTTP_DIVSTART_CLOSE;
	int fieldindex = 0;
	for (int sensor = 0; sensor < SENSOR_COUNT; sensor++) {
		r += "<b>Port " + String(sensor) + "</b><br>";
		if (sensors[sensor]) {
			for (int channel = 0; channel < getSensorValueCount(); channel++) {
				r += "Channel " + String(channel) + ": ";
				r += sensors[sensor]->getValueName(channel);
				r += sHTML_TSFieldPort_INPUT(fieldindex + channel);
				r += "<br>";
			}
		}
		fieldindex += 2;
	}
	r += sHTTP_DIVEND;
	server.sendContent(r);

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
	bool taskclock_found = false;
	bool ts_enable = false;
	bool temp_farhen = false;
	int which_form = 0;	// 0=unknown; 1=Device Config, 2=Thingspeak Config
	if (server.args() > 0) {
		bool found = false;
		debug.println(DebugLevel::HTTPGET, "");
		debug.println(DebugLevel::HTTPGET, F("##########################################################"));
		for (uint8_t i = 0; i < server.args(); i++) {
			String sarg = server.argName(i);
			String varg = server.arg(i);
			debug.print(DebugLevel::HTTPGET, "NAME=" + sarg + ", VALUE=" + varg);
			// Determine which configuration webpage by looking at the configtype value.
			//   This is suppoosed to be at the top of the page so it's the first
			//   value returned. Then page specific initialization code can be put here.
			//   Run the page specific code and initializations.
			if (sarg == String("configtype")) {
				if (varg == "device") {
					which_form = 1;
					if (!ts_enable) {
						dinfo.setFahrenheitUnit(false);
					}
				}
				else if (varg == "thingspeak") {
					which_form = 2;
					if (!temp_farhen) {
						dinfo.setThingspeakEnable(false);
					}
				}
			}
			// --------------------------------------------------------
			// Thingspeak Configuration -- Panel 1
			// --------------------------------------------------------
			if (sarg == String("ts_enable")) {
				dinfo.setThingspeakEnable(true);
				debug.println(DebugLevel::HTTPGET, F(" ok ts_enable"));
				found = true;
				ts_enable = true;
			}
			if (sarg == "tsupdateperiod") {
				unsigned long l = static_cast<unsigned long>(::atol(varg.c_str()));
				dinfo.setThingspeakUpdatePeriodSeconds(l);
				debug.println(DebugLevel::HTTPGET, F(" ok tsupdateperiod"));
				found = true;
			}
			if (sarg == "apikey") {
				dinfo.setThingspeakApikey(varg);
				debug.println(DebugLevel::HTTPGET, F(" ok apikey"));
				found = true;
			}
			if (sarg == "ipaddr") {
				dinfo.setThingspeakIpaddr(varg);
				debug.println(DebugLevel::HTTPGET, F(" ok ipaddr"));
				found = true;
			}
			if (sarg == "tsurl") {
				dinfo.setThingspeakURL(varg);
				debug.println(DebugLevel::HTTPGET, F(" ok tsurl"));
				found = true;
			}
			if (sarg == "tschannel") {
				unsigned long l = static_cast<unsigned long>(::atol(varg.c_str()));
				dinfo.setThingspeakChannel(l);
				debug.println(DebugLevel::HTTPGET, F(" ok tschannel"));
				found = true;
			}
			// --------------------------------------------------------
			// Thingspeak Configuration -- Channel Settings
			// --------------------------------------------------------
			if (sarg == "chname") {
				dinfo.setTSChannelName(varg.c_str());
				debug.println(DebugLevel::HTTPGET, F(" ok chname"));
				found = true;
			}
			if (sarg == "chdesc") {
				dinfo.setTSChannelDesc(varg.c_str());
				debug.println(DebugLevel::HTTPGET, F(" ok chdesc"));
				found = true;
			}
			if (strncmp(sarg.c_str(), "xfldmenu", 8) == 0) {
				found = true;
				char c = sarg.c_str()[8]; // get the menu number
				int fn = 0;
				int n = static_cast<int>(c) - static_cast<int>('0'); // convert menu number to integer
				debug.print(DebugLevel::HTTPGET, ", n=" + String(n));
				if (n >= 0 && n < dinfo.getTSFieldExtraMax()) {
					if (varg.length() > 5) {
						char f = varg.c_str()[5]; // get the field number as a character
						fn = static_cast<int>(f) - static_cast<int>('0'); // convert field number to integer
						if (fn > 0 && fn <= MAX_THINGSPEAK_FIELD_COUNT) {
							debug.println(DebugLevel::HTTPGET,
									" ok, fieldExtra[" + String(n) + "].number set to " + String(fn));
						}
						else {
							fn = 0;
							debug.println(DebugLevel::HTTPGET,
									" ok, fieldExtra[" + String(n) + "].number cleared");
						}
					}
					dinfo.setTSFieldExtraNumber(n, fn);
				}
				else {
					debug.println(DebugLevel::HTTPGET,
							nl + "ERROR: Bug - Invalid ExtraField #(" + String(n) + "," + String(c)
									+ ") found in ConfigurationChange() - " + sarg);
				}
			}
			if (strncmp(sarg.c_str(), "xfldname", 8) == 0) {
				found = true;
				char c = sarg.c_str()[8];
				int n = static_cast<int>(c) - static_cast<int>('0');
				debug.print(DebugLevel::HTTPGET, F(", n="));
				debug.print(DebugLevel::HTTPGET, n);
				if (n >= 0 && n < dinfo.getTSFieldExtraMax()) {
					if (varg.length() > 0) {
						dinfo.setTSFieldExtraName(n, varg);
						debug.println(DebugLevel::HTTPGET,
								" ok, fieldExtra[" + String(n) + "].name set to " + varg);
					}
					else {
						dinfo.setTSFieldExtraName(n, "");
						debug.println(DebugLevel::HTTPGET, F(" ok - cleared"));
					}
				}
				else {
					debug.println(DebugLevel::HTTPGET,
							nl + "ERROR: Bug - Invalid ExtraField #(" + String(n) + "," + String(c)
									+ ") found in ConfigurationChange() - " + sarg);
				}
			}
			if (strncmp(sarg.c_str(), "pfldmenu", 8) == 0) {
				found = true;
				char c = sarg.c_str()[8]; // get the menu number
				int fn = 0;
				int n = static_cast<int>(c) - static_cast<int>('0'); // convert menu number to integer
				debug.print(DebugLevel::HTTPGET, ", n=" + String(n));
				if (n >= 0 && n < dinfo.getTSFieldPortMax()) {
					if (varg.length() > 5) {
						char f = varg.c_str()[5]; // get the field number as a character
						fn = static_cast<int>(f) - static_cast<int>('0'); // convert field number to integer
						if (fn > 0 && fn <= MAX_THINGSPEAK_FIELD_COUNT) {
							debug.println(DebugLevel::HTTPGET,
									" ok, fieldPort[" + String(n) + "].number set to " + String(fn));
						}
						else {
							fn = 0;
							debug.println(DebugLevel::HTTPGET,
									" ok, fieldPort[" + String(n) + "].number cleared");
						}
					}
					dinfo.setTSFieldPortNumber(n, fn);
				}
				else {
					debug.println(DebugLevel::HTTPGET,
							nl + "ERROR: Bug - Invalid PortField #(" + String(n) + "," + String(c)
									+ ") found in ConfigurationChange() - " + sarg);
				}
			}
			if (strncmp(sarg.c_str(), "pfldname", 8) == 0) {
				found = true;
				char c = sarg.c_str()[8];
				int n = static_cast<int>(c) - static_cast<int>('0');
				debug.print(DebugLevel::HTTPGET, F(", n="));
				debug.print(DebugLevel::HTTPGET, n);
				if (n >= 0 && n < dinfo.getTSFieldPortMax()) {
					if (varg.length() > 0) {
						dinfo.setTSFieldPortName(n, varg);
						debug.println(DebugLevel::HTTPGET,
								" ok, fieldPort[" + String(n) + "].name set to " + varg);
					}
					else {
						dinfo.setTSFieldPortName(n, "");
						debug.println(DebugLevel::HTTPGET, F(" ok - cleared"));
					}
				}
				else {
					debug.println(DebugLevel::HTTPGET,
							nl + "ERROR: Bug - Invalid PortField #(" + String(n) + "," + String(c)
									+ ") found in ConfigurationChange() - " + sarg);
				}
			}
			// --------------------------------------------------------
			// Device Configuration
			// --------------------------------------------------------
			if (sarg == String("name")) {
				dinfo.setDeviceName(varg);
				debug.println(DebugLevel::HTTPGET, F(" ok name"));
				found = true;
			}
			if (sarg == String("deviceid")) {
				dinfo.setDeviceID(atoi(varg.c_str()));
				debug.println(DebugLevel::HTTPGET, F(" ok deviceid"));
				found = true;
			}

			if (sarg == String("temp_farhen")) {
				dinfo.setFahrenheitUnit(true);
				debug.println(DebugLevel::HTTPGET, F(" ok tempunitenable"));
				found = true;
				temp_farhen = true;
			}
			if (strncmp(sarg.c_str(), "port", 4) == 0) {
				found = true;
				char c = sarg.c_str()[4];
				int n = static_cast<int>(c) - static_cast<int>('0');
				debug.print(DebugLevel::HTTPGET, F(", n="));
				debug.print(DebugLevel::HTTPGET, n);
				if (n >= 0 && n < dinfo.getPortMax()) {
					if (varg.length() > 0) {
						dinfo.setPortName(n, varg);
						debug.print(DebugLevel::HTTPGET, " ok, port[" + String(n) + "].name set to " + varg);
					}
					else {
						dinfo.setPortName(n, "");
						debug.println(DebugLevel::HTTPGET, F(" ok - cleared"));
					}
				}
				else {
					debug.println(DebugLevel::HTTPGET,
							nl + "ERROR: Bug - Invalid port #(" + String(n) + "," + String(c)
									+ ") found in ConfigurationChange() - " + sarg);
				}
			}

			if (strncmp(sarg.c_str(), "modemenu", 8) == 0) {
				found = true;
				char c1 = sarg.c_str()[8];
				int n1 = static_cast<int>(c1) - static_cast<int>('0');
				debug.print(DebugLevel::HTTPGET, ", n1=");
				debug.print(DebugLevel::HTTPGET, n1);
				if (n1 >= 0 && n1 < dinfo.getPortMax()) {
					bool found1 = false;
					if (varg.length() > 0) {
						//lint -e{26,785} suppress since lint doesn't understand C++11
						for (int j = 0; j < static_cast<int>(sensorModule::END); j++) {
							if (strcmp(varg.c_str(), sensorList[j].name) == 0) {
								sensorModule current = dinfo.getPortMode(n1);
								sensorModule newmode = sensorList[j].id;
								// Handle the special case for taskclock
								if (current == sensorModule::taskclock) {
									if (taskclock_found) {
										// only one sensorModule::taskclock allowed. Turn off
										//   attempts to add more than one
										newmode = sensorModule::off;
									}
									else {
										taskclock_found = true;
									}
								}
								// Set the new port Mode
								if (current != newmode) {
									dinfo.setPortMode(n1, newmode);
									need_reboot = true;
								}
								debug.print(DebugLevel::HTTPGET, nl + "Info: Setting mode to ");
								debug.print(DebugLevel::HTTPGET, static_cast<int>(sensorList[j].id));
								debug.print(DebugLevel::HTTPGET, F("("));
								debug.print(DebugLevel::HTTPGET, sensorList[j].name);
								debug.print(DebugLevel::HTTPGET, F("), Reboot needed: "));
								if (need_reboot) {
									debug.println(DebugLevel::HTTPGET, "yes");
								}
								else {
									debug.println(DebugLevel::HTTPGET, "no");
								}
								found1 = true;
								break;
							}
						}
						if (!found1) {
							debug.println(DebugLevel::HTTPGET, nl + "ERROR: unable to map mode: " + varg);
						}
					}
					else {
						debug.print(DebugLevel::HTTPGET, nl + "ERROR: Invalid varg mode: (" + varg);
						debug.println(DebugLevel::HTTPGET, ") found in ConfigurationChange() - " + sarg);
					}
				}
				else {
					debug.print(DebugLevel::HTTPGET, nl + "ERROR:Invalid port #(");
					debug.print(DebugLevel::HTTPGET, n1);
					debug.print(DebugLevel::HTTPGET, ",");
					debug.print(DebugLevel::HTTPGET, c1);
					debug.println(DebugLevel::HTTPGET, ") found in ConfigurationChange() - " + sarg);
				}
			}

			if (strncmp(sarg.c_str(), "adjport", 7) == 0) {
				found = true;
				char c1 = sarg.c_str()[7];
				char c2 = sarg.c_str()[8];
				int n1 = static_cast<int>(c1) - static_cast<int>('0');
				int n2 = static_cast<int>(c2) - static_cast<int>('0');
				debug.print(DebugLevel::HTTPGET, ", n1=" + String(n1));
				debug.print(DebugLevel::HTTPGET, ", n2=" + String(n2));
				if (n1 >= 0 && n1 < dinfo.getPortMax()) {
					double d = 0;
					if (varg.length() > 0) {
						debug.println(DebugLevel::HTTPGET, F(" ok, set"));
						d = ::atof(varg.c_str());
					}
					else {
						debug.println(DebugLevel::HTTPGET, F(" ok, cleared"));
					}
					dinfo.setPortAdj(n1, n2, d);
				}
				else {
					debug.println(DebugLevel::HTTPGET,
							nl + "ERROR: Bug - Invalid port #(" + String(n1) + "," + String(c1)
									+ ") found in ConfigurationChange() - " + sarg);
				}
			}

//			if (strncmp(sarg.c_str(), "radport", 7) == 0) {
//				found = true;
//				char c1 = sarg.c_str()[7];
//				int n1 = static_cast<int>(c1) - static_cast<int>('0');
//				debug.print(DebugLevel::HTTPGET, ", n1=");
//				debug.print(DebugLevel::HTTPGET, n1);
//				if (n1 >= 0 && n1 < dinfo.getPortMax()) {
//					bool found1 = false;
//					if (varg.length() > 0) {
//						//lint -e{26,785} suppress since lint doesn't understand C++11
//						for (int j = 0; j < static_cast<int>(sensorModule::END); j++) {
//							if (strcmp(varg.c_str(), sensorList[j].name) == 0) {
//								sensorModule current = dinfo.getPortMode(n1);
//								if (current != sensorList[j].id) {
//									dinfo.setPortMode(n1, sensorList[j].id);
//									need_reboot = true;
//								}
//								debug.print(DebugLevel::HTTPGET, nl + "Info: Setting mode to ");
//								debug.print(DebugLevel::HTTPGET, static_cast<int>(sensorList[j].id));
//								debug.print(DebugLevel::HTTPGET, F("("));
//								debug.print(DebugLevel::HTTPGET, sensorList[j].name);
//								debug.print(DebugLevel::HTTPGET, F("), Reboot needed: "));
//								if (need_reboot) {
//									debug.println(DebugLevel::HTTPGET, "yes");
//								}
//								else {
//									debug.println(DebugLevel::HTTPGET, "no");
//								}
//								found1 = true;
//								break;
//							}
//						}
//						if (!found1) {
//							debug.println(DebugLevel::HTTPGET, nl + "ERROR: unable to map mode: " + varg);
//						}
//					}
//					else {
//						debug.print(DebugLevel::HTTPGET, nl + "ERROR: Invalid varg mode: (" + varg);
//						debug.println(DebugLevel::HTTPGET, ") found in ConfigurationChange() - " + sarg);
//					}
//				}
//				else {
//					debug.print(DebugLevel::HTTPGET, nl + "ERROR:Invalid port #(");
//					debug.print(DebugLevel::HTTPGET, n1);
//					debug.print(DebugLevel::HTTPGET, ",");
//					debug.print(DebugLevel::HTTPGET, c1);
//					debug.println(DebugLevel::HTTPGET, ") found in ConfigurationChange() - " + sarg);
//				}
//			}

			if (sarg == "reboot") {
				debug.println(DebugLevel::HTTPGET, F(", reboot button pressed"));
				found = true;
				// do the action
				ESP.reset();
			}

			if (sarg == "status") {
				debug.println(DebugLevel::HTTPGET, F(", status button pressed"));
				found = true;
				// do the action
				server.send(200, "text/plain", dinfo.databaseToString(",\n"));
			}

			if (!found) {
				debug.println(DebugLevel::HTTPGET, "");
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
