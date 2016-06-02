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

//lint -e{26,785} suppress since lint doesn't understand C++11
t_sensor const sensorList[static_cast<int>(sensorModule::END)] = {
//
		{ "off", sensorModule::off },
		// One Digital IO
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
		// One Digital IO and/or one ADC
		{ "Dust", sensorModule::dust },
		{ "Rain", sensorModule::rain },
		{ "Soil", sensorModule::soil },
		{ "SoundH", sensorModule::soundh },
		{ "Methane", sensorModule::methane },
		// IC2 Devices
		{ "GY68", sensorModule::gy68 },
		{ "GY30", sensorModule::gy30 },
		{ "LCD1602", sensorModule::lcd1602 },
		// Serial
		{ "RFID", sensorModule::rfid },
		{ "Marquee", sensorModule::marquee } };

String getModuleNameString(sensorModule sm) {
// FIXME -- this should be a lookup into the sensor[] array, not a repeat of the module names
	switch (sm) {
		// 1 digital
		case sensorModule::dht11:
			return String("DHT11");
			break;
		case sensorModule::dht22:
			return String("DHT22");
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
			return String("Dust");
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
			return String("Methane");
			break;
			// I2C
		case sensorModule::gy68:
			return String("GY68");
			break;
		case sensorModule::gy30:
			return String("GY30");
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

