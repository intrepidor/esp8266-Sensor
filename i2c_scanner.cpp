/*
 * i2c_scanner.cpp
 *
 *  Adapted from: https://github.com/todbot/arduino-i2c-scanner/blob/master/I2CScanner/I2CScanner.ino
 */

#include "Wire.h"
#include "twi.h"  // from Wire library, so we can do bus scanning

const unsigned char start_address = 0x08; // lower addresses are reserved to prevent conflicts with other protocols
const unsigned char end_address = 0xA0;   // higher addresses unlock other modes, like 10-bit addressing

// Called when address is found in scanI2CBus()
void scanFunc(unsigned char addr, unsigned char result) {
	Serial.print("addr: ");
	Serial.print(addr, HEX);
	Serial.print((result == 0) ? " found!" : "       ");
	Serial.print((addr % 4) ? "\t" : "\r\n");
}

// Scan the I2C bus between addresses from_addr and to_addr.
// Assumes Wire.begin() has already been called
void scanI2CBus(void) {
	unsigned char rc;
	unsigned char data = 0; // not used, just an address to feed to twi_writeTo()
	Serial.println("=== I2C Scanner ===");
	for (unsigned char addr = start_address; addr <= end_address; addr++) {
		rc = twi_writeTo(addr, &data, 0, 1);
		scanFunc(addr, rc);
	}
}
