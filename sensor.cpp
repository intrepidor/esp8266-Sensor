/*
 * sensor.cpp
 *
 *  Created on: May 25, 2016
 *      Author: allan
 */

#include <Arduino.h>
#include "main.h"
#include "sensor.h"

///////////////////////////////////////////////////////////////////////////
//
// List of supported sensor modules
//
///////////////////////////////////////////////////////////////////////////

//lint -e{26,785} suppress since lint doesn't understand C++11
t_sensor const sensorList[static_cast<int>(sensorModule::END)] = {
//
		{ "off", sensorModule::off },
		// One Digital IO
		// ... Temperature
		{ "DHT11", sensorModule::dht11 },
		{ "DHT22", sensorModule::dht22 },
		{ "DS18b20", sensorModule::ds18b20 },
		// ... Misc
		{ "Sonar", sensorModule::sonar },
		{ "Sound", sensorModule::sound },
		{ "Reed", sensorModule::reed },
		// ... PIR
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

///////////////////////////////////////////////////////////////////////////
//
// Class-less supporting functions for Sensor Class
//
///////////////////////////////////////////////////////////////////////////
int getValueCount(void) {
	return VALUE_COUNT;
}
int getCalCount(void) {
	return CALIB_COUNT;
}

String c_getModuleName(sensorModule sm) {
// FIXME -- this should be a lookup into the sensor[] array, not a repeat of the module names
	switch (sm) {
		// 1 digital
		case sensorModule::dht11:
			return String("DHT11");
			break;
		case sensorModule::dht22:
			return String("DHT22");
			break;
		case sensorModule::ds18b20:
			return String("DS18b20");
			break;
		case sensorModule::sonar:
			return String("Sonar");
			break;
		case sensorModule::sound:
			return String("Sound");
			break;
		case sensorModule::reed:
			return String("Reed");
			break;
		case sensorModule::hcs501:
			return String("HCS501");
			break;
		case sensorModule::hcsr505:
			return String("HCSR505");
			break;
			// 1 digital + 1 analog
		case sensorModule::dust:
			return String("Dust");
			break;
		case sensorModule::rain:
			return String("Rain");
			break;
		case sensorModule::soil:
			return String("Soil");
			break;
		case sensorModule::soundh:
			return String("SoundH");
			break;
		case sensorModule::methane:
			return String("Methane");
			break;
			// I2C
		case sensorModule::gy68:
			return String("GY68");
			break;
		case sensorModule::gy30:
			return String("GY30");
			break;
		case sensorModule::lcd1602:
			return String("LCD1602");
			break;
			// serial
		case sensorModule::rfid:
			return String("RFID");
			break;
		case sensorModule::marquee:
			return String("Marquee");
			break;
			// None
		case sensorModule::off:
			return String("OFF");
			break;
		default:
			return String("unknown");
			break;
	}
	return "unknown";
}

///////////////////////////////////////////////////////////////////////////
//
// Sensor Class
//
///////////////////////////////////////////////////////////////////////////

String Sensor::getModuleName(void) {
	return c_getModuleName(this->getModule());
}

//--------------------------------
// Values
//--------------------------------
bool Sensor::isValueIndexValid(int _index) {
	if (_index >= 0 && _index < getValueCount()) {
		return true;
	}
	debug.println(DebugLevel::ERROR, "ERROR: isValueIndexValid() index out of bounds");
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
	debug.print(DebugLevel::ALWAYS, "Sensor: ");
	if (getName().length() > 0) {
		debug.print(DebugLevel::ALWAYS, getName());
	}
	debug.println(DebugLevel::ALWAYS, "");
	for (int i = 0; i < getValueCount(); i++) {
		if (value[i].enabled) {
			debug.print(DebugLevel::ALWAYS, "   Val[");
			debug.print(DebugLevel::ALWAYS, i);
			debug.print(DebugLevel::ALWAYS, ",");
			debug.print(DebugLevel::ALWAYS, value[i].name);
			debug.print(DebugLevel::ALWAYS, "]=");
			debug.println(DebugLevel::ALWAYS, getValue(i));
		}
	}
	debug.println(DebugLevel::ALWAYS, "");
}

//--------------------------------
// Cals
//--------------------------------
bool Sensor::isCalIndexValid(int _index) {
	if (_index >= 0 && _index < getCalCount()) {
		return true;
	}
	debug.println(DebugLevel::ERROR, "ERROR: isCalIndexValid() index out of bounds");
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
	debug.print(DebugLevel::ALWAYS, "Sensor: ");
	if (getName().length() > 0) {
		debug.print(DebugLevel::ALWAYS, getName());
	}
	debug.println(DebugLevel::ALWAYS, "");
	for (int i = 0; i < getCalCount(); i++) {
		if (cal[i].enabled) {
			debug.print(DebugLevel::ALWAYS, "   Cal[");
			debug.print(DebugLevel::ALWAYS, i);
			debug.print(DebugLevel::ALWAYS, ",");
			debug.print(DebugLevel::ALWAYS, cal[i].name);
			debug.print(DebugLevel::ALWAYS, "]=");
			debug.println(DebugLevel::ALWAYS, getCal(i));
		}
	}
	debug.println(DebugLevel::ALWAYS, "");
}

