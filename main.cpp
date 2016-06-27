#include <Arduino.h>
#include <EEPROM.h>
#include <Esp.h>
#include <Ticker.h>
#include "main.h"
#include "temperature.h"
#include "network.h"
#include "net_value.h"
#include "Queue.h"
#include "deviceinfo.h"
#include "sensor.h"
#include "util.h"
#include "debugprint.h"
#include "thingspeak.h"
#include "wdog.h"
//#include <src/internal/gdbstub.h>
#ifdef __cplusplus
extern "C" {
#endif

	void gdbstub_init();

#ifdef __cplusplus
}
#endif

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
String ProgramInfo("Environment Sensor v0.04 : Allan Inda 2016-June-26");

// Other
long count = 0;
// PIR Sensor
int PIRcount = 0;     // current PIR count
int PIRcountLast = 0; // last PIR count
unsigned long startup_millis = millis();

//----------------------------------------------------
// D0..D16, SDA, SCL defined by arduino.h
//
const uint8_t PIN_PIRSENSOR = D1;

// Reset and LED
const uint8_t PIN_SOFTRESET = D0;
const uint8_t PIN_BUILTIN_LED = BUILTIN_LED; // D0

// Port Pins
const uint8_t DIGITAL_PIN_1 = D2;
const uint8_t DIGITAL_PIN_2 = D3;
const uint8_t DIGITAL_PIN_3 = D6;
const uint8_t DIGITAL_PIN_4 = D7;
const uint8_t WATCHDOG_WOUT_PIN = D7; // toggle this pin for the external watchdog

const uint8_t ANALOG_PIN = A0;
const uint8_t I2C_SDA_PIN = SDA;
const uint8_t I2C_SCL_PIN = SCL;
//--------------------------------------------------

// LED on the ESP board
#define BUILTIN_LED_ON LOW
#define BUILTIN_LED_OFF HIGH

// Create Objects --- note the order of creation is important
DebugPrint debug;	// This must appear before Sensor and Device class declarations.

const int SENSOR_COUNT = 4; // should be at least equal to the value of MAX_PORTS
Sensor* sensors[SENSOR_COUNT] = { nullptr, nullptr, nullptr, nullptr };

Device dinfo; // create AFTER Sensor declaration above

///////////////////////////////////////////////////////////////////////////////////////////////
//lint -e{1784}   // suppress message about signature conflict between C++ and C
void loop(void) {
}

//lint -e{1784}   // suppress message about signature conflict between C++ and C
void setup(void) {

// Setup GPIO
	pinMode(PIN_PIRSENSOR, INPUT_PULLUP); // Initialize the PIR sensor pin as an input
	pinMode(PIN_BUILTIN_LED, OUTPUT); // Initialize the BUILTIN_LED pin as an output

// Signal that setup is proceeding
	digitalWrite(PIN_BUILTIN_LED, BUILTIN_LED_ON);

// Setup Serial port
	Serial.begin(115200);

// Start the external Watchdog. This also configures the Software Watchdog.
	//reset_config();
	kickExternalWatchdog(); // The first kick calls setup and starts the external and software watchdogs.
	Serial.print("Starting ... ");
	Serial.println(millis());

// Start the Debugger
//	gdbstub_init();

// Finish any object initializations -- stuff that could not go into the constructors
	dinfo.init(); // this must be done before calling any other member functions

// Start EEPROM and setup the Persisted Database
	EEPROM.begin(512);
// Copy persisted data from EEPROM into RAM
	dinfo.restoreDatabaseFromEEPROM();
// Get DebugLevel from EEPROM
// Copy debug level from RAM, which was just copied from the EEPROM
	debug.setDebugLevel(static_cast<DebugLevel>(dinfo.getDebugLevel()));
// write it back to dinfo since debug would have validated it, and potentially changed it to fix errors.
	dinfo.setDebugLevel(static_cast<int>(debug.getDebugLevel()));
// if value is invalid, then fix it and rewrite the EEPROM
	if (eeprom_is_dirty) {
		dinfo.saveDatabaseToEEPROM();
	}

// Setup the WebServer
	kickAllWatchdogs();
	WebInit(); // Calls WiFiManager to make sure connected to access point.
	kickAllWatchdogs();

// Configure Sensors
	ConfigurePorts();
	kickAllWatchdogs();

// Configure Scheduler
	Queue myQueue;
// scheduleFunction (function pointer, task name, start delay in ms, repeat interval in ms)
	myQueue.scheduleFunction(task_readpir, "PIR", 500, 50);
	myQueue.scheduleFunction(task_acquire, "acquire", 1000, 499);
	myQueue.scheduleFunction(task_updatethingspeak, "thingspeak", 2000, 20000);
	myQueue.scheduleFunction(task_flashled, "led", 250, 100);
	myQueue.scheduleFunction(task_serialport_menu, "menu", 2000, 500);
	myQueue.scheduleFunction(task_webServer, "webserver", 3000, 10);
	kickAllWatchdogs();

// Print boot up information and menu
	printInfo();
	printMenu();
	kickAllWatchdogs();

// Signal that setup is complete
	digitalWrite(PIN_BUILTIN_LED, BUILTIN_LED_OFF);

// Start the task scheduler
	setStartupComplete(); // tell the watchdog the long startup is done and to use normal watchdog timeouts
	for (;;) {
		softwareWatchdog(); // must call minimally every 1.6s to avoid the external watchdog reseting the ESP8266
		myQueue.Run(millis());
		yield(); // CONSIDER using an optimistic_yield(xx) instead so background tasks can still run
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
		debug.println(DebugLevel::DEBUGMORE, "portNumber=" + String(portNumber));
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
				debug.println(DebugLevel::ERROR,
						"ERROR: ConfigurePorts() - port number out of range in switch -"
								+ String(portNumber));
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
				debug.println(DebugLevel::INFO,
						"Configuring Port#" + String(portNumber) + ", name= " + String(name) + ", type="
								+ getModuleNameString(static_cast<sensorModule>(portType)));
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
									"ERROR: ConfigurePorts() - sensorModule not found in switch\n ... Created DHT11 instead.");
							break;
					} // switch (portType)
				} // if (portNumber...)
				else {
					debug.println(DebugLevel::ERROR,
							"ERROR: ConfigurePort() - portNumber out of range - " + String(portNumber));
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
			for (int i = 0; i < getSensorCalCount() && i < dinfo.getPortAdjMax(); i++) {
				sensors[idx]->setCal(i, static_cast<float>(dinfo.getPortAdj(idx, i)));
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
String getUptime(void) {
	const unsigned long MS_PER_SECOND = (1000);
	const unsigned long MS_PER_MINUTE = (MS_PER_SECOND * 60);
	const unsigned long MS_PER_HOUR = (MS_PER_MINUTE * 60);
	const unsigned long MS_PER_DAY = (MS_PER_HOUR * 24);
	unsigned long uptime = millis() - startup_millis;
	String s(String(uptime) + " ms, ");
	s += String(uptime / MS_PER_DAY) + "d, ";
	s += String(uptime / MS_PER_HOUR) + "h, ";
	s += String(uptime / MS_PER_MINUTE) + "m, ";
	s += String(uptime / MS_PER_SECOND) + "s";
	// Example: 897234 ms, 10d, 3h, 33m, 3s
	return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////
String getsDeviceInfo(String eol) {
	String s(ProgramInfo + eol);
	s += "Device IP: " + localIPstr() + eol;
	s += "SSID: " + WiFi.SSID() + eol;
	s += "ESP8266_Device_ID=" + String(dinfo.getDeviceID()) + eol;
	s += "Friendly Name: " + String(dinfo.getDeviceName()) + eol;
	s += "Uptime: " + getUptime() + eol;
	return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////
void printInfo(void) {
// Print useful Information
	debug.println(DebugLevel::ALWAYS, getsDeviceInfo(nl));
	debug.println(DebugLevel::ALWAYS, getsThingspeakInfo(nl));
	debug.println(DebugLevel::ALWAYS, "== Port Information ==");
	dinfo.printInfo();
}

///////////////////////////////////////////////////////////////////////////////////////////////
int task_readpir(unsigned long now) {
//lint --e{715}  Ignore unused function arguments
	wdog_timer[static_cast<int>(taskname_pir)] = millis();
	if (digitalRead(PIN_PIRSENSOR)) {
		PIRcount++;
	}
	return 0;
}

int task_acquire(unsigned long now) {
//lint --e{715}  Ignore unused function arguments
	static int next_acquire_number = 0;
	wdog_timer[static_cast<int>(taskname_acquire)] = millis();
	const int MAX_ACQUIRES = ACQUIRES_PER_SENSOR * SENSOR_COUNT;

	if (next_acquire_number < 0 || next_acquire_number > (MAX_ACQUIRES - 1)) {
		next_acquire_number = 0;
	}
	int sensor_number = next_acquire_number / ACQUIRES_PER_SENSOR;
	int subacquire_number = next_acquire_number - int((sensor_number * ACQUIRES_PER_SENSOR));

	switch (subacquire_number) {
		case 0:
			sensors[sensor_number]->acquire_setup();
			break;
		case 1:
			sensors[sensor_number]->acquire1();
			break;
		case 2:
			sensors[sensor_number]->acquire2();
			break;
		default:
			break;
	}
	next_acquire_number++;
	return next_acquire_number;
}

int task_updatethingspeak(unsigned long now) {
//lint --e{715}  Ignore unused function arguments
	wdog_timer[static_cast<int>(taskname_thingspeak)] = millis();
	if (dinfo.getThingspeakEnable()) {
		updateThingspeak();
	}
	PIRcountLast = PIRcount;
	PIRcount = 0; // reset counter for starting a new period
	return 0;
}

int task_flashled(unsigned long now) {
//lint --e{715}  Ignore unused function arguments
	wdog_timer[static_cast<int>(taskname_led)] = millis();
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
	debug.println(DebugLevel::ALWAYS, "S  EspClass::reset()");
	debug.println(DebugLevel::ALWAYS, "A  EspClass::restart()");
	debug.println(DebugLevel::ALWAYS, "W  Test Watchdog (block here forever)");
	debug.println(DebugLevel::ALWAYS,
			"D  [" + debug.getDebugLevelString() + "] Debug level for logging to serial port");
	debug.println(DebugLevel::ALWAYS, "");
}

int task_serialport_menu(unsigned long now) {
//lint --e{715}  Ignore unused function arguments
	wdog_timer[static_cast<int>(taskname_menu)] = millis();
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
					debug.print(DebugLevel::ALWAYS, "CNT\tPIR/Last\t");
					for (int s = 0; s < SENSOR_COUNT; s++) {
						if (sensors[s]) {
							for (int v = 0; v < getSensorValueCount(); v++) {
								if (sensors[s]->getValueEnable(v)) {
									debug.print(DebugLevel::ALWAYS,
											"Raw/" + sensors[s]->getValueName(v) + "\t");
								}
							}
						}
					}
					debug.println(DebugLevel::ALWAYS, "");
				}
				// Display the Data
				debug.print(DebugLevel::ALWAYS, "#" + String(count) + "\t");
				debug.print(DebugLevel::ALWAYS, String(PIRcount) + "/");
				debug.print(DebugLevel::ALWAYS, String(PIRcountLast) + "\t\t");
				for (int s = 0; s < SENSOR_COUNT; s++) {
					if (sensors[s]) {
						for (int v = 0; v < getSensorValueCount(); v++) {
							if (sensors[s]->getValueEnable(v)) {
								debug.print(DebugLevel::ALWAYS,
										String(sensors[s]->getRawValue(v)) + String("/"));
								debug.print(DebugLevel::ALWAYS,
										String(sensors[s]->getValue(v)) + String("\t"));
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
				debug.println(DebugLevel::ALWAYS, "Debug Level set to: " + debug.getDebugLevelString());
				if (eeprom_is_dirty) dinfo.saveDatabaseToEEPROM();
				break;
			case 'E': // Read the EEPROM into the RAM data structure, then dump the contents
				debug.println(DebugLevel::ALWAYS, nl + "=== Data in EEPROM ===");
				dinfo.restoreDatabaseFromEEPROM();
				debug.println(DebugLevel::ALWAYS, dinfo.databaseToString(nl.c_str()));
				debug.println(DebugLevel::ALWAYS, "");
				break;
			case 'M': // Just dump the contents of the RAM data structure
				debug.println(DebugLevel::ALWAYS, nl + "=== Data in RAM ===");
				debug.println(DebugLevel::ALWAYS, dinfo.databaseToString(nl.c_str()));
				debug.println(DebugLevel::ALWAYS, "");
				break;
			case 'I':
				debug.println(DebugLevel::ALWAYS, "CycleCount:\t\t" + String(ESP.getCycleCount()));
				debug.println(DebugLevel::ALWAYS, "Vcc:\t\t\t" + String(ESP.getVcc()));
				debug.print(DebugLevel::ALWAYS, "FreeHeap:\t\t");
				debug.println(DebugLevel::ALWAYS, ESP.getFreeHeap(), HEX);
				debug.print(DebugLevel::ALWAYS, "ChipId:\t\t\t");
				debug.println(DebugLevel::ALWAYS, ESP.getChipId(), HEX);
				debug.println(DebugLevel::ALWAYS, "SdkVersion:\t\t" + String(ESP.getSdkVersion()));
				debug.println(DebugLevel::ALWAYS, "BootVersion:\t\t" + String(ESP.getBootVersion()));
				debug.println(DebugLevel::ALWAYS, "BootMode:\t\t" + String(ESP.getBootMode()));
				debug.println(DebugLevel::ALWAYS, "CpuFreqMHz:\t\t" + String(ESP.getCpuFreqMHz()));
				debug.print(DebugLevel::ALWAYS, "FlashChipId:\t\t");
				debug.println(DebugLevel::ALWAYS, ESP.getFlashChipId(), HEX);
				debug.print(DebugLevel::ALWAYS, "FlashChipRealSize[Mbit]:");
				debug.println(DebugLevel::ALWAYS, ESP.getFlashChipRealSize(), HEX);
				debug.print(DebugLevel::ALWAYS, "FlashChipSize [MBit]:\t");
				debug.println(DebugLevel::ALWAYS, ESP.getFlashChipSize(), HEX);
				debug.println(DebugLevel::ALWAYS, "FlashChipSpeed [Hz]:\t" + String(ESP.getFlashChipSpeed()));
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
				debug.println(DebugLevel::ALWAYS, "Last Reset --> " + ESP.getResetInfo());
				debug.println(DebugLevel::ALWAYS, "");
				break;
			case 'S':
				debug.println(DebugLevel::ALWAYS, "Calling EspClass::reset() in 2 seconds ...");
				delay(2000);
				ESP.reset();
				break;
			case 'A':
				debug.println(DebugLevel::ALWAYS, "Calling EspClass::restart() in 2 seconds ...");
				delay(2000);
				ESP.restart();
				break;
			case 'W':
				debug.println(DebugLevel::ALWAYS,
						"Looping here forever - Software Watchdog for Menu should trip ... ");
				for (;;) {
					yield();
				} // loop forever so the software watchdog for this task trips, but yield to the ESP OS
				break;
			default:
				debug.println(DebugLevel::ALWAYS, "Unknown command: " + String(ch));
				break;
		}
	}
	return 0;
}

int task_webServer(unsigned long now) {
//lint --e{715}  Ignore unused function arguments
	wdog_timer[static_cast<int>(taskname_webserver)] = millis();
	WebWorker();
	return 0;
}

