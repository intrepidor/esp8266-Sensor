#include <Arduino.h>
#include "main.h"
#include "util.h"
#include "pcf8591.h"
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
	setCalName(TEMP_CAL_CHANNEL_ANALOG_OFFSET, "Offset");
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

	if (m == sensorModule::analog || m == sensorModule::analog_digital
			|| m == sensorModule::Sharp_GP2Y10_DustSensor) {
		setCalEnable(TEMP_CAL_CHANNEL_ANALOG_OFFSET, true);
		setCalEnable(TEMP_CAL_CHANNEL_ANALOG_FIRST_ORDER, true);
		setCalEnable(TEMP_CAL_CHANNEL_ANALOG_SECOND_ORDER, true);
		setCalEnable(TEMP_CAL_CHANNEL_ANALOG_THIRD_ORDER, true);
		setValueEnable(TEMP_VALUE_CHANNEL_ANALOG, true);
	}

	if (m == sensorModule::digital || m == sensorModule::analog_digital) {
		setValueEnable(TEMP_VALUE_CHANNEL_DIGITAL, true);
	}

	if (m == sensorModule::taskclock) {
		// none are used
	}

	if (m == sensorModule::Sharp_GP2Y10_DustSensor) {
		setValueName(TEMP_VALUE_CHANNEL_ANALOG, "Dust u/m^3");
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
			DEBUGPRINTLN(DebugLevel::TIMINGS, String(millis()) + ", setup() " + String(analog_pin));
			started = true;
		}
		if (getModule() == sensorModule::digital || getModule() == sensorModule::analog_digital) {
			DEBUGPRINTLN(DebugLevel::TIMINGS, String(millis()) + ", setup() " + String(digital_pin));
			pinMode(digital_pin, INPUT);
			started = true;
		}
		if (getModule() == sensorModule::taskclock || getModule() == sensorModule::Sharp_GP2Y10_DustSensor) {
			DEBUGPRINTLN(DebugLevel::TIMINGS, String(millis()) + ", setup() " + String(digital_pin));
			pinMode(digital_pin, OUTPUT);
			digitalWrite(digital_pin, HIGH); // Turn off the LED
			setStaleAge_ms(60000); // don't timeout the value for at least a minute. The acquire routine has it's own timeout/stale code
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
		unsigned int now = micros();
		////////////////////////////////////////////////////////////////////////////////////////////
		if (getModule() == sensorModule::analog || getModule() == sensorModule::analog_digital) {
			last_reading_timestamp_us = now;
			reading_count++;
			uint16_t a = ads1115.readADC_SingleEnded(analog_pin);

			// raw, non-corrected, values
			setRawAnalog(static_cast<float>(a));
			double a1 = a * ads1115.getVoltsPerCount();
			double a2 = a1 * a1;
			double a3 = a2 * a1;

			a1 = getCal(TEMP_CAL_CHANNEL_ANALOG_OFFSET) + a1 * getCal(TEMP_CAL_CHANNEL_ANALOG_FIRST_ORDER)
					+ a2 * getCal(TEMP_CAL_CHANNEL_ANALOG_SECOND_ORDER)
					+ a3 * getCal(TEMP_CAL_CHANNEL_ANALOG_THIRD_ORDER);

			// Calibration corrected values
			setAnalog(static_cast<float>(a1));
		}
		////////////////////////////////////////////////////////////////////////////////////////////
		if (getModule() == sensorModule::digital || getModule() == sensorModule::analog_digital) {
			last_reading_timestamp_us = now;
			reading_count++;
			int d = digitalRead(digital_pin);
			if (d == 0) {
				setDigital(0);
			}
			else {
				setDigital(1);
			}
		}
		////////////////////////////////////////////////////////////////////////////////////////////
		static const unsigned int GP2Y10_MIN_SAMPLE_PERIOD_US = 100000; // Take measurements not faster than every 0.1 seconds
		static const unsigned int GP2Y10_SAMPLES_PER_CYCLE = 10;
		static const unsigned int GP2Y10_MAX_CYCLE_PERIOD_US = 20 * GP2Y10_MIN_SAMPLE_PERIOD_US
				* GP2Y10_SAMPLES_PER_CYCLE; // all samples must complete in a minimum time, else start cycle over
		if (getModule() == sensorModule::Sharp_GP2Y10_DustSensor) {
			if (now >= (last_reading_timestamp_us + GP2Y10_MIN_SAMPLE_PERIOD_US)) {

				/* Make sure the last reading was not a really long time ago. All readings
				 * within a cycle need to happen with a fixed about of time.
				 */
				if (now > (last_reading_timestamp_us + GP2Y10_MAX_CYCLE_PERIOD_US)) {
					/* Too long a time since last cycle. Invalidate the old data and
					 *     start the cycle over.
					 */
					DEBUGPRINTLN(DebugLevel::SHARPGP2Y10,
							"P" + String(sensorPortNumber) + " GP2Y10 Reseting Cycle");
					setAnalog (NAN);
					setRawAnalog(NAN);
					last_reading_timestamp_us = now;
					reading_count_within_cycle = 0;
					generic_accumulator = 0;
				}

				/*
				 * Generic algorithm per the datasheet
				 * 1. Turn on the IR LED by outputting a LOW on the digital pin.
				 * 2. Wait 280 us
				 * 3. Read the analog output. Finish this step within 40us.
				 * 4. Turn off the IR LED.
				 * Note: The IR LED should be on for 320 +/- 20us.
				 * 5. Convert the analog voltage into a measurement of dust
				 *    density as mg/m^3 using predetermined formulas.
				 * 6. Don't read the sensor faster than once per 10ms
				 *
				 * Algorithm for PCF8591, which takes 120us average per reading (per scope measurement):
				 * 1. Turn on IR LED
				 * 2. Take 5 readings in a row as fast as possible.
				 * 3. Per the PCF8591, the first reading will be the last adc conversion, which in
				 *    this case was 10ms ago, so discard it.
				 *       1st reading ... t=120us (discarded)
				 * 4. The next readings as assumed as follows:
				 *       2nd reading ... t=240us
				 *       3rd reading ... t=360us (very close to data sheet's sample point of 320us)
				 *       4th reading ... t=480us
				 *       5th reading ... t=600us
				 * 5. Find the maximum of those last 4 readings and use that as the final result.
				 * 6. Repeat this over the course of 10 cycles and create a accumulative result.
				 */
				pcf8591.config(2);
				// preconfigure the channel for the ADC.
				const size_t BYTES_TO_READ = 5; // 5 readings, 1 to discard, and 4 used as measurements

				/* Turn on the LED, then read 5 samples from the ADC. The first sample
				 *    represents data from a prior cycle, so discard it. Take the remaining
				 *    samples and find the highest value. That will be the result.
				 */
				digitalWrite(digital_pin, LOW); // power on the IR LED
				now = micros(); // get more accurate timestamp for the data sample
				/* The requestFrom is a complete operation. It sends the Start, IC2Addr, Read bit,
				 *    does the specified number of reads storing the results into a buffer, then sends
				 *    the stop. You access the results from the buffer by making repeated calls to
				 *    Wire.read(). The call to Wire.read() does not actually do the read, but retrieves
				 *    a value already read and stored in that buffer.
				 */
				Wire.requestFrom(pcf8591.getAddr(), BYTES_TO_READ, true /* send stop */); // takes 600us(5 bytes)
				digitalWrite(digital_pin, HIGH); // power off the IR LED

				// Discard the first sample, and find the maximum value of the other samples.
				int a = 0;	// final result
				int next_reading = 0;
				for (size_t i = 0; i < BYTES_TO_READ; i++) {
					next_reading = Wire.read();
					if (i > 0) { // discard first reading
						if (next_reading > a) a = next_reading;
					}
				}
				DEBUGPRINT(DebugLevel::SHARPGP2Y10, "P" + String(sensorPortNumber) + " GP2Y10: " + String(a));

				// Process the result (accumulate results over 2 second period)
				generic_accumulator += a;
				DEBUGPRINT(DebugLevel::SHARPGP2Y10,
						"\tacc=" + String(a) + "/" + String(reading_count_within_cycle));

				reading_count_within_cycle++;
				if (reading_count_within_cycle > GP2Y10_SAMPLES_PER_CYCLE) {
					reading_count++;
					a = generic_accumulator;
					generic_accumulator = 0;
					reading_count_within_cycle = 0;
					last_reading_timestamp_us = now; // timestamp for this long cycle reading

					// Convert accumulated ADC count value to an average over the last N samples
					double a0 = static_cast<double>(a) / (GP2Y10_SAMPLES_PER_CYCLE - 1);
					DEBUGPRINT(DebugLevel::SHARPGP2Y10, "\tavg=" + String(a0));
					// convert the average ADC count value into a voltage
					a0 = a0 * pcf8591.getVoltsPerCount(); // convert to voltage [volts]
							// Store the raw voltage value
					setRawAnalog(static_cast<float>(a0));
					DEBUGPRINT(DebugLevel::SHARPGP2Y10, "\tvolts=" + String(a0));

					// Convert the voltage into the dust density as ug/m^3
					// linear equation is from Chris Nafis http://www.howmuchsnow.com/arduino/airquality/
					double a1 = 172 * a0 - 99.9; // convert output volts to dust density in ug/m^3

					// Apply the calibration
					double a2 = a1 * a1; // a1^2: precalculate square term
					double a3 = a2 * a1; // A1^3: precalculate cubed term

					a1 = getCal(TEMP_CAL_CHANNEL_ANALOG_OFFSET)
							+ a1 * getCal(TEMP_CAL_CHANNEL_ANALOG_FIRST_ORDER)
							+ a2 * getCal(TEMP_CAL_CHANNEL_ANALOG_SECOND_ORDER)
							+ a3 * getCal(TEMP_CAL_CHANNEL_ANALOG_THIRD_ORDER);

					// Calibration corrected values
					setAnalog(static_cast<float>(a1));
					DEBUGPRINT(DebugLevel::SHARPGP2Y10, "\tug/m^3=" + String(a1));
				}
				DEBUGPRINTLN(DebugLevel::SHARPGP2Y10, "");
			}
		}
		////////////////////////////////////////////////////////////////////////////////////////////
		if (getModule() == sensorModule::taskclock) {
			return true; // do nothing
		}
	}
	return true;
}

bool GenericSensor::acquire2(void) {
	return acquire_setup();
}
