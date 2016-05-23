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

const int CURRENT_CONFIG_VERSION = 123;
const int DECIMAL_PRECISION = 5;

//extern void validate_string(char* str, const char* const def, unsigned int size, int lowest, int highest);

//-----------------------------------------------------------------------------------
enum class portModes {
	off = 0, dht11, dht22, ds18b20, sonar, dust, sound, END
};
struct t_sensor {
	const char* const name;
	portModes id;
};
#if 0
// Comment out because PC Lint is not ready
const std::array<portModes, static_cast<int>(portModes::END)> allModes = {portModes::off,
	portModes::dht11, portModes::dht22, portModes::ds18b20, portModes::sonar,
	portModes::dust, portModes::sound};
/* The array portModes and the enum portModes must match with one exception, the END
 * value is not copied into the portModes array.
 */
#endif
extern t_sensor const sensors[];

//-----------------------------------------------------------------------------------
// Device Class
const int MAX_PORTS = 6;
const int MAX_ADJ = 3;
const int STRING_LENGTH = 20;
static bool isValidPort(int portnum);

struct cport {
	char name[STRING_LENGTH + 1];
	portModes mode;
	int pin;
	double adj[MAX_ADJ];
};

class Device {
private:
	struct {
		// EEPROM
		int config_version; // version of the configuration data format in the EEPROM

		// Device
		struct {
			char name[STRING_LENGTH + 1];
			int id;
		} device;

		// Thingspeak'
		struct {
			int status;
			// bit value
			// 1 means "HTTP/1.1 200 OK" received
			// 2 means "Status: 200 OK" received
			// 4 means "connection failure"
			char enabled;
			char apikey[STRING_LENGTH + 1];
			char host[STRING_LENGTH + 1];
			char ipaddr[16]; // nnn.nnn.nnn.nnn = 4*3+3(dots)+1(nul) = 16 chars; 15 for data, and 1 for terminating nul
		} thingspeak;

		// Ports
		cport port[MAX_PORTS];
	} db;

public:
// Default Constructor
	Device() {
		memset(&db, 0, sizeof(db));
	}
	void init(void);
	String toString(void);

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
	void printInfo(void);
//--------------------------------------------------------
// Ports
	int getPortMax(void) {
		return MAX_PORTS;
	}
	portModes getPortMode(int portnum);
	void setPortMode(int portnum, portModes _mode);
	void setPortName(int portnum, String _n);
	String getPortName(int portnum);
	int getPortAdjMax(void) {
		return MAX_ADJ;
	}
	double getPortAdj(int portnum, int adjnum);
	void setPortAdj(int portnum, int adjnum, double v);
	String getModeStr(int portnum);

//--------------------------------------------------------
// EEPROM and DB Stuff
	void RestoreConfigurationFromEEPROM(void);
	bool StoreConfigurationIntoEEPROM(void);

//--------------------------------------------------------
// Thingspeak
	void printThingspeakInfo(void);
	void updateThingspeak(void);

	int getThingspeakStatus() const {
		return db.thingspeak.status;
	}

	void setThingspeakApikey(const char* _apikey) {
		if (_apikey) {
			strncpy(this->db.thingspeak.apikey, _apikey,
					sizeof(db.thingspeak.apikey) - 1);
		}
	}
	void setThingspeakApikey(String _apikey) {
		if (_apikey) {
			strncpy(this->db.thingspeak.apikey, _apikey.c_str(),
					sizeof(db.thingspeak.apikey) - 1);
		}
	}
	String getThinkspeakApikey() const {
		return String(db.thingspeak.apikey);
	}
	const char* getcThinkspeakApikey() const {
		return db.thingspeak.apikey;
	}

	void setThingspeakHost(const char* _host) {
		if (_host) {
			strncpy(this->db.thingspeak.host, _host,
					sizeof(this->db.thingspeak.host) - 1);
		}
	}
	void setThingspeakHost(String _host) {
		setThingspeakHost(_host.c_str());
	}
	String getThingspeakHost() const {
		return String(db.thingspeak.host);
	}
	const char* getcThingspeakHost() const {
		return db.thingspeak.host;
	}

	void setIpaddr(String _ipaddr) {
		setIpaddr(_ipaddr.c_str());
	}
	void setIpaddr(const char* _ipaddr) {
		if (_ipaddr) {
			strncpy(this->db.thingspeak.ipaddr, _ipaddr,
					sizeof(db.thingspeak.ipaddr) - 1);
		}
	}
	String getIpaddr(void) const {
		return String(db.thingspeak.ipaddr);
	}
	const char* getcIpaddr(void) const {
		return db.thingspeak.ipaddr;
	}

	void setEnable(bool _enable) {
		this->db.thingspeak.enabled = _enable;
	}
	bool getEnable(void) const {
		if (db.thingspeak.enabled) return true;
		return false;
	}
	String getEnableString(void) const {
		if (db.thingspeak.enabled) return String("true");
		return String("false");
	}
	const char* PROGMEM getEnableStr();
};

#endif /* DEVICEINFO_H_ */
