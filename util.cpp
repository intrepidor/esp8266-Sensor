/*
 * util.cpp
 *
 *  Created on: Apr 24, 2016
 *      Author: allan
 */

#include <Arduino.h>
#include "network.h"
#include "util.h"

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
			return F("GPIO0/D3/BootMode");
		case 1:
			return F("GPIO1/D10/TX0");
		case 2:
			return F("GPIO2/D4/TX1");
		case 3:
			return F("GPIO3/D9/RX0");
		case 4:
			return F("GPIO4/D2/SDA");
		case 5:
			return F("GPIO5/D1/SCL");
		case 6:
			return F("GPIO6/SDCLK");
		case 7:
			return F("GPIO7/SDD0");
		case 8:
			return F("GPIO8/SDD1");
		case 9:
			return F("GPIO9/SDD2");
		case 10:
			return F("GPIO10/SDD3");
		case 11:
			return F("GPIO11/SDCMD");
		case 12:
			return F("GPIO12/D6/MISO");
		case 13:
			return F("GPIO13/D7/MOSI");
		case 14:
			return F("GPIO14/D5/SPICLK");
		case 15:
			return F("GPIO15/D8/BootMode");
		case 16:
			return F("GPIO16/D0/Wakeup");
		case 17:
			return F("A0");
		default:
			return String(gpio_pin_number) + "unknown";
	}
}
