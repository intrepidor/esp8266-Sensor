/*
 * i2c_scanner.cpp
 *
 *  Adapted from: https://github.com/todbot/arduino-i2c-scanner/blob/master/I2CScanner/I2CScanner.ino
 */

#include "Wire.h"
#include "twi.h"  // from Wire library, so we can do bus scanning

/* === I2C Address Space ===
 * The address is comprised of 7 bits plus a Read/Write bit. The address
 * may be expressed as 8-bit when including the R/W bit, or 7-bit when
 * excluding it. The 7-Bit address of 0x48 expressed as 8-bits is 0x90 Write,
 * and 0x91 Read.
 *
 * Slave Addr	7-bit Hex	R/W	Bit	Description
 *  000 0000	0x00		0	General call address
 *  000 0000	0x00		1	START byte(1)
 *  000 0001	0x01		X	CBUS address(2)
 *  000 0010	0x02		X	Reserved for different bus format (3)
 *  000 0011	0x03		X	Reserved for future purposes
 *  000 01XX	0x04-0x07	X	HS-mode master code
 *  000 1000    0x08		X	First valid 7-bit address
 *  111 0111    0x77		X	Last valid 7-bit address
 *  111 10XX	0x78-0x7B	X	10-bit slave addressing
 *  111 11XX	0x7C-0x7F	X	Reserved for future purposes
 *
 * (1) No device is allowed to acknowledge at the reception of the START byte.
 * (2) The CBUS address has been reserved to enable the inter-mixing of CBUS
 *     compatible and I2C-bus compatible devices in the same system. I2C-bus
 *     compatible devices are not allowed to respond on reception of this address.
 * (3) The address reserved for a different bus format is included to enable I2C
 *     and other protocols to be mixed. Only I2C-bus compatible devices that can
 *     work with such formats and protocols are allowed to respond to this address.
 */

const unsigned char start_address = 0x08; // lower addresses are reserved to prevent conflicts with other protocols
const unsigned char end_address = 0x77; // 7-bit address range. Higher addresses unlock other modes, like 10-bit addressing

// Called when address is found in scanI2CBus()
void scanFunc(unsigned char addr, unsigned char result) {
	Serial.print("addr: ");
	Serial.print(addr, HEX);
	Serial.print((result == 0) ? " found " : "       ");
	Serial.print((addr % 4) ? "\t" : "\r\n");
}

// scanI2CBus
// * Scans the I2C bus between 7-bit addresses from_addr and to_addr.
// * The 10-bit addresses space is not scanned.
// * Assumes Wire.begin() has already been called
void scanI2CBus(void) {
	unsigned char rc;
	unsigned char data = 0; // not used, just an address to feed to twi_writeTo()
	Serial.println("=== I2C Scanner ===");
	for (unsigned char addr = start_address; addr <= end_address; addr++) {
		rc = twi_writeTo(addr, &data, 0, 1);
		scanFunc(addr, rc);
	}
	Serial.println("");
}
