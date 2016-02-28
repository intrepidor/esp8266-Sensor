ARDUINO_VARIANT    = nodemcu
SERIAL_PORT = COM2
USER_DEFINE= -D_SSID_=\"YourSSID\" -D_WIFI_PASSWORD_=\"YourPassword\"
include ../ESP8266-Arduino-Makefile/esp8266Arduino.mk

