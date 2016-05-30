/*
 * debugprint.h
 *
 *  Created on: May 28, 2016
 *      Author: allan
 */

#ifndef DEBUGPRINT_H_
#define DEBUGPRINT_H_

#include <Arduino.h>
#include "deviceinfo.h"

extern String nl;

enum class DebugLevel
	: int {
		ALWAYS = 0, NONE, INFO, ERROR, DEBUG, DEBUGMORE, END
};

class DebugPrint {
private:
	DebugLevel debuglevel;

public:
	DebugPrint()
			: debuglevel(DebugLevel::ERROR) {
	}
	DebugLevel getDebugLevel(void) {
		return debuglevel;
	}
	void setDebugLevel(DebugLevel dlevel) {
		debuglevel = dlevel;
		eeprom_is_dirty = true;
		validateDebugLevel();
	}
	bool validateDebugLevel(void);
	static String convertDebugLevelToString(DebugLevel dl);
	bool isDebugLevel(DebugLevel dlevel);
	DebugLevel incrementDebugLevel(void);
	String getDebugLevelString(void) {
		return convertDebugLevelToString(debuglevel);
	}
	void println(DebugLevel dlevel, const __FlashStringHelper *);
	void println(DebugLevel dlevel, const String &s);
	void println(DebugLevel dlevel, const char[]);
	void println(DebugLevel dlevel, char);
	void println(DebugLevel dlevel, unsigned char, int = DEC);
	void println(DebugLevel dlevel, int, int = DEC);
	void println(DebugLevel dlevel, unsigned int, int = DEC);
	void println(DebugLevel dlevel, long, int = DEC);
	void println(DebugLevel dlevel, unsigned long, int = DEC);
	void println(DebugLevel dlevel, double, int = 2);
	void println(DebugLevel dlevel, const Printable&);
	void println(DebugLevel dlevel);

	void print(DebugLevel dlevel, const __FlashStringHelper *);
	void print(DebugLevel dlevel, const String &s);
	void print(DebugLevel dlevel, const char[]);
	void print(DebugLevel dlevel, char);
	void print(DebugLevel dlevel, unsigned char, int = DEC);
	void print(DebugLevel dlevel, int, int = DEC);
	void print(DebugLevel dlevel, unsigned int, int = DEC);
	void print(DebugLevel dlevel, long, int = DEC);
	void print(DebugLevel dlevel, unsigned long, int = DEC);
	void print(DebugLevel dlevel, double, int = 2);
	void print(DebugLevel dlevel, const Printable&);
};

#endif /* DEBUGPRINT_H_ */
