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
String ProgramInfo("Environment Sensor v0.01 : Allan Inda 2016-May-19");

// Other
long count = 0;
bool debug_output = true;

// Define pins
const uint8_t PIN_SOFTRESET = D0;
const uint8_t PIN_BUILTIN_LED = BUILTIN_LED; // D0
const uint8_t EXT_PORT_0 = D2;
const uint8_t EXT_PORT_1 = D3;
const uint8_t EXT_PORT_2 = D4;	// also used for I2C (SDA)
const uint8_t EXT_PORT_3 = D5;	// also used for I2C (SCL)
const uint8_t EXT_PORT_4 = A0;

// PIR Sensor
const uint8_t PIN_PIRSENSOR = D1;
int PIRcount = 0;     // current PIR count
int PIRcountLast = 0; // last PIR count

// LED on the ESP board
#define BUILTIN_LED_ON LOW
#define BUILTIN_LED_OFF HIGH

// Create Objects
TemperatureSensor t1;
TemperatureSensor t2;
Device dinfo;

// Information
void printMenu(void);
void printInfo(void);

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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"	// disable warnings about the below strings being "const" while the signatures are non-const
	{
		for (int i = 0; i < dinfo.getPortMax(); i++) {
			for (int j = 0; j < static_cast<int>(portModes::END); j++) {
				if (dinfo.getPortMode(i) == static_cast<portModes>(j)) {
					Serial.print("Port#");
					Serial.print(i);
					Serial.print(": ");
					Serial.println(sensors[static_cast<int>(j)].name);

				}
			}
		}

		//lint --e{1776}   Ignore Info about using string literal in place of char*
		t1.Init(sensor_technology::dht22, "3", EXT_PORT_0);
		t2.Init(sensor_technology::dht22, "4", EXT_PORT_1);
	}
#pragma GCC diagnostic pop

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
	bool r1 = t1.read();
	bool r2 = t2.read();

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
				Serial.println("CNT\tRH%\tTemp1*C\tHIdx*C\tRH%\tTemp2*C\tHIdx*C\tMotion");
				Serial.print("#,");
				Serial.print(count);
				Serial.print(",\t");
				Serial.print(t1.getHumidity());
				Serial.print(",\t");
				Serial.print(t1.getTemperature());
				Serial.print(",\t");
				Serial.print(t1.getHeatindex());
				Serial.print(",\t");
				Serial.print(t2.getHumidity());
				Serial.print(",\t");
				Serial.print(t2.getTemperature());
				Serial.print(",\t");
				Serial.print(t2.getHeatindex());
				Serial.print(",\t");
				Serial.print(PIRcount);
				Serial.print(",\t");
				Serial.println(PIRcountLast);
				break;
			case 'c':
				t1.printCalibrationData();
				t2.printCalibrationData();
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

