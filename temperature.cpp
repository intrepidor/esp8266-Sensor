#include <Arduino.h>
//#include <Base64.h>
#include <OneWire.h>
#include <DallasTemperature.h>
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
	digital_pin = static_cast<uint8_t>(p.digital);
	switch (m) {
		case sensorModule::dht11:
			dht = new DHT(digital_pin, DHT11);
			delay(2000); // CONSIDER using an optimistic_yield(2000000) instead so background tasks can still run
			dht->begin();
			break;
		case sensorModule::dht22:
			dht = new DHT(digital_pin, DHT22);
			delay(2000); // CONSIDER using an optimistic_yield(2000000) instead so background tasks can still run
			dht->begin();
			break;
		case sensorModule::ds18b20:
			ow = new OneWire(digital_pin);	// specify pin on creation
			dallas = new DallasTemperature(ow);
			break;
		default:
			break;  // none of these sensors are supported by this module
	}
}

bool TemperatureSensor::acquire_setup(void) {
	if (getModule() == sensorModule::ds18b20) {
		pinMode(digital_pin, INPUT_PULLUP); // set pullup for onewire bus port
		if (dallas) {
			if (!started) {
				dallas->begin();
				started = true;
				return true;
			}
			return acquire1();
		}
		return false;
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
		if (dallas && started) {
			// call sensors.requestTemperatures() to issue a global temperature
			// request to all devices on the bus
			dallas->requestTemperatures();			// Send the command to get temperatures
			float t = dallas->getTempCByIndex(0);
			if (isnan(t)) {
				setTemperature (FP_NAN);
				setRawTemperature(FP_NAN);
				return false;   // error: read failed
			}

			// raw, non-corrected, values
			setRawTemperature(t);

			t = t * getCal(TEMP_CAL_INDEX_TEMP_SLOPE) + getCal(TEMP_CAL_INDEX_TEMP_OFFSET);

			// Calibration corrected values
			setTemperature(t);
			return true;
		}
		else {
			return false;
		}
	}
	else {
		//lint -e506    suppress warning about constant value boolean, i.e. using !0 to mean TRUE. This is coming from isnan().
		// Reading temperature or humidity takes about 250 milliseconds!
		if (dht) {
			float t = dht->readTemperature(true);
			yield();

			if (isnan(t)) {
				setTemperature (FP_NAN);
				setRawTemperature(FP_NAN);
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
		return acquire1();
	}
	else {
		//lint -e506    suppress warning about constant value boolean, i.e. using !0 to mean TRUE. This is coming from isnan().
		// Reading temperature or humidity takes about 250 milliseconds!
		if (dht) {
			float h = dht->readHumidity();
			yield();

			if (isnan(h)) {
				setRawHumidity (FP_NAN);
				setHumidity(FP_NAN);
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
