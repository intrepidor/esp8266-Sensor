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

enum class sensor_technology {
	undefined, dht11, dht22, ds18b22
};

class TemperatureSensor: public Sensor {
private:
	float humidity;
	float temperature;
	float cal_offset;
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
	void readCalibrationData(void);
	bool writeCalibrationData(void); // returns TRUE for no error, FALSE otherwise
	void printCalibrationData(void);
	bool TempRead(void);

	float getHumidity() const { // FIXME move to Sensor class
		return humidity;
	}

	float getTemperature() const { // FIXME move to Sensor class
		return temperature;
	}

	float getCalOffset() const { // FIXME move to Sensor class
		return cal_offset;
	}

	void setCalOffset(float calOffset) { // FIXME move to Sensor class
		cal_offset = calOffset;
	}
};

#endif /* TEMPERATURE_H_ */
