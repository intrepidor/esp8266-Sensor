// This abstract class provides an interface for the different sensors
#ifndef _SENSOR_H_
#define _SENSOR_H_

#include <Arduino.h>
#include <cstring>
#include "sensor_support.h"

const int ACQUIRE_READS_PER_SENSOR = 2;
const int ACQUIRE_SETUPS_PER_SENSOR = 1;
const int ACQUIRES_PER_SENSOR = (ACQUIRE_READS_PER_SENSOR + ACQUIRE_SETUPS_PER_SENSOR);

enum class valueType
	: int {undefined = 0, temperature, humidity, voltage, logic, LAST
};

enum class uomType
	: int {undefined = 0, farhenheit, celsius, rh, volts, level, LAST
};

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
	void setPins(const SensorPins &p) {
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
	unsigned long last_sample_time_ms;
	valueType type;
	uomType uom;
	~SensorValue() { /* nothing to destroy */
	}
	SensorValue()
			: v(0), enabled(false), name(""), last_sample_time_ms(0), type(valueType::undefined),
					uom(uomType::undefined) {
	}
};

//-------------------------------------------------------------------
class Sensor {
private:
	String sensorName;
	SensorValue value[VALUE_COUNT];		// calibrated adjusted values
	SensorValue rawval[VALUE_COUNT];	// raw values
	SensorValue cal[CALIB_COUNT];
	sensorModule module;
	SensorPins pins;
	unsigned long time_till_stale_ms;

public:
	~Sensor() { /* nothing to destroy */
	}
	// CONSIDER creating a constructor that includes the init parameters, then call
	// init via the constructor
	Sensor(void) {
		sensorName = "";
		memset(value, 0, sizeof(value));
		memset(cal, 0, sizeof(cal));
		this->module = sensorModule::off;
		this->time_till_stale_ms = 10000; // default to 10 seconds
		// SensorValue's have their own constructor
		// SensorPins has its own constructor
	}
	virtual void init(sensorModule, SensorPins&) = 0;
	virtual bool acquire_setup(void) = 0;
	virtual bool acquire1(void) = 0;
	virtual bool acquire2(void) = 0;
	virtual String getsInfo(String eol) = 0;

	// General
	String getName(void) {
		return sensorName;
	}
	void setName(String _name) {
		sensorName = _name;
	}

	// Timeout until the values become stale
	void setStaleAge_ms(unsigned long _time_till_stale) {
		time_till_stale_ms = _time_till_stale;
	}
	unsigned long getStaleAge_ms(void) {
		return time_till_stale_ms;
	}

	// pins
	void setPins(const SensorPins &p) {
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
	String getModuleName(void);

	// raw values
	float getRawValue(int _channel);
	bool setRawValue(int _channel, float v);
	bool isRawValueStale(int _channel);
	unsigned long getRawAge(int channel) {
		return millis() - rawval[channel].last_sample_time_ms;
	}

	// values
	bool isValueChannelValid(int channel);
	bool setValueName(int channel, String name);
	String getValueName(int channel);
	bool getValueEnable(int channel);
	bool setValueEnable(int channel, bool b);
	float getValue(int channel);
	bool setValue(int channel, float v);
	bool isValueStale(int _channel);
	unsigned long getAge(int channel) {
		return millis() - value[channel].last_sample_time_ms;
	}
	void printValues(void);

	// cals
	bool isCalChannelValid(int channel);
	bool setCalName(int channel, String name);
	String getCalName(int channel);
	bool getCalEnable(int channel);
	bool setCalEnable(int channel, bool b);
	float getCal(int channel);
	bool setCal(int channel, float v);
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

