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

// NOTE: Temperature is always stored in the db as Celsius. This includes the setValue calls.

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
private:
	float v;
	valueType type;
	uomType uom;

public:
	bool enabled;
	String vname;
	unsigned long last_sample_time_ms;

	~SensorValue() { /* nothing to destroy */
	}
	SensorValue()
			: v(0), type(valueType::undefined), uom(uomType::undefined), enabled(false), vname(""),
					last_sample_time_ms(0) {
	}
	uomType getUOM() {
		return this->uom;
	}
	valueType getType() {
		return this->type;
	}
	void setType(valueType t) {
		type = t;
	}
	void setUOM(uomType u) {
		uom = u;
	}
	float getValue();
	float getValueWithoutUnitConversion() {
		return this->v;
	}
	void setValue(float _v) {
		this->v = _v;
	}
};

//-------------------------------------------------------------------
class Sensor {
private:
	String sensorName;
	SensorValue value[SENSOR_VALUE_COUNT];		// calibrated adjusted values
	SensorValue rawval[SENSOR_VALUE_COUNT];	// raw values
	SensorValue cal[SENSOR_CALIB_COUNT];
	sensorModule module;
	SensorPins pins;
	/*
	 * After time_till_stale_ms time has passed, the values stored in value[] for the
	 * sensor are set to NAN (i.e. invalidated). This avoids having a stale value
	 * being reported such as after a sensor is removed or if the sensor breaks.
	 */
	unsigned long time_till_stale_ms;

public:
	/* ---------------------------------------------------------------------
	 * This variable keeps track of the next subtask to run for a given sensor. The
	 * choices are 0:acquire_setup(), 1:acquire1(), and 2:acquire2(). All other values
	 * are invalid. If the subtask is too large, it's converted to zero. This provides
	 * wrap-around. Wrap-around in the negative direction is not allowed, so that is also
	 * set to zero.
	 */
	int next_subtask;
	void setNextSubtask(int n) {
		next_subtask = n;
		getNextSubTask(); // performs range checking.
	}
	int incNextSubtask(void) {
		next_subtask++;
		return getNextSubTask();
	}
	int getNextSubTask(void) {
		if (next_subtask < 0 || next_subtask > 2) next_subtask = 0;
		return next_subtask;
	}

	/* ---------------------------------------------------------------------
	 * All sensor measurements start with a call to acquire_setup(). The value of
	 * minimum_time_between_acquiresetup_ms configures the minimum time between
	 * successive calls, and essentually sets the sensor's sample rate. The reason
	 * for limiting the rate is that some sensors require a settle down time after
	 * a read and before the next read can occur, e.g. Sharp GY2Y10 dust sensor.
	 */
	unsigned long minimum_time_between_acquiresetup_ms;

	/* ---------------------------------------------------------------------
	 * Most of the time when reading a sensor, the acquire_setup() is used to send
	 * the start conversion trigger or otherwise tell the sensor to perform a reading.
	 * The time until this conversion completes could be quite long (e.g. DHT22).
	 * Rather than have a delay loop that could be 10s or 100s of milliseconds long,
	 * the request for data is sent by acquire_setup(), and the actual reading of that
	 * data is done by acquire1(). The value of minimum_wait_time_after_acquiresetup_ms
	 * sets the minimum time to delay before attempting the read in acquire1().
	 */
	unsigned long minimum_wait_time_after_acquiresetup_ms;

	/* ---------------------------------------------------------------------
	 * This is sets the minimum time between the call to acquire1() and acquire2().
	 */
	unsigned long minimum_wait_time_after_acquire1_ms;

	/* ---------------------------------------------------------------------
	 * last_acquiresetup_timestamp_ms keeps track of the last time a acquire_setup() was
	 * executed on a given sensor for the purpose of ensuring the proper delays and
	 * sample rate controlled by minimum_time_between_acquiresetup_ms and
	 * minimum_wait_time_after_acquiresetup_ms.
	 */
	unsigned long last_acquiresetup_timestamp_ms;

	/* ---------------------------------------------------------------------
	 * last_acquire1_timestamp_ms keeps track of the last time a acquire1() was
	 * executed on a given sensor for the purpose of ensuring the proper delays and
	 * sample rates.
	 */
	unsigned long last_acquire1_timestamp_ms;

	/* ---------------------------------------------------------------------
	 * last_acquire2_timestamp_ms keeps track of the last time a acquire2() was
	 * executed. This time is not needed other than for debugging purposes.
	 */
	unsigned long last_acquire2_timestamp_ms;

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
		this->minimum_time_between_acquiresetup_ms = 0;
		this->minimum_wait_time_after_acquiresetup_ms = 0;
		this->minimum_wait_time_after_acquire1_ms = 0;
		this->last_acquiresetup_timestamp_ms = 0;
		this->last_acquire1_timestamp_ms = 0;
		this->next_subtask = 0;
	}

	virtual void init(sensorModule, SensorPins&) = 0;
	virtual bool acquire_setup(void) = 0;
	virtual bool acquire1(void) = 0;
	virtual bool acquire2(void) = 0;
	virtual String getsInfo(String eol) = 0;

	// Types
	void setType(int _channel, valueType t);
	valueType getType(int _channel);
	void setUOM(int _channel, uomType u);
	uomType getUOM(int _channel);

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
	int getAnalogPin(void) {
		return this->pins.analog;
	}

	// module
	void setModule(sensorModule m) {
		this->module = m;
	}
	sensorModule getModule(void) {
		return this->module;
	}
//	sensorModule getType() const {
//		return this->module;
//	}
	bool isSensorActive(void) {
		if (module == sensorModule::off) return false;
		return true;
	}
	String getModuleName(void);

	// raw values
	float getRawValue(int _channel);
	float getRawValueWithoutUnitConversion(int _channel);
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
	float getValueWithoutUnitConversion(int channel);
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

