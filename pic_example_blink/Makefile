
CC=/opt/microchip/xc8/v2.40/bin/xc8
CHIP=18f14k50

SOURCES=blink.c
PGM_NAME=blink
PORT=/dev/ttyACM0 

compile: build/$(PGM_NAME).hex

build/$(PGM_NAME).hex: $(SOURCES)
	mkdir -p build
	$(CC) --asmlist --chip=$(CHIP) -Obuild/$(PGM_NAME).hex $^

clean:
	rm -rf build

download: build/$(PGM_NAME).hex
	../client/flasher -p $(PORT) -f $^
