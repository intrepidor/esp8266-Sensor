/*
 * temperature.h
 *
 *  Created on: Jan 21, 2016
 *      Author: Allan Inda
 */

#ifndef TEMPERATURE_H_
#define TEMPERATURE_H_

#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHTa.h"
#include "sensor.h"

const int TEMP_VALUE_INDEX_TEMPERATURE = 0;
const int TEMP_VALUE_INDEX_HUMIDITY = 1;

const int TEMP_CAL_INDEX_TEMP_SLOPE = 0;
const int TEMP_CAL_INDEX_TEMP_OFFSET = 1;
const int TEMP_CAL_INDEX_HUMIDITY_SLOPE = 2;
const int TEMP_CAL_INDEX_HUMIDITY_OFFSET = 3;

class TemperatureSensor: public Sensor {
private:
	DHT* dht;
	OneWire* ow;
	DallasTemperature* dallas;
	uint8_t digital_pin;
	uint8_t sda_pin;
	uint8_t scl_pin;
	bool started;
	int sensorPortNumber;
public:
	~TemperatureSensor() {
		dht = nullptr;
		ow = nullptr;
		dallas = nullptr;
		sensorPortNumber = 255;
		digital_pin = 255;
		sda_pin = 255;
		scl_pin = 255;
		started = false;
	}

	TemperatureSensor() {
		dht = nullptr;
		ow = nullptr;
		dallas = nullptr;
		sensorPortNumber = 255;
		digital_pin = 255;
		sda_pin = 255;
		scl_pin = 255;
		started = false;
	}

	// Interface Functions (virtual in Sensor class)
	void init(sensorModule, SensorPins&);
	bool acquire_setup(void);
	bool acquire1(void);
	bool acquire2(void);
	String getsInfo(String eol);

	// getters and setters
	float getHumidity() {
		float v = getValue(TEMP_VALUE_INDEX_HUMIDITY);
		return v;
	}

	float getTemperature() {
		float v = getValue(TEMP_VALUE_INDEX_TEMPERATURE);
		return v;
	}

	void setHumidity(float v) {
		setValue(TEMP_VALUE_INDEX_HUMIDITY, v);
	}

	void setTemperature(float v) {
		setValue(TEMP_VALUE_INDEX_TEMPERATURE, v);

	}

	void setRawHumidity(float v) {
		setRawValue(TEMP_VALUE_INDEX_HUMIDITY, v);
	}

	void setRawTemperature(float v) {
		setRawValue(TEMP_VALUE_INDEX_TEMPERATURE, v);

	}

	float getCalOffset() {
		return getCal(TEMP_CAL_INDEX_TEMP_OFFSET);
	}
};

#endif /* TEMPERATURE_H_ */
