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

void validate_string(char* str, const char* const def, unsigned int size, int lowest, int highest) {
	bool ok = true;
	for (unsigned int n = 0; n < size; n++) {
		if (str[n] < lowest || str[n] > highest) {
			ok = false;
			break;
		}
	}
	if (ok == false) {
		memset(str, 'A', size);
		strncpy(str, def, size - 1);
	}
	str[size - 1] = 0;
}

void Device::init(void) {
	validate_string(db.device.name, "not_named", sizeof(db.device.name), 32, 126);
	validate_string(db.thingspeak.apikey, "no_apikey", sizeof(db.thingspeak.apikey), 48, 95);
	validate_string(db.thingspeak.host, "localhost", sizeof(db.thingspeak.host), 32, 126);
}

void Device::setcDeviceName(const char* newname) {
	if (newname) {
		memset(db.device.name, 0, sizeof(db.device.name));
		strncpy(db.device.name, newname, sizeof(db.device.name) - 1);
	}
}

static bool isValidPort(int portnum) {
	if (portnum >= 0 && portnum < MAX_PORTS) return true;
	return false;
}

const char* PROGMEM Device::getEnableStr() {
	if (db.thingspeak.enabled) return "checked";
	return " ";
}

portModes Device::getPortMode(int portnum) {
	if (isValidPort(portnum)) {
		return static_cast<portModes>(db.port[portnum].mode);
	}
	else {
		return portModes::undefined;
	}
}
void Device::setPortMode(int portnum, portModes _mode) {
	if (isValidPort(portnum)) {
		db.port[portnum].mode = static_cast<char>(_mode);
	}
}
void Device::setPortName(int portnum, String _n) {
	if (isValidPort(portnum)) {
		strncpy(db.port[portnum].name, _n.c_str(), sizeof(db.port[portnum].name) - 1);
	}
}
String Device::getPortName(int portnum) {
	if (isValidPort(portnum)) {
		return this->db.port[portnum].name;
	}
	return String("undefined");
}

double Device::getPortAdj(int portnum, int adjnum) {

	if (isValidPort(portnum)) {
		if (adjnum >= 0 && adjnum < getPortAdjMax()) {
			return static_cast<double>((static_cast<double>(portnum) + 1) * 10 + static_cast<double>(adjnum)); // fixme this line is wrong and temporarily here. Needs to be fixed.
			// fixme this causes the chip to crash -- return db.port[portnum].adj[adjnum];
		}
	}
	return 0.0;
}
void Device::setPortAdj(int portnum, int adjnum, double v) {
	if (isValidPort(portnum)) {
		if (adjnum >= 0 && adjnum < getPortAdjMax()) {
			db.port[portnum].adj[adjnum] = v;
		}
	}
}

void Device::RestoreConfigurationFromEEPROM(void) {
	uint8_t* pp = static_cast<uint8_t*>(static_cast<void*>(&db));
	for (unsigned int addr = 0; addr < sizeof(db); addr++) {
		*(pp + addr) = EEPROM.read(static_cast<int>(addr));
	}
}

bool Device::StoreConfigurationIntoEEPROM(void) {
	uint8_t* pp = static_cast<uint8_t*>(static_cast<void*>(&db));
	uint8_t v = 0;
	for (unsigned int addr = 0; addr < sizeof(db); addr++) {
		v = *(pp + addr);
		EEPROM.write(static_cast<int>(addr), v);
	}

	if (EEPROM.commit()) {
		RestoreConfigurationFromEEPROM();
		Serial.println("SUCCESS: Write to EEPROM ok.");
		return true;
	}
	else {
		Serial.println("ERROR: Write to EEPROM failed!");
		return false; // signal error
	}
}

void Device::printThingspeakInfo(void) {
	Serial.print("Thingspeak host: ");
	Serial.println(db.thingspeak.host);
	Serial.print("API Write  Key: ");
	Serial.println(db.thingspeak.apikey);
	Serial.println("");
}

void Device::updateThingspeak(void) {
	WiFiClient client;

	if (!client.connect(db.thingspeak.host, 80)) {
		Serial.print("error: connection to ");
		Serial.print(db.thingspeak.host);
		Serial.println(" failed");
	}
	else {
		// Send message to Thingspeak
		String getStr = "/update?api_key=";
		getStr += String(db.thingspeak.apikey);
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
				String("GET ") + getStr + " HTTP/1.1\r\n" + "Host: " + db.thingspeak.host + "\r\n"
						+ "Connection: close\r\n\r\n");
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
