/*
 * reset.cpp
 *
 *  Created on: May 29, 2016
 *      Author: allan
 */

#if 0

#include <Arduino.h>
#include "main.h"
#include "debugprint.h"

// ------------------------------------------------------------------------------------
void reset_config(void) {
	// Setup the external Reset circuit
	digitalWrite(PIN_SOFTRESET, HIGH);
	/* Make sure the pin is High so it does not reset until we want it to. Do
	 * this before setting the direction as an output to avoid accidental
	 * reset during pin configuration. */
	pinMode(PIN_SOFTRESET, INPUT);
	/* Set the pin used to cause a reset as an input. This avoids it reseting
	 * when we don't want a reset.
	 */
}

void reset(void) {
	/* Note on the nodeMCU, pin D0 (aka GPIO16) may or may not already be
	 * connected to the RST pin. If it's not connected, connect via a 10K
	 * resistor. Do no connect directly.
	 * The reason it's connected is in order to be able to wake up from deep sleep.
	 * When the esp8266 is put into deep sleep everything but the RTC is powered off.
	 * You can set a timer in the RTC that toggles GPIO16 when it expires and that
	 * resets the esp8266 causing it to power up again. This is the only way the
	 * esp8266 can wake itself up from deep sleep, so it's quite a useful function
	 * to have. On the esp-03 module there is a tiny jumper to connect GPIO16 to reset
	 * so one can connect/disconnect it. On the esp-12, since everything is under a
	 * shield that jumper is evidently not available, so they had to decide one way
	 * or the other...
	 */

	/* Write a low to the pin that is physically connected to the RST pin.
	 * This will force the hardware to reset.
	 */
	reset_config();

	Serial.println("Rebooting ... this may take 15 seconds or more.");

	pinMode(PIN_SOFTRESET, OUTPUT);
	for (int a = 0; a < 10; a++) {
		digitalWrite(PIN_SOFTRESET, LOW);
		delay(1000); // CONSIDER using an optimistic_yield(1000000) instead so background tasks can still run
		digitalWrite(PIN_SOFTRESET, HIGH);
		delay(1000);// CONSIDER using an optimistic_yield(1000000) instead so background tasks can still run
	}
}

#endif
