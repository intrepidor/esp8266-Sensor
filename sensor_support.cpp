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

String getSensorName(sensorModule mode) {
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

// FIXME -- figure out if both of these two are needed or can they be combined?

//lint -e{26,785} suppress since lint doesn't understand C++11
t_sensor const sensorList[static_cast<int>(sensorModule::END)] = {
//
		// These strings are shown on the configuration web page and
		//    in text output to the serial port.
		{ "off", sensorModule::off },
		// Generic
		{ "Digital", sensorModule::digital },
		{ "Analog", sensorModule::analog },
		{ "AnalogDigital", sensorModule::analog_digital },
		// ... Temperature
		{ "DHT11", sensorModule::dht11 },
		{ "DHT22", sensorModule::dht22 },
		{ "DS18b20", sensorModule::ds18b20 },
		// ... Misc
		{ "Sonar", sensorModule::sonar },
		{ "Sound", sensorModule::sound },
		{ "Reed", sensorModule::reed },
		// ... PIR
		{ "HCS501", sensorModule::hcs501 },
		{ "HCSR505", sensorModule::hcsr505 },
		// ... ANALOG
		// One Digital IO and/or one ADC
		{ "Dust", sensorModule::dust },
		{ "Rain", sensorModule::rain },
		{ "Soil", sensorModule::soil },
		{ "SoundH", sensorModule::soundh },
		{ "Methane", sensorModule::methane },
		// IC2 Devices
		{ "BMP180", sensorModule::gy68_BMP180 },
		{ "BH1750FVI", sensorModule::gy30_BH1750FVI },
		{ "HTU21D_Si7021", sensorModule::htu21d_si7102 },
		{ "LCD1602", sensorModule::lcd1602 },
		// Serial
		{ "RFID", sensorModule::rfid },
		{ "Marquee", sensorModule::marquee } };

String getModuleNameString(sensorModule sm) {
// FIXME -- this should be a lookup into the sensor[] array, not a repeat of the module names
	switch (sm) {
		// Generic
		case sensorModule::analog:
			return String("Analog");
			break;
		case sensorModule::analog_digital:
			return String("AnalogDigital");
			break;
		case sensorModule::digital:
			return String("Digital");
			break;
			// 1 digital
		case sensorModule::dht11:
			return String("DHT11");
			break;
		case sensorModule::dht22:
			return String("DHT22");
			break;
		case sensorModule::htu21d_si7102:
			return String("HTU21D_Si7021");
			break;
		case sensorModule::ds18b20:
			return String("DS18b20");
			break;
		case sensorModule::sonar:
			return String("Sonar");
			break;
		case sensorModule::sound:
			return String("Sound");
			break;
		case sensorModule::reed:
			return String("Reed");
			break;
		case sensorModule::hcs501:
			return String("HCS501");
			break;
		case sensorModule::hcsr505:
			return String("HCSR505");
			break;
			// 1 digital + 1 analog
		case sensorModule::dust:
			return String("GP2Y1010");
			break;
		case sensorModule::rain:
			return String("Rain");
			break;
		case sensorModule::soil:
			return String("Soil");
			break;
		case sensorModule::soundh:
			return String("SoundH");
			break;
		case sensorModule::methane:
			return String("MQ-4");
			break;
			// I2C
		case sensorModule::gy68_BMP180:
			return String("BMP180");
			break;
		case sensorModule::gy30_BH1750FVI:
			return String("BH1750FVI");
			break;
		case sensorModule::lcd1602:
			return String("LCD1602");
			break;
			// serial
		case sensorModule::rfid:
			return String("RFID");
			break;
		case sensorModule::marquee:
			return String("Marquee");
			break;
			// None
		case sensorModule::off:
			return String("OFF");
			break;
		case sensorModule::END:
		default:
			return String("unknown");
			break;
	}
	return "unknown";
}

