#include <Arduino.h>
#include <EEPROM.h>
#include "main.h"
#include "temperature.h"
#include "network.h"
#include "net_value.h"
#include "Queue.h"
#include "deviceinfo.h"
#include "sensor.h"
#include "util.h"
#include <Esp.h>

extern int task_readpir(unsigned long now);
extern int task_acquire(unsigned long now);
extern int task_updatethingspeak(unsigned long now);
extern int task_flashled(unsigned long now);
extern int task_serialport_menu(unsigned long now);
extern int task_webServer(unsigned long now);

// -----------------------
// Custom configuration
// -----------------------
String ProgramInfo("\r\nEnvironment Sensor v0.01 : Allan Inda 2016-May-27");

// Other
long count = 0;
bool debug_output = true;

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

// Information
void printMenu(void);
void printExtendedMenu(void);
void printInfo(void);
void ConfigurePorts(void);
void reset_config(void);

// ------------------------------------------------------------------------------------
void reset_config(void) {
	// Setup the external Reset circuit
	digitalWrite(PIN_SOFTRESET, HIGH);
	/* Make sure the pin is High so it does not reset until we want it to. Do
	 * this before setting the direction as an output to avoid accidental
	 * reset during pin configuration. */
	pinMode(PIN_SOFTRESET, INPUT);
	/* Set the pin used to cause a reset as an input. This avoids it reseting
	 * when we don't want a reset.
	 */
}

void reset(void) {
	/* Note on the nodeMCU, pin D0 (aka GPIO16) may or may not already be
	 * connected to the RST pin. If it's not connected, connect via a 10K
	 * resistor. Do no connect directly.
	 * The reason it's connected is in order to be able to wake up from deep sleep.
	 * When the esp8266 is put into deep sleep everything but the RTC is powered off.
	 * You can set a timer in the RTC that toggles GPIO16 when it expires and that
	 * resets the esp8266 causing it to power up again. This is the only way the
	 * esp8266 can wake itself up from deep sleep, so it's quite a useful function
	 * to have. On the esp-03 module there is a tiny jumper to connect GPIO16 to reset
	 * so one can connect/disconnect it. On the esp-12, since everything is under a
	 * shield that jumper is evidently not available, so they had to decide one way
	 * or the other...
	 */

	/* Write a low to the pin that is physically connected to the RST pin.
	 * This will force the hardware to reset.
	 */
	reset_config();

	Serial.println("Rebooting ... this may take 15 seconds or more.");

	pinMode(PIN_SOFTRESET, OUTPUT);
	for (int a = 0; a < 10; a++) {
		digitalWrite(PIN_SOFTRESET, LOW);
		delay(1000);
		digitalWrite(PIN_SOFTRESET, HIGH);
		delay(1000);
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
		//Serial.print("portNumber=");
		//Serial.println(portNumber);
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
				Serial.print("BUG: ConfigurePorts() - port number out of range in switch -");
				Serial.println(portNumber);
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
				Serial.print("Configuring Port#");
				Serial.print(portNumber);
				Serial.print(", name= ");
				Serial.print(name);
				Serial.print(", type=");
				Serial.println(c_getModuleName(static_cast<sensorModule>(portType)).c_str());
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
							Serial.println(
									"BUG: ConfigurePorts() - sensorModule not found in switch");
							Serial.println(" ... Created DHT11 instead.");
							break;
					} // switch (portType)
				} // if (portNumber...)
				else {
					Serial.print("BUG: ConfigurePort() - portNumber out of range - ");
					Serial.println(portNumber);
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
	dinfo.RestoreConfigurationFromEEPROM();

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

void printInfo(void) {
// Print useful Information
	Serial.println(ProgramInfo);
	Serial.println(String("Device IP: ") + localIPstr());
	dinfo.printThingspeakInfo();
	Serial.print(String("ESP8266_Device_ID=") + String(dinfo.getDeviceID()));
	Serial.println(String("\r\nFriendly Name: ") + String(dinfo.getDeviceName()));
	dinfo.printInfo();
}

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
	Serial.println("MENU ----------------------");
	Serial.println("?  show this menu");
	Serial.println("i  show High-level configuration");
	Serial.println("c  show calibration values");
	Serial.println("m  show measured values");
	Serial.println("s  show status");
	Serial.println("w  show web URLs");
	Serial.println("z  Extended menu");
	Serial.println("");
}

void printExtendedMenu(void) {
	Serial.println("MENU EXTENDED -------------");
	Serial.println("E  show data structure in EEPROM");
	Serial.println("M  show data structure in RAM");
	Serial.println("R  show reason for last reset");
	Serial.println("I  show ESP information");
	Serial.println("Q  reset()");
	Serial.println("W  EspClass::reset()");
	Serial.println("A  EspClass::restart()");
	Serial.print("D  [");
	if (debug_output) {
		Serial.print("ON");
	}
	else {
		Serial.print("OFF");
	}
	Serial.println("] toggle debug output to serial port");
	Serial.println("");
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
					Serial.print("CNT\tMotion\tLast\t");
					for (int s = 0; s < SENSOR_COUNT; s++) {
						if (sensors[s]) {
							for (int v = 0; v < getValueCount(); v++) {
								if (sensors[s]->getValueEnable(v)) {
									Serial.print(sensors[s]->getValueName(v));
									Serial.print("\t");
								}
							}
						}
					}
					Serial.println("");
				}
				// Display the Data
				Serial.print("#");
				Serial.print(count);
				Serial.print("\t");
				Serial.print(PIRcount);
				Serial.print("\t");
				Serial.print(PIRcountLast);
				Serial.print("\t");
				for (int s = 0; s < SENSOR_COUNT; s++) {
					if (sensors[s]) {
						for (int v = 0; v < getValueCount(); v++) {
							if (sensors[s]->getValueEnable(v)) {
								Serial.print(sensors[s]->getValue(v));
								Serial.print("\t");
							}
						}
					}
				}
				Serial.println("");
				break;
			case 'c':
				Serial.println("Calibration Data");
				for (int i = 0; i < SENSOR_COUNT; i++) {
					if (sensors[i]) {
						sensors[i]->printCals();
					}
				}
				break;
			case 'm':
				Serial.println("Value Data");
				for (int i = 0; i < SENSOR_COUNT; i++) {
					if (sensors[i]) {
						sensors[i]->printValues();
					}
				}
				break;
			case 'w':
				WebPrintInfo();
				break;
///// Extended Menu ////
			case 'D':
				if (debug_output) {
					debug_output = false;
					Serial.println("Debug Disabled");
				}
				else {
					debug_output = true;
					Serial.println("Debug Enabled");
				}
				break;
			case 'E': // Read the EEPROM into the RAM data structure, then dump the contents
				Serial.println("");
				dinfo.RestoreConfigurationFromEEPROM();
				Serial.println(dinfo.toString().c_str());
				Serial.println("");
				break;
			case 'M': // Just dump the contents of the RAM data structure
				Serial.println("");
				Serial.println(dinfo.toString().c_str());
				Serial.println("");
				break;
			case 'I':
				Serial.print("CycleCount:\t\t");
				Serial.println(ESP.getCycleCount());
				Serial.print("Vcc:\t\t\t");
				Serial.println(ESP.getVcc());
				Serial.print("FreeHeap:\t\t");
				Serial.println(ESP.getFreeHeap(), HEX);
				Serial.print("ChipId:\t\t\t");
				Serial.println(ESP.getChipId(), HEX);
				Serial.print("SdkVersion:\t\t");
				Serial.println(ESP.getSdkVersion());
				Serial.print("BootVersion:\t\t");
				Serial.println(ESP.getBootVersion());
				Serial.print("BootMode:\t\t");
				Serial.println(ESP.getBootMode());
				Serial.print("CpuFreqMHz:\t\t");
				Serial.println(ESP.getCpuFreqMHz());
				Serial.print("FlashChipId:\t\t");
				Serial.println(ESP.getFlashChipId(), HEX);
				Serial.print("FlashChipRealSize[Mbit]:");
				Serial.println(ESP.getFlashChipRealSize(), HEX);
				Serial.print("FlashChipSize [MBit]:\t");
				Serial.println(ESP.getFlashChipSize(), HEX);
				Serial.print("FlashChipSpeed [Hz]:\t");
				Serial.println(ESP.getFlashChipSpeed());
				Serial.print("FlashChipMode:\t\t");
				switch (ESP.getFlashChipMode()) {
					case FM_QIO:
						Serial.println("QIO");
						break;
					case FM_QOUT:
						Serial.println("QOUT");
						break;
					case FM_DIO:
						Serial.println("DIO");
						break;
					case FM_DOUT:
						Serial.println("DOUT");
						break;
					case FM_UNKNOWN:
					default:
						Serial.println("Unknown");
						break;
				}
				Serial.print("FlashChipSizeByChipId:\t");
				Serial.println(ESP.getFlashChipSizeByChipId(), HEX);
				Serial.print("SketchSize [Bytes]:\t");
				Serial.println(ESP.getSketchSize(), HEX);
				Serial.print("FreeSketchSpace [Bytes]:");
				Serial.println(ESP.getFreeSketchSpace(), HEX);
				Serial.println("");
				break;
			case 'R':
				Serial.print("Last Reset --> ");
				Serial.println(ESP.getResetInfo());
				break;
			case 'Q':
				Serial.println("Calling reset() in 2 seconds ...");
				reset();
				break;
			case 'W':
				Serial.println("Calling EspClass::reset() in 2 seconds ...");
				delay(2000);
				ESP.reset();
				break;
			case 'A':
				Serial.println("Calling EspClass::restart() in 2 seconds ...");
				delay(2000);
				ESP.restart();
				break;
			default:
				Serial.print("Unknown command: ");
				Serial.println(ch);
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

