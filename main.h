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

// NodeMCU 1.0 pinout
static const uint8_t pD0 = 16;
static const uint8_t pD1 = 5;
static const uint8_t pD2 = 4;
static const uint8_t pD3 = 0;
static const uint8_t pD4 = 2;
static const uint8_t pD5 = 14;
static const uint8_t pD6 = 12;
static const uint8_t pD7 = 13;
static const uint8_t pD8 = 15;
static const uint8_t pD9 = 3;
static const uint8_t pD10 = 1;

extern void reset(void);
extern long count;
extern int PIRcount;
extern bool debug_output;

extern Device dinfo;
extern TemperatureSensor t1;
extern TemperatureSensor t2;

//extern void printChipInfo(void);
extern uint32_t spi_flash_get_id(void);

#endif /* MAIN_H_ */
