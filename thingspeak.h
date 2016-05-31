/*
 * thingspeak.h
 *
 *  Created on: May 30, 2016
 *      Author: allan
 */

#ifndef THINGSPEAK_H_
#define THINGSPEAK_H_

extern size_t thingspeak_update_counter;
extern size_t thingspeak_error_counter;

extern void printThingspeakInfo(void);
extern void updateThingspeak(void);

#endif /* THINGSPEAK_H_ */
