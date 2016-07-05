#include <Arduino.h>
//#include <Base64.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "main.h"
#include "util.h"
#include "ow_util.h"
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

	digital_pin = static_cast<uint8_t>(p.digital);

	//lint -e{788} Many modules are intentionally omitted from the switch. Don't complain about it.
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

bool TemperatureSensor::printInfo(void) {
	OneWireAddress addr;

	debug.println(DebugLevel::ALWAYS, ".dht: " + String((unsigned long) dht));
	debug.println(DebugLevel::ALWAYS, ".ow: " + String((unsigned long) ow));
	debug.println(DebugLevel::ALWAYS, ".dallas: " + String((unsigned long) dallas));
	debug.println(DebugLevel::ALWAYS,
			".digital_pin: " + String(digital_pin) + " " + GPIO2Arduino(digital_pin));
	debug.print(DebugLevel::ALWAYS, ".started: ");
	debug.println(DebugLevel::ALWAYS, started);
	if (started && dallas && ow) {
		debug.println(DebugLevel::ALWAYS, ".dallas->getDeviceCount: " + String(dallas->getDeviceCount()));
		debug.println(DebugLevel::ALWAYS, ".dallas->getResolution: " + String(dallas->getResolution()));
		debug.print(DebugLevel::ALWAYS, ".dallas->isParasitePowerMode: ");
		debug.println(DebugLevel::ALWAYS, dallas->isParasitePowerMode());
		uint8_t search_result = 1;
		debug.println(DebugLevel::ALWAYS, "Searching for devices ...");
		ow->reset_search();
		while (search_result) {
			memset(addr.all, 0, sizeof(addr.all));
			ow->reset();
			search_result = ow->search(addr.all);
			if (search_result > 0) {
				debug.print(DebugLevel::ALWAYS, "  Found device: ");
				debug.printhex(DebugLevel::ALWAYS, (char*) addr.all, sizeof(addr.all), HexDirection::REVERSE);
				debug.println(DebugLevel::ALWAYS,
						" " + OWFamilyCode2PartName(addr.p.family_code) + " "
								+ OWFamilyCode2Description(addr.p.family_code));
			}
		}
	}
	return true;
}

bool TemperatureSensor::acquire_setup(void) {
	if (getModule() == sensorModule::ds18b20) {
		debug.println(DebugLevel::DEBUGMORE, String(millis()) + ", setup() " + String(digital_pin));
		pinMode(digital_pin, INPUT);
		if (dallas) {
			if (!started) {
				dallas->begin();
				// do not while-loop waiting for the conversion to finish. Tell the sensor to acquire
				//    the temperature in acquire1, then when acquire2 runs, read that temperature. The
				//    only requirement is that the time between acquire1 and acquire2 must be long enough,
				//    which is 94/188/375/750ms for 9/10/11/12-bit resolution. Make sure the task
				//    scheduler interleaves the sensors so there is enough delay.
				dallas->setWaitForConversion(false);
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
			debug.println(DebugLevel::DEBUGMORE, String(millis()) + ", acquire1() " + String(digital_pin));
			// call sensors.requestTemperatures() to issue a global temperature
			// request to all devices on the bus
			dallas->requestTemperatures();			// Send the command to get temperatures
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
		if (dallas && started) {
			debug.println(DebugLevel::DEBUGMORE, String(millis()) + ", acquire2() " + String(digital_pin));
			float t = dallas->getTempCByIndex(0);
			if (t == DEVICE_DISCONNECTED_C || t < -55 || t > 125) { // device limits are -55C to +125C
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
