#ifndef NET_CONFIG_H
#define NET_CONFIG_H

extern const char sHTTP_DIVBASE[];
extern const char sHTTP_DIVEND[];

extern void config(void);
extern void tsconfig(void);
extern String getWebFooter(bool all, bool ts);
extern void sendHTML_Header(bool sendCSS);

#endif //NET_CONFIG_H
