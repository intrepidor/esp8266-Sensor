/*
 * deviceinfo.cpp
 *
 *  Created on: Jan 28, 2016
 *      Author: Allan Inda
 */

#include <Arduino.h>
#include "deviceinfo.h"
#include "main.h"
#include "debugprint.h"
#include "sensor.h"
#include "thingspeak.h"

//-------------------------------------------------------------------------------------------------
void Device::setThingspeakUpdatePeriodMS(unsigned long new_periodms) {
	if (new_periodms < MIN_THINGSPEAK_UPDATE_PERIOD_MS) {
		db.thingspeak.time_between_updates_ms = MIN_THINGSPEAK_UPDATE_PERIOD_MS; // fastest possible per thingspeak rules
	}
	else if (new_periodms > MS_PER_DAY) {
		db.thingspeak.time_between_updates_ms = MS_PER_DAY; // slowest is once per day
	}
	else {
		db.thingspeak.time_between_updates_ms = new_periodms;
	}
}

void Device::setThingspeakUpdatePeriodSeconds(unsigned long new_periodsec) {
	setThingspeakUpdatePeriodMS(new_periodsec * MS_PER_SECOND);
}

//-------------------------------------------------------------------------------------------------
void Device::corruptConfigurationMemory(void) {
	db.end_of_eeprom_signature = 0;
}

//-------------------------------------------------------------------------------------------------
void Device::writeDefaultsToDatabase(void) {
	eraseDatabase();
	db.end_of_eeprom_signature = EEPROM_SIGNATURE; // special code used to detect if configuration structure exists
	setDeviceName("");
	setDeviceID(999);
	setFahrenheitUnit(true);
	setSpare1(false);
	setSpare2(false);
	setSpare3(false);
	setThingspeakEnable(false);
	setThingspeakUpdatePeriodMS (DEF_THINGSPEAK_UPDATE_PERIOD_MS); // once per minute
	setThingspeakApikey("");
	setThingspeakURL("api.thingspeak.com");
	setThingspeakChannel(0);
	setThingspeakIpaddr("184.106.153.149");
	setTSChannelName("MyChannel");
	setTSChannelDesc("MyDescription");
	for (int i = 0; i < dinfo.getTSFieldExtraMax(); i++) {
		setTSFieldExtraNumber(i, i + 1);
		switch (i) {
			case 0:
				setTSFieldPortName(i, String("PIR [/min]"));
				break;
			case 1:
				setTSFieldPortName(i, String("RSSI [dBm]"));
				break;
			case 2:
				setTSFieldPortName(i, String("Uptime [min]"));
				break;
			default:
				setTSFieldPortName(i, String("nothing"));
				break;
		}
	}
	for (int i = 0; i < dinfo.getTSFieldPortMax(); i++) {
		setTSFieldPortNumber(i, 0);
		setTSFieldPortName(i, String("nothing"));
	}
	db.debuglevel = 0;
	for (int i = 0; i < MAX_PORTS; i++) {
		setPortName(i, "");
		setPortMode(i, sensorModule::off);
		for (int j = 0; j < MAX_ADJ; j++) {
			setPortAdj(i, j, 0.0);
		}
	}
	// Do not set eeprom is dirty since these defaults are only temporary.
	// FIXME -- are these really only temporary. Reevaluate of the dirty flag should be set
}

String Device::databaseToString(String eol) {
	String s;
	s = ".end_of_eeprom_signature=" + String(db.end_of_eeprom_signature) + eol;
	s += ".debugLevel=" + DebugPrint::convertDebugLevelToString(static_cast<DebugLevel>(getDebugLevel()))
			+ eol;
	s += "== device ==" + eol;
	s += ".name=" + String(getDeviceName()) + eol;
	s += ".id=" + String(getDeviceID()) + eol;
	s += ".fahrenheit_unit=" + String(isTrueStr(isFahrenheit())) + eol;
	s += ".spare1=" + String(isTrueStr(isSpare1())) + eol;
	s += ".spare2=" + String(isTrueStr(isSpare2())) + eol;
	s += ".spare3=" + String(isTrueStr(isSpare3())) + eol;
	s += "== thingspeak ==" + eol;
	s += ".status=" + String(getThingspeakStatus()) + eol;
	s += ".enable=" + getThingspeakEnableString() + eol;
	s += ".time_between_updates_sec=" + String(getThingspeakUpdatePeriodSeconds()) + " sec" + eol;
	s += ".apikey=" + getThingspeakApikey() + eol;
	s += ".userapikey=" + getThingspeakUserApikey() + eol;
	s += ".url=" + getThingspeakURL() + eol;
	s += ".channel=" + String(getThingspeakChannel()) + eol;
	s += ".ipaddr=" + getThingspeakIpaddr() + eol;
	s += "== thingspeakChannelSettings ==" + eol;
	s += ".ChannelName=" + getTSChannelName() + eol;
	s += ".ChannelDesc=" + getTSChannelDesc() + eol;
	for (int i = 0; i < getTSFieldExtraMax(); i++) {
		s += ".fieldExtra[" + String(i) + "].number=" + getTSFieldExtraNumber(i) + ", .name="
				+ getTSFieldExtraName(i) + eol;
	}
	for (int i = 0; i < getTSFieldPortMax(); i++) {
		s += ".fieldPort[" + String(i) + "].number=" + getTSFieldPortNumber(i) + ", .name="
				+ getTSFieldPortName(i) + eol;
	}
	s += "== cport ==";
	for (int i = 0; i < getPortMax(); i++) {
		s += eol + "port[" + String(i) + "].name=" + getPortName(i) + ", .mode=" + String(getModeStr(i));
//		s += String(", pin=") + String(db.port[i].pin); /// pin is not used
		for (int k = 0; k < MAX_ADJ; k++) {
			s += ", .adj[" + String(k) + "]=";
			s += String(getPortAdj(i, k), DECIMAL_PRECISION);
		}
	}
	return s;
}

void Device::printInfo(void) {
	for (int i = 0; i < getPortMax(); i++) {
		Serial.println("Port #" + String(i + 1) + ": " + getSensorModuleName(getPortMode(i)));
	}
	Serial.println("");
}

//-------------------------------------------------------------------------------------------------
void Device::setTSChannelName(const char* s) {
	if (s) {
		memset(db.thingspeakChannelSettings.name, 0, sizeof(db.thingspeakChannelSettings.name));
		strncpy(db.thingspeakChannelSettings.name, s, sizeof(db.thingspeakChannelSettings.name) - 1);
	}
	else {
		DEBUGPRINT(DebugLevel::ERROR, nl);
		DEBUGPRINTLN(DebugLevel::ERROR, ("ERROR: setTSChannelName() - null value"));
	}
}

void Device::setTSChannelDesc(const char* s) {
	if (s) {
		memset(db.thingspeakChannelSettings.desc, 0, sizeof(db.thingspeakChannelSettings.desc));
		strncpy(db.thingspeakChannelSettings.desc, s, sizeof(db.thingspeakChannelSettings.desc) - 1);
	}
	else {
		DEBUGPRINT(DebugLevel::ERROR, nl);
		DEBUGPRINTLN(DebugLevel::ERROR, ("ERROR: setTSChannelDesc() - null value"));
	}
}

//-------------------------------------------------------------------------------------------------
int Device::getTSFieldPortNumber(int _i) {
	if (isValidTSFieldPortIndex(_i)) {
		return db.thingspeakChannelSettings.fieldPort[_i].number;
	}
	return 0;
}

void Device::setTSFieldPortNumber(int _i, int n) {
	if (isValidTSFieldPortIndex(_i)) {
		db.thingspeakChannelSettings.fieldPort[_i].number = n;
	}
}

String Device::getTSFieldPortName(int _i) {
	if (isValidTSFieldPortIndex(_i)) {
		return String(db.thingspeakChannelSettings.fieldPort[_i].name);
	}
	return String("");
}

void Device::setTSFieldPortName(int _i, String s) {
	if (isValidTSFieldPortIndex(_i)) {
		memset(db.thingspeakChannelSettings.fieldPort[_i].name, 0,
				sizeof(db.thingspeakChannelSettings.fieldPort[_i].name));
		strncpy(db.thingspeakChannelSettings.fieldPort[_i].name, s.c_str(),
				sizeof(db.thingspeakChannelSettings.fieldPort[_i].name) - 1);
	}
}

//-------------------------------------------------------------------------------------------------
int Device::getTSFieldExtraNumber(int _i) {
	if (isValidTSFieldExtraIndex(_i)) {
		return db.thingspeakChannelSettings.fieldExtra[_i].number;
	}
	return 0;
}

void Device::setTSFieldExtraNumber(int _i, int n) {
	if (isValidTSFieldExtraIndex(_i)) {
		db.thingspeakChannelSettings.fieldExtra[_i].number = n;
	}
}

String Device::getTSFieldExtraName(int _i) {
	if (isValidTSFieldExtraIndex(_i)) {
		return String(db.thingspeakChannelSettings.fieldExtra[_i].name);
	}
	return String("");
}

void Device::setTSFieldExtraName(int _i, String s) {
	if (isValidTSFieldExtraIndex(_i)) {
		memset(db.thingspeakChannelSettings.fieldExtra[_i].name, 0,
				sizeof(db.thingspeakChannelSettings.fieldExtra[_i].name));
		strncpy(db.thingspeakChannelSettings.fieldExtra[_i].name, s.c_str(),
				sizeof(db.thingspeakChannelSettings.fieldExtra[_i].name) - 1);
	}
}

//-------------------------------------------------------------------------------------------------
String Device::getNameByPosition(int _pos) {
	if (_pos < 0) {
		;
	}
	else if (_pos < getTSFieldPortMax()) {
		return getTSFieldPortName(_pos);
	}
	else {
		int ext = _pos - getTSFieldPortMax();
		if (ext >= 0 && ext < getTSFieldExtraMax()) {
			return getTSFieldExtraName(ext);
		}
	}
	return String("not set");
}

//----------------------------------------------------------------------
/* Field numbers correspond to those of Thingspeak, field1 to field8,
 * which are numbers 1 to 8. Convert field number to Position. The first
 * Position found starting with fieldExtra[] will be returned. So if
 * the field number is used for multiple fieldPort/FieldExtra, then only
 * the first one found is considered.
 * Returns -1 on error
 */
int Device::getPositionByTSField18Number(int fld /* 1 .. 8 */) {
	int pm = dinfo.getTSFieldPortMax();
	if (fld > 0 && fld <= MAX_THINGSPEAK_FIELD_COUNT) {
		// Look for assignment to field<fld>
		// Prioritize the Extra fields, which are the built-in ones.
		for (int e = 0; e < dinfo.getTSFieldExtraMax(); e++) {
			if (fld == dinfo.getTSFieldExtraNumber(e)) return (e + pm);
		}
		// If the TS field # is not used by the extra fields above, then look
		//    in the ports for use of the field.
		for (int p = 0; p < pm; p++) {
			if (fld == dinfo.getTSFieldPortNumber(p)) return p;
		}
	}
	return -1;
}

bool Device::isTSField18Used(int fld /* 1 .. 8 */) {
// Description: Returns true if the Thingspeak field (1..8) is assigned to something.
// Algorithm: Loop through all the ports and Extra and see if the field is assigned to anything.
	if (getPositionByTSField18Number(fld) < 0) return false;
	return true;
}

//----------------------------------------------------------------------
//  FIXME the code below is just wrong!
//int Device::getFieldByPosition(int _pos) {
//	if (_pos < 10) {
//		return dinfo.getTSFieldExtraNumber(_pos - dinfo.getTSFieldPortMax());
//	}
//	else if (_pos < dinfo.getTSFieldPortMax()) {
//		return dinfo.getTSFieldPortNumber(_pos);
//	}
//	return 0;
//}

//-------------------------------------------------------------------------------------------------
void Device::setcDeviceName(const char* newname) {
	if (newname) {
		memset(db.device.name, 0, sizeof(db.device.name));
		strncpy(db.device.name, newname, sizeof(db.device.name) - 1);
	}
	else {
		DEBUGPRINT(DebugLevel::ERROR, nl);
		DEBUGPRINTLN(DebugLevel::ERROR, ("ERROR: setcDeviceName() - null value"));
	}
}

const char* Device::getThingspeakEnableStr() {
	if (db.thingspeak.enabled) return "checked";
	return " ";
}

//-------------------------------------------------------------------------------------------------
static bool isValidPort(int portnum) {
	if (portnum >= 0 && portnum < MAX_PORTS) return true;
	return false;
}

sensorModule Device::getPortMode(int portnum) {
	if (isValidPort(portnum)) {
		return static_cast<sensorModule>(db.port[portnum].mode);
	}
	else {
		return sensorModule::off;
	}
}
void Device::setPortMode(int portnum, sensorModule _mode) {
	if (isValidPort(portnum)) {
		db.port[portnum].mode = _mode;
	}
	else {
		DEBUGPRINTLN(DebugLevel::ERROR, ("ERROR: Device::setPortMode() - invalid port"));
	}
}

String Device::getModeStr(int portnum) {
	String s("");
	if (isValidPort(portnum)) {
		int m = static_cast<int>(db.port[portnum].mode);
		//lint -e{26} suppress since lint doesn't understand sensorModule C++11
		if (m >= 0 && m < static_cast<int>(sensorModule::END)) {
			s = String(String(m) + ":" + String(sensorList[m].name));
		}
		else {
			DEBUGPRINT(DebugLevel::ERROR, ("ERROR: Device::getModeStr() - invalid mode"));
		}
	}
	else {
		DEBUGPRINTLN(DebugLevel::ERROR, ("ERROR: Device::getModeStr() - invalid port"));
	}
	return s;
}

void Device::setPortName(int portnum, String _n) {
	if (isValidPort(portnum)) {
		strncpy(db.port[portnum].name, _n.c_str(), sizeof(db.port[portnum].name) - 1);
	}
	else {
		DEBUGPRINTLN(DebugLevel::ERROR, ("ERROR: Device::setPortName() - invalid port"));
	}
}

String Device::getPortName(int portnum) {
	if (isValidPort(portnum)) {
		return this->db.port[portnum].name;
	}
	else {
		DEBUGPRINTLN(DebugLevel::ERROR, ("ERROR: Device::getPortName() - invalid port"));
	}
	return String("undefined");
}

double Device::getPortAdj(int portnum, int adjnum) {

	if (isValidPort(portnum)) {
		if (adjnum >= 0 && adjnum < getPortAdjMax()) {
			return this->db.port[portnum].adj[adjnum];
		}
		else {
			DEBUGPRINTLN(DebugLevel::ERROR, ("ERROR: Device::getPortAdj() - invalid adj index"));
		}
	}
	else {
		DEBUGPRINTLN(DebugLevel::ERROR, ("ERROR: Device::getPortAdj() - invalid port"));
	}
	return 0.0;
}

void Device::setPortAdj(int portnum, int adjnum, double v) {
	if (isValidPort(portnum)) {
		if (adjnum >= 0 && adjnum < getPortAdjMax()) {
			db.port[portnum].adj[adjnum] = v;
		}
		else {
			DEBUGPRINTLN(DebugLevel::ERROR, ("ERROR: Device::setPortAdj() - invalid adj index"));
		}
	}
	else {
		DEBUGPRINTLN(DebugLevel::ERROR, ("ERROR: Device::setPortAdj() - invalid port"));
	}
}

String Device::getPortAdjName(int portnum, int adjnum) {
	if (portnum >= 0 && portnum < SENSOR_COUNT && adjnum >= 0 && adjnum < MAX_ADJ) {
		if (sensors[portnum]->getCalEnable(adjnum)) {
			return sensors[portnum]->getCalName(adjnum);
		}
	}
	return "not used";
}
