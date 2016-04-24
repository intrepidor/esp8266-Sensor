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
//#include "net_config.h"
#include "main.h"

//lint -e{26,785} suppress since lint doesn't understand C++11
t_sensor sensors[static_cast<int>(portModes::END)] = { { "off", portModes::off }, {
		"DHT11", portModes::dht11 }, { "DHT22", portModes::dht22 }, { "DS18b20",
		portModes::ds18b20 }, { "Sonar", portModes::sonar }, { "Dust", portModes::dust },
		{ "Sound", portModes::sound } };

unsigned int validate_string(char* str, const char* const def, unsigned int size,
		int lowest, int highest) {
	bool ok = true;
	unsigned int n = 0; /*lint -e838 */
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
	else {
		return 0;
	}
}

void Device::init(void) {
	validate_string(db.device.name, "<not named>", sizeof(db.device.name), 32, 126);
	validate_string(db.thingspeak.apikey, "<no api key>", sizeof(db.thingspeak.apikey),
			48, 95);
	validate_string(db.thingspeak.host, "<no host>", sizeof(db.thingspeak.host), 32, 126);
}

String Device::toString(void) {
	String s;
	s = String(
			String("config_version=") + String(db.config_version)
					+ String("\r\ndevice.name=") + String(getDeviceName())
					+ String("\r\ndevice.id=") + String(getDeviceID())
					+ String("\r\nthingspeak.status=") + String(getThingspeakStatus())
					+ String("\r\nthingspeak.enable=") + getEnableString()
					+ String("\r\nTS_apikey=") + getThinkspeakApikey()
					+ String("\r\nthingspeak.host=") + getThingspeakHost()
					+ String("\r\nthingspeak.ipaddr=") + getIpaddr());
	for (int i = 0; i < MAX_PORTS; i++) {
		s = s
				+ String(
						String("\r\nport[") + String(i) + String("].name=")
								+ getPortName(i) + String(", mode=")
								+ String(getModeStr(i)) + String(", pin=")
								+ String(db.port[i].pin));
		for (int k = 0; k < MAX_ADJ; k++) {
			s = s
					+ String(
							String(", adj[") + String(k) + String("]=")
									+ String(db.port[i].adj[k]));
		}
	}
	return s;
}

void Device::setcDeviceName(const char* newname) {
	if (newname) {
		memset(db.device.name, 0, sizeof(db.device.name));
		strncpy(db.device.name, newname, sizeof(db.device.name) - 1);
	}
	else {
		Serial.println("\r\nERROR: setcDeviceName() - null value");
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

portModes Device::getPortMode(int portnum) {
	if (isValidPort(portnum)) {
		return static_cast<portModes>(db.port[portnum].mode);
	}
	else {
		return portModes::off;
	}
}
void Device::setPortMode(int portnum, portModes _mode) {
	if (isValidPort(portnum)) {
		db.port[portnum].mode = _mode;
	}
	else {
		Serial.println("\r\nERROR: Device::setPortMode() - invalid port");
	}
}

String Device::getModeStr(int portnum) {
	String s("");
	if (isValidPort(portnum)) {
		int m = static_cast<int>(db.port[portnum].mode);
		//lint -e{26} suppress since lint doesn't understand C++11
		if (m >= 0 && m < static_cast<int>(portModes::END)) {
			s = String(String(m) + ":" + String(sensors[m].name));
		}
		else {
			Serial.print("\r\nERROR: Device::getModeStr() - invalid mode");
		}
	}
	else {
		Serial.println("\r\nERROR: Device::getModeStr() - invalid port");
	}
	return s;
}

void Device::setPortName(int portnum, String _n) {
	if (isValidPort(portnum)) {
		strncpy(db.port[portnum].name, _n.c_str(), sizeof(db.port[portnum].name) - 1);
	}
	else {
		Serial.println("\r\nERROR: Device::setPortName() - invalid port");
	}
}

String Device::getPortName(int portnum) {
	if (isValidPort(portnum)) {
		//FIXME unsigned int n = validate_string(db.port[portnum].name, "not_named", sizeof(db.port[portnum].name), 32, 126);
		//FIXME Serial.print("INFO: getPortName() - n=");
		//FIXME Serial.println(n);
		return this->db.port[portnum].name;
	}
	else {
		Serial.println("\r\nERROR: Device::getPortName() - invalid port");
	}
	return String("undefined");
}

double Device::getPortAdj(int portnum, int adjnum) {

	if (isValidPort(portnum)) {
		if (adjnum >= 0 && adjnum < getPortAdjMax()) {
			return static_cast<double>((static_cast<double>(portnum) + 1) * 10
					+ static_cast<double>(adjnum)); // fixme this line is wrong and temporarily here. Needs to be fixed.
			// fixme this causes the chip to crash -- return db.port[portnum].adj[adjnum];
		}
		else {
			Serial.println("\r\nERROR: Device::getPortAdj() - invalid adj index");
		}
	}
	else {
		Serial.println("\r\nERROR: Device::getPortAdj() - invalid port");
	}
	return 0.0;
}

void Device::setPortAdj(int portnum, int adjnum, double v) {
	if (isValidPort(portnum)) {
		if (adjnum >= 0 && adjnum < getPortAdjMax()) {
			db.port[portnum].adj[adjnum] = v;
		}
		else {
			Serial.println("\r\nERROR: Device::setPortAdj() - invalid adj index");
		}
	}
	else {
		Serial.println("\r\nERROR: Device::setPortAdj() - invalid port");
	}
}

void Device::RestoreConfigurationFromEEPROM(void) {
	uint8_t* pp = static_cast<uint8_t*>(static_cast<void*>(&db));
	for (unsigned int addr = 0; addr < sizeof(db); addr++) {
		*(pp + addr) = EEPROM.read(static_cast<int>(addr));
	}
	Serial.println("\r\nINFO: Data copied from EEPROM into RAM data structure");
}

bool Device::StoreConfigurationIntoEEPROM(void) {
	uint8_t* pp = static_cast<uint8_t*>(static_cast<void*>(&db));
	uint8_t v = 0;
	for (unsigned int addr = 0; addr < sizeof(db); addr++) {
		v = *(pp + addr);
		EEPROM.write(static_cast<int>(addr), v);
//		if (debug_output) {
//			Serial.write(v);
//		}
	}
//	if (debug_output) Serial.println("");

	if (EEPROM.commit()) {
		Serial.println("\r\nSUCCESS: Write from RAM data structure to EEPROM ok.");
		RestoreConfigurationFromEEPROM();
		return true;
	}
	else {
		Serial.println("\r\nERROR: Write to EEPROM failed!");
		return false; // signal error
	}
}

void Device::printThingspeakInfo(void) {
	Serial.print("Thingspeak host: ");
	Serial.println(getcThingspeakHost());
	Serial.print("API Write  Key: ");
	Serial.println(getcThinkspeakApikey());
	Serial.println("");
}

void Device::updateThingspeak(void) {
	WiFiClient client;

	if (!client.connect(db.thingspeak.host, 80)) {
		Serial.print("error: connection to ");
		Serial.print(getcThingspeakHost());
		Serial.println(" failed");
	}
	else {
		// Send message to Thingspeak
		String getStr = "/update?api_key=";
		getStr += getThinkspeakApikey();
		getStr += "&field1=";
		getStr += String(t1.getTemperature());
		getStr += "&field2=";
		getStr += String(t1.getHumidity());
		getStr += "&field3=";
		getStr += String(t1.getHeatindex());
		getStr += "&field4=";
		getStr += String(t2.getTemperature());
		getStr += "&field5=";
		getStr += String(t2.getHumidity());
		getStr += "&field6=";
		getStr += String(t2.getHeatindex());
		getStr += "&field7=";
		getStr += String(PIRcount);
		client.print(
				String("GET ") + getStr + " HTTP/1.1\r\n" + "Host: "
						+ getcThingspeakHost() + "\r\n" + "Connection: close\r\n\r\n");
		delay(10);

		// Read the response
		db.thingspeak.status = 0;
		while (client.available()) {
			String line = client.readStringUntil('\r');
			if (line.startsWith("HTTP/1.1 200 OK")) {
				db.thingspeak.status |= 0x01;
				Serial.println(line);
			}
			if (line.startsWith("Status: 200 OK")) {
				db.thingspeak.status |= 0x02;
				Serial.println(line);
			}
			if (line.startsWith("error")) {
				db.thingspeak.status |= 0x04;
				Serial.println(line);
			}
		}
	}
}
