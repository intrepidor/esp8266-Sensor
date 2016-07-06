#include <Arduino.h>
//#include <Base64.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "main.h"
#include "util.h"
#include "ow_util.h"
#include "temperature.h"

void TemperatureSensor::init(sensorModule m, SensorPins& p) {
	/* Each temperature sensor has a maximum of two channels. The first channel
	 * is always Temperature. The second channel may or may not be used. For
	 * DHT11/22 devices, the second channel is humidity.
	 */
	setModule(m);
	setPins(p);

	//  Configure Channel 1 -- this is always temperature
	setCalEnable(TEMP_CAL_INDEX_TEMP_SLOPE, true);
	setCalName(TEMP_CAL_INDEX_TEMP_SLOPE, "temp_slope");
	setCalEnable(TEMP_CAL_INDEX_TEMP_OFFSET, true);
	setCalName(TEMP_CAL_INDEX_TEMP_OFFSET, "temp_offset");
	setValueEnable(TEMP_VALUE_INDEX_TEMPERATURE, true);
	setValueName(TEMP_VALUE_INDEX_TEMPERATURE, "tempC");

	// Configure Channel 2 -- this may or may not be used for all sensors
	//lint -e{788} Many modules are intentionally omitted from the switch. Don't complain about it.
	switch (m) {
		case sensorModule::dht11:
		case sensorModule::dht22:
			setCalEnable(TEMP_CAL_INDEX_HUMIDITY_SLOPE, true);
			setCalName(TEMP_CAL_INDEX_HUMIDITY_SLOPE, "humidity_slope");
			setCalEnable(TEMP_CAL_INDEX_HUMIDITY_OFFSET, true);
			setCalName(TEMP_CAL_INDEX_HUMIDITY_OFFSET, "humidity_offset");
			setValueEnable(TEMP_VALUE_INDEX_HUMIDITY, true);
			setValueName(TEMP_VALUE_INDEX_HUMIDITY, "rH%");
			break;
		default:
			setCalEnable(TEMP_CAL_INDEX_HUMIDITY_SLOPE, false);
			setCalName(TEMP_CAL_INDEX_HUMIDITY_SLOPE, "not used");
			setCalEnable(TEMP_CAL_INDEX_HUMIDITY_OFFSET, false);
			setCalName(TEMP_CAL_INDEX_HUMIDITY_OFFSET, "not used");
			setValueEnable(TEMP_VALUE_INDEX_HUMIDITY, false);
			setValueName(TEMP_VALUE_INDEX_HUMIDITY, "not used");
			break;
	}

	// Th pins used to interact with the sensor
	digital_pin = static_cast<uint8_t>(p.digital);

	// Create and initialize the sensor objects
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

String TemperatureSensor::getsInfo(String eol) {
	OneWireAddress addr;
	String s("Class: TemperatureSensor" + eol);

	s += ".dht: " + String((unsigned long) dht) + eol;
	s += ".ow: " + String((unsigned long) ow) + eol;
	s += ".dallas: " + String((unsigned long) dallas) + eol;
	s += ".digital_pin: " + String(digital_pin) + " " + GPIO2Arduino(digital_pin) + eol;
	s += ".started: ";
	if (started) {
		s += "true";
	}
	else {
		s += "false";
	}
	s += eol;
	if (started && dallas && ow) {
		uint8_t devicecount = dallas->getDeviceCount();
		s += ".dallas.getDeviceCount: " + String(devicecount) + eol;
		if (devicecount > 0) {
			s += ".dallas.getResolution: " + String(dallas->getResolution()) + eol;
			s += ".dallas.isParasitePowerMode: ";
			s += dallas->isParasitePowerMode();
			s += eol;
			uint8_t search_result = 1;
			s += "Searching for devices ..." + eol;
			ow->reset_search();
			while (search_result) {
				memset(addr.all, 0, sizeof(addr.all));
				ow->reset();
				search_result = ow->search(addr.all);
				if (search_result > 0) {
					s += "  Found device: ";
					s += memoryToHex((char*) addr.all, sizeof(addr.all), HexDirection::REVERSE);
					s += ", " + OWFamilyCode2PartName(addr.p.family_code) + ", "
							+ OWFamilyCode2Description(addr.p.family_code) + eol;
				}
			}
		}
	}
	return s;
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
			started = true;
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
		if (dht && started) {
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
		if (dht && started) {
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
