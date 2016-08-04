/*
 * sensor_support.h
 *
 *  Created on: May 29, 2016
 *      Author: allan
 */

#ifndef SENSOR_SUPPORT_H_
#define SENSOR_SUPPORT_H_

// FIXME move these to the .cpp file and replace occurrences with the getter function call.
const int SENSOR_VALUE_COUNT = 2;
const int SENSOR_CALIB_COUNT = 4;	// should be the same as MAX_ADJ

extern float getValueByPosition(int _pos);
String getDescriptionByPosition(int _pos);

//-------------------------------------------------------------------
enum class sensorModule {
	off = 0,
// Generic
	analog_digital,
	digital,
	analog,
	// 1 digital
	dht11,
	dht22,
	htu21d_si7102,
	ds18b20,
	sonar,
	sound,
	reed,
	hcs501,
	hcsr505,
// 1 digital + 1 analog
	dust,
	rain,
	soil,
	soundh,
	methane,
// I2C
	gy68_BMP180, // Barometric pressure sensor
	gy30_BH1750FVI, // light sensor
	lcd1602,
// serial
	rfid,
	marquee,
// other
	taskclock,
	END
};
struct t_sensor {
	const char* const name;
	sensorModule id;
};
extern t_sensor const sensorList[];

extern int getSensorValueCount(void);
extern int getSensorCalCount(void);
extern String getSensorModuleName(sensorModule mode);

#endif /* SENSOR_SUPPORT_H_ */
