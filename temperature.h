/*
 * temperature.h
 *
 *  Created on: Jan 21, 2016
 *      Author: Allan Inda
 */

#ifndef TEMPERATURE_H_
#define TEMPERATURE_H_

#include <DHT.h>

enum class sensor_technology {
	undefined, dht11, dht22, ds18b22
};

class TemperatureSensor {
private:
	float humidity;
	float temperature;
	float cal_offset;
	uint8_t pin;    // Use standard Arduino pin names
	sensor_technology type;
	DHT* dht;
	bool readDHT(void);
	bool readDS18b22(void);

public:
	TemperatureSensor()
			: humidity(0), temperature(0), cal_offset(0), pin(0),
					type(sensor_technology::undefined) {
		dht = nullptr;
	}
	void Init(sensor_technology _type, uint8_t _pin);
	void readCalibrationData(void);
	bool writeCalibrationData(void); // returns TRUE for no error, FALSE otherwise
	void printCalibrationData(void);
	bool read(void);
	const char* getstrType(void);

	float getHumidity() const {
		return humidity;
	}

	uint8_t getPin() const {
		return pin;
	}

	float getTemperature() const {
		return temperature;
	}

	sensor_technology getType() const {
		return type;
	}

	float getCalOffset() const {
		return cal_offset;
	}

	void setCalOffset(float calOffset) {
		cal_offset = calOffset;
	}
};

#endif /* TEMPERATURE_H_ */
