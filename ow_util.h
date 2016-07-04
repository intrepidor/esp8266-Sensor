/*
 * ow_util.h
 *
 *  Created on: Jul 4, 2016
 *      Author: allan
 */

#ifndef OW_UTIL_H_
#define OW_UTIL_H_

#include <Arduino.h>

struct OneWireRegistrationMap {
	uint8_t family_code;
	uint8_t serial[6];
	uint8_t crc;
};

union OneWireAddress {
	uint8_t all[8]; // 64-bit unique code lasered into the device
	OneWireRegistrationMap p;
};

extern String OWFamilyCode2PartName(uint8_t family_code);
extern String OWFamilyCode2Description(uint8_t family_code);

#endif /* OW_UTIL_H_ */
