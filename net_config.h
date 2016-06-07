#ifndef NET_CONFIG_H
#define NET_CONFIG_H

extern const char sHTTP_END[];
extern const char sHTTP_DIVBASE[];
extern const char sHTTP_DIVEND[];
extern const char sHTTP_BR[];

extern void config(void);
extern String getWebFooter(bool all);
extern void sendHTML_Header(bool sendCSS);

#endif //NET_CONFIG_H
