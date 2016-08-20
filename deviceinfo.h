/*
 * deviceinfo.h
 *
 *  Created on: Jan 28, 2016
 *      Author: Allan Inda
 */

#ifndef DEVICEINFO_H_
#define DEVICEINFO_H_

//#include <array>
#include <Arduino.h>
#include "sensor.h"
#include "util.h"
#include "sensor_support.h"

const uint32_t EEPROM_SIGNATURE = 0xA357; // This is a pattern used to detect if the EEPROM data is present

const int CURRENT_CONFIG_VERSION = 123;
const int DECIMAL_PRECISION = 5;

const unsigned long DEF_THINGSPEAK_UPDATE_PERIOD_MS = (60000); // 60 seconds
const unsigned long MIN_THINGSPEAK_UPDATE_PERIOD_MS = (20000); // 20 seconds

//-----------------------------------------------------------------------------------
// Device Class
const int MAX_PORTS = 3;
const int MAX_ADJ = 4;
const int STRING_LENGTH = 19; // 1 less than 4-byte boundary for terminating null
const int URL_LENGTH = 119;	// 1 less than 4 byte boundary for terminating null
const int FIELD_NAME_LENGTH = 29; // 1 less than 4 byte boundary for terminating null

const int THINGSPEAK_CHANNEL_NAME = 29;
const int THINGSPEAK_CHANNEL_DESC = 119;
const int THINGSPEAK_EXTRA_FIELDS = 4;
const int THINGSPEAK_PORT_FIELDS = MAX_PORTS * SENSOR_VALUE_COUNT;

extern bool eeprom_is_dirty; // false whenever EEPROM and RAM are different.
static bool isValidPort(int portnum);

struct ThingspeakField { // 30 bytes
	int number;
	char name[FIELD_NAME_LENGTH];
};

struct cport { // 40 bytes
	char name[STRING_LENGTH + 1];
	sensorModule mode;
	double adj[MAX_ADJ];
};

class Device {
private:
	struct {
		// Device
		struct {	// 28 bytes
			char name[STRING_LENGTH + 1];
			int id;
			bool fahrenheit_unit;
			bool spare1;
			bool spare2;
			bool spare3;
		} device;

		// Thingspeak
		struct { // 169 bytes
			unsigned long channel;
			unsigned long time_between_updates_ms;
			int status;
			// bit value
			// 1 means "HTTP/1.1 200 OK" received
			// 2 means "Status: 200 OK" received
			// 4 means "connection failure"
			char enabled;
			char apikey[STRING_LENGTH + 1];
			char userapikey[STRING_LENGTH + 1];
			char url[URL_LENGTH + 1];
			char ipaddr[16]; // nnn.nnn.nnn.nnn = 4*3+3(dots)+1(nul) = 16 chars; 15 for data, and 1 for terminating nul

		} thingspeak;

		// These values get pushed to the Thingspeak website to describe the channel
		//    and fields.
		struct { // 546 bytes
			char name[THINGSPEAK_CHANNEL_NAME + 1];
			char desc[THINGSPEAK_CHANNEL_DESC + 1];
			ThingspeakField fieldPort[THINGSPEAK_PORT_FIELDS]; // Positions 0 .. 7
			ThingspeakField fieldExtra[THINGSPEAK_EXTRA_FIELDS]; // Positions 8 .. 11
		} thingspeakChannelSettings;

		// Ports // 120 bytes
		cport port[MAX_PORTS];

		// Logging level // 4 bytes
		int debuglevel;

		// EEPROM // 4 bytes
		uint32_t end_of_eeprom_signature; // version of the configuration data format in the EEPROM
		// put at the bottom so it moves around

	} db; // estimate 838 bytes

public:
// Default Constructor
	Device() {
		memset(&db, 0, sizeof(db));
	}
	void init(void) {
		// This cannot be put in the constructor because it references other
		//   classes, and the constructor could be a global variable, in which
		//   case objects of those other classes might not have been declared.
		writeDefaultsToDatabase();
	}
	String databaseToString(String _eol); // pass end of line delimiter
	void writeDefaultsToDatabase(void);
	void eraseDatabase(void) {
		memset(&db, 0, sizeof(db));
	}
	void corruptConfigurationMemory(void); // this is used for force a return to factory defaults
	int getDatabaseSize(void) {
		return sizeof(db);
	}

//--------------------------------------------------------
	int getDebugLevel(void) {
		return db.debuglevel;
	}
	void setDebugLevel(int dlevel) {
		db.debuglevel = dlevel;
		eeprom_is_dirty = true;
	}

//--------------------------------------------------------
// Thingspeak Channel Settings
	String getTSChannelName() {
		return String(this->db.thingspeakChannelSettings.name);
	}
	String getTSChannelDesc() {
		return String(this->db.thingspeakChannelSettings.desc);
	}
	void setTSChannelName(const char* s);
	void setTSChannelDesc(const char* s);

//--------------------------------------------------------
// Thingspeak Port Fields
	String getTSFieldPortName(int index);
	void setTSFieldPortName(int index, String n);

	int getTSFieldPortNumber(int index);
	void setTSFieldPortNumber(int index, int n);

	int getTSFieldPortMax(void) {
		return THINGSPEAK_PORT_FIELDS;
	}
	bool isValidTSFieldPortIndex(int _i) {
		if (_i >= 0 && _i < getTSFieldPortMax()) return true;
		return false;
	}

//--------------------------------------------------------
// Thingspeak Extra Fields
	String getTSFieldExtraName(int index);
	void setTSFieldExtraName(int index, String n);

	int getTSFieldExtraNumber(int index);
	void setTSFieldExtraNumber(int index, int n);

	int getTSFieldExtraMax(void) {
		return THINGSPEAK_EXTRA_FIELDS;
	}
	bool isValidTSFieldExtraIndex(int _i) {
		if (_i >= 0 && _i < getTSFieldPortMax()) return true;
		return false;
	}

//--------------------------------------------------------
// Thingspeak Position
// Position is a number 0 to N where N equals the sum of
//    getTSFieldExtraMax() and getTSFieldPortMax();
//
	String getNameByPosition(int _pos);
	int getPositionByTSField18Number(int fld);
	int getFieldByPosition(int _pos);

//--------------------------------------------------------
// Thingspeak Fields (field1, field2, ... field 8
//
	bool isTSField18Used(int fld);

//--------------------------------------------------------
// Device Name and ID
	const char* getDeviceName(void) const {
		return this->db.device.name;
	}
	void setcDeviceName(const char* newname);
	void setDeviceName(String newname) {
		setcDeviceName(newname.c_str());
	}
	int getDeviceID(void) {
		return this->db.device.id;
	}
	void setDeviceID(int newID) {
		this->db.device.id = newID;
	}
	void setFahrenheitUnit(bool true_for_fahrenheit) {
		this->db.device.fahrenheit_unit = true_for_fahrenheit;
	}
	bool isFahrenheit(void) {
		return this->db.device.fahrenheit_unit;
	}
	void setSpare1(bool b) {
		this->db.device.spare1 = b;
	}
	bool isSpare1(void) {
		return this->db.device.spare1;
	}
	void setSpare2(bool b) {
		this->db.device.spare2 = b;
	}
	bool isSpare2(void) {
		return this->db.device.spare2;
	}
	void setSpare3(bool b) {
		this->db.device.spare3 = b;
	}
	bool isSpare3(void) {
		return this->db.device.spare3;
	}
	void printInfo(void);
//--------------------------------------------------------
// Ports
	int getPortMax(void) {	// CONSIDER making this static
		return MAX_PORTS;
	}
	sensorModule getPortMode(int portnum);
	void setPortMode(int portnum, sensorModule _mode);
	void setPortName(int portnum, String _n);
	String getPortName(int portnum);
	int getPortAdjMax(void) {
		return MAX_ADJ;
	}
	double getPortAdj(int portnum, int adjnum);
	void setPortAdj(int portnum, int adjnum, double v);
	String getModeStr(int portnum);

// provide a custom adj field name based on the sensor module type
	String getPortAdjName(int portnum, int adjunum);

//--------------------------------------------------------
// EEPROM and DB Stuff
	void restoreDatabaseFromEEPROM(void);
	bool saveDatabaseToEEPROM(void);
	bool eraseEEPROM(void);

//--------------------------------------------------------
// Thingspeak Data
	int getThingspeakStatus() const {
		return db.thingspeak.status;
	}
	void setThingspeakStatus(int s) {
		db.thingspeak.status = s;
	}

// Update Period
	unsigned long getThingspeakUpdatePeriodSeconds() {
		if (db.thingspeak.time_between_updates_ms < MIN_THINGSPEAK_UPDATE_PERIOD_MS) {
			db.thingspeak.time_between_updates_ms = MIN_THINGSPEAK_UPDATE_PERIOD_MS;
		}
		return db.thingspeak.time_between_updates_ms / MS_PER_SECOND;
	}
	unsigned long getThingspeakUpdatePeriodMS() {
		return getThingspeakUpdatePeriodSeconds() * MS_PER_SECOND;
	}
	void setThingspeakUpdatePeriodSeconds(unsigned long new_periodsec);
	void setThingspeakUpdatePeriodMS(unsigned long new_periodms);

// Channel Write API Key
	void setThingspeakApikey(const char* _apikey) {
		if (_apikey) {
			strncpy(this->db.thingspeak.apikey, _apikey, sizeof(db.thingspeak.apikey) - 1);
		}
	}
	void setThingspeakApikey(String _apikey) {
		if (_apikey) {
			strncpy(this->db.thingspeak.apikey, _apikey.c_str(), sizeof(db.thingspeak.apikey) - 1);
		}
	}
	String getThingspeakApikey() const {
		return String(db.thingspeak.apikey);
	}
	const char* getcThingspeakApikey() const {
		return db.thingspeak.apikey;
	}
// User API Key
	void setThingspeakUserApikey(const char* _apikey) {
		if (_apikey) {
			strncpy(this->db.thingspeak.userapikey, _apikey, sizeof(db.thingspeak.userapikey) - 1);
		}
	}
	void setThingspeakUserApikey(String _apikey) {
		if (_apikey) {
			strncpy(this->db.thingspeak.userapikey, _apikey.c_str(), sizeof(db.thingspeak.userapikey) - 1);
		}
	}
	String getThingspeakUserApikey() const {
		return String(db.thingspeak.userapikey);
	}
	String getcThingspeakUserApikey() const {
		return db.thingspeak.userapikey;
	}
// URL
	void setThingspeakURL(const char* _url) {
		if (_url) {
			strncpy(this->db.thingspeak.url, _url, sizeof(this->db.thingspeak.url) - 1);
		}
	}
	void setThingspeakURL(String _url) {
		setThingspeakURL(_url.c_str());
	}
	String getThingspeakURL() const {
		return String(db.thingspeak.url);
	}
	const char* getcThingspeakURL() const {
		return db.thingspeak.url;
	}
// CHANNEL
	void setThingspeakChannel(unsigned long _channel) {
		db.thingspeak.channel = _channel;
	}
	unsigned long getThingspeakChannel() const {
		return db.thingspeak.channel;
	}
// IPADDR
	void setThingspeakIpaddr(String _ipaddr) {
		setThingspeakIpaddr(_ipaddr.c_str());
	}
	void setThingspeakIpaddr(const char* _ipaddr) {
		if (_ipaddr) {
			strncpy(this->db.thingspeak.ipaddr, _ipaddr, sizeof(db.thingspeak.ipaddr) - 1);
		}
	}
	String getThingspeakIpaddr(void) const {
		return String(db.thingspeak.ipaddr);
	}
	const char* getThingspeakIpaddr_c(void) const {
		return db.thingspeak.ipaddr;
	}
// ENABLE
	void setThingspeakEnable(bool _enable) {
		this->db.thingspeak.enabled = _enable;
	}
	bool getThingspeakEnable(void) const {
		if (db.thingspeak.enabled) return true;
		return false;
	}
	String getThingspeakEnableString(void) const {
		if (db.thingspeak.enabled) return String("true");
		return String("false");
	}
	const char* PROGMEM getThingspeakEnableStr();
};

#endif /* DEVICEINFO_H_ */
