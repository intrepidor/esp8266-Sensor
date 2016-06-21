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
const unsigned long WATCHDOG_TIMEOUT_MS = 30000; // reset after this many milliseconds of no kicking
const unsigned long EXTERNAL_WATCHDOG_TIMEOUT_MS = 1600; // resets after this many milliseconds

bool stillStartingUp = true;	// if starting up is not set to false within set time, the watchdog resets
unsigned long startedTime = millis();
const unsigned long STARTUP_ABORT_TIMEOUT_MS = 60000; // allow 60 seconds to startup up, else reboot

void setStartupComplete(void) {
	stillStartingUp = false;
}

void kickAllWatchdogs(void) {
	kickAllSoftwareWatchdogs();
	kickExternalWatchdog();
}

void kickAllSoftwareWatchdogs(void) {
	for (int i = 0; i < NUM_TASKS; i++) {
		wdog_timer[i] = millis();
	}
}

void softwareWatchdog(void) {
	unsigned long now = millis();
	// loop through each watchdog timer for the different tasks and reset if any are too old
	if (stillStartingUp) {
		kickExternalWatchdog();
		if ((now - startedTime) > STARTUP_ABORT_TIMEOUT_MS) {
			debug.println(DebugLevel::ALWAYS, "Startup time exceeded, restarting");
			ESP.reset();
		}
	}
	else {
		for (int i = 0; i < NUM_TASKS; i++) {
			if ((now - wdog_timer[i]) >= WATCHDOG_TIMEOUT_MS) {
				ESP.reset();
			}
			else {
				kickExternalWatchdog();
			}
		}
	}
}

void kickExternalWatchdog(void) {
	static bool wdog_state = false;
	static bool wdog_configured = false;
	if (!wdog_configured) {
//		pinMode(WATCHDOG_WOUT_PIN, OUTPUT);
		kickAllSoftwareWatchdogs();
		// Call the routine to check the watchdog every 1/2 second.
		SWWatchdog.attach_ms(1000, softwareWatchdog);
		wdog_configured = true;
		startedTime = millis();
		Serial.println("Software watchdog initialized.");
	}
	pinMode(WATCHDOG_WOUT_PIN, OUTPUT);
	if (wdog_state) {
		digitalWrite(WATCHDOG_WOUT_PIN, HIGH);
		wdog_state = false;
	}
	else {
		digitalWrite(WATCHDOG_WOUT_PIN, LOW);
		wdog_state = true;
	}
}

