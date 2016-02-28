#include <Arduino.h>
#include <EEPROM.h>
#include <ctype.h>
#include "main.h"
#include "temperature.h"
#include "network.h"
#include "Queue.h"
#include "deviceinfo.h"

extern int task_readpir(unsigned long now);
extern int task_readtemperature(unsigned long now);
extern int task_updatethingspeak(unsigned long now);
extern int task_flashled(unsigned long now);
extern int task_printstatus(unsigned long now);
extern int task_webServer(unsigned long now);

// -----------------------
// Custom configuration
// -----------------------
String ProgramInfo("Environment Server v1.20\nAllan Inda 2016-Jan-30\n");

// Other
long count = 0;

// PIR Sensor
#define PIRPIN pD1
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
String header("CNT\tRH%\tTemp1*C\tHIdx*C\tRH%\tTemp2*C\tHIdx*C\tMotion");

///////////////////////////////////////////////////////////////////////////////////////////////
void setup(void) {

    // Setup GPIO
    pinMode(PIRPIN, INPUT);   // Initialize the PIR sensor pin as an input
    pinMode(BUILTIN_LED, OUTPUT);  // Initialize the BUILTIN_LED pin as an output

    // Signal that setup is proceeding
    digitalWrite(BUILTIN_LED, BUILTIN_LED_ON);

    // Setup Serial port
    Serial.begin(115200);
    Serial.println(ProgramInfo.c_str());

    // Start EEPROM
    EEPROM.begin(512);
    dinfo.RestoreConfigurationFromEEPROM();

    // Configure Objects

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"	// disable warnings about the below strings being "const" while the signatures are non-const
    {
        //lint --e{1776}   Ignore Info about using string literal in place of char*
        t1.Init(sensor_technology::dht22, "3", pD3);
        t2.Init(sensor_technology::dht22, "4", pD4);
    }
#pragma GCC diagnostic pop

    // Setup the WebServer
    WebInit();
    WebPrintInfo();

    // Print useful Information
    dinfo.printThingspeakInfo();
    Serial.print(String(String("ESP8266_Device_ID=") + String(dinfo.getDeviceID())).c_str());
    Serial.println(String(String("\nFriendly Name: ") + String(dinfo.getDeviceName()) + String("\n")).c_str());
    Serial.println(
            String(
                    (String("DHT#1=") + String(t1.getstrType()) + String("\nDHT#2=") + String(t2.getstrType())
                            + String("\n"))).c_str());
    Serial.println("");
    Serial.println("");
    Serial.println(header);

    //fixme  dinfo.printInfo();

    Queue myQueue;
    // scheduleFunction arguments (function pointer, task name, start delay in ms, repeat interval in ms)
    myQueue.scheduleFunction(task_readpir, "PIR", 500, 50);
    // FIXME disable for now -- myQueue.scheduleFunction(task_readtemperature, "Temperature", 1000, 499);
    // FIXME disable for now -- myQueue.scheduleFunction(task_updatethingspeak, "Thingspeak", 1500, 10000);
    myQueue.scheduleFunction(task_flashled, "LED", 250, 1000);
    myQueue.scheduleFunction(task_printstatus, "Status", 2000, 5000);
    myQueue.scheduleFunction(task_webServer, "WebServer", 3000, 1);

    // Signal that setup is done
    digitalWrite(BUILTIN_LED, BUILTIN_LED_OFF);

    for (;;) {
        myQueue.Run(millis());
        delay(10);
    }
}

int task_readpir(unsigned long now) {
//lint --e{715}  Ignore unused function arguments
    if (digitalRead (PIRPIN)) {
        PIRcount++;
    }
    return 0;
}

int task_readtemperature(unsigned long now) {
//lint --e{715}  Ignore unused function arguments
    if (!t1.read()) {
        Serial.print("Err sensor #");
        Serial.print(t1.getPin());
        Serial.println("");
    }
    if (!t2.read()) {
        Serial.print("Err sensor #");
        Serial.print(t2.getPin());
        Serial.println("");
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
    return 0;
}

int task_webServer(unsigned long now) {
//lint --e{715}  Ignore unused function arguments
    WebWorker();
    return 0;
}

void loop(void) {
}
