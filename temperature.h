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

const int VALUE_INDEX_TEMPERATURE = 0;
const int VALUE_INDEX_HUMIDITY = 1;
const int CAL_INDEX_OFFSET = 0;

class TemperatureSensor: public Sensor {
private:
	DHT* dht;
	bool readDHT(void);
	bool readDS18b20(void);

public:
	~TemperatureSensor() {
		dht = nullptr;
	}
	TemperatureSensor() {
		dht = nullptr;
	}

	// Interface Functions (virtual in Sensor class)
	void init(sensorModule, SensorPins&);
	void acquire(void);
	float compute(void);

	//
	void readCalibrationData(void); // FIXME move to Sensor class
	bool writeCalibrationData(void); // returns TRUE for no error, FALSE otherwise
	void printCalibrationData(void); // FIXME move to Sensor class
	bool TempRead(void); // FIXME move to Sensor class

	float getHumidity() {
		float v = getValue(VALUE_INDEX_HUMIDITY);
		//Serial.print("getHumidity()=");
		//Serial.println(v);
		return v;
	}

	float getTemperature() {
		float v = getValue(VALUE_INDEX_TEMPERATURE);
		//Serial.print("getTemperature()=");
		//Serial.println(v);
		return v;
	}

	void setHumidity(float v) {
		setValue(VALUE_INDEX_HUMIDITY, v);
		//Serial.print("setHumidity()=");
		//Serial.println(v);
	}

	void setTemperature(float v) {
		setValue(VALUE_INDEX_TEMPERATURE, v);
		//Serial.print("setTemperature()=");
		//Serial.println(v);
	}

	float getCalOffset() {
		return getCal(CAL_INDEX_OFFSET);
	}
};

#endif /* TEMPERATURE_H_ */
