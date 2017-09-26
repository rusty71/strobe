ARDUINO_PATH = /usr/local/share/arduino/
SKETCHBOOK   =  $(CURDIR)
SKETCH       = strobe
TARGET_DIR   = $(SKETCHBOOK)/build
MONITOR_PORT = /dev/ttyUSB0

all:
	@ mkdir -p $(TARGET_DIR)

	$(ARDUINO_PATH)/arduino-builder -compile -logger=machine \
	-hardware "$(ARDUINO_PATH)/hardware" \
	-hardware "$(SKETCHBOOK)/hardware" \
	-tools "$(ARDUINO_PATH)/tools-builder" \
	-tools "$(ARDUINO_PATH)/hardware/tools/avr" \
	-built-in-libraries "$(ARDUINO_PATH)/libraries" \
	-libraries "$(SKETCHBOOK)/lib" \
	-fqbn=arduino:avr:uno \
	-ide-version=10606 \
	-build-path "$(TARGET_DIR)" \
	-warnings=none \
	-prefs=build.warn_data_percentage=75 \
	-verbose "src/$(SKETCH).ino"

flash:
	avrdude -C/usr/local/share/arduino-1.6.9/hardware/tools/avr/etc/avrdude.conf -carduino -patmega328p -P/dev/ttyUSB0 -b57600 -D -Uflash:w:build/$(SKETCH).ino.hex
	#/usr/local/share/arduino-1.6.9/hardware/tools/avr/bin/avrdude -C/usr/local/share/arduino-1.6.9/hardware/tools/avr/etc/avrdude.conf -q -q -patmega328p -carduino -P/dev/ttyUSB0 -b57600 -D -Uflash:w:/tmp/build8090c44de114e676e2ec6b77af5e7200.tmp/Blink.ino.hex:i

upload: all flash

clean:
	rm -rf $(TARGET_DIR)

monitor:
	screen $(MONITOR_PORT) 115200
