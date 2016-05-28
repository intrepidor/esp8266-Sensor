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

extern const int SENSOR_COUNT;
extern Sensor* sensors[];

extern Device dinfo;

extern const uint8_t PIN_SOFTRESET;
extern const uint8_t PIN_BUILTIN_LED;
extern const uint8_t PIN_PIRSENSOR;
extern const uint8_t DIGITAL_PIN_1;
extern const uint8_t DIGITAL_PIN_2;
extern const uint8_t DIGITAL_PIN_3;
extern const uint8_t DIGITAL_PIN_4;
extern const uint8_t DIGITAL_PIN_5;
extern const uint8_t ANALOG_PIN;
extern const uint8_t I2C_SDA_PIN;
extern const uint8_t I2C_SCL_PIN;

//extern void printChipInfo(void);
extern uint32_t spi_flash_get_id(void);

#endif /* MAIN_H_ */
