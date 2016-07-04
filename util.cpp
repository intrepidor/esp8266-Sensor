/*
 * util.cpp
 *
 *  Created on: Apr 24, 2016
 *      Author: allan
 */

#include <Arduino.h>
#include "network.h"
#include "util.h"

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
		default:
			return String(gpio_pin_number) + "unknown";
	}
}
