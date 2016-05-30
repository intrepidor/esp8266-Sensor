#include <Arduino.h>
#include <EEPROM.h>
#include <Esp.h>
#include "main.h"
#include "temperature.h"
#include "network.h"
#include "net_value.h"
#include "Queue.h"
#include "deviceinfo.h"
#include "sensor.h"
#include "util.h"
#include "reset.h"
#include "debugprint.h"

// Forward declarations
extern int task_readpir(unsigned long now);
extern int task_acquire(unsigned long now);
extern int task_updatethingspeak(unsigned long now);
extern int task_flashled(unsigned long now);
extern int task_serialport_menu(unsigned long now);
extern int task_webServer(unsigned long now);

extern void printMenu(void);
extern void printExtendedMenu(void);
extern void printInfo(void);
extern void ConfigurePorts(void);

// -----------------------
// Custom configuration
// -----------------------
String ProgramInfo("\r\nEnvironment Sensor v0.02 : Allan Inda 2016-May-29");

// Other
long count = 0;

// PIR Sensor
int PIRcount = 0;     // current PIR count
int PIRcountLast = 0; // last PIR count
const uint8_t PIN_PIRSENSOR = D1;

// Reset and LED
const uint8_t PIN_SOFTRESET = D0;
const uint8_t PIN_BUILTIN_LED = BUILTIN_LED; // D0

// Port Pins
const uint8_t DIGITAL_PIN_1 = D2;
const uint8_t DIGITAL_PIN_2 = D3;
const uint8_t DIGITAL_PIN_3 = D6;
const uint8_t DIGITAL_PIN_4 = D7;
const uint8_t DIGITAL_PIN_5 = D8;

const uint8_t ANALOG_PIN = A0;
const uint8_t I2C_SDA_PIN = D4;
const uint8_t I2C_SCL_PIN = D5;

// LED on the ESP board
#define BUILTIN_LED_ON LOW
#define BUILTIN_LED_OFF HIGH

// Create Objects
const int SENSOR_COUNT = 4; // should be at least equal to the value of MAX_PORTS
Sensor* sensors[SENSOR_COUNT] = { nullptr, nullptr, nullptr, nullptr };
Device dinfo;
DebugPrint debug;

///////////////////////////////////////////////////////////////////////////////////////////////
//lint -e{1784}   // suppress message about signature conflict between C++ and C
void loop(void) {
}

//lint -e{1784}   // suppress message about signature conflict between C++ and C
void setup(void) {

// Setup Reset circuit
	reset_config();

// Setup GPIO
	pinMode(PIN_PIRSENSOR, INPUT);			// Initialize the PIR sensor pin as an input
	pinMode(PIN_BUILTIN_LED, OUTPUT);		// Initialize the BUILTIN_LED pin as an output

// Signal that setup is proceeding
	digitalWrite(PIN_BUILTIN_LED, BUILTIN_LED_ON);

// Setup Serial port
	Serial.begin(115200);

// Start EEPROM
	EEPROM.begin(512);
	// Copy persisted data from EEPROM into RAM
	dinfo.RestoreConfigurationFromEEPROM();
	debug.print(DebugLevel::ALWAYS, "Debug level started as [1 dinfo]: ");
	debug.println(DebugLevel::ALWAYS, dinfo.getDebugLevel());
	debug.print(DebugLevel::ALWAYS, "Debug level started as [2 debug]: ");
	debug.println(DebugLevel::ALWAYS, debug.getDebugLevelString());

	// Get DebugLevel from EEPROM
	// Copy debug level from RAM, which was just copied from the EEPROM
	debug.setDebugLevel(static_cast<DebugLevel>(dinfo.getDebugLevel()));
	// write it back to dinfo since debug would have validated it, and potentially changed it to fix errors.
	dinfo.setDebugLevel(static_cast<int>(debug.getDebugLevel()));
	debug.print(DebugLevel::ALWAYS, "Debug level now [1 dinfo]: ");
	debug.println(DebugLevel::ALWAYS, dinfo.getDebugLevel());
	debug.print(DebugLevel::ALWAYS, "Debug level now [2 debug]: ");
	debug.println(DebugLevel::ALWAYS, debug.getDebugLevelString());
	// if value is invalid, then fix it and rewrite the EEPROM
	if (eeprom_is_dirty) {
		dinfo.StoreConfigurationIntoEEPROM();
	}

// Setup the WebServer
	WebInit();

// Configure Objects
	//dinfo.init(); // commented out -- causes EEPROM to get reset, not sure why
	ConfigurePorts();
	printInfo();

// Configure Scheduler
	Queue myQueue;
	// scheduleFunction (function pointer, task name, start delay in ms, repeat interval in ms)
	myQueue.scheduleFunction(task_readpir, "PIR", 500, 50);
	myQueue.scheduleFunction(task_acquire, "Temperature", 1000, 499);
	myQueue.scheduleFunction(task_updatethingspeak, "Thingspeak", 1500, 10000);
	myQueue.scheduleFunction(task_flashled, "LED", 250, 1000);
	myQueue.scheduleFunction(task_serialport_menu, "Status", 2000, 500);
	myQueue.scheduleFunction(task_webServer, "WebServer", 3000, 1);

// Signal that setup is done
	digitalWrite(PIN_BUILTIN_LED, BUILTIN_LED_OFF);
	printMenu();

	for (;;) {
		myQueue.Run(millis());
		delay(10);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
void ConfigurePorts(void) {
	// Each port has access to a predefined subset of the following pin types, and for
	//    each port, the pin assignments may differ, and some will be the same.
	SensorPins p;

	// Loop through each of the ports
	for (int portNumber = 0; portNumber < dinfo.getPortMax(); portNumber++) {
		// Get the pins used for this port
		debug.print(DebugLevel::DEBUGMORE, "portNumber=");
		debug.println(DebugLevel::DEBUGMORE, portNumber);
		switch (portNumber) {
			case 0: // port#0
				p.digital = DIGITAL_PIN_1;
				p.analog = ANALOG_PIN;
				p.sda = I2C_SDA_PIN;
				p.scl = I2C_SCL_PIN;
				;
				break;
			case 1: // port#1
				p.digital = DIGITAL_PIN_2;
				p.analog = ANALOG_PIN;
				p.sda = I2C_SDA_PIN;
				p.scl = I2C_SCL_PIN;
				break;
			case 2: // port#2
				p.digital = DIGITAL_PIN_3;
				p.analog = ANALOG_PIN;
				p.sda = I2C_SDA_PIN;
				p.scl = I2C_SCL_PIN;
				break;
			case 3: // port#3
				p.digital = DIGITAL_PIN_4;
				p.analog = ANALOG_PIN;
				p.sda = I2C_SDA_PIN;
				p.scl = I2C_SCL_PIN;
				break;
			default:
				debug.print(DebugLevel::ERROR,
						"ERROR: ConfigurePorts() - port number out of range in switch -");
				debug.println(DebugLevel::ERROR, portNumber);
				break;
		}

		// Figure out the configuration of the port
		//lint -e{26} suppress error false error about the static_cast
		for (int portType = 0; portType < static_cast<int>(sensorModule::END); portType++) {
			// loop through each of the port setting types until finding a match
			//lint -e{26} suppress error false error about the static_cast
			if (dinfo.getPortMode(portNumber) == static_cast<sensorModule>(portType)) {
				char name[20];
				strncpy(name, sensorList[static_cast<int>(portType)].name, 19);
				name[19] = 0;
				// found the setting
				debug.print(DebugLevel::INFO, "Configuring Port#");
				debug.print(DebugLevel::INFO, portNumber);
				debug.print(DebugLevel::INFO, ", name= ");
				debug.print(DebugLevel::INFO, name);
				debug.print(DebugLevel::INFO, ", type=");
				debug.println(DebugLevel::INFO, c_getModuleName(static_cast<sensorModule>(portType)).c_str());
				//lint -e{30, 142} suppress error due to lint not understanding enum classes
				if (portNumber >= 0 && portNumber < SENSOR_COUNT) {
					switch (portType) {
//						case static_cast<int>(sensorModule::off):
//							break;
						case static_cast<int>(sensorModule::dht11):
							sensors[portNumber] = new TemperatureSensor();
							sensors[portNumber]->init(sensorModule::dht11, p);
							sensors[portNumber]->setName("DHT11");
							break;
						case static_cast<int>(sensorModule::dht22):
							sensors[portNumber] = new TemperatureSensor;
							sensors[portNumber]->init(sensorModule::dht22, p);
							sensors[portNumber]->setName("DHT22");
							break;
//						case static_cast<int>(sensorModule::ds18b20):
//							sensors[portNumber] = new TemperatureSensor;
//							sensors[portNumber]->init(sensorModule::ds18b20, p);
//							sensors[portNumber]->setName("DS18b20");
//							break;
//						case static_cast<int>(sensorModule::sonar):
//							break;
//						case static_cast<int>(sensorModule::sound):
//							break;
//						case static_cast<int>(sensorModule::reed):
//							break;
//						case static_cast<int>(sensorModule::hcs501):
//							break;
//						case static_cast<int>(sensorModule::hcsr505):
//							break;
//						case static_cast<int>(sensorModule::dust):
//							break;
//						case static_cast<int>(sensorModule::rain):
//							break;
//						case static_cast<int>(sensorModule::soil):
//							break;
//						case static_cast<int>(sensorModule::soundh):
//							break;
//						case static_cast<int>(sensorModule::methane):
//							break;
//						case static_cast<int>(sensorModule::gy68):
//							break;
//						case static_cast<int>(sensorModule::gy30):
//							break;
//						case static_cast<int>(sensorModule::lcd1602):
//							break;
//						case static_cast<int>(sensorModule::rfid):
//							break;
//						case static_cast<int>(sensorModule::marquee):
//							break;
						default:
							sensors[portNumber] = new TemperatureSensor();
							sensors[portNumber]->init(sensorModule::dht11, p);
							sensors[portNumber]->setName("DHT11");
							debug.println(DebugLevel::ERROR,
									"ERROR: ConfigurePorts() - sensorModule not found in switch");
							debug.println(DebugLevel::ERROR, " ... Created DHT11 instead.");
							break;
					} // switch (portType)
				} // if (portNumber...)
				else {
					debug.print(DebugLevel::ERROR, "ERROR: ConfigurePort() - portNumber out of range - ");
					debug.println(DebugLevel::ERROR, portNumber);
				}
			} // if (portMode ...)
		} // for (portType ...)

	} // for (portNumber ...)

	// Copy the calibration data to the sensors
	CopyCalibrationDataToSensors();
}

void CopyCalibrationDataToSensors(void) {
	// Note: This routine assumes the SENSOR_COUNT and getPortMax() are identical values
	for (int idx = 0; idx < SENSOR_COUNT && idx < dinfo.getPortMax(); idx++) {
		if (sensors[idx] /* make sure it exists*/) {
			// copy each of the multiple calibration values into the sensor
			for (int i = 0; i < getCalCount() && i < dinfo.getPortAdjMax(); i++) {
				sensors[idx]->setCal(i, static_cast<float>(dinfo.getPortAdj(idx, i)));
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
void printInfo(void) {
// Print useful Information
	debug.println(DebugLevel::ALWAYS, ProgramInfo);
	debug.println(DebugLevel::ALWAYS, "Device IP: " + localIPstr());
	dinfo.printThingspeakInfo();
	debug.print(DebugLevel::ALWAYS, "ESP8266_Device_ID=" + String(dinfo.getDeviceID()));
	debug.println(DebugLevel::ALWAYS, nl + "Friendly Name: " + String(dinfo.getDeviceName()));
	dinfo.printInfo();
}

///////////////////////////////////////////////////////////////////////////////////////////////
int task_readpir(unsigned long now) {
//lint --e{715}  Ignore unused function arguments
	if (digitalRead(PIN_PIRSENSOR)) {
		PIRcount++;
	}
	return 0;
}

int task_acquire(unsigned long now) {
//lint --e{715}  Ignore unused function arguments
	unsigned int r = 0;
	for (int i = 0; i < SENSOR_COUNT; i++) {
		if (sensors[i]) {
			if (sensors[i]->acquire()) {
				r |= (1U << i);
			}
		}
	}
	return static_cast<int>(r);
}

int task_updatethingspeak(unsigned long now) {
//lint --e{715}  Ignore unused function arguments
	if (dinfo.getEnable()) {
		dinfo.updateThingspeak();
	}
	PIRcountLast = PIRcount;
	PIRcount = 0; // reset counter for starting a new period
	return 0;
}

int task_flashled(unsigned long now) {
//lint --e{715}  Ignore unused function arguments
	static uint8_t current_state = 0;
	if (current_state == 0) {
		digitalWrite(BUILTIN_LED, BUILTIN_LED_ON);
		current_state = 1;
	}
	else {
		digitalWrite(BUILTIN_LED, BUILTIN_LED_OFF);
		current_state = 0;
	}
	return 0;
}

void printMenu(void) {
	debug.println(DebugLevel::ALWAYS, "MENU ----------------------");
	debug.println(DebugLevel::ALWAYS, "?  show this menu");
	debug.println(DebugLevel::ALWAYS, "i  show High-level configuration");
	debug.println(DebugLevel::ALWAYS, "c  show calibration values");
	debug.println(DebugLevel::ALWAYS, "m  show measured values");
	debug.println(DebugLevel::ALWAYS, "s  show status");
	debug.println(DebugLevel::ALWAYS, "w  show web URLs");
	debug.println(DebugLevel::ALWAYS, "z  Extended menu");
	debug.println(DebugLevel::ALWAYS, "");
}

void printExtendedMenu(void) {
	debug.println(DebugLevel::ALWAYS, "MENU EXTENDED -------------");
	debug.println(DebugLevel::ALWAYS, "E  show data structure in EEPROM");
	debug.println(DebugLevel::ALWAYS, "M  show data structure in RAM");
	debug.println(DebugLevel::ALWAYS, "R  show reason for last reset");
	debug.println(DebugLevel::ALWAYS, "I  show ESP information");
	debug.println(DebugLevel::ALWAYS, "Q  reset()");
	debug.println(DebugLevel::ALWAYS, "W  EspClass::reset()");
	debug.println(DebugLevel::ALWAYS, "A  EspClass::restart()");
	debug.print(DebugLevel::ALWAYS, "D  [");
	debug.print(DebugLevel::ALWAYS, debug.getDebugLevelString());
	debug.println(DebugLevel::ALWAYS, "] Debug level for logging to serial port");
	debug.println(DebugLevel::ALWAYS, "");
}

int task_serialport_menu(unsigned long now) {
//lint --e{715}  Ignore unused function arguments
	count++;
	static bool need_new_heading = true;

	if (Serial.available() > 0) {
		char ch = static_cast<char>(Serial.read());
		if (ch != 's') {
			need_new_heading = true;
		}
		switch (ch) {
			case '?':
				printMenu();
				break;
			case 'z':
				printExtendedMenu();
				break;
			case 'i':
				printInfo();
				break;
			case 's':
				// Display the heading
				if (need_new_heading) {
					need_new_heading = false;
					debug.print(DebugLevel::ALWAYS, "CNT\tMotion\tLast\t");
					for (int s = 0; s < SENSOR_COUNT; s++) {
						if (sensors[s]) {
							for (int v = 0; v < getValueCount(); v++) {
								if (sensors[s]->getValueEnable(v)) {
									debug.print(DebugLevel::ALWAYS, sensors[s]->getValueName(v));
									debug.print(DebugLevel::ALWAYS, "\t");
								}
							}
						}
					}
					debug.println(DebugLevel::ALWAYS, "");
				}
				// Display the Data
				debug.print(DebugLevel::ALWAYS, "#");
				debug.print(DebugLevel::ALWAYS, count);
				debug.print(DebugLevel::ALWAYS, "\t");
				debug.print(DebugLevel::ALWAYS, PIRcount);
				debug.print(DebugLevel::ALWAYS, "\t");
				debug.print(DebugLevel::ALWAYS, PIRcountLast);
				debug.print(DebugLevel::ALWAYS, "\t");
				for (int s = 0; s < SENSOR_COUNT; s++) {
					if (sensors[s]) {
						for (int v = 0; v < getValueCount(); v++) {
							if (sensors[s]->getValueEnable(v)) {
								debug.print(DebugLevel::ALWAYS, sensors[s]->getValue(v));
								debug.print(DebugLevel::ALWAYS, "\t");
							}
						}
					}
				}
				debug.println(DebugLevel::ALWAYS, "");
				break;
			case 'c':
				debug.println(DebugLevel::ALWAYS, "=== Calibration Data ===");
				for (int i = 0; i < SENSOR_COUNT; i++) {
					if (sensors[i]) {
						sensors[i]->printCals();
					}
				}
				break;
			case 'm':
				debug.println(DebugLevel::ALWAYS, "=== Value Data ===");
				for (int i = 0; i < SENSOR_COUNT; i++) {
					if (sensors[i]) {
						sensors[i]->printValues();
					}
				}
				break;
			case 'w':
				debug.println(DebugLevel::ALWAYS, WebPrintInfo(nl));
				break;
///// Extended Menu ////
			case 'D':
				dinfo.setDebugLevel(static_cast<int>(debug.incrementDebugLevel()));
				debug.print(DebugLevel::ALWAYS, "Debug Level set to: ");
				debug.println(DebugLevel::ALWAYS, debug.getDebugLevelString());
				if (eeprom_is_dirty) dinfo.StoreConfigurationIntoEEPROM();
				break;
			case 'E': // Read the EEPROM into the RAM data structure, then dump the contents
				debug.println(DebugLevel::ALWAYS, nl + "=== Data in EEPROM ===");
				dinfo.RestoreConfigurationFromEEPROM();
				debug.println(DebugLevel::ALWAYS, dinfo.toString(nl.c_str()).c_str());
				debug.println(DebugLevel::ALWAYS, "");
				break;
			case 'M': // Just dump the contents of the RAM data structure
				debug.println(DebugLevel::ALWAYS, nl + "=== Data in RAM ===");
				debug.println(DebugLevel::ALWAYS, dinfo.toString(nl.c_str()).c_str());
				debug.println(DebugLevel::ALWAYS, "");
				break;
			case 'I':
				debug.print(DebugLevel::ALWAYS, "CycleCount:\t\t");
				debug.println(DebugLevel::ALWAYS, ESP.getCycleCount());
				debug.print(DebugLevel::ALWAYS, "Vcc:\t\t\t");
				debug.println(DebugLevel::ALWAYS, ESP.getVcc());
				debug.print(DebugLevel::ALWAYS, "FreeHeap:\t\t");
				debug.println(DebugLevel::ALWAYS, ESP.getFreeHeap(), HEX);
				debug.print(DebugLevel::ALWAYS, "ChipId:\t\t\t");
				debug.println(DebugLevel::ALWAYS, ESP.getChipId(), HEX);
				debug.print(DebugLevel::ALWAYS, "SdkVersion:\t\t");
				debug.println(DebugLevel::ALWAYS, ESP.getSdkVersion());
				debug.print(DebugLevel::ALWAYS, "BootVersion:\t\t");
				debug.println(DebugLevel::ALWAYS, ESP.getBootVersion());
				debug.print(DebugLevel::ALWAYS, "BootMode:\t\t");
				debug.println(DebugLevel::ALWAYS, ESP.getBootMode());
				debug.print(DebugLevel::ALWAYS, "CpuFreqMHz:\t\t");
				debug.println(DebugLevel::ALWAYS, ESP.getCpuFreqMHz());
				debug.print(DebugLevel::ALWAYS, "FlashChipId:\t\t");
				debug.println(DebugLevel::ALWAYS, ESP.getFlashChipId(), HEX);
				debug.print(DebugLevel::ALWAYS, "FlashChipRealSize[Mbit]:");
				debug.println(DebugLevel::ALWAYS, ESP.getFlashChipRealSize(), HEX);
				debug.print(DebugLevel::ALWAYS, "FlashChipSize [MBit]:\t");
				debug.println(DebugLevel::ALWAYS, ESP.getFlashChipSize(), HEX);
				debug.print(DebugLevel::ALWAYS, "FlashChipSpeed [Hz]:\t");
				debug.println(DebugLevel::ALWAYS, ESP.getFlashChipSpeed());
				debug.print(DebugLevel::ALWAYS, "FlashChipMode:\t\t");
				switch (ESP.getFlashChipMode()) {
					case FM_QIO:
						debug.println(DebugLevel::ALWAYS, "QIO");
						break;
					case FM_QOUT:
						debug.println(DebugLevel::ALWAYS, "QOUT");
						break;
					case FM_DIO:
						debug.println(DebugLevel::ALWAYS, "DIO");
						break;
					case FM_DOUT:
						debug.println(DebugLevel::ALWAYS, "DOUT");
						break;
					case FM_UNKNOWN:
					default:
						debug.println(DebugLevel::ALWAYS, "Unknown");
						break;
				}
				debug.print(DebugLevel::ALWAYS, "FlashChipSizeByChipId:\t");
				debug.println(DebugLevel::ALWAYS, ESP.getFlashChipSizeByChipId(), HEX);
				debug.print(DebugLevel::ALWAYS, "SketchSize [Bytes]:\t");
				debug.println(DebugLevel::ALWAYS, ESP.getSketchSize(), HEX);
				debug.print(DebugLevel::ALWAYS, "FreeSketchSpace [Bytes]:");
				debug.println(DebugLevel::ALWAYS, ESP.getFreeSketchSpace(), HEX);
				debug.println(DebugLevel::ALWAYS, "");
				break;
			case 'R':
				debug.print(DebugLevel::ALWAYS, "Last Reset --> ");
				debug.println(DebugLevel::ALWAYS, ESP.getResetInfo());
				debug.println(DebugLevel::ALWAYS, "");
				break;
			case 'Q':
				debug.println(DebugLevel::ALWAYS, "Calling reset() in 2 seconds ...");
				reset();
				break;
			case 'W':
				debug.println(DebugLevel::ALWAYS, "Calling EspClass::reset() in 2 seconds ...");
				delay(2000);
				ESP.reset();
				break;
			case 'A':
				debug.println(DebugLevel::ALWAYS, "Calling EspClass::restart() in 2 seconds ...");
				delay(2000);
				ESP.restart();
				break;
			default:
				debug.print(DebugLevel::ALWAYS, "Unknown command: ");
				debug.println(DebugLevel::ALWAYS, ch);
				break;
		}
	}
	return 0;
}

int task_webServer(unsigned long now) {
//lint --e{715}  Ignore unused function arguments
	WebWorker();
	return 0;
}

