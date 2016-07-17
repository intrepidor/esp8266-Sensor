/*
 * sensor_support.cpp
 *
 *  Created on: May 29, 2016
 *      Author: allan
 */

#include <Arduino.h>
#include "sensor_support.h"

int getSensorValueCount(void) {
	return VALUE_COUNT;
}

int getSensorCalCount(void) {
	return CALIB_COUNT;
}

String getSensorModuleName(sensorModule mode) {
	String s("unknown");

	//lint -e{26} suppress error false error about the sensorModule
	for (int j = 0; j < static_cast<int>(sensorModule::END); j++) {
		if (mode == static_cast<sensorModule>(j)) {
			s = sensorList[static_cast<int>(j)].name;
			break;
		}
	}
	return s;
}

//lint -e{26,785} suppress since lint doesn't understand C++11
t_sensor const sensorList[static_cast<int>(sensorModule::END)] = {

/* ---------------------------------------------------------------
 * IMPORTANT: This list must be in the same order as the enum list
 * ---------------------------------------------------------------*/
//
		{ "off", sensorModule::off },
		// Generic
		{ "AnalogDigital", sensorModule::analog_digital },	// analog value, digital bit
		{ "Digital", sensorModule::digital },				// digital bit
		{ "Analog", sensorModule::analog },                 // analog value
		// ... Temperature and Humidity
		{ "DHT11", sensorModule::dht11 },                   // 1 wire serial data
		{ "DHT22", sensorModule::dht22 },                   // 1 wire serial data
		{ "HTU21D_Si7021", sensorModule::htu21d_si7102 },   // I2C
		{ "DS18b20", sensorModule::ds18b20 },               // Official 1-Wire
		// ... Misc
		{ "Sonar", sensorModule::sonar },
		{ "Sound", sensorModule::sound },
		{ "Reed", sensorModule::reed },
		// ... PIR
		{ "HCS501", sensorModule::hcs501 },                 // digital bit
		{ "HCSR505", sensorModule::hcsr505 },               // digital bit
		// ... ANALOG
		// One Digital IO and/or one ADC
		{ "Dust", sensorModule::dust },
		{ "Rain", sensorModule::rain },
		{ "Soil", sensorModule::soil },
		{ "SoundH", sensorModule::soundh },
		{ "Methane", sensorModule::methane },
		// Misc IC2 Devices
		{ "BMP180", sensorModule::gy68_BMP180 },			// I2C
		{ "BH1750FVI", sensorModule::gy30_BH1750FVI },		//
		{ "LCD1602", sensorModule::lcd1602 },				// I2C
		// Serial
		{ "RFID", sensorModule::rfid },
		{ "Marquee", sensorModule::marquee },
		// OTHER
		{ "TaskClock", sensorModule::taskclock } };

