#include <Arduino.h>
#include "main.h"
#include "temperature.h"

void TemperatureSensor::init(sensorModule m, SensorPins& p) {
	setModule(m);
	setPins(p);
	setCalEnable(TEMP_CAL_INDEX_TEMP_SLOPE, true);
	setCalName(TEMP_CAL_INDEX_TEMP_SLOPE, "temp_slope");

	setCalEnable(TEMP_CAL_INDEX_TEMP_OFFSET, true);
	setCalName(TEMP_CAL_INDEX_TEMP_OFFSET, "temp_offset");

	setCalEnable(TEMP_CAL_INDEX_HUMIDITY_SLOPE, true);
	setCalName(TEMP_CAL_INDEX_HUMIDITY_SLOPE, "humidity_slope");

	setCalEnable(TEMP_CAL_INDEX_HUMIDITY_OFFSET, true);
	setCalName(TEMP_CAL_INDEX_HUMIDITY_OFFSET, "humidity_offset");

	setValueEnable(TEMP_VALUE_INDEX_TEMPERATURE, true);
	setValueName(TEMP_VALUE_INDEX_TEMPERATURE, "tempC");
	setValueEnable(TEMP_VALUE_INDEX_HUMIDITY, true);
	setValueName(TEMP_VALUE_INDEX_HUMIDITY, "rH%");

	//lint -e{788} Many modules are intentionally omitted from the switch. Don't complain about it.
	switch (m) {
		case sensorModule::dht11:
			dht = new DHT(static_cast<uint8_t>(p.digital), DHT11);
			delay(2000); // CONSIDER using an optimistic_yield(2000000) instead so background tasks can still run
			dht->begin();
			break;
		case sensorModule::dht22:
			dht = new DHT(static_cast<uint8_t>(p.digital), DHT22);
			delay(2000); // CONSIDER using an optimistic_yield(2000000) instead so background tasks can still run
			dht->begin();
			break;
		case sensorModule::ds18b20:
			break;
		default:
			break;  // none of these sensors are supported by this module
	}
}

bool TemperatureSensor::acquire_setup(void) {
	if (getModule() == sensorModule::ds18b20) {
		return false; // FIXME not yet implemented
	}
	else {
		//lint -e506    suppress warning about constant value boolean, i.e. using !0 to mean TRUE. This is coming from isnan().
		// Reading temperature or humidity takes about 250 milliseconds!
		if (dht) {
			dht->read_setup();
			return true;
		}
		return false;       // error: DHT object not created
	}
}

bool TemperatureSensor::acquire1(void) {
	if (getModule() == sensorModule::ds18b20) {
		return false; // FIXME not yet implemented
	}
	else {
		//lint -e506    suppress warning about constant value boolean, i.e. using !0 to mean TRUE. This is coming from isnan().
		// Reading temperature or humidity takes about 250 milliseconds!
		if (dht) {
			float t = dht->readTemperature(true);
			yield();

			if (isnan(t)) {
				return false;   // error: read failed
			}

			// raw, non-corrected, values
			setRawTemperature(t);

			t = t * getCal(TEMP_CAL_INDEX_TEMP_SLOPE) + getCal(TEMP_CAL_INDEX_TEMP_OFFSET);

			// Calibration corrected values
			setTemperature(t);
			return true;
		}
		return false;       // error: DHT object not created
	}
}

bool TemperatureSensor::acquire2(void) {
	if (getModule() == sensorModule::ds18b20) {
		return false; // ds18b20 only has one sensor
	}
	else {
		//lint -e506    suppress warning about constant value boolean, i.e. using !0 to mean TRUE. This is coming from isnan().
		// Reading temperature or humidity takes about 250 milliseconds!
		if (dht) {
			float h = dht->readHumidity();
			yield();

			if (isnan(h)) {
				return false;   // error: read failed
			}

			// raw, non-corrected, values
			setRawHumidity(h);

			h = h * getCal(TEMP_CAL_INDEX_HUMIDITY_SLOPE) + getCal(TEMP_CAL_INDEX_HUMIDITY_OFFSET);

			// Calibration corrected values
			setHumidity(h);
			return true;
		}
		return false;       // error: DHT object not created
	}
}
