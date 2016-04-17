#include <Arduino.h>
//#include <EEPROM.h>
#include "temperature.h"

void TemperatureSensor::Init(sensor_technology _type, char* _name, uint8_t _pin) {
	type = _type;
	name = _name;
	pin = _pin;
	switch (type) {
		case sensor_technology::dht11:
			dht = new DHT(pin, DHT11);
			delay(2000);
			dht->begin();
			break;
		case sensor_technology::dht22:
			dht = new DHT(pin, DHT22);
			delay(2000);
			dht->begin();
			break;
		case sensor_technology::ds18b22:
			break;
		case sensor_technology::undefined:
		default:
			break;
	}
	readCalibrationData();
	//printCalibrationData();
}

void TemperatureSensor::readCalibrationData(void) {
//    signed char cal = static_cast<signed char>(EEPROM.read(pin + eeprom_byte_offset));
//    cal_offset = (float) (cal / 10.0);
}
void TemperatureSensor::printCalibrationData(void) {
	Serial.print("Temp ");
	Serial.print(name);
	Serial.print(" Offset: ");
	Serial.print(String(cal_offset));
	Serial.println(" *F)");
}

bool TemperatureSensor::writeCalibrationData(void) {
//    signed char offset = static_cast<signed char>(cal_offset * 10);
//    EEPROM.write(pin + eeprom_byte_offset, static_cast<unsigned char>(offset));
//    if (EEPROM.commit()) {
//        readCalibrationData();
	return true;
//    }
//    else {
//        return false; // signal error
//    }
}

const char* TemperatureSensor::getstrType(void) {
	switch (type) {
		case sensor_technology::dht11:
			return "DHT11";
			break;
		case sensor_technology::dht22:
			return "DHT11";
			break;
		case sensor_technology::ds18b22:
			return "DS18b22";
			break;
		case sensor_technology::undefined:
		default:
			break;
	}
	return "unknown";
}

bool TemperatureSensor::read(void) {
	switch (type) {
		case sensor_technology::dht11:
		case sensor_technology::dht22:
			return readDHT();
			break;
		case sensor_technology::ds18b22:
			return readDS18b22();
			break;
		case sensor_technology::undefined:
		default:
			return false;
			break;
	}
	return false;
}

bool TemperatureSensor::readDS18b22(void) {
	return false;
}

bool TemperatureSensor::readDHT(void) {
//lint -e506    suppress warning about constant value boolean, i.e. using !0 to mean TRUE. This is coming from isnan().
	// Reading temperature or humidity takes about 250 milliseconds!
	if (dht) {
		humidity = dht->readHumidity();
		temperature = dht->readTemperature(true) + cal_offset;
		if (isnan(humidity) || isnan(temperature)) {
			return false;   // read failed
		}
		heatindex = dht->computeHeatIndex(temperature, humidity);
		return true;    // read successful
	}
	return false;       // error: DHT object not created
}

