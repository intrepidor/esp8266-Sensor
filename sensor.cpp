/*
 * sensor.cpp
 *
 *  Created on: May 25, 2016
 *      Author: allan
 */

#include <Arduino.h>
#include "sensor.h"

//lint -e{26,785} suppress since lint doesn't understand C++11
t_sensor const sensorList[static_cast<int>(sensorModule::END)] = { {
		"off",
		sensorModule::off },
// One Digital IO
		{ "DHT11", sensorModule::dht11 },
		{ "DHT22", sensorModule::dht22 },
		{ "DS18b20", sensorModule::ds18b20 },
		{ "Sonar", sensorModule::sonar },
		{ "Sound", sensorModule::sound },
		{ "Reed", sensorModule::reed },
		{ "HCS501", sensorModule::hcs501 },
		{ "HCSR505", sensorModule::hcsr505 },
		// One Digital IO and/or one ADC
		{ "Dust", sensorModule::dust },
		{ "Rain", sensorModule::rain },
		{ "Soil", sensorModule::soil },
		{ "SoundH", sensorModule::soundh },
		{ "Methane", sensorModule::methane },
		// IC2 Devices
		{ "GY68", sensorModule::gy68 },
		{ "GY30", sensorModule::gy30 },
		{ "LCD1602", sensorModule::lcd1602 },
		// Serial
		{ "RFID", sensorModule::rfid },
		{ "Marquee", sensorModule::marquee } };

const char* Sensor::getModule_cstr(void) {
	switch (this->getModule()) {
		// 1 digital
		case sensorModule::dht11:
			return "DHT11";
			break;
		case sensorModule::dht22:
			return "DHT22";
			break;
		case sensorModule::ds18b20:
			return "DS18b20";
			break;
		case sensorModule::sonar:
			return "Sonar";
			break;
		case sensorModule::sound:
			return "Sound";
			break;
		case sensorModule::reed:
			return "Reed";
			break;
		case sensorModule::hcs501:
			return "HCS501";
			break;
		case sensorModule::hcsr505:
			return "HCSR505";
			break;
			// 1 digital + 1 analog
		case sensorModule::dust:
			return "Dust";
			break;
		case sensorModule::rain:
			return "Rain";
			break;
		case sensorModule::soil:
			return "Soil";
			break;
		case sensorModule::soundh:
			return "SoundH";
			break;
		case sensorModule::methane:
			return "Methane";
			break;
			// I2C
		case sensorModule::gy68:
			return "GY68";
			break;
		case sensorModule::gy30:
			return "GY30";
			break;
		case sensorModule::lcd1602:
			return "LCD1602";
			break;
			// serial
		case sensorModule::rfid:
			return "RFID";
			break;
		case sensorModule::marquee:
			return "Marquee";
			break;
			// None
		case sensorModule::off:
			return "OFF";
			break;
		default:
			return "unknown";
			break;
	}
	return "unknown";
}

///////////////////////////////////////////////////////////////////////////////////////
// Values
///////////////////////////////////////////////////////////////////////////////////////
bool Sensor::isValueIndexValid(int _index) {
	if (_index >= 0 && _index < VALUE_COUNT) {
		return true;
	}
	Serial.println("BUG: isValueIndexValid() index out of bounds");
	return false;
}

bool Sensor::setValueName(int _index, String name) {
	if (isValueIndexValid(_index)) {
		value[_index].name = name;
		return true;
	}
	return false;
}

String Sensor::getValueName(int _index) {
	if (isValueIndexValid(_index)) {
		return value[_index].name;
	}
	return String("");
}

bool Sensor::getValueEnable(int _index) {
	if (isValueIndexValid(_index)) {
		return value[_index].enabled;
	}
	return false;
}

bool Sensor::setValueEnable(int _index, bool _b) {
	if (isValueIndexValid(_index)) {
		value[_index].enabled = _b;
	}
	return false;
}

float Sensor::getValue(int _index) {
	if (isValueIndexValid(_index)) {
		return value[_index].v;
	}
	return NAN;
}

bool Sensor::setValue(int _index, float v) {
	if (isValueIndexValid(_index)) {
		value[_index].v = v;
		return true;
	}
	return false;
}

void Sensor::printValues(void) {
	for (int i = 0; i < VALUE_COUNT; i++) {
		if (value[i].enabled) {
			Serial.print("Val[");
			Serial.print(i);
			Serial.print("]=");
			Serial.println(getValue(i));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////
// Cals
///////////////////////////////////////////////////////////////////////////////////////
bool Sensor::isCalIndexValid(int _index) {
	if (_index >= 0 && _index < VALUE_COUNT) {
		return true;
	}
	Serial.println("BUG: isCalIndexValid() index out of bounds");
	return false;
}

bool Sensor::setCalName(int _index, String name) {
	if (isCalIndexValid(_index)) {
		cal[_index].name = name;
		return true;
	}
	return false;
}

String Sensor::getCalName(int _index) {
	if (isCalIndexValid(_index)) {
		return cal[_index].name;
	}
	return String("");
}

bool Sensor::getCalEnable(int _index) {
	if (isCalIndexValid(_index)) {
		return cal[_index].enabled;
	}
	return false;
}

bool Sensor::setCalEnable(int _index, bool _b) {
	if (isCalIndexValid(_index)) {
		cal[_index].enabled = _b;
	}
	return false;
}

float Sensor::getCal(int _index) {
	if (isCalIndexValid(_index)) {
		return cal[_index].v;
	}
	return NAN;
}

bool Sensor::setCal(int _index, float v) {
	if (isCalIndexValid(_index)) {
		cal[_index].v = v;
		return true;
	}
	return false;
}

void Sensor::printCals(void) {
	for (int i = 0; i < CALIB_COUNT; i++) {
		if (cal[i].enabled) {
			Serial.print("Cal[");
			Serial.print(i);
			Serial.print("]=");
			Serial.println(getCal(i));
		}
	}
}

