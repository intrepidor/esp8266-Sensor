/*
 * network.h
 *
 *  Created on: Jan 21, 2016
 *      Author: Allan Inda
 */

#ifndef NETWORK_H_
#define NETWORK_H_

#include <Arduino.h>
#include <functional>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "WiFiManager.h"         //https://github.com/tzapu/WiFiManager

extern int32_t rssi;
extern const char* const factory_default_ssid;
extern ESP8266WebServer server;
extern String uri_v;

void WebInit(void);
void WebWorker(void);

#endif /* NETWORK_H_ */
