/* PCF8591 Driver for ESP8266
 * by Allan Inda
 * 2016-Aug-22
 */

#include <Arduino.h>
#include <Wire.h>
#include "main.h"
#include "pcf8591.h"

void PCF8591::begin() {
#ifdef ARDUINO_ESP8266_ESP12
	Wire.begin(2, 14); // the pins for SDA and SCL in nodemcu/pins_arduino.h are wrong! So specify them directly here.
#else
	Wire.begin(); // does not work for ESP8266 due to bug in ESP8266_Arduino library
	Serial.println("*** ESP8266 PCF8591 BUG ***");
#endif
	config();
}

bool PCF8591::config(int channel) {
	configReg = analog_output_status | auto_increment_channel_status | static_cast<uint8_t>(inputType)
			| (channel & PCF8591_ADC_CHANNEL_NUMBER_MASK);

	Wire.beginTransmission(i2cAddress); // | PCF8591_WRITE	);
	size_t bytes_written = Wire.write(configReg);
	bytes_written += Wire.write(dacReg);
	Wire.endTransmission();

	if (bytes_written == 2 /* two Wire.write() calls above */) return true;
	return false;
}

int PCF8591::readADC(const int _channel) {
	const size_t BYTES_TO_READ = 2; // discard the first reads, take the second one.
	int r = PCF8591_ERROR_READ;
	if (_channel >= 0 && _channel < PCF8591_ADC_MAX_CHANNELS) {
		if (config(_channel)) {
			if (Wire.requestFrom(i2cAddress, BYTES_TO_READ, true /* send stop */) == BYTES_TO_READ) {
				for (int i = 0; i < BYTES_TO_READ; i++) {
					r = Wire.read();
				}
				lastADCReading[_channel] = r;
			}
			else {
				DEBUGPRINTLN(DebugLevel::PCF8591, F("readADC: requestFrom error"));
			}
		}
		else {
			DEBUGPRINTLN(DebugLevel::PCF8591, F("readADC: config returned error"));
		}
	}
	else {
		DEBUGPRINTLN(DebugLevel::PCF8591, F("readADC: Channel out of range"));
	}
	DEBUGPRINTLN(DebugLevel::PCF8591, "readADC: result[" + String(_channel) + "]=" + String(r));
	return r;
}

void test_pcf8591(void) {
	const unsigned char PCF8591 = 0x48; // I2C bus address
	const size_t BYTES_TO_READ = 5; // Throw away the first read.

	for (int chan = 0; chan < 4; chan++) {
		Wire.beginTransmission(PCF8591); // wake up PCF8591
		Wire.write(chan); // control byte - select channel (0 to 3)
		Wire.endTransmission();
		delayMicroseconds(500);
		Wire.requestFrom(PCF8591, BYTES_TO_READ, true /* send stop */);
		for (int i = 0; i < BYTES_TO_READ; i++) {
			unsigned char v = Wire.read();
			if (i == 1) {
				Serial.print("[" + String(v) + "]");
			}
			else {
				Serial.print(v);
			}
			Serial.print(",");
		}
		Serial.print(" \t");
		delayMicroseconds(100);
	}
	Serial.println("");
}
