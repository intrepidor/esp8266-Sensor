/*
 * util.h
 *
 *  Created on: Apr 24, 2016
 *      Author: allan
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <Arduino.h>

////////////
enum class HexDirection
	: bool {REVERSE = false, FORWARD = true
};
extern String memoryToHex(const char* addr, int _len, HexDirection dir);

////////////
extern String localIPstr(void);
extern String GPIO2Arduino(uint8_t gpio_pin_number);

#endif /* UTIL_H_ */
