#include <Arduino.h>
#include "main.h"
#include "util.h"
#include "generic.h"

void GenericSensor::init(sensorModule m, SensorPins& p) {
	/* The generic sensor has two inputs, one is analog, the other digital. Each
	 * input corresponds to a different channel, 0 and 1 respectively. The analog
	 * value is read first as a raw value, then converted to a measured value
	 * using the equation  measured = offset + a*raw + b*raw^2 + c*raw^3.
	 */
	setModule(m);
	setPins(p);

	// Configure Sensor Parameters
	setStaleAge_ms(10000);	// timeout of the value if not update for 10 seconds.

	// These are the defaults.
	setCalEnable(TEMP_CAL_CHANNEL_ANALOG_OFFSET, false);
	setCalName(TEMP_CAL_CHANNEL_ANALOG_OFFSET, "Analog Offset");
	setCalEnable(TEMP_CAL_CHANNEL_ANALOG_FIRST_ORDER, false);
	setCalName(TEMP_CAL_CHANNEL_ANALOG_FIRST_ORDER, "1st order Gain");
	setCalEnable(TEMP_CAL_CHANNEL_ANALOG_SECOND_ORDER, false);
	setCalName(TEMP_CAL_CHANNEL_ANALOG_SECOND_ORDER, "2nd order Gain");
	setCalEnable(TEMP_CAL_CHANNEL_ANALOG_THIRD_ORDER, false);
	setCalName(TEMP_CAL_CHANNEL_ANALOG_THIRD_ORDER, "3rd order Gain");

	setValueEnable(TEMP_VALUE_CHANNEL_ANALOG, false);
	setValueName(TEMP_VALUE_CHANNEL_ANALOG, "Analog");
	setValueEnable(TEMP_VALUE_CHANNEL_DIGITAL, false);
	setValueName(TEMP_VALUE_CHANNEL_DIGITAL, "Digital");

	if (m == sensorModule::analog || m == sensorModule::analog_digital) {
		setCalEnable(TEMP_CAL_CHANNEL_ANALOG_OFFSET, true);
		setCalEnable(TEMP_CAL_CHANNEL_ANALOG_FIRST_ORDER, true);
		setCalEnable(TEMP_CAL_CHANNEL_ANALOG_SECOND_ORDER, true);
		setCalEnable(TEMP_CAL_CHANNEL_ANALOG_THIRD_ORDER, true);
		setValueEnable(TEMP_VALUE_CHANNEL_ANALOG, true);
	}

	if (m == sensorModule::digital || m == sensorModule::analog_digital) {
		setValueEnable(TEMP_VALUE_CHANNEL_DIGITAL, true);
	}

	// The pins used to interact with the sensor
	digital_pin = static_cast<uint8_t>(p.digital);
	analog_pin = static_cast<uint8_t>(p.analog);
}

String GenericSensor::getsInfo(String eol) {
	String s("Class: GenericSensor" + eol);

	s += ".digital_pin: (" + String(digital_pin) + ") " + GPIO2Arduino(digital_pin) + eol;
	s += ".analog_pin: (" + String(analog_pin) + ") " + GPIO2Arduino(analog_pin) + eol;
	s += ".started: ";
	if (started) {
		s += "true";
	}
	else {
		s += "false";
	}
	s += eol;
	if (started) {
		;
	}
	return s;
}

bool GenericSensor::acquire_setup(void) {
	if (started) {
		return acquire1();
	}
	else {
		if (getModule() == sensorModule::analog || getModule() == sensorModule::analog_digital) {
			debug.println(DebugLevel::DEBUGMORE, String(millis()) + ", setup() " + String(analog_pin));
			started = true;
		}
		if (getModule() == sensorModule::digital || getModule() == sensorModule::analog_digital) {
			debug.println(DebugLevel::DEBUGMORE, String(millis()) + ", setup() " + String(digital_pin));
			pinMode(digital_pin, INPUT);
			started = true;
		}
		if (getModule() == sensorModule::off) {
			started = true;
		}
	}
	return true;
}

bool GenericSensor::acquire1(void) {
	if (started) {
		if (getModule() == sensorModule::analog || getModule() == sensorModule::analog_digital) {
			int a = analogRead(analog_pin);

			// raw, non-corrected, values
			setRawAnalog(static_cast<float>(a));
			double a1 = a;
			double a2 = a1 * a1;
			double a3 = a2 * a1;

			a1 = getCal(TEMP_CAL_CHANNEL_ANALOG_OFFSET) + a1 * getCal(TEMP_CAL_CHANNEL_ANALOG_FIRST_ORDER)
					+ a2 * getCal(TEMP_CAL_CHANNEL_ANALOG_SECOND_ORDER)
					+ a3 * getCal(TEMP_CAL_CHANNEL_ANALOG_THIRD_ORDER);

			// Calibration corrected values
			setAnalog(static_cast<float>(a1));
		}
		if (getModule() == sensorModule::digital || getModule() == sensorModule::analog_digital) {
			int d = digitalRead(digital_pin);
			if (d == 0) {
				setDigital(0);
			}
			else {
				setDigital(1);
			}
		}
	}
	return true;
}

bool GenericSensor::acquire2(void) {
	return acquire_setup();
}
