/*
 * util.cpp
 *
 *  Created on: Apr 24, 2016
 *      Author: allan
 */

#include <Arduino.h>
#include "network.h"

String localIPstr(void) {
	IPAddress ip = WiFi.localIP();
	String s = String(ip[0]) + String(".");
	s += String(ip[1]) + String(".");
	s += String(ip[2]) + String(".");
	s += String(ip[3]);
	return s;
}

