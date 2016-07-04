/*
 * ow_util.cpp
 *
 *  Created on: Jul 4, 2016
 *      Author: allan
 */

#include <Arduino.h>
#include "ow_util.h"

String OWFamilyCode2PartName(uint8_t family_code) {
	switch (family_code) {
		case 0x01:
			return F("DS1990A, DS1990R, DS2401, DS2411");
		case 0x02:
			return F("DS1991");
		case 0x04:
			return F("DS1994, DS2404");
		case 0x05:
			return F("DS2405");
		case 0x06:
			return F("DS1993");
		case 0x08:
			return F("DS1992");
		case 0x09:
			return F("DS1982, DS2502");
		case 0x0A:
			return F("DS1995");
		case 0x0B:
			return F("DS1985, DS2505");
		case 0x0C:
			return F("DS1996");
		case 0x0F:
			return F("DS1986, DS2506");
		case 0x10:
			return F("DS1920");
		case 0x12:
			return F("DS2406, DS2407");
		case 0x14:
			return F("DS1971, DS2430A");
		case 0x1A:
			return F("DS1963L");
		case 0x1C:
			return F("DS28E04-100");
		case 0x1D:
			return F("DS2423");
		case 0x1F:
			return F("DS2409");
		case 0x20:
			return F("DS2450");
		case 0x21:
			return F("DS1921G, DS1921H, DS1921Z");
		case 0x23:
			return F("DS1973, DS2433");
		case 0x24:
			return F("DS1904, DS2415");
		case 0x27:
			return F("DS2417");
		case 0x28:
			return F("DS18B20");
		case 0x29:
			return F("DS2408");
		case 0x2C:
			return F("DS2890");
		case 0x2D:
			return F("DS1972, DS2431");
		case 0x37:
			return F("DS1977");
		case 0x3A:
			return F("DS2413");
		case 0x41:
			return F("DS1922L, DS1922T, DS1923, DS2422");
		case 0x42:
			return F("DS28EA00");
		case 0x43:
			return F("DS28EC20");
		default:
			return F("unknown");
	}
}

String OWFamilyCode2Description(uint8_t family_code) {
	switch (family_code) {
		case 0x01:
			return F("1-Wire net address (registration number) only");
		case 0x02:
			return F("Multikey iButton, 1152-bit secure memory");
		case 0x04:
			return F("4Kb NV RAM memory and clock, timer, alarms");
		case 0x05:
			return F("Single addressable switch");
		case 0x06:
			return F("4Kb NV RAM memory");
		case 0x08:
			return F("1Kb NV RAM memory");
		case 0x09:
			return F("1Kb EPROM memory");
		case 0x0A:
			return F("16Kb NV RAM memory");
		case 0x0B:
			return F("16Kb EPROM memory");
		case 0x0C:
			return F("64Kb NV RAM memory");
		case 0x0F:
			return F("64Kb EPROM memory");
		case 0x10:
			return F("Temperature with alarm trips");
		case 0x12:
			return F("1Kb EPROM memory, 2-channel addressable switch");
		case 0x14:
			return F("256-bit EEPROM memory and 64-bit OTP register");
		case 0x1A:
			return F("4Kb NV RAM memory with write cycle counters");
		case 0x1C:
			return F("4096-bit EEPROM memory, 2-channel addressable switch");
		case 0x1D:
			return F("4Kb NV RAM memory with external counters");
		case 0x1F:
			return F("2-channel addressable coupler for sub-netting");
		case 0x20:
			return F("4-channel A/D converter (ADC)");
		case 0x21:
			return F("Thermochron® temperature logger");
		case 0x23:
			return F("4Kb EEPROM memory");
		case 0x24:
			return F("Real-time clock (RTC)");
		case 0x27:
			return F("RTC with interrupt");
		case 0x28:
			return F("Temperature Sensor");
		case 0x29:
			return F("8-channel addressable switch");
		case 0x2C:
			return F("1-channel digital potentiometer");
		case 0x2D:
			return F("1024-bit, 1-Wire EEPROM");
		case 0x37:
			return F("Password-protected 32KB (bytes) EEPROM");
		case 0x3A:
			return F("2-channel addressable switch");
		case 0x41:
			return F("High-capacity Thermochron (temperature) and Hygrochron™ (humidity) loggers");
		case 0x42:
			return F("Programmable resolution digital thermometer with sequenced detection and PIO");
		case 0x43:
			return F("20Kb 1-Wire EEPROM");
		default:
			return F("unknown");
	}
}
