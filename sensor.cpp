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

