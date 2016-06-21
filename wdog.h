/*
 * wdog.h
 *
 *  Created on: Jun 16, 2016
 *      Author: allan
 */

#ifndef WDOG_H_
#define WDOG_H_

// There is one watchdog lasttime variable for each task being monitored
enum class TaskName {
	menu = 0, pir, acquire, thingspeak, led, webserver, NUM_TASKS
};
const int NUM_TASKS = static_cast<int>(TaskName::NUM_TASKS);

extern unsigned long wdog_timer[NUM_TASKS];

extern void kickExternalWatchdog(void);
extern void kickAllSoftwareWatchdogs(void);
extern void kickAllWatchdogs(void);
extern void setStartupComplete(void);

#endif /* WDOG_H_ */
