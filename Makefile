CC=gcc
CFLAGS= `imlib2-config --cflags` -Wall -O2 -c -I. -DVERSION="\"0.1\""
LDFLAGS= `imlib2-config --libs` -lX11 -lXext -lm

all: sal

sal: bubble.o image.o aquarium.o background.o window.o fish.o leds.o \
	thermometer.o cpuload_linux.o analog-clock.o fuzzy-clock.o \
	digital-clock.o sun.o suncalc.o date.o diagram.o
	$(CC) $(LDFLAGS) $^ -o $@
clean:
	rm -f sal *.o *~

aquarium.o: aquarium.c config.h sal.h fish.h background.h window.h bubble.h \
	image.h leds.h thermometer.h analog-clock.h fuzzy-clock.h \
	digital-clock.h sun.h date.h
background.o: background.c background.h config.h sal.h window.h aquarium.h image.h
bubble.o: bubble.c bubble.h image.h sal.h config.h window.h aquarium.h
image.o: image.c image.h config.h sal.h
window.o: window.c window.h sal.h config.h aquarium.h
fish.o: fish.c fish.h image.h sal.h config.h aquarium.h window.h
leds.o: leds.c leds.h image.h sal.h config.h window.h aquarium.h
thermometer.o: thermometer.c thermometer.h image.h sal.h config.h window.h aquarium.h cpuload.h
cpuload_linux.o: cpuload_linux.c cpuload.h
analog-clock.o: analog-clock.c analog-clock.h sal.h config.h window.h aquarium.h
fuzzy-clock.o: fuzzy-clock.c fuzzy-clock.h sal.h config.h window.h aquarium.h
digital-clock.o: digital-clock.c digital-clock.h sal.h config.h window.h aquarium.h
suncalc.o: suncalc.c suncalc.h
sun.o: sun.c sun.h suncalc.h sal.h config.h window.h aquarium.h
date.o: date.c date.h sal.h config.h window.h aquarium.h
diagram.o: diagram.c diagram.h sal.h config.h window.h aquarium.h cpuload.h


