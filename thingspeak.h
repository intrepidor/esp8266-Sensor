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
extern long thinkspeak_total_entries; // response from thingspeak after update

extern String getsThingspeakInfo(String eol);
extern void updateThingspeak(void);

#endif /* THINGSPEAK_H_ */
