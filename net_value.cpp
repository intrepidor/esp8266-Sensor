/*
 * net_value.cpp
 *
 *  Created on: Jan 30, 2016
 *      Author: Allan Inda
 */

#include "network.h"
#include "temperature.h"
#include "main.h"
#include "deviceinfo.h"
#include "net_value.h"

// FIXME: The web triggered reset stops the device, but it does not boot back up and requires a power-cycle
void (*resetFunc)(void) = (void(*)())0x40000080;  // ESP8266 reset vector is 40000080h

void sendValue(void) {

    String file("<!DOCTYPE HTML><html><head></head><body>,");
    String value("");
// READ API
    bool arg = server.hasArg("read");
    if (arg) {
        String sarg = server.arg("read");
        if (sarg == "1") {
            value = String(t1.getTemperature());
        }
        if (sarg == "2") {
            value = String(t1.getHumidity());
        }
        if (sarg == "3") {
            value = String(t1.getHeatindex());
        }
        if (sarg == "4") {
            value = String(t2.getTemperature());
        }
        if (sarg == "5") {
            value = String(t2.getHumidity());
        }
        if (sarg == "6") {
            value = String(t2.getHeatindex());
        }
        if (sarg == "7") {
            value = String(PIRcount);
        }
        if (sarg == "api") {
            value = String(dinfo.getcThinkspeakApikey());
        }
        if (sarg == "deviceid") {
            value = String(dinfo.getDeviceID());
        }
        if (sarg == "name") {
            value = String(dinfo.getDeviceName());
        }
        if (sarg == "factory_default_ssid") {
            value = String(factory_default_ssid);
        }
        if (sarg == "status") {
            value = ""; // FIXME status1 + status2 + status3;
        }
        if (sarg == "thingspeak") {
            value = String(dinfo.getThingspeakStatus());
        }
        if (sarg == "rssi") {
            value = String(rssi);
        }
        if (sarg == "offset1") {
            value = String(t1.getCalOffset());
        }
        if (sarg == "offset2") {
            value = String(t2.getCalOffset());
        }
        if (sarg == "sherbet") {
            value = String(t1.getTemperature()) + "F ";
            value += String(t1.getHumidity()) + "%,";
            value += String(t2.getTemperature()) + "F ";
            value += String(t2.getHumidity()) + "%,";
            value += String(PIRcount) + ",";
            value += String(dinfo.getThingspeakStatus());
        }
    }

    bool eeprom_needs_update = false;
// WRITE API
    if (server.hasArg("reset")) {
        Serial.println("Resetting in 4 seconds...");
        delay(4000);
        resetFunc();
    }
    if (server.hasArg("offset1")) {
        String s = server.arg("offset1");
        float f = static_cast<float>(atof(s.c_str()));
        Serial.print("arg(offset1)=");
        Serial.println(s.c_str());
        Serial.print("f=");
        Serial.println(f);

        if (f > -9.9 && f < 9.9) {
            t1.setCalOffset(f);
            Serial.print("Setting offset1 to ");
            Serial.println(String(t1.getCalOffset()));
            eeprom_needs_update = true;
        }
        else {
            Serial.print("Error: Invalid offset1 value: ");
            Serial.println(server.arg("offset1"));
        }
    }
    if (server.hasArg("offset2")) {
        String s = server.arg("offset2");
        float f = static_cast<float>(atof(s.c_str()));
        Serial.print("arg(offset2)=");
        Serial.println(s.c_str());
        Serial.print("f=");
        Serial.println(f);

        if (f > -9.9 && f < 9.9) {
            t2.setCalOffset(f);
            Serial.print("Setting offset2 to ");
            Serial.println(String(t2.getCalOffset()));
            eeprom_needs_update = true;
        }
        else {
            Serial.print("Error: Invalid offset2 value: ");
            Serial.println(server.arg("offset2"));
        }
    }

    file += value;
    file += String(",</body></html>");
    server.send(200, "text/html", file);

    if (eeprom_needs_update) {
        if (!t1.writeCalibrationData()) {
            Serial.print("Error: Write failed for calibration data for sensor #");
            Serial.println(t1.getPin());
        }
        if (!t2.writeCalibrationData()) {
            Serial.print("Error: Write failed for calibration data for sensor #");
            Serial.println(t2.getPin());
        }
    }
}
