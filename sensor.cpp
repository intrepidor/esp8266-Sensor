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
bool Sensor::isValueChannelValid(int _channel) {
	if (_channel >= 0 && _channel < VALUE_COUNT) { // FIXME getSensorValueCount()) {
		return true;
	}
	debug.println(DebugLevel::ERROR, nl + "ERROR: isValueChannelValid() channel out of bounds");
	return false;
}

bool Sensor::isValueStale(int _channel) {
	if (isValueChannelValid(_channel)) {
		if ((value[_channel].last_sample_time_ms + time_till_stale_ms) > millis()) {
			return false;
		}
	}
	return true;
}

bool Sensor::setValueName(int _channel, String name) {
	if (isValueChannelValid(_channel)) {
		value[_channel].name = name;
		return true;
	}
	return false;
}

String Sensor::getValueName(int _channel) {
	if (isValueChannelValid(_channel)) {
		return value[_channel].name;
	}
	return String("");
}

bool Sensor::getValueEnable(int _channel) {
	if (isValueChannelValid(_channel)) {
		return value[_channel].enabled;
	}
	return false;
}

bool Sensor::setValueEnable(int _channel, bool _b) {
	if (isValueChannelValid(_channel)) {
		value[_channel].enabled = _b;
	}
	return false;
}

float Sensor::getValue(int _channel) {
	if (isValueChannelValid(_channel) && !isValueStale(_channel)) {
		return value[_channel].v;
	}
	return NAN;
}

bool Sensor::setValue(int _channel, float v) {
	if (isValueChannelValid(_channel)) {
		value[_channel].v = v;
		value[_channel].last_sample_time_ms = millis();
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////

float Sensor::getRawValue(int _channel) {
	if (isValueChannelValid(_channel) && !isRawValueStale(_channel)) {
		return rawval[_channel].v;
	}
	return NAN;
}

bool Sensor::isRawValueStale(int _channel) {
	if (isValueChannelValid(_channel)) {
		if ((rawval[_channel].last_sample_time_ms + time_till_stale_ms) > millis()) {
			return false;
		}
	}
	return true;
}

bool Sensor::setRawValue(int _channel, float v) {
	if (isValueChannelValid(_channel)) {
		rawval[_channel].v = v;
		rawval[_channel].last_sample_time_ms = millis();
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
	for (int i = 0; i < getSensorValueCount(); i++) {
		if (value[i].enabled) {
			debug.print(DebugLevel::ALWAYS, "   Val[");
			debug.print(DebugLevel::ALWAYS, i);
			debug.print(DebugLevel::ALWAYS, ",");
			debug.print(DebugLevel::ALWAYS, value[i].name);
			debug.print(DebugLevel::ALWAYS, "]=");
			debug.print(DebugLevel::ALWAYS, getValue(i));

			debug.print(DebugLevel::ALWAYS, ", Age=");
			debug.println(DebugLevel::ALWAYS, getAge(i));

			debug.print(DebugLevel::ALWAYS, "   Raw[");
			debug.print(DebugLevel::ALWAYS, i);
			debug.print(DebugLevel::ALWAYS, ",");
			debug.print(DebugLevel::ALWAYS, value[i].name);
			debug.print(DebugLevel::ALWAYS, "]=");
			debug.print(DebugLevel::ALWAYS, getRawValue(i));
			debug.print(DebugLevel::ALWAYS, ", Age=");
			debug.println(DebugLevel::ALWAYS, getRawAge(i));

		}
	}
	debug.println(DebugLevel::ALWAYS, "");
}

//--------------------------------
// Cals
//--------------------------------
bool Sensor::isCalChannelValid(int _channel) {
	if (_channel >= 0 && _channel < getSensorCalCount()) {
		return true;
	}
	debug.println(DebugLevel::ERROR, nl + "ERROR: isCalChannelValid() channel out of bounds");
	return false;
}

bool Sensor::setCalName(int _channel, String name) {
	if (isCalChannelValid(_channel)) {
		cal[_channel].name = name;
		return true;
	}
	return false;
}

String Sensor::getCalName(int _channel) {
	String s("");
	if (isCalChannelValid(_channel)) {
		s = cal[_channel].name;
	}
	return s;
}

bool Sensor::getCalEnable(int _channel) {
	if (isCalChannelValid(_channel)) {
		return cal[_channel].enabled;
	}
	return false;
}

bool Sensor::setCalEnable(int _channel, bool _b) {
	if (isCalChannelValid(_channel)) {
		cal[_channel].enabled = _b;
	}
	return false;
}

float Sensor::getCal(int _channel) {
	if (isCalChannelValid(_channel)) {
		return cal[_channel].v;
	}
	return NAN;
}

bool Sensor::setCal(int _channel, float v) {
	if (isCalChannelValid(_channel)) {
		cal[_channel].v = v;
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
	for (int i = 0; i < getSensorCalCount(); i++) {
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

