#include <Arduino.h>
#include <EEPROM.h>
#include "main.h"
#include "temperature.h"
#include "network.h"
#include "Queue.h"
#include "deviceinfo.h"
#include "util.h"

extern int task_readpir(unsigned long now);
extern int task_readtemperature(unsigned long now);
extern int task_updatethingspeak(unsigned long now);
extern int task_flashled(unsigned long now);
extern int task_printstatus(unsigned long now);
extern int task_webServer(unsigned long now);

// -----------------------
// Custom configuration
// -----------------------
String ProgramInfo("Environment Sensor v0.01 : Allan Inda 2016-May-25");

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
TemperatureSensor t1, t2, t3, t4;	// DHT11 and DHT22
Device dinfo;

// Information
void printMenu(void);
void printInfo(void);
void ConfigurePorts(void);

// ------------------------------------------------------------------------------------
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
	Serial.println("Rebooting ... this may take 15 seconds or more.");

	pinMode(PIN_SOFTRESET, OUTPUT);
	for (int a = 0; a < 10; a++) {
		digitalWrite(PIN_SOFTRESET, LOW);
		delay(1000);
		digitalWrite(PIN_SOFTRESET, HIGH);
		delay(1000);
	}
}
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

///////////////////////////////////////////////////////////////////////////////////////////////
void ConfigurePorts(void) {
	// Each port has access to a predefined subset of the following pin types, and for
	//    each port, the pin assignments may differ, and some will be the same.
	SensorPins p;

	// Loop through each of the ports
	for (int portNumber = 0; portNumber < dinfo.getPortMax(); portNumber++) {
		// Get the pins used for this port
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
				Serial.println(
						"BUG: ConfigurePorts() - port number out of range in switch");
				break;
		}
		// Figure out the configuration of the port
		//lint -e{26} suppress error false error about the static_cast
		for (int portType = 0; portType < static_cast<int>(sensorModule::END);
				portType++) {
			// loop through each of the port setting types until finding a match
			//lint -e{26} suppress error false error about the static_cast
			if (dinfo.getPortMode(portNumber) == static_cast<sensorModule>(portType)) {
				char name[20];
				strncpy(name, sensorList[static_cast<int>(portType)].name, 19);
				name[19] = 0;
				// found the setting
				Serial.print("Configuring Port#");
				Serial.print(portNumber);
				Serial.print(": ");
				Serial.println(name);
				//lint -e{30, 142} suppress error due to lint not understanding enum classes
				switch (portType) {
					case static_cast<int>(sensorModule::off):
						break;
					case static_cast<int>(sensorModule::dht11):
						switch (portNumber) {
							case 0:
								t1.init(sensorModule::dht11, p);
								t1.setName("DHT11_1");
								break;
							case 1:
								t2.init(sensorModule::dht11, p);
								t2.setName("DHT11_2");
								break;
							case 2:
								t3.init(sensorModule::dht11, p);
								t3.setName("DHT11_3");
								break;
							case 3:
								t4.init(sensorModule::dht11, p);
								t4.setName("DHT11_4");
								break;
							default:
								Serial.println(
										"BUG: ConfigurePorts() dht11 - Invalid portType");
								break;
						}
						break;
						//lint -e{26} suppress error false error about the static_cast
					case static_cast<int>(sensorModule::dht22):
						switch (portNumber) {
							case 0:
								t1.init(sensorModule::dht22, p);
								t1.setName("DHT22_1");
								break;
							case 1:
								t2.init(sensorModule::dht22, p);
								t2.setName("DHT22_2");
								break;
							case 2:
								t3.init(sensorModule::dht22, p);
								t3.setName("DHT22_3");
								break;
							case 3:
								t4.init(sensorModule::dht22, p);
								t4.setName("DHT22_4");
								break;
							default:
								Serial.println(
										"BUG: ConfigurePorts() dht22 - Invalid portNumber");
								break;
						}

						break;
					case static_cast<int>(sensorModule::ds18b20):
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
					case static_cast<int>(sensorModule::gy68):
						break;
					case static_cast<int>(sensorModule::gy30):
						break;
					case static_cast<int>(sensorModule::lcd1602):
						break;
					case static_cast<int>(sensorModule::rfid):
						break;
					case static_cast<int>(sensorModule::marquee):
						break;
					default:
						Serial.println(
								"BUG: ConfigurePorts() - Sensor Module not found in switch");
						break;
				}
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
	pinMode(PIN_PIRSENSOR, INPUT);   // Initialize the PIR sensor pin as an input
	pinMode(PIN_BUILTIN_LED, OUTPUT);  // Initialize the BUILTIN_LED pin as an output

// Signal that setup is proceeding
	digitalWrite(PIN_BUILTIN_LED, BUILTIN_LED_ON);

// Setup Serial port
	Serial.begin(115200);
//Serial.println("\r\n");
//Serial.println(ProgramInfo);

// Start EEPROM
	EEPROM.begin(512);
	dinfo.RestoreConfigurationFromEEPROM();

// Configure Objects
//dinfo.init();

	ConfigurePorts();

// Setup the WebServer
	WebInit();
	Serial.println("");
	printInfo();

	Queue myQueue;
// scheduleFunction arguments (function pointer, task name, start delay in ms, repeat interval in ms)
	myQueue.scheduleFunction(task_readpir, "PIR", 500, 50);
	myQueue.scheduleFunction(task_readtemperature, "Temperature", 1000, 499);
// FIXME disable for now -- myQueue.scheduleFunction(task_updatethingspeak, "Thingspeak", 1500, 10000);
	myQueue.scheduleFunction(task_flashled, "LED", 250, 1000);
	myQueue.scheduleFunction(task_printstatus, "Status", 2000, 500);
	myQueue.scheduleFunction(task_webServer, "WebServer", 3000, 1);

// Signal that setup is done
	digitalWrite(PIN_BUILTIN_LED, BUILTIN_LED_OFF);
	printMenu();

	for (;;) {
		myQueue.Run(millis());
		delay(10);
	}
}

#if 0
void printChipInfo(void) {
//uint32_t fid = spi_flash_get_id();
//uint32_t chip = (fid & 0xff00) | ((fid >> 16) & 0xff);
//Serial.print("Flash id: ");
//Serial.println(fid);
//Serial.print("Chip    : ");
//Serial.println(chip);
}
#endif

void printInfo(void) {
// Print useful Information
	Serial.println(ProgramInfo);
	Serial.println(String("Device IP: ") + localIPstr());
	dinfo.printThingspeakInfo();
	Serial.print(String("ESP8266_Device_ID=") + String(dinfo.getDeviceID()));
	Serial.println(String("\r\nFriendly Name: ") + String(dinfo.getDeviceName()));
	dinfo.printInfo();
//printChipInfo();
}

void printMenu(void) {
	Serial.println("MENU ----------------------");
	Serial.println("c  show calibration values");
	Serial.println("v  show measured values");
	Serial.println("m  show menu");
	Serial.println("s  show status");
	Serial.println("i  show High-level configuration");
	Serial.println("w  show web URLs");
	Serial.println("e  show data structure in EEPROM");
	Serial.println("r  show data structure in RAM");
	Serial.print("d  [");
	if (debug_output) {
		Serial.print("ON");
	}
	else {
		Serial.print("OFF");
	}
	Serial.println("] toggle debug output to serial port");
	Serial.println("");
}

int task_readpir(unsigned long now) {
//lint --e{715}  Ignore unused function arguments
	if (digitalRead(PIN_PIRSENSOR)) {
		PIRcount++;
	}
	return 0;
}

int task_readtemperature(unsigned long now) {
//lint --e{715}  Ignore unused function arguments
	bool r1 = t1.acquire();
	bool r2 = t2.acquire();

	if (!r1) {
		//Serial.print("Err sensor #");
		//Serial.print(t1.getPin());
		//Serial.println("");
	}
	if (!r2) {
		//Serial.print("Err sensor #");
		//Serial.print(t2.getPin());
		//Serial.println("");
	}
	return 0;
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

int task_printstatus(unsigned long now) {
//lint --e{715}  Ignore unused function arguments
	count++;

	if (Serial.available() > 0) {
		char ch = static_cast<char>(Serial.read());
		switch (ch) {
			case 'm':
				printMenu();
				break;
			case 'd':
				if (debug_output) {
					debug_output = false;
					Serial.println("Debug Disabled");
				}
				else {
					debug_output = true;
					Serial.println("Debug Enabled");
				}
				break;
			case 'i':
				printInfo();
				break;
			case 'r': // Just dump the contents of the RAM data structure
				Serial.println("");
				Serial.println(dinfo.toString().c_str());
				Serial.println("");
				break;
			case 'e': // Read the EEPROM into the RAM data structure, then dump the contents
				Serial.println("");
				dinfo.RestoreConfigurationFromEEPROM();
				Serial.println(dinfo.toString().c_str());
				Serial.println("");
				break;
			case 's':
				Serial.print("CNT\tMotion\tLast\t");
				for (int v = 0; v < getValueCount(); v++) {
					if (t1.getValueEnable(v)) {
						Serial.print(t1.getValueName(v));
						Serial.print("\t");
					}
				}
				for (int v = 0; v < getValueCount(); v++) {
					if (t2.getValueEnable(v)) {
						Serial.print(t2.getValueName(v));
						Serial.print("\t");
					}
				}
				Serial.println("");
				Serial.print("#");
				Serial.print(count);
				Serial.print("\t");
				Serial.print(PIRcount);
				Serial.print("\t");
				Serial.print(PIRcountLast);
				Serial.print("\t");
				for (int v = 0; v < getValueCount(); v++) {
					if (t1.getValueEnable(v)) {
						Serial.print(t1.getValue(v));
						Serial.print("\t");
					}
				}
				for (int v = 0; v < getValueCount(); v++) {
					if (t2.getValueEnable(v)) {
						Serial.print(t2.getValue(v));
						Serial.print("\t");
					}
				}
				Serial.println("");
				break;
			case 'c':
				Serial.println("Calibration Data");
				t1.printCals();
				t2.printCals();
				break;
			case 'v':
				Serial.println("Value Data");
				t1.printValues();
				t2.printValues();
				break;
			case 'w':
				WebPrintInfo();
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

