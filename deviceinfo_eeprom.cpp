/*
 * deviceinfo_eeprom.cpp
 *
 *  Created on: May 30, 2016
 *      Author: allan
 */

#include <Arduino.h>
#include <EEPROM.h>
#include "deviceinfo.h"
#include "main.h"
#include "debugprint.h"

bool eeprom_is_dirty = false;

void Device::restoreDatabaseFromEEPROM(void) {
	uint8_t* pp = static_cast<uint8_t*>(static_cast<void*>(&db));
	for (unsigned int addr = 0; addr < sizeof(db); addr++) {
		*(pp + addr) = EEPROM.read(static_cast<int>(addr));
	}
	eeprom_is_dirty = false;
	debug.println(DebugLevel::DEBUG, nl + "Data copied from EEPROM into RAM data structure");
}

bool Device::saveDatabaseToEEPROM(void) {
	uint8_t* pp = static_cast<uint8_t*>(static_cast<void*>(&db));
	uint8_t v = 0;
	for (unsigned int addr = 0; addr < sizeof(db); addr++) {
		v = *(pp + addr);
		EEPROM.write(static_cast<int>(addr), v);
	}

	if (EEPROM.commit()) {
		debug.println(DebugLevel::DEBUG, nl + "Write from RAM data structure to EEPROM ok.");
		restoreDatabaseFromEEPROM();
		eeprom_is_dirty = false;
		return true;
	}
	else {
		debug.println(DebugLevel::ERROR, nl + "ERROR: Write to EEPROM failed!");
		return false; // signal error
	}
}

bool Device::eraseEEPROM(void) {
	for (unsigned int addr = 0; addr < sizeof(db); addr++) {
		EEPROM.write(static_cast<int>(addr), 0); // store 0 in each location
	}

	if (EEPROM.commit()) {
		debug.println(DebugLevel::DEBUG, nl + "EEPROM Cleared.");
		restoreDatabaseFromEEPROM();
		eeprom_is_dirty = true;
		return true;
	}
	else {
		debug.println(DebugLevel::ERROR, nl + "ERROR: EEPROM Clear failed.");
		return false; // signal error
	}
	return false;
}

