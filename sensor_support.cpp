/*
 * sensor_support.cpp
 *
 *  Created on: May 29, 2016
 *      Author: allan
 */

#include <Arduino.h>
#include "main.h"
#include "network.h"
#include "thingspeak.h"
#include "deviceinfo.h"
#include "util.h"
#include "sensor_support.h"

int getSensorValueCount(void) {
	return SENSOR_VALUE_COUNT;
}

int getSensorCalCount(void) {
	return SENSOR_CALIB_COUNT;
}

//============================================================================
/* Position	Sensor					DeviceInfo Database
 *     0	Port 0, channel 0		db.thingspeakChannelSettings.fieldPort[0]
 *     1	Port 0, channel 1		db.thingspeakChannelSettings.fieldPort[1]
 *     2	Port 1, channel 0		db.thingspeakChannelSettings.fieldPort[2]
 *     3	Port 1, channel 1		db.thingspeakChannelSettings.fieldPort[3]
 *     4	Port 2, channel 0		db.thingspeakChannelSettings.fieldPort[4]
 *     5	Port 2, channel 1		db.thingspeakChannelSettings.fieldPort[5]
 *     6	Port 3, channel 0		db.thingspeakChannelSettings.fieldPort[6]
 *     7	Port 3, channel 1		db.thingspeakChannelSettings.fieldPort[7]
 *     8	PIR						db.thingspeakChannelSettings.fieldExtra[0]
 *     9	RSSI					db.thingspeakChannelSettings.fieldExtra[1]
 *    10	Uptime					db.thingspeakChannelSettings.fieldExtra[2]
 *    11	Spare					db.thingspeakChannelSettings.fieldExtra[3]
 */
//============================================================================
//----------------------------------------------------------------------
/* Field numbers correspond to those of Thingspeak, field1 to field8,
 * which are numbers 1 to 8. Convert field number to Position. The first
 * Position found starting with fieldExtra[] will be returned. So if
 * the field number is used for multiple fieldPort/FieldExtra, then only
 * the first one found is considered.
 */
int getPositionByTSFieldNumber(int fld) {
	int pm = dinfo.getTSFieldPortMax();
	if (fld > 0 && fld <= MAX_THINGSPEAK_FIELD_COUNT) {
		// Look for assignment to field<fld>
		for (int e = 0; e < dinfo.getTSFieldExtraMax(); e++) {
			if (fld == dinfo.getTSFieldExtraNumber(e)) return (e + pm);
		}
		for (int p = 0; p < pm; p++) {
			if (fld == dinfo.getTSFieldPortNumber(p)) return p;
		}
	}
	return 0;
}

//----------------------------------------------------------------------
int getFieldByPosition(int _pos) {
	if (_pos < 10) {
		return dinfo.getTSFieldExtraNumber(_pos - dinfo.getTSFieldPortMax());
	}
	else if (_pos < dinfo.getTSFieldPortMax()) {
		return dinfo.getTSFieldPortNumber(_pos);
	}
	return 0;
}

//----------------------------------------------------------------------

float getValueByPosition(int _pos) {
	switch (_pos) {
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
		case 6:
			return sensors[3]->getValue(0);
		case 7:
			return sensors[3]->getValue(1);
		case 8:
			return static_cast<float>(PIRcount);
		case 9:
			return static_cast<float>(WiFi.RSSI());
		case 10:
			return static_cast<float>(millis() / MS_PER_MINUTE);
		default:
			return NAN;
	}
	return NAN;
}

String getNameByPosition(int _pos) {
	switch (_pos) {
		case 0:
			return F("Port0/chan0 (sensors[0].value[0])");
		case 1:
			return F("Port0/chan1 (sensors[0].value[1])");
		case 2:
			return F("Port1/chan0 (sensors[1].value[0])");
		case 3:
			return F("Port1/chan1 (sensors[1].value[1])");
		case 4:
			return F("Port2/chan0 (sensors[2].value[0])");
		case 5:
			return F("Port2/chan1 (sensors[2].value[1])");
		case 6:
			return F("Port3/chan0 (sensors[3].value[0])");
		case 7:
			return F("Port3/chan1 (sensors[3].value[1])");
		case 8:
			return F("PIR");
		case 9:
			return F("RSSI");
		case 10:
			return F("Uptime");
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

