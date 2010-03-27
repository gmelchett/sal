CC=gcc
CFLAGS= `imlib2-config --cflags` -Wall -O2 -c -I.
LDFLAGS= `imlib2-config --libs` -lX11 -lXext

all: sll

sll: x.o main.o
	$(CC) $(LDFLAGS) $^ -o $@
