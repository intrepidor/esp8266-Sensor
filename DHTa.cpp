/* DHT library

 Revised and based on the DHT code by Adafruit Industries.

 MIT license
 written by Adafruit Industries
 */

#include "DHTa.h"

const unsigned long MIN_INTERVAL = 2000;

DHT::DHT(uint8_t pin, uint8_t type, uint8_t count) {
//lint --e{715} ignore warnings about unreferenced arguments, i.e. count
	_pin = pin;
	_type = type;
#ifdef __AVR
	_bit = digitalPinToBitMask(pin);
	_port = digitalPinToPort(pin);
#endif
	_maxcycles = microsecondsToClockCycles(1000);  // 1 millisecond timeout for
												   // reading pulses from DHT sensor.
	// Note that count is now ignored as the DHT reading algorithm adjusts itself
	// based on the speed of the processor.
	data[0] = data[1] = data[2] = data[3] = data[4] = 0;
	_lastreadtime = -MIN_INTERVAL; //lint !e501
	_lastresult = false;

}

void DHT::begin(void) {
	// set up the pins!
	pinMode(_pin, INPUT_PULLUP);
	// Using this value makes sure that millis() - lastreadtime will be
	// >= MIN_INTERVAL right away. Note that this assignment wraps around,
	// but so will the subtraction.
	_lastreadtime = -MIN_INTERVAL; //lint !e501
//	DEBUG_PRINT("Max clock cycles: ");
//	DEBUG_PRINTLN(_maxcycles, DEC);
}

//boolean S == Scale.  True == Fahrenheit; False == Celcius
float DHT::readTemperature(bool S, bool force) {
	//lint --e{715} ignore warnings about unreferenced arguments, i.e. force
	float f = NAN;

	if (read(force)) {
		switch (_type) {
			case DHT11:
				f = data[2];
				if (S) {
					f = convertCtoF(f);
				}
				break;
			case DHT22:
			case DHT21:
				f = data[2] & 0x7F;
				f *= 256.0F;
				f += data[3];
				f *= 0.1F;
				if (data[2] & 0x80) {
					f *= -1.0F;
				}
				if (S) {
					f = convertCtoF(f);
				}
				break;
			default:
				break;
		}
	}
	return f;
}

float DHT::convertCtoF(float c) {
	return c * 1.8F + 32.0F;
}

float DHT::convertFtoC(float f) {
	return (f - 32.0F) * 0.55555F;
}

float DHT::readHumidity(bool force) {
	//lint --e{715} ignore warnings about unreferenced arguments, i.e. force
	float f = NAN;
	if (read()) {
		switch (_type) {
			case DHT11:
				f = data[0];
				break;
			case DHT22:
			case DHT21:
				f = data[0];
				f *= 256.0F;
				f += data[1];
				f *= 0.1F;
				break;
			default:
				break;

		}
	}
	return f;
}

//boolean isFahrenheit: True == Fahrenheit; False == Celcius
float DHT::computeHeatIndex(float temperature, float percentHumidity, bool isFahrenheit) {
	// Using both Rothfusz and Steadman's equations
	// http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml
	double hi;

	if (!isFahrenheit) temperature = convertCtoF(temperature);

	hi = 0.5 * (temperature + 61.0 + ((temperature - 68.0) * 1.2) + (percentHumidity * 0.094));

	if (hi > 79.0) {
		hi = -42.379F + 2.04901523 * temperature + 10.14333127 * percentHumidity
				+ -0.22475541 * temperature * percentHumidity + -0.00683783 * pow(temperature, 2)
				+ -0.05481717 * pow(percentHumidity, 2) + 0.00122874 * pow(temperature, 2) * percentHumidity
				+ 0.00085282 * temperature * pow(percentHumidity, 2)
				+ -0.00000199 * pow(temperature, 2) * pow(percentHumidity, 2);

		if ((percentHumidity < 13) && (temperature >= 80.0) && (temperature <= 112.0)) hi -= ((13.0
				- percentHumidity) * 0.25) * sqrt((17.0 - abs(temperature - 95.0)) * 0.05882);

		else if ((percentHumidity > 85.0) && (temperature >= 80.0) && (temperature <= 87.0)) hi +=
				((percentHumidity - 85.0) * 0.1) * ((87.0 - temperature) * 0.2);
	}

	return isFahrenheit ? static_cast<float>(hi) : convertFtoC(static_cast<float>(hi));
}

void DHT::read_setup(void) {
	// Go into high impedance state to let pull-up raise data line level and
	// start the reading process.
//	Serial.println(String(millis()) + " DHT::read_setup()");
	digitalWrite(_pin, HIGH);

	// FIXME -- reenable this (for AVR), but enable/disable with a #ifdef ... #endif
	//  delay(250);

//This is commented out because the the Queue library used for scheduling will not call
// sensors[x].acquire again for 500 ms. So there is a natural delay and no need to hog
// the CPU for 1/4 second.
//
//	unsigned long start = millis();
//	while ((millis() - start) < 250) {
//		yield();
//	}
}

boolean DHT::read(bool force) {
	// Check if sensor was read less than two seconds ago and return early
	// to use last reading.
	unsigned long currenttime = millis();
//	Serial.println(
//			String(currenttime) + " DHT::read() START\t\tsince start: " + (millis() - currenttime) + " ms");

	if (!force && ((currenttime - _lastreadtime) < 2000)) {
		return _lastresult; // return last correct measurement
	}
	_lastreadtime = currenttime;

	// Reset 40 bits of received data to zero.
	data[0] = data[1] = data[2] = data[3] = data[4] = 0;

	// Send start signal.  See DHT datasheet for full signal diagram:
	//   http://www.adafruit.com/datasheets/Digital%20humidity%20and%20temperature%20sensor%20AM2302.pdf

	// WARNING: It's required to call DHT::read_setup() before calling read. The setup
	//   is separated from this function to enable strategies for limiting how long the
	//   DHT read blocks other functions.

//  // Go into high impedance state to let pull-up raise data line level and
//  // start the reading process.
//  digitalWrite(_pin, HIGH);
////  delay(250);
//  unsigned long start = millis();
//  while ((millis() - start) < 250) {
//	  yield();
//  }

	// First set data line low for 20 milliseconds.
	pinMode(_pin, OUTPUT);
	digitalWrite(_pin, LOW);
//  delay(20);
	unsigned long start = millis();
	while ((millis() - start) < 20) {
		yield();
	}
//	Serial.println(
//			String(millis()) + " DHT::read() PHASE2\t\tsince start: " + (millis() - currenttime) + " ms");

	uint32_t cycles[82]; // sized to 82, which is 2 extra to avoid lint errors
	{
		//lint --e{1788} ignore error about variable lock referenced only by x-structors

		// Turn off interrupts temporarily because the next sections are timing critical
		// and we don't want any interruptions.
		InterruptLock lock;

		// End the start signal by setting data line high for 40 microseconds.
		digitalWrite(_pin, HIGH);
		delayMicroseconds(40);

		// Now start reading the data line to get the value from the DHT sensor.
		pinMode(_pin, INPUT_PULLUP);
		delayMicroseconds(10);  // Delay a bit to let sensor pull data line low.

		// First expect a low signal for ~80 microseconds followed by a high signal
		// for ~80 microseconds again.
		if (expectPulse(LOW) == 0) {
//			DEBUG_PRINTLN(F("Timeout waiting for start signal low pulse."));
//			Serial.println(
//					String(millis()) + " DHT::read() PHASE2 TIMEOUT ERROR\t\tsince start: "
//							+ (millis() - currenttime) + " ms");

			_lastresult = false;
			return _lastresult;
		}
		if (expectPulse(HIGH) == 0) {
//			Serial.println(
//					String(millis()) + " DHT::read() PHASE2 TIMEOUT ERROR\t\tsince start: "
//							+ (millis() - currenttime) + " ms");
//			DEBUG_PRINTLN(F("Timeout waiting for start signal high pulse."));
			_lastresult = false;
			return _lastresult;
		}

		// Now read the 40 bits sent by the sensor.  Each bit is sent as a 50
		// microsecond low pulse followed by a variable length high pulse.  If the
		// high pulse is ~28 microseconds then it's a 0 and if it's ~70 microseconds
		// then it's a 1.  We measure the cycle count of the initial 50us low pulse
		// and use that to compare to the cycle count of the high pulse to determine
		// if the bit is a 0 (high state cycle count < low state cycle count), or a
		// 1 (high state cycle count > low state cycle count). Note that for speed all
		// the pulses are read into a array and then examined in a later step.
		for (int i = 0; i < 80; i += 2) {
			cycles[i] = expectPulse(LOW);
			cycles[i + 1] = expectPulse(HIGH);
		}
	} // Timing critical code is now complete.
//	Serial.println(
//			String(millis()) + " DHT::read() PHASE3\t\tsince start: " + (millis() - currenttime) + " ms");

//	Serial.println(
//			String(millis()) + " DHT::read() PHASE4\t\tsince start: " + (millis() - currenttime) + " ms");

	// Inspect pulses and determine which ones are 0 (high state cycle count < low
	// state cycle count), or 1 (high state cycle count > low state cycle count).
	for (int i = 0; i < 40; ++i) {
		uint32_t lowCycles = cycles[2 * i];
		uint32_t highCycles = cycles[2 * i + 1];
		if ((lowCycles == 0) || (highCycles == 0)) {
//			DEBUG_PRINTLN(F("Timeout waiting for pulse."));
//			Serial.println(
//					String(millis()) + " DHT::read() PHASE4 TIMEOUT ERROR\t\tsince start: "
//							+ (millis() - currenttime) + " ms");
			_lastresult = false;
			return _lastresult;
		}
		data[i / 8] <<= 1;
		// Now compare the low and high cycle times to see if the bit is a 0 or 1.
		if (highCycles > lowCycles) {
			// High cycles are greater than 50us low cycle count, must be a 1.
			data[i / 8] |= 1;
		}
		// Else high cycles are less than (or equal to, a weird case) the 50us low
		// cycle count so this must be a zero.  Nothing needs to be changed in the
		// stored data.
	}
//	Serial.println(
//			String(millis()) + " DHT::read() PHASE5\t\tsince start: " + (millis() - currenttime) + " ms");

//  DEBUG_PRINTLN(F("Received:"));
//	DEBUG_PRINT(data[0], HEX);
//	DEBUG_PRINT(F(", "));
//	DEBUG_PRINT(data[1], HEX);
//	DEBUG_PRINT(F(", "));
//	DEBUG_PRINT(data[2], HEX);
//	DEBUG_PRINT(F(", "));
//	DEBUG_PRINT(data[3], HEX);
//	DEBUG_PRINT(F(", "));
//	DEBUG_PRINT(data[4], HEX);
//	DEBUG_PRINT(F(" =? "));
//	DEBUG_PRINTLN((data[0] + data[1] + data[2] + data[3]) & 0xFF, HEX);

	// Check we read 40 bits and that the checksum matches.
	if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
		_lastresult = true;
	}
	else {
//		DEBUG_PRINTLN(F("Checksum failure!"));
//		Serial.println(
//				String(millis()) + " DHT::read() PHASE5 CHECKSUM FAILURE\t\tsince start: "
//						+ (millis() - currenttime) + " ms");
		_lastresult = false;
	}
//	Serial.println(String(millis()) + " DHT::read() END\t\t" + (millis() - currenttime) + " ms");
	return _lastresult;
}

// Expect the signal line to be at the specified level for a period of time and
// return a count of loop cycles spent at that level (this cycle count can be
// used to compare the relative time of two pulses).  If more than a millisecond
// ellapses without the level changing then the call fails with a 0 response.
// This is adapted from Arduino's pulseInLong function (which is only available
// in the very latest IDE versions):
//   https://github.com/arduino/Arduino/blob/master/hardware/arduino/avr/cores/arduino/wiring_pulse.c
uint32_t DHT::expectPulse(int level) {
	uint32_t count = 0;
	// On AVR platforms use direct GPIO port access as it's much faster and better
	// for catching pulses that are 10's of microseconds in length:
#ifdef __AVR
	uint8_t portState = level ? _bit : 0;
	while ((*portInputRegister(_port) & _bit) == portState) {
		if (count++ >= _maxcycles) {
			return 0; // Exceeded timeout, fail.
		}
	}
	// Otherwise fall back to using digitalRead (this seems to be necessary on ESP8266
	// right now, perhaps bugs in direct port access functions?).
#else
	while (digitalRead(_pin) == level) {
		if (count++ >= _maxcycles) {
			return 0; // Exceeded timeout, fail.
		}
	}
#endif

	return count;
}
