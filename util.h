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
extern const char* getCheckedStr(bool);
extern const char* isTrueStr(bool value_to_check);
extern const char* getTempUnits(bool true_for_farhenheit);

////////////
extern void yield_ms(unsigned long time_duration_to_yield_ms);
extern void yield_us(unsigned long time_duration_to_yield_us);
extern String localIPstr(void);
extern String GPIO2Arduino(uint8_t gpio_pin_number);
extern String padEndOfString(String str, unsigned int desired_length, char pad_character, bool trim = false);

#endif /* UTIL_H_ */
