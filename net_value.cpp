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
#include "util.h"

void WebPrintInfo(void) {
	// Describe Webserver access
	Serial.print("Read Everything: http://");
	Serial.println(localIPstr());

	Serial.print("Read API:   http://");
	Serial.print(localIPstr());
	Serial.println(uri_v);
	Serial.println("?read=pir        :: read Motion detector value");
	Serial.println("?read=valXY      :: read value X from Port Y, e.g. read=val23");
	Serial.println("?read=calXY      :: read calibration X from Port Y, e.g. read=cal23");
	Serial.println("?read=api        :: read ThingSpeak API key");
	Serial.println("?read=rssi       :: read AP signal strength in dBm at connect time");
	Serial.println("?read=deviceid   :: read deviceid number");
	Serial.println("?read=name       :: read name of device");
	Serial.println("?read=thingspeak :: read thingspeak api codes");
	Serial.println("?read=csv        :: read summary of sensor data");
	Serial.println("?calXY=Z         :: write calibration Z to cal X port Y");
	Serial.println("?reset=0         :: reboot device");
	Serial.println("");
}

void sendValue(void) {

	String file("<!DOCTYPE HTML><html><head></head><body>,");
	String value("");
// READ API
	bool arg = server.hasArg("read");
	if (arg) {
		String sarg = server.arg("read");
		// FIXME -- make the parsing more streamlined rather than brute force
		if (sarg == "val11") {
			value = String(sensors[0]->getValue(0));
		}
		if (sarg == "val12") {
			value = String(sensors[0]->getValue(1));
		}
		if (sarg == "val21") {
			value = String(sensors[1]->getValue(0));
		}
		if (sarg == "val22") {
			value = String(sensors[1]->getValue(1));
		}
		if (sarg == "val31") {
			value = String(sensors[2]->getValue(0));
		}
		if (sarg == "val32") {
			value = String(sensors[2]->getValue(1));
		}
		if (sarg == "val41") {
			value = String(sensors[3]->getValue(0));
		}
		if (sarg == "val42") {
			value = String(sensors[3]->getValue(1));
		}
		if (sarg == "cal11") {
			value = String(sensors[0]->getCal(0));
		}
		if (sarg == "cal21") {
			value = String(sensors[1]->getCal(0));
		}
		if (sarg == "pir") {
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
		if (sarg == "thingspeak") {
			value = String(dinfo.getThingspeakStatus());
		}
		if (sarg == "rssi") {
			value = String(rssi);
		}
		if (sarg == "csv") {
			value = String("");
			for (int i = 0; i < SENSOR_COUNT; i++) {
				if (sensors[i]) {
					for (int j = 0; j < getValueCount(); j++) {
						if (sensors[i]->getValueEnable(j)) {
							value += String("val") + String(i) + String(j) + "="
									+ String(sensors[i]->getValue(j)) + ",";
						}
					} // for j
					for (int j = 0; j < getCalCount(); j++) {
						if (sensors[i]->getCalEnable(j)) {
							value += String("cal") + String(i) + String(j) + "="
									+ String(sensors[i]->getCal(j)) + ",";
						}
					} // for j
				} // for i
			}
			value += String(PIRcount) + ",";
			value += String(dinfo.getThingspeakStatus());
		}
	}

	bool eeprom_needs_update = false;
// WRITE API
	if (server.hasArg("reset")) {
		reset();
	}
	if (server.hasArg("offset1")) {
		String s = server.arg("offset1");
		float f = static_cast<float>(atof(s.c_str()));
		Serial.print("arg(offset1)=");
		Serial.println(s.c_str());
		Serial.print("f=");
		Serial.println(f);

		if (f > -9.9 && f < 9.9) {
			//t1.setCalOffset(f);
			//Serial.print("Setting offset1 to ");
			//Serial.println(String(t1.getCalOffset()));
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
			//t2.setCalOffset(f);
			//Serial.print("Setting offset2 to ");
			//Serial.println(String(t2.getCalOffset()));
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

// FIXME
//	if (eeprom_needs_update) {
//		if (!t1.writeCalibrationData()) {
//			Serial.print("Error: Write failed for calibration data for sensor #");
//			Serial.println(t1.getDigitalPin());
//		}
//		if (!t2.writeCalibrationData()) {
//			Serial.print("Error: Write failed for calibration data for sensor #");
//			Serial.println(t2.getDigitalPin());
//		}
//	}
}
