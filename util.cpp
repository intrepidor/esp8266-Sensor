/*
 * util.cpp
 *
 *  Created on: Apr 24, 2016
 *      Author: allan
 */

#include <Arduino.h>
#include "network.h"
#include "util.h"
#include "main.h"
#include "deviceinfo.h"

String getTimeString(unsigned long thetime) {
	unsigned long up_day = thetime / MS_PER_DAY;
	unsigned long up_hrs = (thetime - (up_day * MS_PER_DAY)) / MS_PER_HOUR;
	unsigned long up_min = (thetime - ((up_day * MS_PER_DAY) + (up_hrs * MS_PER_HOUR))) / MS_PER_MINUTE;
	unsigned long up_sec = (thetime
			- ((up_day * MS_PER_DAY) + (up_hrs * MS_PER_HOUR) + (up_min * MS_PER_MINUTE))) / MS_PER_SECOND;
	String s(String(thetime) + " ms, or ");
	s += String(up_day) + "d ";
	if (up_hrs < 10) s += "0";
	s += String(up_hrs) + ":";
	if (up_min < 10) s += "0";
	s += String(up_min) + ":";
	if (up_sec < 10) s += "0";
	s += String(up_sec);
	// Example: 897234 ms, 10d 01:23:04
	return s;
}

//////////////////////////////////////////////////////////////////////////////////////
/* These are replacements for delay() and delayMicroseconds(). While delay() loops
 * doing nothing and holding the CPU until the time duration passes, these functions
 * yield() during that time. This is not as accurate as a delay, since the time
 * taken by yield() is indeterminate. But the yield permits the RTOS to do other
 * work and avoids blocking the ESP SW and HW watchdogs from being serviced.
 * Don't use these if timing is critical.
 * Note also these will perform minimally one yield before returning.
 */
void yield_ms(unsigned long time_duration_to_yield_ms) {
	unsigned long atstart = millis(); // used to protect from roll-overs
	unsigned long yield_until = millis() + time_duration_to_yield_ms;
	unsigned long now = 0; // initialize to something easy
	do {
		now = millis();
		yield();

	} while (now < yield_until && now > atstart /* if now<atstart, then millis() rolled over*/);
}
void yield_us(unsigned long time_duration_to_yield_us) {
	unsigned long atstart = micros(); // used to protect from roll-overs
	unsigned long yield_until = micros() + time_duration_to_yield_us;
	unsigned long now = 0; // initialize to something easy
	do {
		now = micros();
		yield();

	} while (now < yield_until && now > atstart /* if now<atstart, then micros() rolled over*/);
}

//////////////////////////////////////////////////////////////////////////////////////

String padEndOfString(String str, unsigned int desired_length, char pad_character, bool trim) {
	String s("");
	if (str && desired_length <= 256 && desired_length > 0) {
		s = str;
		while (s.length() < desired_length) {
			s += pad_character;
		}
		if (trim && s.length() > desired_length) {
			s = s.substring(0, desired_length);
			s.setCharAt(desired_length - 1, '#');
		}
	}
	return s;
}

String memoryToHex(const char* addr, int _len, HexDirection dir) {
	String s("");
	if (_len > 0 && _len <= 256) {
		int i = 0;
		char c = 0;
		if (addr) {
			if (dir == HexDirection::FORWARD) {
				i = 0; // redundant, but kept for symmetry
				while (i < _len) {
					c = *(addr + i);
					s += String(c, HEX);
					i++;
				}
			}
			else {
				i = _len - 1;
				while (i >= 0) {
					c = *(addr + i);
					s += String(c, HEX);
					i--;
				}
			}
		}
	}
	return s;
}

String localIPstr(void) {
	IPAddress ip = WiFi.localIP();
	String s = String(ip[0]) + ".";
	s += String(ip[1]) + ".";
	s += String(ip[2]) + ".";
	s += String(ip[3]);
	return s;
}

String GPIO2Arduino(uint8_t gpio_pin_number) {
	switch (gpio_pin_number) {
		case 0:
			return F("GPIO0/D3/SPI_CS2/FLASH");
		case 1:
			return F("GPIO1/D10/SPI_CS1/TX0");
		case 2:
			return F("GPIO2/D4/SDA/TX1");
		case 3:
			return F("GPIO3/D9/RX0");
		case 4:
			return F("GPIO4/D2/PWM3");
		case 5:
			return F("GPIO5/D1");
		case 6:
			return F("GPIO6/SDCLK/SCLK");
		case 7:
			return F("GPIO7/SDD0/MI");
		case 8:
			return F("GPIO8/SDD1/INT/RX1");
		case 9:
			return F("GPIO9/D11/SDD2");
		case 10:
			return F("GPIO10/D12/SDD3");
		case 11:
			return F("GPIO11/SDCMD/M0");
		case 12:
			return F("GPIO12/D6/MISO/PWM0");
		case 13:
			return F("GPIO13/D7/MOSI/CTS0");
		case 14:
			return F("GPIO14/D5/SCL/SPICLK/PWM2");
		case 15:
			return F("GPIO15/D8/SPI_CS/RTS0/PWM1/Boot");
		case 16:
			return F("GPIO16/D0/WAKE");
		case 17:
			return F("A0(ADC0)");
		default:
			return String(gpio_pin_number) + "unknown";
	}
}

const char* getCheckedStr(bool value_to_check) {
	if (value_to_check) return "checked";
	return " ";
}

const char* isTrueStr(bool value_to_check) {
	if (value_to_check) return "true";
	return "false";
}

const char* getTempUnits(bool true_for_farhenheit) {
	if (true_for_farhenheit) return "Fahrenheit";
	else return "Celsius";
}

float fixUOM(float val) {
// FIXME need to determine if this is a temperature value or something else.
	if (dinfo.isFahrenheit()) return val * 1.8 + 32.0;
	return val;
}

double fixUOM(double val) {
	if (dinfo.isFahrenheit()) return val * 1.8 + 32.0;
	return val;
}
