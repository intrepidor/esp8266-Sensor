/*
 * esp8266_EnvironmentSensor.h
 *
 *  Created on: Jan 21, 2016
 *      Author: Allan Inda
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <Arduino.h>
#include <Adafruit_ADS1015.h>
#include "Queue.h"
#include "temperature.h"
#include "deviceinfo.h"
#include "debugprint.h"
#include "pcf8591.h"

#define USE_DEBUG_PRINT

#ifndef ARDUINO_ARCH_ESP8266
#define ARDUINO_ARCH_ESP8266
#endif

extern long count;
extern int PIRcount;
extern int PIRcountLast;
extern String ProgramInfo;

extern const int SENSOR_COUNT;
extern Sensor* sensors[];
extern Adafruit_ADS1115 ads1115;
extern PCF8591 pcf8591;

extern Device dinfo;
extern Queue myQueue;

extern DebugPrint debug;
#ifdef USE_DEBUG_PRINT
#define DEBUGPRINT(a, b) debug.print(a, b)
#define DEBUGPRINTLN(a, b) debug.println(a, b)
const bool DEBUGPRINT_ENABLED = true;
#else
#define DEBUGPRINT
#define DEBUGPRINTLN
const bool DEBUGPRINT_ENABLED = false;
//lint -e522	// only suppress if DEBUGPRINT is an empty macro
#endif
////////////////
extern const uint8_t PIN_SOFTRESET;
extern const uint8_t PIN_BUILTIN_LED;
extern const uint8_t PIN_PIRSENSOR;
extern const uint8_t DIGITAL_PIN_1;
extern const uint8_t DIGITAL_PIN_2;
extern const uint8_t DIGITAL_PIN_3;
extern const uint8_t DIGITAL_PIN_4;
extern const uint8_t WATCHDOG_WOUT_PIN;
extern const uint8_t ANALOG_PIN;
extern const uint8_t I2C_SDA_PIN;
extern const uint8_t I2C_SCL_PIN;

extern void CopyCalibrationDataToSensors(void);
extern String getsDeviceInfo(String eol);
extern String getsSensorInfo(String eol);

#endif /* MAIN_H_ */
