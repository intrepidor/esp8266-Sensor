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

unsigned long wdog_timer[static_cast<int>(taskname_NUM_TASKS)];
const unsigned long WATCHDOG_TIMEOUT_MS = 30000; // reset after this many milliseconds of no kicking
//const unsigned long EXTERNAL_WATCHDOG_TIMEOUT_MS = 1600; // resets after this many milliseconds

bool stillStartingUp = true;	// if starting up is not set to false within set time, the watchdog resets
unsigned long startedTime = millis();

/* Set the maximum startup up time before a reboot. This time
 * includes what the user spends on a fresh unit to scan
 * for APs, select one, enter SSID, enter PW, and connect
 * to the AP. It might take a while to enter a long password,
 * so the time has to be long enough. But not too long that
 * the unit becomes unresponsive.
 */
const unsigned long STARTUP_ABORT_TIMEOUT_MS = 240000; // 4 minutes

void setStartupComplete(void) {
	stillStartingUp = false;
	kickExternalWatchdog();

	/* Remove the call to "softwareWatchdog" from the Ticker. The task schedule
	 will need to take over calling softwareWatch(). This helps avoid too long
	 of delays calling it because of blocking by the ESP8266 internal code.
	 */
//	SWWatchdog.detach();
}

void kickAllWatchdogs(void) {
	kickAllSoftwareWatchdogs();
	kickExternalWatchdog();
}

void kickAllSoftwareWatchdogs(void) {
	for (int i = 0; i < static_cast<int>(taskname_NUM_TASKS); i++) {
		wdog_timer[i] = millis();
	}
}

void softwareWatchdog(void) {
	static bool locked = false;
	noInterrupts();
	if (locked) {
		interrupts();
		// restore interrupts and exit
	}
	else {
		locked = true;
		interrupts();
		unsigned long now = millis();
		// loop through each watchdog timer for the different tasks and reset if any are too old
		if (stillStartingUp) {
			kickExternalWatchdog();
			if ((now - startedTime) > STARTUP_ABORT_TIMEOUT_MS) {
				Serial.println("User startup time exceeded, restarting");
				ESP.reset();
			}
		}
		else {
			for (int i = 0; i < static_cast<int>(taskname_NUM_TASKS); i++) {
				if ((now - wdog_timer[i]) >= WATCHDOG_TIMEOUT_MS) {
					Serial.println("User software wdt timeout for task " + String(i));
					ESP.reset();
				}
				else {
					kickExternalWatchdog();
				}
			}
		}
		locked = false;
	}
}

void kickExternalWatchdog(void) {
	static bool locked = false;
	static bool wdog_state = false;
	static bool wdog_configured = false;
	if (!wdog_configured) {
		pinMode(WATCHDOG_WOUT_PIN, OUTPUT);
		kickAllSoftwareWatchdogs();
		SWWatchdog.attach_ms(300, softwareWatchdog); // Check for stuck tasks and kick external Watchdog
		// The external wdog trips after 1.6s, but the timing is sloppy for the ESP8266, need this as 300ms.
		wdog_configured = true;
		startedTime = millis();
		Serial.println(F("Software watchdog initialized."));
	}

	if (!locked) {
		locked = true;
		pinMode(WATCHDOG_WOUT_PIN, OUTPUT);
		if (wdog_state) {
			digitalWrite(WATCHDOG_WOUT_PIN, HIGH);
			wdog_state = false;
		}
		else {
			digitalWrite(WATCHDOG_WOUT_PIN, LOW);
			wdog_state = true;
		}
		locked = false;
	}
}

