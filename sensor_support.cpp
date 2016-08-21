/*
 * sensor_support.cpp
 *
 *  Created on: May 29, 2016
 *      Author: allan
 */

#include <Arduino.h>
#include "main.h"
#include "network.h"
#include "deviceinfo.h"
#include "util.h"
#include "sensor_support.h"

int getSensorValueCount(void) {
	return SENSOR_VALUE_COUNT;
}

int getSensorCalCount(void) {
	return SENSOR_CALIB_COUNT;
}

//----------------------------------------------------------------------
float getValueByPosition(int _pos) {
	switch (_pos) {
		// PORT FIELDS: 0 to (THINGSPEAK_PORT_FIELDS-1)
		case 0:
			return sensors[0]->getValue(0);
		case 1:
			return sensors[0]->getValue(1);
		case 2:
			return sensors[1]->getValue(0);
		case 3:
			return sensors[1]->getValue(1);
		case 4:
			return sensors[2]->getValue(0);
		case 5:
			return sensors[2]->getValue(1);
			// EXTRA Fields: THINGSPEAK_PORT_FIELDS to (THINGSPEAK_PORT_FIELDS+THINGSPEAK_EXTRA_FIELDS-1)
		case 6:
			return static_cast<float>(PIRcount);
		case 7:
			return static_cast<float>(WiFi.RSSI());
		case 8:
			return static_cast<float>(millis() / static_cast<float>(MS_PER_MINUTE));
		case 9:
			return 0;	// not used
		default:
			return NAN;
	}
	return NAN;
}

String getDescriptionByPosition(int _pos) {
	switch (_pos) {
		// PORT FIELDS: 0 to (THINGSPEAK_PORT_FIELDS-1)
		case 0:
			return F("Port0/chan0");  // (sensors[0].value[0])
		case 1:
			return F("Port0/chan1");  // (sensors[0].value[1])
		case 2:
			return F("Port1/chan0");  // (sensors[1].value[0])
		case 3:
			return F("Port1/chan1");  // (sensors[1].value[1])
		case 4:
			return F("Port2/chan0");  // (sensors[2].value[0])
		case 5:
			return F("Port2/chan1");  // (sensors[2].value[1])
			// EXTRA Fields: THINGSPEAK_PORT_FIELDS to (THINGSPEAK_PORT_FIELDS+THINGSPEAK_EXTRA_FIELDS-1)
		case 6:
			return F("PIR [/min]");
		case 7:
			return F("RSSI [dBm]");
		case 8:
			return F("Uptime [min]");
		case 9:
			return F("not used");
		default:
			return F("none");
	}
	return F("unknown");
}

//----------------------------------------------------------------------
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
		{ "TH DHT11", sensorModule::dht11 },                   // 1 wire serial data
		{ "TH DHT22", sensorModule::dht22 },                   // 1 wire serial data
		{ "TH HTU21D_Si7021", sensorModule::htu21d_si7102 },   // I2C
		{ "T DS18b20", sensorModule::ds18b20 },               // Official 1-Wire
		// ... Misc
		{ "Sonar", sensorModule::sonar },
		{ "Sound", sensorModule::sound },
		{ "Reed", sensorModule::reed },
		// ... PIR
		{ "P HCS501", sensorModule::hcs501 },                 // digital bit
		{ "P HCSR505", sensorModule::hcsr505 },               // digital bit
		// ... ANALOG
		// One Digital IO and/or one ADC
		{ "D GY2Y10", sensorModule::dust },
		{ "M Rain", sensorModule::rain },
		{ "M Soil", sensorModule::soil },
		{ "S SoundH", sensorModule::soundh },
		{ "G Methane", sensorModule::methane },
		// Misc IC2 Devices
		{ "TP BMP180", sensorModule::gy68_BMP180 },			// I2C Temperature and Pressure
		{ "L BH1750FVI", sensorModule::gy30_BH1750FVI },	// I2C Light Sensor
		{ "LCD1602", sensorModule::lcd1602 },				// I2C
		// Serial
		{ "RFID", sensorModule::rfid },
		{ "Marquee", sensorModule::marquee },
		// OTHER
		{ "TaskClock", sensorModule::taskclock } };

