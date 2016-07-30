#include <EEPROM.h>
#include <Esp.h>
#include <Ticker.h>
//#include <gdbstub.h>
#include "main.h"
#include "temperature.h"
#include "generic.h"
#include "network.h"
#include "net_value.h"
#include "Queue.h"
#include "deviceinfo.h"
#include "sensor.h"
#include "util.h"
#include "debugprint.h"
#include "thingspeak.h"
#include "wdog.h"
#include "util.h"

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
String ProgramInfo("Environment Sensor v0.12 Allan Inda 2016July24");

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
const uint8_t DIGITAL_PIN_4 = D8;
const uint8_t WATCHDOG_WOUT_PIN = D7; // toggle this pin for the external watchdog

const uint8_t ANALOG_PIN = A0;
const uint8_t I2C_SDA_PIN = 2; // Use number directly since arduino_pins.h for nodemcu has the SDA mapping wrong
const uint8_t I2C_SCL_PIN = 14; // Use number directly since arduino_pins.h for nodemcu has the SCL mapping wrong
//--------------------------------------------------

// LED on the ESP board
#define BUILTIN_LED_ON LOW
#define BUILTIN_LED_OFF HIGH

// Create Objects --- note the order of creation is important
DebugPrint debug;	// This must appear before Sensor and Device class declarations.

const int SENSOR_COUNT = 4; // should be at least equal to the value of MAX_PORTS
Sensor* sensors[SENSOR_COUNT] = { nullptr, nullptr, nullptr, nullptr };

Device dinfo; // create AFTER Sensor declaration above
Queue myQueue;
bool outputTaskClock = false;
uint8_t outputTaskClockPin = BUILTIN_LED;

//extern void pp_soft_wdt_stop();

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
//	// This device uses an external watchdog, so no need for the ESP version, and it will just
//	//  cause issues.
//	ESP.wdtDisable();
//	system_soft_wdt_stop();
//	pp_soft_wdt_stop();
//	reset_config();
	kickExternalWatchdog(); // The first kick calls setup and starts the external and software watchdogs.
	Serial.print(F("Starting ... "));
	Serial.println(millis());

// Start the Debugger
//	gdbstub_init();

// Finish any object initializations -- stuff that could not go into the constructors
	dinfo.init(); // this must be done before calling any other member functions

// Start EEPROM and setup the Persisted Database
	EEPROM.begin(1536);

	/* The SDA pin is supposed to be high when not used. If during startup the pin is held low,
	 * then erase the EEPROM.
	 */
	pinMode(I2C_SDA_PIN, INPUT);
	if (digitalRead(I2C_SDA_PIN) == 0) {
		Serial.println(F("SDA held low -- erasing the EEPROM"));
		dinfo.eraseEEPROM();
	}
	kickAllWatchdogs();

	/* Copy persisted data from EEPROM into RAM */
	dinfo.restoreDatabaseFromEEPROM();
	/* Get DebugLevel from EEPROM
	 * Copy debug level from RAM, which was just copied from the EEPROM
	 */
	debug.setDebugLevel(static_cast<DebugLevel>(dinfo.getDebugLevel()));
	/* write it back to dinfo since debug would have validated it, and potentially changed it to fix errors.*/
	dinfo.setDebugLevel(static_cast<int>(debug.getDebugLevel()));
	/* if value is invalid, then fix it and rewrite the EEPROM */
	if (eeprom_is_dirty) {
		dinfo.saveDatabaseToEEPROM();
	}
	kickAllWatchdogs();

// Setup the WebServer
	WebInit(); // Calls WiFiManager to make sure connected to access point.
	kickAllWatchdogs();

// Configure Sensors
	ConfigurePorts();
	kickAllWatchdogs();

// Configure Scheduler, scheduleFunction (function pointer, task name, start delay in ms, repeat interval in ms)
// WARNING: Tasks must run minimally every 30 seconds to prevent software watchdog timer resets
	myQueue.scheduleFunction(task_readpir, "PIR", 500, 50);
	myQueue.scheduleFunction(task_acquire, "acquire", 1000, 500);
	myQueue.scheduleFunction(task_updatethingspeak, "thingspeak", 2000, 1000); // 1 second resolution
	myQueue.scheduleFunction(task_flashled, "led", 250, 100);
	myQueue.scheduleFunction(task_serialport_menu, "menu", 2000, 500);
	myQueue.scheduleFunction(task_webServer, "webserver", 3000, 10);

// Print boot up information and menu
	printInfo();
	printMenu();

// Signal that setup is complete
	digitalWrite(PIN_BUILTIN_LED, BUILTIN_LED_OFF);

// Start the task scheduler
	setStartupComplete(); // tell the watchdog the long startup is done and to use normal watchdog timeouts
	kickAllSoftwareWatchdogs();
	Serial.println("Startup complete");
	bool pol = false;
	for (;;) {
		softwareWatchdog(); // must call minimally every 1.6s to avoid the external watchdog reseting the ESP8266
		myQueue.Run(millis());
		if (outputTaskClock) {
			if (pol) {
				digitalWrite(outputTaskClockPin, 0);
			}
			else {
				digitalWrite(outputTaskClockPin, 1);
			}
			pol = !pol;
		}
		//yield();
		optimistic_yield(10000);
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
		debug.println(DebugLevel::TIMINGS, "portNumber=" + String(portNumber));
		switch (portNumber) {
			case 0: // port#0
				p.digital = DIGITAL_PIN_1;
				p.analog = ANALOG_PIN;
				p.sda = I2C_SDA_PIN;
				p.scl = I2C_SCL_PIN;
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
				debug.print(DebugLevel::ERROR, F("ERROR: ConfigurePorts() - port number out of range -"));
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
				debug.println(DebugLevel::INFO,
						"Configuring Port#" + String(portNumber) + ", name= " + String(name) + ", type="
								+ getSensorModuleName(static_cast<sensorModule>(portType)));
				//lint -e{30, 142} suppress error due to lint not understanding enum classes
				if (portNumber >= 0 && portNumber < SENSOR_COUNT) {
//					for (int ii = 0; ii < MAX_ADJ; ii++) {
//						sensors[portNumber]->setCalEnable(ii, false);
//					}
					switch (portType) {
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
						case static_cast<int>(sensorModule::ds18b20):
							sensors[portNumber] = new TemperatureSensor;
							sensors[portNumber]->init(sensorModule::ds18b20, p);
							sensors[portNumber]->setName("DS18b20");
							break;
						case static_cast<int>(sensorModule::analog):
							sensors[portNumber] = new GenericSensor();
							sensors[portNumber]->init(sensorModule::analog, p);
							sensors[portNumber]->setName("Analog");
							break;
						case static_cast<int>(sensorModule::digital):
							sensors[portNumber] = new GenericSensor();
							sensors[portNumber]->init(sensorModule::digital, p);
							sensors[portNumber]->setName("Digital");
							break;
						case static_cast<int>(sensorModule::analog_digital):
							sensors[portNumber] = new GenericSensor();
							sensors[portNumber]->init(sensorModule::analog_digital, p);
							sensors[portNumber]->setName("Analog+Digital");
							break;
						case static_cast<int>(sensorModule::htu21d_si7102):
							sensors[portNumber] = new TemperatureSensor;
							sensors[portNumber]->init(sensorModule::htu21d_si7102, p);
							sensors[portNumber]->setName("HTU21D/Si7102");
							break;
						case static_cast<int>(sensorModule::sonar):
							break;
						case static_cast<int>(sensorModule::sound):
							break;
						case static_cast<int>(sensorModule::reed):
							break;
						case static_cast<int>(sensorModule::hcs501):
							break;
						case static_cast<int>(sensorModule::hcsr505):
							break;
						case static_cast<int>(sensorModule::dust):
							break;
						case static_cast<int>(sensorModule::rain):
							break;
						case static_cast<int>(sensorModule::soil):
							break;
						case static_cast<int>(sensorModule::soundh):
							break;
						case static_cast<int>(sensorModule::methane):
							break;
						case static_cast<int>(sensorModule::gy68_BMP180):
							break;
						case static_cast<int>(sensorModule::gy30_BH1750FVI):
							break;
						case static_cast<int>(sensorModule::lcd1602):
							break;
						case static_cast<int>(sensorModule::rfid):
							break;
						case static_cast<int>(sensorModule::marquee):
							break;
						case static_cast<int>(sensorModule::taskclock):
							sensors[portNumber] = new GenericSensor();
							sensors[portNumber]->init(sensorModule::taskclock, p);
							sensors[portNumber]->setName("taskclock");
							outputTaskClock = true;
							outputTaskClockPin = static_cast<uint8_t>(p.digital); // yes. it overwrites if already set in a different port
							break;
						case static_cast<int>(sensorModule::off):
						default:
							sensors[portNumber] = new GenericSensor();
							sensors[portNumber]->init(sensorModule::off, p);
							sensors[portNumber]->setName("off");
							break;
					} // switch (portType)
				} // if (portNumber...)
				else {
					debug.print(DebugLevel::ERROR, F("ERROR: ConfigurePort() - portNumber out of range - "));
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
			for (int i = 0; i < getSensorCalCount() && i < dinfo.getPortAdjMax(); i++) {
				sensors[idx]->setCal(i, static_cast<float>(dinfo.getPortAdj(idx, i)));
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////

String getsDeviceInfo(String eol) {
	String s(ProgramInfo + eol);
	s += "Hostname: " + WiFi.hostname() + eol;
	s += "Device MAC: " + WiFi.macAddress() + eol;
	s += "Device IP: " + localIPstr() + eol;
	s += "AP MAC: " + WiFi.BSSIDstr() + eol;
	s += "SSID: " + WiFi.SSID() + eol;
//	s += "psd: " + WiFi.psk() + eol; // wifi password
	s += "RSSI: " + String(WiFi.RSSI()) + " dBm" + eol;
	s += "ESP8266_Device_ID=" + String(dinfo.getDeviceID()) + eol;
	s += "Friendly Name: " + String(dinfo.getDeviceName()) + eol;
	s += "Temperature Units: " + String(getTempUnits(dinfo.isFahrenheit())) + eol;
	s += "Uptime: " + getTimeString(millis() - startup_millis) + eol;
	return s;
}

String getsSensorInfo(String eol) {
	String s("");
	for (int sensor_number = 0; sensor_number < dinfo.getPortMax() && sensor_number < SENSOR_COUNT;
			sensor_number++) {
		s += eol + "Port #" + String(sensor_number);
		s += eol + "Type: " + getSensorModuleName(dinfo.getPortMode(sensor_number));
		s += eol + sensors[sensor_number]->getsInfo(eol);
	}
	return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////
void printInfo(void) {
// Print useful Information
	Serial.println(getsDeviceInfo(nl));
	Serial.println(getsThingspeakInfo(nl));
	Serial.println(F("== Port Information =="));
	dinfo.printInfo();
}

///////////////////////////////////////////////////////////////////////////////////////////////
int task_readpir(unsigned long now) {
	wdog_timer[static_cast<int>(taskname_pir)] = now; // kick the software watchdog
	if (digitalRead(PIN_PIRSENSOR)) {
		PIRcount++;
	}
	return 0;
}

int task_acquire(unsigned long now) {
	static int next_acquire_number = 0;
	wdog_timer[static_cast<int>(taskname_acquire)] = now; // kick the software watchdog
	const int MAX_ACQUIRES = ACQUIRES_PER_SENSOR * SENSOR_COUNT;

	if (next_acquire_number < 0 || next_acquire_number > (MAX_ACQUIRES - 1)) {
		next_acquire_number = 0;
	}

	// Interleave the sensors, so setup is run for each sensor in order, then
	//    acquire1 is run for each sensor, then acquire2 and then back to setup.
	int subtask_number = next_acquire_number / SENSOR_COUNT;
	int sensor_number = next_acquire_number - int(subtask_number * SENSOR_COUNT);

	// Just in case, double check so a out of bounds error does not occur.
	if (subtask_number >= ACQUIRES_PER_SENSOR) {
		Serial.println(F("ERROR in task_acquire(): subacquire_number>=ACQUIRES_PER_SENSOR"));
		next_acquire_number = -1;
	}
	if (sensor_number >= SENSOR_COUNT) {
		Serial.println(F("ERROR in task_acquire(): sensor_number>=SENSOR_COUNT"));
		next_acquire_number = -1;
	}

	// run the next sub-task
	unsigned long _now = millis();
	unsigned long _then = _now;
	unsigned long _dur = _now;
	if (next_acquire_number >= 0) {
		switch (subtask_number) {
			case 0:
				debug.print(DebugLevel::TIMINGS, String(_now) + " sensors[" + String(sensor_number));
				debug.println(DebugLevel::TIMINGS, F("]->acquire_setup START"));
				sensors[sensor_number]->acquire_setup(); //lint !e661
				_then = millis();
				_dur = _then - _now;
				debug.print(DebugLevel::TIMINGS, String(_then) + " sensors[" + String(sensor_number));
				debug.print(DebugLevel::TIMINGS, F("]->acquire_setup DONE \t\ttime="));
				debug.println(DebugLevel::TIMINGS, _dur);
				break;
			case 1:
				debug.print(DebugLevel::TIMINGS, String(_now) + " sensors[" + String(sensor_number));
				debug.println(DebugLevel::TIMINGS, F("]->acquire1 START"));
				sensors[sensor_number]->acquire1(); //lint !e661
				_then = millis();
				_dur = _then - _now;
				debug.print(DebugLevel::TIMINGS, String(_then) + " sensors[" + String(sensor_number));
				debug.print(DebugLevel::TIMINGS, F("]->acquire1 DONE \t\ttime="));
				debug.println(DebugLevel::TIMINGS, _dur);
				break;
			case 2:
				debug.print(DebugLevel::TIMINGS, String(_now) + " sensors[" + String(sensor_number));
				debug.println(DebugLevel::TIMINGS, F("]->acquire2 START"));
				sensors[sensor_number]->acquire2(); //lint !e661
				_then = millis();
				_dur = _then - _now;
				debug.print(DebugLevel::TIMINGS, String(_then) + " sensors[" + String(sensor_number));
				debug.print(DebugLevel::TIMINGS, F("]->acquire2 DONE \t\ttime="));
				debug.println(DebugLevel::TIMINGS, _dur);
				break;
			default:
				break;
		}
	}
	next_acquire_number++;
	return next_acquire_number;
}

int task_updatethingspeak(unsigned long now) {
	wdog_timer[static_cast<int>(taskname_thingspeak)] = now; // kick the software watchdog
	if (now > (last_thingspeak_update_time_ms + dinfo.getThingspeakUpdatePeriodMS())) {
		last_thingspeak_update_time_ms = now;
		if (dinfo.getThingspeakEnable()) {
			updateThingspeak();
		}
		PIRcountLast = PIRcount;
		PIRcount = 0; // reset counter for starting a new period
	}
	return 0;
}

int task_flashled(unsigned long now) {
	wdog_timer[static_cast<int>(taskname_led)] = now; // kick the software watchdog
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
	Serial.println(F("MENU ------------------------------"));
	Serial.println(F("?  show this menu"));
	Serial.println(F("i  show High-level configuration"));
	Serial.println(F("t  show Thingspeak Channel Settings"));
	Serial.println(F("c  show calibration values"));
	Serial.println(F("m  show measured values"));
	Serial.println(F("r  show chart of values (raw)"));
	Serial.println(F("s  show chart of values"));
	Serial.println(F("w  show web URLs"));
	Serial.println(F("p  push Thingspeak Channel Settings"));
	Serial.println(F("z  Extended menu"));
	Serial.println("");
}

void printExtendedMenu(void) {
	Serial.println(F("MENU EXTENDED ---------------------"));
	Serial.println(F("E  show data structure in EEPROM"));
	Serial.println(F("M  show data structure in RAM"));
	Serial.println(F("C  write defaults to configuration memory"));
	Serial.println(F("S  show Sensor debug info"));
	Serial.println(F("R  show reason for last reset"));
	Serial.println(F("I  show ESP information"));
	Serial.println(F("X  EspClass::reset()"));
	Serial.println(F("Y  EspClass::restart()"));
	Serial.println(F("W  Test Watchdog (block here forever)"));
	Serial.print("D  [" + debug.getDebugLevelString());
	Serial.println(F("] Debug level for logging to serial port"));
	Serial.println("");
}

int task_serialport_menu(unsigned long now) {
	wdog_timer[static_cast<int>(taskname_menu)] = now; // kick the software watchdog
	count++;
	static bool need_new_heading = true;
	static bool raw_need_new_heading = true;

	if (Serial.available() > 0) {
		char ch = static_cast<char>(Serial.read());
		if (ch != 's') {
			need_new_heading = true;
		}
		if (ch != 'r') {
			raw_need_new_heading = true;
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
			case 't':
				Serial.println(getsThingspeakChannelInfo(nl));
				break;
			case 'p':
				ThingspeakPushChannelSettings();
				break;
			case 'r':
				// Display the heading
				if (raw_need_new_heading) {
					raw_need_new_heading = false;
					Serial.print(F("COUNT   | PIR "));
					for (int s = 0; s < SENSOR_COUNT; s++) {
						if (sensors[s]) {
							for (int v = 0; v < getSensorValueCount(); v++) {
								if (sensors[s]->getValueEnable(v)) {
									Serial.print(F("| "));
									Serial.print(
											padEndOfString("Raw/" + sensors[s]->getValueName(v), 12, ' ',
													true));
								}
							}
						}
					}
					Serial.println("");
				}
				// Display the Data
				Serial.print("#" + padEndOfString(String(count), 6, ' ') + " | ");
				Serial.print(padEndOfString(String(PIRcount), 4, ' '));
				for (int s = 0; s < SENSOR_COUNT; s++) {
					if (sensors[s]) {
						for (int v = 0; v < getSensorValueCount(); v++) {
							if (sensors[s]->getValueEnable(v)) {
								Serial.print(F("| "));
								Serial.print(
										padEndOfString(
												String(
														String(sensors[s]->getRawValue(v)) + "/"
																+ String(sensors[s]->getValue(v))), 12, ' ',
												true));
							}
						}
					}
				}
				Serial.println("");
				break;
			case 's':
				// Display the heading
				if (need_new_heading) {
					need_new_heading = false;
					Serial.print(F("COUNT   | PIR "));
					for (int s = 0; s < SENSOR_COUNT; s++) {
						if (sensors[s]) {
							Serial.print("| ");
							for (int v = 0; v < getSensorValueCount(); v++) {
								if (sensors[s]->getValueEnable(v)) {
									Serial.print(padEndOfString(sensors[s]->getValueName(v), 6, ' ', true));
								}
							}
						}
					}
					Serial.println("");
				}
				// Display the Data
				Serial.print("#" + padEndOfString(String(count), 6, ' ') + " | ");
				Serial.print(padEndOfString(String(PIRcount), 4, ' '));
				for (int s = 0; s < SENSOR_COUNT; s++) {
					if (sensors[s]) {
						Serial.print(F("| "));
						for (int v = 0; v < getSensorValueCount(); v++) {
							if (sensors[s]->getValueEnable(v)) {
								Serial.print(padEndOfString(String(sensors[s]->getValue(v)), 6, ' ', true));
							}
						}
					}
				}
				Serial.println("");
				break;
			case 'c':
				Serial.println(F("=== Calibration Data ==="));
				for (int i = 0; i < SENSOR_COUNT; i++) {
					if (sensors[i]) {
						sensors[i]->printCals();
					}
				}
				break;
			case 'm':
				Serial.println(F("=== Value Data ==="));
				for (int i = 0; i < SENSOR_COUNT; i++) {
					if (sensors[i]) {
						sensors[i]->printValues();
					}
				}
				break;
			case 'w':
				Serial.println(WebPrintInfo(nl));
				break;
///// Extended Menu ////
			case 'C':
				dinfo.eraseEEPROM();
				Serial.print(nl);
				Serial.println(F("EEPROM Erased."));
				dinfo.writeDefaultsToDatabase();
				Serial.print(nl);
				Serial.println(F("Defaults written to Database."));
				dinfo.saveDatabaseToEEPROM();
				Serial.print(nl);
				Serial.println(F("Database written to EEPROM."));
				break;
			case 'D':
				dinfo.setDebugLevel(static_cast<int>(debug.incrementDebugLevel()));
				Serial.println("Debug Level set to: " + debug.getDebugLevelString());
				if (eeprom_is_dirty) dinfo.saveDatabaseToEEPROM();
				break;
			case 'E': // Read the EEPROM into the RAM data structure, then dump the contents
				Serial.print(nl);
				Serial.println(F("=== Data in EEPROM ==="));
				dinfo.restoreDatabaseFromEEPROM();
				Serial.println(dinfo.databaseToString(nl.c_str()));
				Serial.println("");
				break;
			case 'M': // Just dump the contents of the RAM data structure
				Serial.print(nl);
				Serial.println(F("=== Data in RAM ==="));
				Serial.println(dinfo.databaseToString(nl.c_str()));
				Serial.println("");
				break;
			case 'I':
				Serial.print(F("CycleCount:\t\t"));
				Serial.println(String(ESP.getCycleCount()));

				Serial.print(F("Vcc:\t\t\t"));
				Serial.println(String(ESP.getVcc()));

				Serial.print(F("FreeHeap:\t\t0x"));
				Serial.println(ESP.getFreeHeap(), HEX);

				Serial.print(F("ChipId:\t\t\t0x"));
				Serial.println(ESP.getChipId(), HEX);

				Serial.print(F("SdkVersion:\t\t"));
				Serial.println(String(ESP.getSdkVersion()));

				Serial.print(F("BootVersion:\t\t"));
				Serial.println(String(ESP.getBootVersion()));

				Serial.print(F("BootMode:\t\t"));
				Serial.println(String(ESP.getBootMode()));

				Serial.print(F("CpuFreqMHz:\t\t"));
				Serial.println(String(ESP.getCpuFreqMHz()));

				Serial.print(F("FlashChipId:\t\t0x"));
				Serial.println(ESP.getFlashChipId(), HEX);

				Serial.print(F("FlashChipRealSize[Mbit]:0x"));
				Serial.println(ESP.getFlashChipRealSize(), HEX);

				Serial.print(F("FlashChipSize [MBit]:\t0x"));
				Serial.println(ESP.getFlashChipSize(), HEX);

				Serial.print(F("FlashChipSpeed [Hz]:\t"));
				Serial.println(String(ESP.getFlashChipSpeed()));

				Serial.print(F("FlashChipMode:\t\t"));
				switch (ESP.getFlashChipMode()) {
					case FM_QIO:
						Serial.println(F("QIO"));
						break;
					case FM_QOUT:
						Serial.println(F("QOUT"));
						break;
					case FM_DIO:
						Serial.println(F("DIO"));
						break;
					case FM_DOUT:
						Serial.println(F("DOUT"));
						break;
					case FM_UNKNOWN:
					default:
						Serial.println(F("Unknown"));
						break;
				}
				Serial.print(F("FlashChipSizeByChipId:\t0x"));
				Serial.println(ESP.getFlashChipSizeByChipId(), HEX);

				Serial.print(F("SketchSize [Bytes]:\t0x"));
				Serial.println(ESP.getSketchSize(), HEX);

				Serial.print(F("FreeSketchSpace [Bytes]:0x"));
				Serial.println(ESP.getFreeSketchSpace(), HEX);

				Serial.print(F("sizeof(dinfo) [Bytes]:\t0x"));
				Serial.println(dinfo.getDatabaseSize(), HEX);

				Serial.println("");
				break;
			case 'R':
				Serial.print(F("Last Reset --> "));
				Serial.println(ESP.getResetInfo() + nl);
				break;
			case 'S':
				Serial.print(nl);
				Serial.println(F("Sensor Debug Information"));
				Serial.println(getsSensorInfo(nl));
				break;
			case 'X':
				Serial.println(F("Calling EspClass::reset()"));
				ESP.reset();
				break;
			case 'Y':
				Serial.println(F("Calling EspClass::restart()"));
				ESP.restart();
				break;
			case 'W':
				Serial.println(F("Looping here forever - Software Watchdog for Menu should trip ... "));
				for (;;) {
					yield();
				} // loop forever so the software watchdog for this task trips, but yield to the ESP OS
				break;
			default:
				Serial.println("Unknown command: " + String(ch));
				break;
		}
	}
	return 0;
}

int task_webServer(unsigned long now) {
	wdog_timer[static_cast<int>(taskname_webserver)] = now; // kick the software watchdog
	WebWorker();
	return 0;
}

