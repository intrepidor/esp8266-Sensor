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
// SensorValue Class
//
///////////////////////////////////////////////////////////////////////////
float SensorValue::getValue() {
	if (type == valueType::temperature && dinfo.isFahrenheit()) {
		return v * 1.8F + 32.0F;
	}
	return v;
}

///////////////////////////////////////////////////////////////////////////
//
// Sensor Class
//
///////////////////////////////////////////////////////////////////////////

bool Sensor::acquire(void) {
	unsigned long _now = millis();
	unsigned long _then = _now;
	unsigned long _dur = _now;
	unsigned long lasttime = 0;
	if (isSensorActive()) {
		switch (getNextSubTask()) {
			case 0:
				DEBUGPRINT(DebugLevel::TIMINGS, F("]->acquire_setup START"));
				if ((_now - last_acquiresetup_timestamp_ms) >= minimum_time_between_acquiresetup_ms) {
					lasttime = last_acquiresetup_timestamp_ms;
					last_acquiresetup_timestamp_ms = _now;
					acquire_setup();
					incNextSubtask();
					_then = millis();
					_dur = _then - _now;
					DEBUGPRINT(DebugLevel::TIMINGS, F("\t\tct="));
					DEBUGPRINT(DebugLevel::TIMINGS, (_now - lasttime));
					DEBUGPRINT(DebugLevel::TIMINGS, "/");
					DEBUGPRINTLN(DebugLevel::TIMINGS, minimum_time_between_acquiresetup_ms);
				}
				else {
					_then = millis();
					_dur = _then - _now;
					DEBUGPRINTLN(DebugLevel::TIMINGS, "\tdelayed");
				}
				DEBUGPRINT(DebugLevel::TIMINGS, String(_then) + " sensors[" + String(sensorID));
				DEBUGPRINT(DebugLevel::TIMINGS, F("]->acquire_setup DONE \t\t\tdur="));
				DEBUGPRINTLN(DebugLevel::TIMINGS, _dur);
				break;
			case 1:
				DEBUGPRINT(DebugLevel::TIMINGS, F("]->acquire1      START"));
				if ((_now - last_acquiresetup_timestamp_ms) >= minimum_wait_time_after_acquiresetup_ms) {
					lasttime = last_acquiresetup_timestamp_ms;
					last_acquire1_timestamp_ms = _now;
					acquire1();
					incNextSubtask();
					_then = millis();
					_dur = _then - _now;
					DEBUGPRINT(DebugLevel::TIMINGS, F("\t\tct="));
					DEBUGPRINT(DebugLevel::TIMINGS, (_now - lasttime));
					DEBUGPRINT(DebugLevel::TIMINGS, "/");
					DEBUGPRINTLN(DebugLevel::TIMINGS, minimum_wait_time_after_acquiresetup_ms);
				}
				else {
					_then = millis();
					_dur = _then - _now;
					DEBUGPRINTLN(DebugLevel::TIMINGS, "\tdelayed");
				}
				DEBUGPRINT(DebugLevel::TIMINGS, String(_then) + " sensors[" + String(sensorID));
				DEBUGPRINT(DebugLevel::TIMINGS, F("]->acquire1      DONE \t\t\tdur="));
				DEBUGPRINTLN(DebugLevel::TIMINGS, _dur);
				break;
			case 2:
				DEBUGPRINT(DebugLevel::TIMINGS, F("]->acquire2      START"));
				if ((_now - last_acquire1_timestamp_ms) >= minimum_wait_time_after_acquire1_ms) {
					lasttime = last_acquire1_timestamp_ms;
					last_acquire2_timestamp_ms = _now;
					acquire2();
					incNextSubtask();
					_then = millis();
					_dur = _then - _now;
					DEBUGPRINT(DebugLevel::TIMINGS, F("\t\tct="));
					DEBUGPRINT(DebugLevel::TIMINGS, (_now - lasttime));
					DEBUGPRINT(DebugLevel::TIMINGS, "/");
					DEBUGPRINTLN(DebugLevel::TIMINGS, minimum_wait_time_after_acquire1_ms);
				}
				else {
					_then = millis();
					_dur = _then - _now;
					DEBUGPRINTLN(DebugLevel::TIMINGS, "\tdelayed");
				}
				DEBUGPRINT(DebugLevel::TIMINGS, String(_then) + " sensors[" + String(sensorID));
				DEBUGPRINT(DebugLevel::TIMINGS, F("]->acquire2      DONE \t\t\tdur="));
				DEBUGPRINTLN(DebugLevel::TIMINGS, _dur);
				break;
			default:
				break;
		}
	}
	return true;
}

void Sensor::setType(int _channel, valueType t) {
	if (isValueChannelValid(_channel)) {
		value[_channel].setType(t);
		rawval[_channel].setType(t);
	}
}

valueType Sensor::getType(int _channel) {
	if (isValueChannelValid(_channel)) {
		return value[_channel].getType();
	}
	else {
		return valueType::undefined;
	}
}

void Sensor::setUOM(int _channel, uomType u) {
	if (isValueChannelValid(_channel)) {
		value[_channel].setUOM(u);
		rawval[_channel].setUOM(u);
	}
}

uomType Sensor::getUOM(int _channel) {
	if (isValueChannelValid(_channel)) {
		return value[_channel].getUOM();
	}
	else {
		return uomType::undefined;
	}
}

String Sensor::getModuleName(void) {
	return getSensorModuleName(this->getModule());
}

//--------------------------------
// Values
//--------------------------------
bool Sensor::isValueChannelValid(int _channel) {
	if (_channel >= 0 && _channel < SENSOR_VALUE_COUNT) { // FIXME getSensorValueCount()) {
		return true;
	}
	DEBUGPRINTLN(DebugLevel::ERROR, F("ERROR: isValueChannelValid() channel out of bounds"));
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
		value[_channel].vname = name;
		return true;
	}
	return false;
}

String Sensor::getValueName(int _channel) {
	if (isValueChannelValid(_channel)) {
		return value[_channel].vname;
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
		return value[_channel].getValue();
	}
	return NAN;
}

float Sensor::getValueWithoutUnitConversion(int _channel) {
	if (isValueChannelValid(_channel) && !isValueStale(_channel)) {
		return value[_channel].getValueWithoutUnitConversion();
	}
	return NAN;
}

bool Sensor::setValue(int _channel, float v) {
	if (isValueChannelValid(_channel)) {
		value[_channel].setValue(v);
		value[_channel].last_sample_time_ms = millis();
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////

float Sensor::getRawValue(int _channel) {
	if (isValueChannelValid(_channel) && !isRawValueStale(_channel)) {
		return rawval[_channel].getValue();
	}
	return NAN;
}
float Sensor::getRawValueWithoutUnitConversion(int _channel) {
	if (isValueChannelValid(_channel) && !isRawValueStale(_channel)) {
		return rawval[_channel].getValueWithoutUnitConversion();
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
		rawval[_channel].setValue(v);
		rawval[_channel].last_sample_time_ms = millis();
		return true;
	}
	return false;
}

void Sensor::printValues(void) {
	Serial.print(F("Sensor: "));
	if (getName().length() > 0) {
		Serial.print(getName());
	}
	Serial.println("");
	for (int i = 0; i < getSensorValueCount(); i++) {
		if (value[i].enabled) {
			Serial.print(F("   Val["));
			Serial.print(i);
			Serial.print(F(","));
			Serial.print(value[i].vname);
			Serial.print(F("]="));
			Serial.print(getValue(i));

			Serial.print(F(", Age="));
			Serial.println(getAge(i));

			Serial.print(F("   Raw["));
			Serial.print(i);
			Serial.print(F(","));
			Serial.print(value[i].vname);
			Serial.print(F("]="));
			Serial.print(getRawValue(i));
			Serial.print(F(", Age="));
			Serial.println(getRawAge(i));

		}
	}
	Serial.println("");
}

//--------------------------------
// Cals
//--------------------------------
bool Sensor::isCalChannelValid(int _channel) {
	if (_channel >= 0 && _channel < getSensorCalCount()) {
		return true;
	}
	DEBUGPRINTLN(DebugLevel::ERROR, F("ERROR: isCalChannelValid() channel out of bounds"));
	return false;
}

bool Sensor::setCalName(int _channel, String name) {
	if (isCalChannelValid(_channel)) {
		cal[_channel].vname = name;
		return true;
	}
	return false;
}

String Sensor::getCalName(int _channel) {
	String s("");
	if (isCalChannelValid(_channel)) {
		s = cal[_channel].vname;
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
		return cal[_channel].getValue();
	}
	return NAN;
}

bool Sensor::setCal(int _channel, float v) {
	if (isCalChannelValid(_channel)) {
		cal[_channel].setValue(v);
		return true;
	}
	return false;
}

void Sensor::printCals(void) {
	Serial.print(F("Sensor: "));
	if (getName().length() > 0) {
		Serial.print(getName());
	}
	Serial.println("");
	for (int i = 0; i < getSensorCalCount(); i++) {
		if (cal[i].enabled) {
			Serial.print(F("   Cal["));
			Serial.print(i);
			Serial.print(F(","));
			Serial.print(cal[i].vname);
			Serial.print(F("]="));
			Serial.println(getCal(i));
		}
	}
	Serial.println("");
}

