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

String getURLEncode(String str) {
	return getURLEncode(str.c_str());
}

String HTMLifyNewlines(String str) {
	str.replace("\r\n", "<br>");
	str.replace("\r", "<br>");
	return "<pre>" + str + "</pre>";
}

String getURLEncode(const char* msg) {
// http://www.icosaedro.it/apache/urlencode.c
	const char* hex = "0123456789ABCDEF";
	String encodedMsg = "";
	if (msg) {
		while (*msg != '\0') {
			if (('a' <= *msg && *msg <= 'z') || ('A' <= *msg && *msg <= 'Z')
					|| ('0' <= *msg && *msg <= '9')) {
				encodedMsg += *msg;
			}
			else {
				encodedMsg += '%';
				encodedMsg += hex[static_cast<unsigned char>(*msg) >> 4];
				encodedMsg += hex[*msg & 15];
			}
			msg++;
		}
	}
	return encodedMsg;
}

String repeatString(String str, int number_of_repeats) {
	String s("");
	int i = number_of_repeats;
	while (i > 0) {
		s += str;
		i--;
	}
	return s;
}

String indentString(String str, int indentamount) {
	String s("");
	if (str) {
		if (indentamount > 0) {
			s = repeatString(" ", 5);
			unsigned int i = 0;
			unsigned int n = 0;
			int _indx = 0;
			while (i < str.length()) {
				_indx = str.indexOf("\r\n", i);		// find the next CRLF
				if (_indx >= 0) {
					n = static_cast<unsigned int>(_indx);
					s += str.substring(i, n + 2);	// extract the portion up and including the CRLF
					s += repeatString(" ", 5);		// add indent (after the CRLF)
					i = n + 2;		// move the index to the character just after the CRLF and keep looking
				}
				else {
					return str; // Should not be <0. Something might be wrong. Return the original string.
				}
			}
		}
		else {
			s = str;
		}
	}
	/* As a basic check to see if something might have gone wrong, make sure
	 * the resultant string not less then the input string. Indenting should
	 * make the resultant string larger. An error must have occurred if it's
	 * smaller. Return the original string in case of potential errors.
	 */
	if (s.length() < str.length()) return str;
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

