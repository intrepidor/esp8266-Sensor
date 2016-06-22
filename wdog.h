/*
 * wdog.h
 *
 *  Created on: Jun 16, 2016
 *      Author: allan
 */

#ifndef WDOG_H_
#define WDOG_H_

// There is one watchdog lasttime variable for each task being monitored
enum taskname_t {
	taskname_menu = 0,
	taskname_pir,
	taskname_acquire,
	taskname_thingspeak,
	taskname_led,
	taskname_webserver,
	taskname_NUM_TASKS
};

extern unsigned long wdog_timer[static_cast<int>(taskname_NUM_TASKS)];

extern void kickExternalWatchdog(void);
extern void kickAllSoftwareWatchdogs(void);
extern void kickAllWatchdogs(void);
extern void setStartupComplete(void);

#endif /* WDOG_H_ */
