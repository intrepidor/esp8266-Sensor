/*
 * sensor_support.h
 *
 *  Created on: May 29, 2016
 *      Author: allan
 */

#ifndef SENSOR_SUPPORT_H_
#define SENSOR_SUPPORT_H_

const int VALUE_COUNT = 2;
const int CALIB_COUNT = 4;	// should be the same as MAX_ADJ

//-------------------------------------------------------------------
enum class sensorModule {
	off = 0,
// 1 digital
	dht11,
	dht22,
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
	gy68,
	gy30,
	lcd1602,
// serial
	rfid,
	marquee,
	END
};
struct t_sensor {
	const char* const name;
	sensorModule id;
};
extern t_sensor const sensorList[];

extern String getModuleNameString(sensorModule sm);
extern int getSensorValueCount(void);
extern int getSensorCalCount(void);

#endif /* SENSOR_SUPPORT_H_ */
