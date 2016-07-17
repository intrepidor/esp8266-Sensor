#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h> // not used, yet, but needed for SparkFunHTU21D.h
#include <SparkFunHTU21D.h>
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

	// Configure Sensor Parameters
	setStaleAge_ms(10000);	// timeout of the value if not update for 10 seconds.

	// Channel 1 is always defined like this
	setCalEnable(TEMP_CAL_INDEX_TEMP_SLOPE, true);
	setCalName(TEMP_CAL_INDEX_TEMP_SLOPE, "Temperature Gain");
	setCalEnable(TEMP_CAL_INDEX_TEMP_OFFSET, true);
	setCalName(TEMP_CAL_INDEX_TEMP_OFFSET, "Temperature Offset");
	setValueEnable(TEMP_VALUE_INDEX_TEMPERATURE, true);
	setValueName(TEMP_VALUE_INDEX_TEMPERATURE, "tempC");

	// Channel 2 defaults -- assume not used
	setCalEnable(TEMP_CAL_INDEX_HUMIDITY_SLOPE, false);
	setCalName(TEMP_CAL_INDEX_HUMIDITY_SLOPE, "not used");
	setCalEnable(TEMP_CAL_INDEX_HUMIDITY_OFFSET, false);
	setCalName(TEMP_CAL_INDEX_HUMIDITY_OFFSET, "not used");
	setValueEnable(TEMP_VALUE_INDEX_HUMIDITY, false);
	setValueName(TEMP_VALUE_INDEX_HUMIDITY, "not used");

	if (m == sensorModule::dht11 || m == sensorModule::dht22 || m == sensorModule::htu21d_si7102) {
		setCalEnable(TEMP_CAL_INDEX_HUMIDITY_SLOPE, true);
		setCalName(TEMP_CAL_INDEX_HUMIDITY_SLOPE, "Humidity Gain");
		setCalEnable(TEMP_CAL_INDEX_HUMIDITY_OFFSET, true);
		setCalName(TEMP_CAL_INDEX_HUMIDITY_OFFSET, "Humidity Offset");
		setValueEnable(TEMP_VALUE_INDEX_HUMIDITY, true);
		setValueName(TEMP_VALUE_INDEX_HUMIDITY, "rH%");
	}

	// The pins used to interact with the sensor
	digital_pin = static_cast<uint8_t>(p.digital);
	sda_pin = static_cast<uint8_t>(p.sda);
	scl_pin = static_cast<uint8_t>(p.scl);

	// Create and initialize the sensor objects
	//lint -e{788} Many modules are intentionally omitted from the switch. Don't complain about it.
	switch (m) {
		case sensorModule::dht11:
			dht = new DHT(digital_pin, DHT11);
			delay(2000); // FIXME -- there is no reason for this delay. Neither the constructor or begin do anything other that initialize variables.
			dht->begin();
			break;
		case sensorModule::dht22:
			dht = new DHT(digital_pin, DHT22);
			delay(2000); // FIXME -- there is no reason for this delay. Neither the constructor or begin do anything other that initialize variables.
			dht->begin();
			break;
		case sensorModule::ds18b20:
			ow = new OneWire(digital_pin);	// specify pin on creation
			dallas = new DallasTemperature(ow);
			// do the begin later since it takes a while
			break;
		case sensorModule::htu21d_si7102:
			htu21d = new HTU21D();
//			htu21d->begin(callbackfunction);
			htu21d->begin();
			break;
		default:
			break;  // none of these sensors are supported by this module
	}
}

void callbackfunction(void) {
//	static char c = '0';
//	// The purpose of this function is to kick the watchdog. The I2C read/write
//	//   can take a long time, so the callback lets the I2C API call back so
//	//   this function can service the watchdog.
//	kickExternalWatchdog(); // don't kick the software watchdog, just the external hardware one.
//	Serial.print(c);
//	if (++c > '9') c = '0';
}

String TemperatureSensor::getsInfo(String eol) {
	OneWireAddress addr;
	String s("Class: TemperatureSensor" + eol);

	s += ".dht: " + String((unsigned long) dht) + eol;
	s += ".ow: " + String((unsigned long) ow) + eol;
	s += ".dallas: " + String((unsigned long) dallas) + eol;
	s += ".htu21d: " + String((unsigned long) htu21d) + eol;
	s += ".digital_pin: (" + String(digital_pin) + ") " + GPIO2Arduino(digital_pin) + eol;
	s += ".sda_pin: (" + String(sda_pin) + ") " + GPIO2Arduino(sda_pin) + eol;
	s += ".scl_pin: (" + String(scl_pin) + ") " + GPIO2Arduino(scl_pin) + eol;
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
	debug.println(DebugLevel::DEBUG2, String(millis()) + ", setup() " + String(digital_pin));
	pinMode(digital_pin, INPUT);
	sensorModule m = getModule();
	if (m == sensorModule::ds18b20) {
		if (dallas) {
			if (!started) {
				dallas->begin();
				// setWaitForConversion=false means the driver will not while-loop waiting for the
				//    conversion to finish. Tell the sensor to acquire the temperature in acquire1,
				//    then when acquire2 runs, read that temperature. The only requirement is that
				//    the time between acquire1 and acquire2 must be long enough, which is
				//    94/188/375/750ms for 9/10/11/12-bit resolution. Make sure the task scheduler
				//    interleaves the sensors so there is enough delay.
				dallas->setWaitForConversion(false);
				started = true;
				return true;
			}
			return acquire1();
		}
	}
	else if (m == sensorModule::dht11 || m == sensorModule::dht22) {
		// Reading temperature or humidity takes about 250 milliseconds!
		if (dht) {
			dht->read_setup();
			started = true;
			return true;
		}
	}
	else if (m == sensorModule::htu21d_si7102) {
		if (htu21d) {
			if (!started) {
				htu21d->setResolution(USER_REGISTER_RESOLUTION_RH12_TEMP14);
				started = true;
			}
			return acquire1();	// there is no setup for this sensor
		}
	}
	return false;
}

bool TemperatureSensor::acquire1(void) {
	sensorModule m = getModule();
	if (m == sensorModule::ds18b20) {
		if (dallas && started) {
			// call sensors.requestTemperatures() to issue a global temperature
			// request to all devices on the bus. It takes a while, up to 750ms,
			// so acquire2 will read the value.
			dallas->requestTemperatures();	// tell all devices on the bus to measure the temperature
			return true;
		}
	}
	else if (m == sensorModule::dht11 || m == sensorModule::dht22) {
		//lint -e506    suppress warning about constant value boolean, i.e. using !0 to mean TRUE. This is coming from isnan().
		// Reading temperature or humidity takes about 250 milliseconds!
		if (dht && started) {
			float t = dht->readTemperature(false); // read celcius
			return StoreTemperature(t);
		}
	}
	else if (m == sensorModule::htu21d_si7102) {
		if (htu21d && started) {
			float t = htu21d->readTemperature();	// takes up to 100ms
			if (t >= 998.0F) {
				return StoreTemperature(NAN);
			}
			else {
				return StoreTemperature(t);
			}
		}
	}
	return false;
}

bool TemperatureSensor::acquire2(void) {
	sensorModule m = getModule();
	if (m == sensorModule::ds18b20) {
		if (dallas && started) {
			float t = dallas->getTempCByIndex(0);
			if (t == DEVICE_DISCONNECTED_C || t < -55.0F || t > 125.0F) { // device limits are -55C to +125C
				return StoreTemperature(NAN);
			}
			else {
				return StoreTemperature(t);
			}
		}
	}
	else if (m == sensorModule::dht11 || m == sensorModule::dht22) {
		//lint -e506    suppress warning about constant value boolean, i.e. using !0 to mean TRUE. This is coming from isnan().
		// Reading temperature or humidity takes about 250 milliseconds!
		if (dht && started) {
			float h = dht->readHumidity();
			return StoreHumidity(h);
		}
	}
	else if (m == sensorModule::htu21d_si7102) {
		if (htu21d && started) {
			float h = htu21d->readHumidity();	// takes up to 100ms
			if (h >= 998.0F) {
				return StoreHumidity(NAN);
			}
			else {
				return StoreHumidity(h);
			}
		}
	}
	return false;
}

bool TemperatureSensor::StoreTemperature(float t) {
	if (t == NAN) {
		// error reading value
		setTemperature (NAN);
		setRawTemperature(NAN);
		return false;
	}
	else {
		// raw, non-corrected, values
		setRawTemperature(t);
		// correct based on calibration values
		t = t * getCal(TEMP_CAL_INDEX_TEMP_SLOPE) + getCal(TEMP_CAL_INDEX_TEMP_OFFSET);

		// store the corrected temperature value
		setTemperature(t);
		return true;
	}
	return false;
}

bool TemperatureSensor::StoreHumidity(float h) {
	if (h == NAN) {
		setRawHumidity (NAN);
		setHumidity(NAN);
		return false;   // error: read failed
	}

	// raw, non-corrected, values
	setRawHumidity(h);

	// correct the measured value based on the calibration values
	h = h * getCal(TEMP_CAL_INDEX_HUMIDITY_SLOPE) + getCal(TEMP_CAL_INDEX_HUMIDITY_OFFSET);

	// stored the calibration corrected value
	setHumidity(h);
	return true;
}

