ARDUINO_VARIANT    = nodemcu
SERIAL_PORT = /dev/ttyUSB0
#UPLOAD_SPEED = 115200
#UPLOAD_SPEED = 230400
UPLOAD_SPEED = 460800
#UPLOAD_SPEED = 921600
FLASH_MODE = qio
USER_DEFINE= -D_SSID_=\"YourSSID\" -D_WIFI_PASSWORD_=\"YourPassword\"
include ../ESP8266-Arduino-Makefile/esp8266Arduino.mk

