/*
 * esp8266_EnvironmentSensor.h
 *
 *  Created on: Jan 21, 2016
 *      Author: Allan Inda
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <Arduino.h>
#include "temperature.h"
#include "deviceinfo.h"

extern void reset(void);
extern long count;
extern int PIRcount;
extern bool debug_output;

extern Device dinfo;
extern TemperatureSensor t1;
extern TemperatureSensor t2;

extern const uint8_t PIN_SOFTRESET;
extern const uint8_t PIN_BUILTIN_LED;
extern const uint8_t PIN_PIRSENSOR;
extern const uint8_t EXT_PORT_0;
extern const uint8_t EXT_PORT_1;
extern const uint8_t EXT_PORT_2;
extern const uint8_t EXT_PORT_3;
extern const uint8_t EXT_PORT_4;

//extern void printChipInfo(void);
extern uint32_t spi_flash_get_id(void);

#endif /* MAIN_H_ */
