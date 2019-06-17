#!/bin/bash
IDEVER="1.8.9"
WORKDIR="/tmp/autobuild_$$"
mkdir -p ${WORKDIR}
# Install Ardino IDE in work directory
if [ -f ~/Downloads/arduino-${IDEVER}-linux64.tar.xz ]
then
    tar xf ~/Downloads/arduino-${IDEVER}-linux64.tar.xz -C ${WORKDIR}
else
    wget -O arduino.tar.xz https://downloads.arduino.cc/arduino-${IDEVER}-linux64.tar.xz
    tar xf arduino.tar.xz -C ${WORKDIR}
    rm arduino.tar.xz
fi
# Create portable sketchbook and library directories
IDEDIR="${WORKDIR}/arduino-${IDEVER}"
LIBDIR="${IDEDIR}/portable/sketchbook/libraries"
mkdir -p "${LIBDIR}"
export PATH="${IDEDIR}:${PATH}"
cd ${IDEDIR}
which arduino
arduino --install-library "WiFiNINA"
arduino --install-library "ArduinoJson"
arduino --pref "compiler.warning_level=default" --save-prefs
arduino --install-boards "arduino:samd"
BOARD="arduino:samd:mkrwifi1010"
arduino --board "${BOARD}" --save-prefs
CC="arduino --verify --board ${BOARD}"
cd ${IDEDIR}/portable/sketchbook
ln -s ~/Sync/howsmyssl/howsmyssl .
$CC howsmyssl/howsmyssl.ino >/tmp/samd21_$$.txt 2>&1
# ESP32
arduino --pref "boardsmanager.additional.urls=https://dl.espressif.com/dl/package_esp32_index.json" --save-prefs
arduino --install-boards "esp32:esp32"
BOARD="esp32:esp32:pico32"
CC="arduino --verify --board ${BOARD}"
$CC howsmyssl/howsmyssl.ino >/tmp/esp32_$$.txt 2>&1
# ESP8266
arduino --pref "boardsmanager.additional.urls=https://arduino.esp8266.com/stable/package_esp8266com_index.json,https://dl.espressif.com/dl/package_esp32_index.json" --save-prefs
arduino --install-boards "esp8266:esp8266"
arduino --pref "custom_CpuFrequency=huzzah_160" \
		--pref "custom_UploadSpeed=huzzah_921600" \
		--pref "custom_baud=huzzah_921600" \
		--pref "custom_dbg=huzzah_Disabled" \
		--pref "custom_eesz=huzzah_4M3M" \
		--pref "custom_exception=huzzah_disabled" \
		--pref "custom_ip=huzzah_lm2f" \
		--pref "custom_lvl=huzzah_None____" \
		--pref "custom_ssl=huzzah_all" \
		--pref "custom_vt=huzzah_flash" \
		--pref "custom_wipe=huzzah_none" \
		--pref "custom_xtal=huzzah_160"  --save-prefs
BOARD="esp8266:esp8266:huzzah"
CC="arduino --verify --board ${BOARD}"
$CC howsmyssl/howsmyssl.ino >/tmp/esp8266_$$.txt 2>&1
# Adafruit Feather M0 atwinc1500
arduino --pref "boardsmanager.additional.urls=https://adafruit.github.io/arduino-board-index/package_adafruit_index.json" --save-prefs
arduino --install-boards "adafruit:samd"
arduino --install-library "WiFi101"
BOARD="adafruit:samd:adafruit_feather_m0"
CC="arduino --verify --board ${BOARD}"
$CC howsmyssl/howsmyssl.ino >/tmp/atwinc1500_$$.txt 2>&1