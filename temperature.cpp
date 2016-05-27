#include <Arduino.h>
//#include <EEPROM.h>
#include "temperature.h"

void TemperatureSensor::init(sensorModule m, SensorPins& p) {
	this->setModule(m);
	this->setPins(p);
	switch (m) {
		case sensorModule::dht11:
			dht = new DHT(static_cast<uint8_t>(p.digital), DHT11);
			delay(2000);
			dht->begin();
			break;
		case sensorModule::dht22:
			dht = new DHT(static_cast<uint8_t>(p.digital), DHT22);
			delay(2000);
			dht->begin();
			break;
		case sensorModule::ds18b20:
			break;
		default:
			break;
	}
	readCalibrationData();
}

void TemperatureSensor::acquire(void) {
	if (this->getModule() == sensorModule::ds18b20) {
		readDS18b20();
	}
	else {
		readDHT();
	}
}

void TemperatureSensor::readCalibrationData(void) {
//    signed char cal = static_cast<signed char>(EEPROM.read(pin + eeprom_byte_offset));
//    cal_offset = (float) (cal / 10.0);
}
void TemperatureSensor::printCalibrationData(void) {
	Serial.print("Temp Offset: ");
	Serial.print(String(getCal(CAL_INDEX_OFFSET)));
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

bool TemperatureSensor::TempRead(void) {
	switch (getModule()) {
		case sensorModule::dht11:
		case sensorModule::dht22:
			return readDHT();
			break;
		case sensorModule::ds18b20:
			return readDS18b20();
			break;
		default:
			return false;
			break;
	}
	return false;
}

bool TemperatureSensor::readDS18b20(void) {
	return false;
}

bool TemperatureSensor::readDHT(void) {
//lint -e506    suppress warning about constant value boolean, i.e. using !0 to mean TRUE. This is coming from isnan().
// Reading temperature or humidity takes about 250 milliseconds!
	if (dht) {
		float h = dht->readHumidity();
		float t = dht->readTemperature(true) + getCal(CAL_INDEX_OFFSET);
		setTemperature(t);
		setHumidity(h);
		if (isnan(h) || isnan(t)) {
			return false;   // read failed
		}
	}
	return false;       // error: DHT object not created
}

