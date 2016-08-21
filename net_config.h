#ifndef NET_CONFIG_H
#define NET_CONFIG_H

extern const char sHTTP_DIVBASE[];
extern const char sHTTP_DIVEND[];

extern void config(void);
extern void tsconfig(void);
extern void sendHTML_Header(bool sendCSS);

enum menu
	: int { /* enum integer values must be binary bits */
		Main = 1, DeviceConfig = 2, Thingspeak = 4, Utility = 8, Admin = 16
};
extern String getWebFooter(menu m);

#endif //NET_CONFIG_H
