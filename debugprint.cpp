/*
 * debug.cpp
 *
 *  Created on: May 28, 2016
 *      Author: allan
 */

#include "util.h"
#include "debugprint.h"

String nl("\r\n");

bool DebugPrint::validateDebugLevel(void) {
	if (debuglevel >= DebugLevel::END) {
		debuglevel = DebugLevel::OFF;
		eeprom_is_dirty = true;
	}
	if (debuglevel < DebugLevel::OFF) {
		debuglevel = DebugLevel::OFF;
		eeprom_is_dirty = true;
	}
	return eeprom_is_dirty;
}

bool DebugPrint::getBitwiseAND(DebugLevel dlevel, DebugLevel b) {
	// return true if all the bits of 'b' are set in 'dlevel'
	if ((static_cast<int>(dlevel) & static_cast<int>(b)) == static_cast<int>(b)) {
		return true;
	}
	return false;
}

DebugLevel DebugPrint::getBitwiseOR(DebugLevel a, DebugLevel b) {
	int ored = static_cast<int>(a) | static_cast<int>(b);
	return static_cast<DebugLevel>(ored);
}

bool DebugPrint::isDebugLevel(DebugLevel dlevel) {
	if (debuglevel == DebugLevel::OFF) return false;
	return getBitwiseAND(dlevel, debuglevel);
}

DebugLevel DebugPrint::decrementDebugLevel(void) {
	switch (debuglevel) {
		case DebugLevel::END:
		case DebugLevel::INFO:
			debuglevel = DebugLevel::OFF;
			break;
		case DebugLevel::ERROR:
			debuglevel = DebugLevel::INFO;
			break;
		case DebugLevel::DEBUG:
			debuglevel = DebugLevel::ERROR;
			break;
		case DebugLevel::EEPROM:
			debuglevel = DebugLevel::DEBUG;
			break;
		case DebugLevel::TIMINGS:
			debuglevel = DebugLevel::EEPROM;
			break;
		case DebugLevel::HTTPPUT:
			debuglevel = DebugLevel::TIMINGS;
			break;
		case DebugLevel::HTTPGET:
			debuglevel = DebugLevel::HTTPPUT;
			break;
		case DebugLevel::WEBPAGEPROCESSING:
			debuglevel = DebugLevel::HTTPGET;
			break;
		case DebugLevel::PCF8591:
			debuglevel = DebugLevel::WEBPAGEPROCESSING;
			break;
		case DebugLevel::SHARPGP2Y10:
			debuglevel = DebugLevel::PCF8591;
			break;
		case DebugLevel::OFF:
		default:
			debuglevel = DebugLevel::SHARPGP2Y10;
			break;
	}
	eeprom_is_dirty = true;
	validateDebugLevel();
	return debuglevel;
}

DebugLevel DebugPrint::incrementDebugLevel(void) {
	switch (debuglevel) {
		case DebugLevel::OFF:
			debuglevel = DebugLevel::INFO;
			break;
		case DebugLevel::INFO:
			debuglevel = DebugLevel::ERROR;
			break;
		case DebugLevel::ERROR:
			debuglevel = DebugLevel::DEBUG;
			break;
		case DebugLevel::DEBUG:
			debuglevel = DebugLevel::EEPROM;
			break;
		case DebugLevel::EEPROM:
			debuglevel = DebugLevel::TIMINGS;
			break;
		case DebugLevel::TIMINGS:
			debuglevel = DebugLevel::HTTPPUT;
			break;
		case DebugLevel::HTTPPUT:
			debuglevel = DebugLevel::HTTPGET;
			break;
		case DebugLevel::HTTPGET:
			debuglevel = DebugLevel::WEBPAGEPROCESSING;
			break;
		case DebugLevel::WEBPAGEPROCESSING:
			debuglevel = DebugLevel::PCF8591;
			break;
		case DebugLevel::PCF8591:
			debuglevel = DebugLevel::SHARPGP2Y10;
			break;
		case DebugLevel::SHARPGP2Y10:
		case DebugLevel::END:
		default:
			debuglevel = DebugLevel::OFF;
			break;
	}
	eeprom_is_dirty = true;
	validateDebugLevel();
	return debuglevel;
}

String DebugPrint::convertDebugLevelToString(DebugLevel dl) {
	String r("");
	if (dl == DebugLevel::OFF) r += String("OFF");
	if (getBitwiseAND(dl, DebugLevel::INFO)) r += String(" INFO");
	if (getBitwiseAND(dl, DebugLevel::ERROR)) r += String(" ERROR");
	if (getBitwiseAND(dl, DebugLevel::DEBUG)) r += String(" DEBUG");
	if (getBitwiseAND(dl, DebugLevel::EEPROM)) r += String(" EEPROM");
	if (getBitwiseAND(dl, DebugLevel::TIMINGS)) r += String(" TIMINGS");
	if (getBitwiseAND(dl, DebugLevel::HTTPGET)) r += String(" HTTPGET");
	if (getBitwiseAND(dl, DebugLevel::HTTPPUT)) r += String(" HTTPPUT");
	if (getBitwiseAND(dl, DebugLevel::WEBPAGEPROCESSING)) r += String(" WEBPAGEPROCESSING");
	if (getBitwiseAND(dl, DebugLevel::PCF8591)) r += String(" PCF8591");
	if (getBitwiseAND(dl, DebugLevel::SHARPGP2Y10)) r += String(" SHARPGP2Y10");
	if (r.length() < 1) r += "UNKNOWN";
	r += " (" + String(static_cast<int>(dl)) + ")";
	return r;
}

void DebugPrint::println(DebugLevel dlevel, const String &s) {
	if (isDebugLevel(dlevel)) {
		Serial.println(s);
	}
}

void DebugPrint::println(DebugLevel dlevel, const __FlashStringHelper* p) {
	if (isDebugLevel(dlevel)) {
		Serial.println(p);
	}
}

void DebugPrint::println(DebugLevel dlevel, const char s[]) {
	if (isDebugLevel(dlevel)) {
		Serial.println(s);
	}
}

void DebugPrint::println(DebugLevel dlevel, char c) {
	if (isDebugLevel(dlevel)) {
		Serial.println(c);
	}
}

void DebugPrint::println(DebugLevel dlevel, unsigned char c, int m) {
	if (isDebugLevel(dlevel)) {
		Serial.println(c, m);
	}
}

void DebugPrint::println(DebugLevel dlevel, int n, int m) {
	if (isDebugLevel(dlevel)) {
		Serial.println(n, m);
	}
}

void DebugPrint::println(DebugLevel dlevel, unsigned int n, int m) {
	if (isDebugLevel(dlevel)) {
		Serial.println(n, m);
	}
}

void DebugPrint::println(DebugLevel dlevel, long n, int m) {
	if (isDebugLevel(dlevel)) {
		Serial.println(n, m);
	}
}

void DebugPrint::println(DebugLevel dlevel, unsigned long n, int m) {
	if (isDebugLevel(dlevel)) {
		Serial.println(n, m);
	}
}

void DebugPrint::printhexln(DebugLevel dlevel, const char* n, int _len, HexDirection dir) {
	if (isDebugLevel(dlevel)) {
		printhex(dlevel, n, _len, dir);
	}
	Serial.println();
}

void DebugPrint::println(DebugLevel dlevel, double n, int m) {
	if (isDebugLevel(dlevel)) {
		Serial.println(n, m);
	}
}

void DebugPrint::println(DebugLevel dlevel, bool b) {
	if (isDebugLevel(dlevel)) {
		if (b) {
			Serial.println(F("true"));
		}
		else {
			Serial.println(F("false"));
		}
	}
}

void DebugPrint::println(DebugLevel dlevel, const Printable& c) {
	if (isDebugLevel(dlevel)) {
		Serial.println(c);
	}
}

void DebugPrint::println(DebugLevel dlevel) {
	if (isDebugLevel(dlevel)) {
		Serial.println();
	}
}

void DebugPrint::print(DebugLevel dlevel, const __FlashStringHelper* p) {
	if (isDebugLevel(dlevel)) {
		Serial.print(p);
	}
}

void DebugPrint::print(DebugLevel dlevel, const String &s) {
	if (isDebugLevel(dlevel)) {
		Serial.print(s);
	}
}

void DebugPrint::print(DebugLevel dlevel, const char s[]) {
	if (isDebugLevel(dlevel)) {
		Serial.print(s);
	}
}

void DebugPrint::print(DebugLevel dlevel, char c) {
	if (isDebugLevel(dlevel)) {
		Serial.print(c);
	}
}

void DebugPrint::print(DebugLevel dlevel, unsigned char n, int m) {
	if (isDebugLevel(dlevel)) {
		Serial.print(n, m);
	}
}

void DebugPrint::print(DebugLevel dlevel, int n, int m) {
	if (isDebugLevel(dlevel)) {
		Serial.print(n, m);
	}
}

void DebugPrint::print(DebugLevel dlevel, unsigned int n, int m) {
	if (isDebugLevel(dlevel)) {
		Serial.print(n, m);
	}
}

void DebugPrint::print(DebugLevel dlevel, long n, int m) {
	if (isDebugLevel(dlevel)) {
		Serial.print(n, m);
	}
}

void DebugPrint::print(DebugLevel dlevel, unsigned long n, int m) {
	if (isDebugLevel(dlevel)) {
		Serial.print(n, m);
	}
}

void DebugPrint::printhex(DebugLevel dlevel, const char* n, int _len, HexDirection dir) {
	if (isDebugLevel(dlevel)) {
		Serial.print(memoryToHex(n, _len, dir));
	}
}

void DebugPrint::print(DebugLevel dlevel, double n, int m) {
	if (isDebugLevel(dlevel)) {
		Serial.print(n, m);
	}
}

void DebugPrint::print(DebugLevel dlevel, bool b) {
	if (isDebugLevel(dlevel)) {
		if (b) {
			Serial.print(F("true"));
		}
		else {
			Serial.print(F("false"));
		}
	}
}

void DebugPrint::print(DebugLevel dlevel, const Printable& p) {
	if (isDebugLevel(dlevel)) {
		Serial.print(p);
	}
}
