ARDUINO_ARCH = esp8266
ARDUINO_VARIANT = nodemcu
SERIAL_PORT = /dev/ttyUSB0
#UPLOAD_SPEED = 115200
#UPLOAD_SPEED = 230400
UPLOAD_SPEED = 460800
#UPLOAD_SPEED = 921600
FLASH_MODE = qio
USER_DEFINE= -D_SSID_=\"YourSSID\" -D_WIFI_PASSWORD_=\"YourPassword\"
include ../ESP8266-Arduino-Makefile/esp8266Arduino.mk

tidy:
	tidy -upper -modify -wrap 120 -indent \
	--break-before-br true \
	--indent-attributes true \
	--wrap-attributes true \
	--wrap-sections false \
	config.html

lint-std: _LINT.TMP
	./lint.sh *.c*

lint-easy: _LINT.TMP
	./lint.sh easy.lnt local.lnt *.c*

lint-local: _LINT.TMP
	./lint.sh local.lnt *.c*

m: upload
	minicom
	
