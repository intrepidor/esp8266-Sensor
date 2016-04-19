#ifndef NET_CONFIG_H
#define NET_CONFIG_H

#define MAX_SENSOR 7
struct t_sensor {
	const char* const name;
	int id;
};

extern t_sensor sensors[];

extern void config(void);

#endif //NET_CONFIG_H
