/*
 * sensor.cpp
 *
 *  Created on: May 25, 2016
 *      Author: allan
 */

#include "sensor.h"

//lint -e{26,785} suppress since lint doesn't understand C++11
t_sensor const sensorList[static_cast<int>(sensorModule::END)] = { {
		"off",
		sensorModule::off },
// One Digital IO
		{ "DHT11", sensorModule::dht11 },
		{ "DHT22", sensorModule::dht22 },
		{ "DS18b20", sensorModule::ds18b20 },
		{ "Sonar", sensorModule::sonar },
		{ "Sound", sensorModule::sound },
		{ "Reed", sensorModule::reed },
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

const char* Sensor::getModule_cstr(void) {
	switch (this->getModule()) {
		// 1 digital
		case sensorModule::dht11:
			return "DHT11";
			break;
		case sensorModule::dht22:
			return "DHT22";
			break;
		case sensorModule::ds18b20:
			return "DS18b20";
			break;
		case sensorModule::sonar:
			return "Sonar";
			break;
		case sensorModule::sound:
			return "Sound";
			break;
		case sensorModule::reed:
			return "Reed";
			break;
		case sensorModule::hcs501:
			return "HCS501";
			break;
		case sensorModule::hcsr505:
			return "HCSR505";
			break;
			// 1 digital + 1 analog
		case sensorModule::dust:
			return "Dust";
			break;
		case sensorModule::rain:
			return "Rain";
			break;
		case sensorModule::soil:
			return "Soil";
			break;
		case sensorModule::soundh:
			return "SoundH";
			break;
		case sensorModule::methane:
			return "Methane";
			break;
			// I2C
		case sensorModule::gy68:
			return "GY68";
			break;
		case sensorModule::gy30:
			return "GY30";
			break;
		case sensorModule::lcd1602:
			return "LCD1602";
			break;
			// serial
		case sensorModule::rfid:
			return "RFID";
			break;
		case sensorModule::marquee:
			return "Marquee";
			break;
			// None
		case sensorModule::off:
			return "OFF";
			break;
		default:
			return "unknown";
			break;
	}
	return "unknown";
}

