// This abstract class provides an interface for the different sensors
#ifndef _SENSOR_H_
#define _SENSOR_H_

#include <Arduino.h>
#include <cstring>

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
extern const char* getModule_cstr(sensorModule sm);

//-------------------------------------------------------------------
class SensorPins {
public:
	int digital;
	int analog;
	int sda;
	int scl;
	SensorPins()
			: digital(-1), analog(-1), sda(-1), scl(-1) {
	}
	void setPins(SensorPins p) {
		digital = p.digital;
		analog = p.analog;
		sda = p.sda;
		scl = p.scl;
	}
};

//-------------------------------------------------------------------
class SensorValue {
public:
	float v;
	bool enabled;
	String name;
	~SensorValue() { /* nothing to destroy */
	}
	SensorValue()
			: v(0), enabled(false), name("") {
	}
};

//-------------------------------------------------------------------
const int VALUE_COUNT = 2;
const int CALIB_COUNT = 3;
int getValueCount(void);
int getCalCount(void);

//-------------------------------------------------------------------
class Sensor {
private:
	String sensorName;
	SensorValue value[VALUE_COUNT];
	SensorValue cal[CALIB_COUNT];
	sensorModule module;
	SensorPins pins;

public:
	~Sensor() { /* nothing to destroy */
	}
	Sensor(void) {
		this->module = sensorModule::off;
		// SensorValue's have their own constructor
		// SensorPins has its own constructor
	}
	virtual void init(sensorModule, SensorPins&) = 0;
	virtual bool acquire(void) = 0;

	// General
	String getName(void) {
		return sensorName;
	}
	void setName(String _name) {
		sensorName = _name;
	}

	// pins
	void setPins(SensorPins p) {
		this->pins.setPins(p);
	}
	int getDigitalPin(void) {
		return this->pins.digital;
	}

	// module
	void setModule(sensorModule m) {
		this->module = m;
	}
	sensorModule getModule(void) {
		return this->module;
	}
	sensorModule getType() const {
		return this->module;
	}
	const char* getModule_cstr(void);

	// values
	bool isValueIndexValid(int index);
	bool setValueName(int index, String name);
	String getValueName(int index);
	bool getValueEnable(int index);
	bool setValueEnable(int index, bool b);
	float getValue(int index);
	bool setValue(int index, float v);
	void printValues(void);

	// cals
	bool isCalIndexValid(int index);
	bool setCalName(int index, String name);
	String getCalName(int index);
	bool getCalEnable(int index);
	bool setCalEnable(int index, bool b);
	float getCal(int index);
	bool setCal(int index, float v);
	void printCals(void);
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

#endif

