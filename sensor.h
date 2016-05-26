// This abstract class provides an interface for the different sensors
#ifndef _SENSOR_H_
#define _SENSOR_H_

#include <cstring>

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
#if 0
// Comment out because PC Lint is not ready
const std::array<sensorModule, static_cast<int>(sensorModule::END)> allModes = {sensorModule::off,
	sensorModule::dht11, sensorModule::dht22, sensorModule::ds18b20, sensorModule::sonar,
	sensorModule::dust, sensorModule::sound};
/* The array portModes and the enum portModes must match with one exception, the END
 * value is not copied into the portModes array.
 */
#endif
extern t_sensor const sensorList[];

enum class subcode {
	value1, value2, cal1, cal2, cal3
};

class Pins {
public:
	int digital;
	int analog;
	int sda;
	int scl;
	Pins()
			: digital(-1), analog(-1), sda(-1), scl(-1) {
	}
};

class SensorValue {
public:
	float v;
	bool enabled;
	char name[10];
	SensorValue()
			: v(0), enabled(false) {
		std::memset(name, 0, 10);
	}
};

class Sensor {
private:
	SensorValue value1;
	SensorValue value2;
	SensorValue cal1;
	SensorValue cal2;
	SensorValue cal3;

public:
//	void Sensor(void);
	virtual ~Sensor() = 0;
	virtual void init(sensorModule, Pins&) = 0;
	virtual void acquire(void) = 0;
	virtual float compute(Pins&) = 0;

	void init_values(void);
	float read(subcode);
	void write(subcode, float value);
	void print(void);
};

#endif

