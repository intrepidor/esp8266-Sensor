/*
 * pcf8591.h
 *
 *  Created on: Aug 24, 2016
 *      Author: allan
 */

#ifndef PCF8591_H_
#define PCF8591_H_

#include <Arduino.h>
#include <Wire.h>

extern void test_pcf8591(void);

// ERROR codes
const int PCF8591_ERROR_READ = -1;

// Misc constants
const int PCF8591_MAX_RANGE = 256;
const float PCF8591_VOLTAGE = 3.3F;
const int PCF8591_ADC_MAX_CHANNELS = 4;

/* I2C Address = 1001abcr
 *   bit 7 = 1
 *   bit 6 = 0
 *   bit 5 = 0
 *   bit 4 = 1
 *   bit 3 = A2 (set to zero for YL-40 module)
 *   bit 2 = A1 (set to zero for TL-40 module)
 *   bit 1 = A0 (set to zero for TL-40 module)
 *   bit 0 = R/~W
 */
const uint8_t PCF8591_I2C_BASE_ADDR = 0x48; // 7-bit I2C address, excludes R/~W bit 0
const uint8_t PCF8591_READ = 0x01;	// bit 0 = 1
const uint8_t PCF8591_WRITE = 0x00;	// bit 0 = 0

// MASKS for the Control register
//   bit 7   = 0 not used
//   bit 6   = Analog output enable flag (when set)
//   bit 5|4 = AnInput type (00 = 4 single-ended inputs)
//   bit 3   = 0 not used
//   bit 2   = auto increment flag (when set)
//   bit 1|0 = ADC channel number (chan0=00, chan1=01, chan2=10, chan3=11)
const uint8_t PCF8591_ANALOG_OUTPUT_ENABLE_MASK = 0x40;
const uint8_t PCF8591_ANALOG_INPUT_MODE_MASK = 0x30;
const uint8_t PCF8591_AUTO_INCREMENT_CHANNEL_MASK = 0x04;
const uint8_t PCF8591_ADC_CHANNEL_NUMBER_MASK = 0x03;

// Analog input choices
enum pcf8591_AnInput_Type
	: uint8_t {
		// Bits 5 and 4 of the control register control the types of inputs
	four_single_ended = 0x00,
	three_differential = 0x10,
	single_and_diff_mixed = 0x20,
	two_differential = 0x30
};

class PCF8591 {

private:
	int lastADCReading[PCF8591_ADC_MAX_CHANNELS];
	pcf8591_AnInput_Type inputType;
	uint8_t analog_output_status; // 0x00 = off, 0x40 = on
	uint8_t auto_increment_channel_status; // 0x00 = off, 0x40 = on

	uint8_t i2cAddress;
	uint8_t configReg;
	uint8_t dacReg;

public:

	~PCF8591() {
	}
	PCF8591() {
		i2cAddress = PCF8591_I2C_BASE_ADDR;
		setDefaults();
	}
	PCF8591(uint8_t _i2caddr) {
		i2cAddress = _i2caddr;
		setDefaults();
	}
	void setDefaults(void) {
		dacReg = 0;
		disableDAC();
		disableAutoIncrementChannel();
		setInputs(pcf8591_AnInput_Type::four_single_ended);
	}

	//
	void setInputs(pcf8591_AnInput_Type a) {
		inputType = a;
	}
	pcf8591_AnInput_Type getInputs(void) {
		return inputType;
	}

	//
	void enableDAC(void) {
		analog_output_status = PCF8591_ANALOG_OUTPUT_ENABLE_MASK;
	}
	void disableDAC(void) {
		analog_output_status = 0;
	}
	bool getDACStatus(void) {
		if (analog_output_status) return true;
		else return false;
	}

	//
	void enableAutoIncrementChannel(void) {
		auto_increment_channel_status = PCF8591_AUTO_INCREMENT_CHANNEL_MASK;
	}
	void disableAutoIncrementChannel(void) {
		analog_output_status = 0;
	}
	bool getAutoIncrementChannel(void) {
		if (analog_output_status) return true;
		else return false;
	}

	//
	void begin(void);
	void begin(int _sda, int _scl) {
		Wire.begin(_sda, _scl);
	}

	bool config(void) {
		return config(0);
	}
	bool config(int channel);

	int readADC(const int _channel /* 0 to 3 */);

	void writeDAC(uint8_t dac) {
		dacReg = dac;
		config();
	}

	float getVoltsPerCount(void) {
		return PCF8591_VOLTAGE / PCF8591_MAX_RANGE;
	}
};

#endif /* PCF8591_H_ */
