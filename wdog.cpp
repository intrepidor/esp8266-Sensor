/*
 * wdog.cpp
 *
 *  Created on: Jun 16, 2016
 *      Author: allan
 */

#include <Arduino.h>
#include <Esp.h>
#include <Ticker.h>
#include "main.h"
#include "wdog.h"

Ticker SWWatchdog;

unsigned long wdog_timer[NUM_TASKS];

const unsigned long WATCHDOG_TIMEOUT = 30000; // reset after this many milliseconds of no kicking
const unsigned long EXTERNAL_WATCHDOG_TIMEOUT = 1600; // resets after this many milliseconds

void kickAllWatchdogs(void) {
	kickAllSoftwareWatchdogs();
	kickExternalWatchdog();
}

void kickAllSoftwareWatchdogs(void) {
//	Serial.print("{s}");
	for (int i = 0; i < NUM_TASKS; i++) {
		wdog_timer[i] = millis();
	}
}

void softwareWatchdog(void) {
	unsigned long now = millis();
	// loop through each watchdog timer for the different tasks and reset if any are too old
	for (int i = 0; i < NUM_TASKS; i++) {
		if ((now - wdog_timer[i]) >= WATCHDOG_TIMEOUT) {
			ESP.reset();
		}
		else {
			kickExternalWatchdog();
		}
	}
}

void kickExternalWatchdog(void) {
	static bool wdog_state = false;
	static bool wdog_configured = false;
	if (!wdog_configured) {
		pinMode(WATCHDOG_WOUT_PIN, OUTPUT);
		kickAllSoftwareWatchdogs();
		// Call the routine to check the watchdog every 1/2 second.
		SWWatchdog.attach_ms(100, softwareWatchdog);
		wdog_configured = true;
	}
	//Serial.print("{e}");
	if (wdog_state) {
		digitalWrite(WATCHDOG_WOUT_PIN, HIGH);
		wdog_state = false;
	}
	else {
		digitalWrite(WATCHDOG_WOUT_PIN, LOW);
		wdog_state = true;
	}
}

