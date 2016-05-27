/*
 * temperature.h
 *
 *  Created on: Jan 21, 2016
 *      Author: Allan Inda
 */

#ifndef TEMPERATURE_H_
#define TEMPERATURE_H_

#include <DHT.h>
#include "sensor.h"

const int TEMP_VALUE_INDEX_TEMPERATURE = 0;
const int TEMP_VALUE_INDEX_HUMIDITY = 1;
const int TEMP_CAL_INDEX_OFFSET = 0;
const int TEMP_CAL_INDEX_UNUSED1 = 1;
const int TEMP_CAL_INDEX_UNUSED2 = 2;

class TemperatureSensor: public Sensor {
private:
	DHT* dht;
public:
	~TemperatureSensor() {
		dht = nullptr;
	}
	TemperatureSensor() {
		dht = nullptr;
	}

	// Interface Functions (virtual in Sensor class)
	void init(sensorModule, SensorPins&);
	bool acquire(void);

	// getters and setters
	float getHumidity() {
		float v = getValue(TEMP_VALUE_INDEX_HUMIDITY);
		//Serial.print("getHumidity()=");
		//Serial.println(v);
		return v;
	}

	float getTemperature() {
		float v = getValue(TEMP_VALUE_INDEX_TEMPERATURE);
		//Serial.print("getTemperature()=");
		//Serial.println(v);
		return v;
	}

	void setHumidity(float v) {
		setValue(TEMP_VALUE_INDEX_HUMIDITY, v);
		//Serial.print("setHumidity()=");
		//Serial.println(v);
	}

	void setTemperature(float v) {
		setValue(TEMP_VALUE_INDEX_TEMPERATURE, v);
		//Serial.print("setTemperature()=");
		//Serial.println(v);
	}

	float getCalOffset() {
		return getCal(TEMP_CAL_INDEX_OFFSET);
	}
};

#endif /* TEMPERATURE_H_ */
