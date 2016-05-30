/*
 * sensor.cpp
 *
 *  Created on: May 25, 2016
 *      Author: allan
 */

#include <Arduino.h>
#include "main.h"
#include "sensor.h"
#include "sensor_support.h"

///////////////////////////////////////////////////////////////////////////
//
// Sensor Class
//
///////////////////////////////////////////////////////////////////////////

String Sensor::getModuleName(void) {
	return getModuleNameString(this->getModule());
}

//--------------------------------
// Values
//--------------------------------
bool Sensor::isValueIndexValid(int _index) {
	if (_index >= 0 && _index < getValueCount()) {
		return true;
	}
	debug.println(DebugLevel::ERROR, nl + "ERROR: isValueIndexValid() index out of bounds");
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
	debug.println(DebugLevel::ERROR, nl + "ERROR: isCalIndexValid() index out of bounds");
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

