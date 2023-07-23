flasher: main.c serial.c
	gcc -o $@ $^


test:
	./flasher -p /dev/ttyACM0 -f d.hex

clean:
	rm -rf *.o flasher
