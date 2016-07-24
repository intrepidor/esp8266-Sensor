/*
 * thingspeak.h
 *
 *  Created on: May 30, 2016
 *      Author: allan
 */

#ifndef THINGSPEAK_H_
#define THINGSPEAK_H_

int const MAX_THINGSPEAK_FIELD_COUNT = 8; // Thingspeak only accept 8 fields

extern size_t thingspeak_update_counter;
extern size_t thingspeak_error_counter;
extern long thinkspeak_total_entries; // response from thingspeak after update
extern unsigned long last_thingspeak_update_time_ms;

extern String getsThingspeakInfo(String eol);
extern void updateThingspeak(void);
extern String getsThingspeakChannelInfo(String eol);
extern void ThingspeakPushChannelSettings(void);

#endif /* THINGSPEAK_H_ */
