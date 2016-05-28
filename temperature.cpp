#include <Arduino.h>
//#include <EEPROM.h>
#include "temperature.h"

void TemperatureSensor::init(sensorModule m, SensorPins& p) {
	setModule(m);
	setPins(p);
	setCalEnable(TEMP_CAL_INDEX_OFFSET, true);
	setCalName(TEMP_CAL_INDEX_OFFSET, "offset");
	setValueEnable(TEMP_VALUE_INDEX_TEMPERATURE, true);
	setValueName(TEMP_VALUE_INDEX_TEMPERATURE, "tempC");
	setValueEnable(TEMP_VALUE_INDEX_HUMIDITY, true);
	setValueName(TEMP_VALUE_INDEX_HUMIDITY, "rH%");

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
}

bool TemperatureSensor::acquire(void) {
	if (getModule() == sensorModule::ds18b20) {
		return false;
	}
	else {
		//lint -e506    suppress warning about constant value boolean, i.e. using !0 to mean TRUE. This is coming from isnan().
		// Reading temperature or humidity takes about 250 milliseconds!
		if (dht) {
			float h = dht->readHumidity();
			float t = dht->readTemperature(true) + getCal(TEMP_CAL_INDEX_OFFSET);
			setTemperature(t);
			setHumidity(h);
			if (isnan(h) || isnan(t)) {
				return false;   // read failed
			}
		}
		return false;       // error: DHT object not created
	}
}
