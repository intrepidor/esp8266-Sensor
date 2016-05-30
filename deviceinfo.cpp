/*
 * deviceinfo.cpp
 *
 *  Created on: Jan 28, 2016
 *      Author: Allan Inda
 */

#include <Arduino.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include "deviceinfo.h"
#include "main.h"
#include "debugprint.h"
#include "sensor.h"

bool eeprom_is_dirty = false;

unsigned int validate_string(char* str, const char* const def, unsigned int size, int lowest, int highest) {
	bool ok = true;
	unsigned int n = 0; /*lint -e838 */
	if (str && def) {
		if (size < strlen(str)) {
			debug.println(DebugLevel::DEBUG,
					nl
							+ "ERROR: validate_string() string passed by pointer is shorter than the passed size value");
		}

		str[size - 1] = 0;
		for (n = 0; n < size - 1; n++) {
			if (str[n] < lowest || str[n] > highest) {
				ok = false;
				break;
			}
		}
		if (ok == false) {
			memset(str, 0, size);
			strncpy(str, def, size - 1);
			str[size - 1] = 0;
			return n;
		}
	}
	else {
		debug.println(DebugLevel::DEBUG,
				nl + "ERROR: validate_string() value for either/both def* or str* is null");
	}
	return 0;
}

void Device::init(void) {
	validate_string(db.device.name, "<not named>", sizeof(db.device.name), 32, 126);
	validate_string(db.thingspeak.apikey, "<no api key>", sizeof(db.thingspeak.apikey), 48, 95);
	validate_string(db.thingspeak.url, "<no url>", sizeof(db.thingspeak.url), 32, 126);
}

String Device::toString(String eol) {
	String s;
	s = "config_version=" + String(db.config_version) + eol;
	s += "debugLevel=" + DebugPrint::convertDebugLevelToString(static_cast<DebugLevel>(dinfo.getDebugLevel()))
			+ eol;
	s += "device.name=" + String(getDeviceName()) + eol;
	s += "device.id=" + String(getDeviceID()) + eol;
	s += "thingspeak.status=" + String(getThingspeakStatus()) + eol;
	s += "thingspeak.enable=" + getEnableString() + eol;
	s += "TS_apikey=" + getThinkspeakApikey() + eol;
	s += "thingspeak.url=" + getThingspeakURL() + eol;
	s += "thingspeak.channel=" + getThingspeakChannel() + eol;
	s += "thingspeak.ipaddr=" + getIpaddr();
	for (int i = 0; i < getPortMax(); i++) {
		s += eol + "port[" + String(i) + "].name=" + getPortName(i) + ", mode=" + String(getModeStr(i));
//		s += String(", pin=") + String(db.port[i].pin); /// pin is not used
		for (int k = 0; k < MAX_ADJ; k++) {
			s += ", adj[" + String(k) + "]=";
			s += String(getPortAdj(i, k), DECIMAL_PRECISION);
		}
	}
	return s;
}

void Device::printInfo(void) {
	for (int i = 0; i < dinfo.getPortMax(); i++) {
		//lint -e{26} suppress error false error about the sensorModule
		for (int j = 0; j < static_cast<int>(sensorModule::END); j++) {
			if (dinfo.getPortMode(i) == static_cast<sensorModule>(j)) {
				debug.println(DebugLevel::ALWAYS,
						"Port#" + String(i) + ": " + sensorList[static_cast<int>(j)].name);
			}
		}
	}
	debug.println(DebugLevel::ALWAYS, "");
}

void Device::setcDeviceName(const char* newname) {
	if (newname) {
		memset(db.device.name, 0, sizeof(db.device.name));
		strncpy(db.device.name, newname, sizeof(db.device.name) - 1);
	}
	else {
		debug.println(DebugLevel::ERROR, nl + "ERROR: setcDeviceName() - null value");
	}
}

static bool isValidPort(int portnum) {
	if (portnum >= 0 && portnum < MAX_PORTS) return true;
	return false;
}

const char* PROGMEM
Device::getEnableStr() {
	if (db.thingspeak.enabled) return "checked";
	return " ";
}

sensorModule Device::getPortMode(int portnum) {
	if (isValidPort(portnum)) {
		return static_cast<sensorModule>(db.port[portnum].mode);
	}
	else {
		return sensorModule::off;
	}
}
void Device::setPortMode(int portnum, sensorModule _mode) {
	if (isValidPort(portnum)) {
		db.port[portnum].mode = _mode;
	}
	else {
		debug.println(DebugLevel::ERROR, nl + "ERROR: Device::setPortMode() - invalid port");
	}
}

String Device::getModeStr(int portnum) {
	String s("");
	if (isValidPort(portnum)) {
		int m = static_cast<int>(db.port[portnum].mode);
		//lint -e{26} suppress since lint doesn't understand sensorModule C++11
		if (m >= 0 && m < static_cast<int>(sensorModule::END)) {
			s = String(String(m) + ":" + String(sensorList[m].name));
		}
		else {
			debug.print(DebugLevel::ERROR, nl + "ERROR: Device::getModeStr() - invalid mode");
		}
	}
	else {
		debug.println(DebugLevel::ERROR, nl + "ERROR: Device::getModeStr() - invalid port");
	}
	return s;
}

void Device::setPortName(int portnum, String _n) {
	if (isValidPort(portnum)) {
		strncpy(db.port[portnum].name, _n.c_str(), sizeof(db.port[portnum].name) - 1);
	}
	else {
		debug.println(DebugLevel::ERROR, nl + "ERROR: Device::setPortName() - invalid port");
	}
}

String Device::getPortName(int portnum) {
	if (isValidPort(portnum)) {
		return this->db.port[portnum].name;
	}
	else {
		debug.println(DebugLevel::ERROR, nl + "ERROR: Device::getPortName() - invalid port");
	}
	return String("undefined");
}

double Device::getPortAdj(int portnum, int adjnum) {

	if (isValidPort(portnum)) {
		if (adjnum >= 0 && adjnum < getPortAdjMax()) {
			return this->db.port[portnum].adj[adjnum];
		}
		else {
			debug.println(DebugLevel::ERROR, nl + "ERROR: Device::getPortAdj() - invalid adj index");
		}
	}
	else {
		debug.println(DebugLevel::ERROR, nl + "ERROR: Device::getPortAdj() - invalid port");
	}
	return 0.0;
}

void Device::setPortAdj(int portnum, int adjnum, double v) {
	if (isValidPort(portnum)) {
		if (adjnum >= 0 && adjnum < getPortAdjMax()) {
			db.port[portnum].adj[adjnum] = v;
		}
		else {
			debug.println(DebugLevel::ERROR, nl + "ERROR: Device::setPortAdj() - invalid adj index");
		}
	}
	else {
		debug.println(DebugLevel::ERROR, nl + "ERROR: Device::setPortAdj() - invalid port");
	}
}

void Device::RestoreConfigurationFromEEPROM(void) {
	uint8_t* pp = static_cast<uint8_t*>(static_cast<void*>(&db));
	for (unsigned int addr = 0; addr < sizeof(db); addr++) {
		*(pp + addr) = EEPROM.read(static_cast<int>(addr));
	}
	eeprom_is_dirty = false;
	debug.println(DebugLevel::DEBUG, nl + "Data copied from EEPROM into RAM data structure");
}

bool Device::StoreConfigurationIntoEEPROM(void) {
	uint8_t* pp = static_cast<uint8_t*>(static_cast<void*>(&db));
	uint8_t v = 0;
	for (unsigned int addr = 0; addr < sizeof(db); addr++) {
		v = *(pp + addr);
		EEPROM.write(static_cast<int>(addr), v);
	}

	if (EEPROM.commit()) {
		debug.println(DebugLevel::DEBUG, nl + "Write from RAM data structure to EEPROM ok.");
		RestoreConfigurationFromEEPROM();
		eeprom_is_dirty = false;
		return true;
	}
	else {
		debug.println(DebugLevel::ERROR, nl + "ERROR: Write to EEPROM failed!");
		return false; // signal error
	}
}

void Device::printThingspeakInfo(void) {
	debug.println(DebugLevel::ALWAYS, "Thingspeak URL: " + getThingspeakURL());
	debug.println(DebugLevel::ALWAYS, "API Write  Key: " + getThinkspeakApikey());
}

void Device::updateThingspeak(void) {
	WiFiClient client;
	int const MAX_THINGSPEAK_FIELD_COUNT = 8; // Thingspeak only accept 8 fields

	if (!client.connect(db.thingspeak.url, 80)) {
		debug.println(DebugLevel::ALWAYS, "error: connection to " + getThingspeakURL() + " failed");
	}
	else {
		// Send message to Thingspeak
		String getStr = "/update?api_key=";
		getStr += getThinkspeakApikey();
		getStr += "&field1=" + String(PIRcount); // The built-in PIR sensor is always field1
		// This code is written so the fields are static regardless of which sensor is used in
		//    which port, or how many ports are present, or how many values are enabled per
		//    a sensor. This is done so the Thingspeak channel configuration is not dependent
		//    on the configuration of the device. Since Thingspeak only permits 8 channels,
		//    and there are nine possible, 1 PIR + 2 per each of 4 sensors, the 4th sensor's
		//    second value will not be sent to Thingspeak.
		int nextfield = 2;
		for (int i = 0; i < SENSOR_COUNT && nextfield <= MAX_THINGSPEAK_FIELD_COUNT; i++) {
			if (sensors[i]) {
				for (int j = 0; j < getValueCount() && nextfield <= MAX_THINGSPEAK_FIELD_COUNT; j++) {
					getStr += "&field" + String(nextfield++) + "=";
					if (sensors[i]->getValueEnable(j)) {
						getStr += String(sensors[i]->getValue(j));
					}
					else {
						getStr += "0";
					}
				}
			}
			else { // sensor not present; assume zero for the values
				for (int k = 0; k < getValueCount() && nextfield <= MAX_THINGSPEAK_FIELD_COUNT; k++) {
					getStr += "&field" + String(nextfield++) + "=0";
				}
			}
		}
		debug.println(DebugLevel::DEBUG, "Thinkspeak URL:" + getStr);
		client.print(
				String("GET ") + getStr + " HTTP/1.1\r\n" + "Host: " + getcThingspeakURL() + "\r\n"
						+ "Connection: close\r\n\r\n");
		delay(10);

		// Read the response
		db.thingspeak.status = 0;
		while (client.available()) {
			String line = client.readStringUntil('\r');
			if (line.startsWith("HTTP/1.1 200 OK")) {
				db.thingspeak.status |= 0x01;
				debug.println(DebugLevel::DEBUG, line);
			}
			if (line.startsWith("Status: 200 OK")) {
				db.thingspeak.status |= 0x02;
				debug.println(DebugLevel::DEBUG, line);
			}
			if (line.startsWith("error")) {
				db.thingspeak.status |= 0x04;
				debug.println(DebugLevel::DEBUG, line);
			}
		}
	}
}
