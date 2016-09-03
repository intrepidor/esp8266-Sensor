/*
 * temperature.h
 *
 *  Created on: July 6, 2016
 *      Author: Allan Inda
 */

#ifndef GENERIC_H_
#define GENERIC_H_

#include "sensor.h"

const int TEMP_VALUE_CHANNEL_ANALOG = 0;
const int TEMP_VALUE_CHANNEL_DIGITAL = 1;

const int TEMP_CAL_CHANNEL_ANALOG_OFFSET = 0;
const int TEMP_CAL_CHANNEL_ANALOG_FIRST_ORDER = 1;
const int TEMP_CAL_CHANNEL_ANALOG_SECOND_ORDER = 2;
const int TEMP_CAL_CHANNEL_ANALOG_THIRD_ORDER = 3;

class GenericSensor: public Sensor {
private:
	uint8_t digital_pin;
	uint8_t analog_pin;
	bool started;
	int sensorPortNumber;
	unsigned int last_reading_timestamp_us;
	int reading_count;
	int generic_accumulator;
	int reading_count_within_cycle;
public:
	~GenericSensor() {
		sensorPortNumber = 255;
		digital_pin = 255;
		analog_pin = 255;
		started = false;
	}

	GenericSensor() {
		sensorPortNumber = 255;
		digital_pin = 255;
		analog_pin = 255;
		started = false;
		last_reading_timestamp_us = 0;
		reading_count = 0;
		reading_count_within_cycle = 0;
		generic_accumulator = 0;
	}

	// Interface Functions (virtual in Sensor class)
	void init(sensorModule, SensorPins&);
	bool acquire_setup(void);
	bool acquire1(void);
	bool acquire2(void);
	String getsInfo(String eol);

	// getters and setters
	float getDigital() {
		float v = getValue(TEMP_VALUE_CHANNEL_DIGITAL);
		return v;
	}

	void setDigital(float v) {
		setValue(TEMP_VALUE_CHANNEL_DIGITAL, v);
		setRawValue(TEMP_VALUE_CHANNEL_DIGITAL, v);
	}

	float getAnalog() {
		float v = getValue(TEMP_VALUE_CHANNEL_ANALOG);
		return v;
	}

	void setAnalog(float v) {
		setValue(TEMP_VALUE_CHANNEL_ANALOG, v);
	}

	void setRawAnalog(float v) {
		setRawValue(TEMP_VALUE_CHANNEL_ANALOG, v);
	}
};

#endif /* GENERIC_H_ */
