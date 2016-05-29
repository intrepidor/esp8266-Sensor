/*
 * debug.cpp
 *
 *  Created on: May 28, 2016
 *      Author: allan
 */

#include "debugprint.h"

String nl("\r\n");

bool DebugPrint::isDebugLevel(DebugLevel dlevel) {
	if (dlevel == debuglevel || dlevel == DebugLevel::ALWAYS
			|| static_cast<int>(debuglevel) >= static_cast<int>(dlevel)) {
		return true;
	}
	return false;
}

void DebugPrint::incrementDebugLevel(void) {
	int newd = static_cast<int>(debuglevel) + 1;
	debuglevel = static_cast<DebugLevel>(newd);
	if (debuglevel >= DebugLevel::END) {
		debuglevel = DebugLevel::NONE;
	}
}

String DebugPrint::getDebugLevelString(void) {
	switch (debuglevel) {
		case DebugLevel::ALWAYS:
			return String("ALWAYS");
			break;
		case DebugLevel::NONE:
			return String("NONE");
			break;
		case DebugLevel::INFO:
			return String("INFO");
			break;
		case DebugLevel::ERROR:
			return String("ERROR");
			break;
		case DebugLevel::DEBUG:
			return String("DEBUG");
			break;
		case DebugLevel::DEBUGMORE:
			return String("DEBUGMORE");
			break;
		case DebugLevel::END:
		default:
			break;
	}
	return ("UNKNOWN");
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

void DebugPrint::println(DebugLevel dlevel, double n, int m) {
	if (isDebugLevel(dlevel)) {
		Serial.println(n, m);
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

void DebugPrint::print(DebugLevel dlevel, double n, int m) {
	if (isDebugLevel(dlevel)) {
		Serial.print(n, m);
	}
}

void DebugPrint::print(DebugLevel dlevel, const Printable& p) {
	if (isDebugLevel(dlevel)) {
		Serial.print(p);
	}
}
