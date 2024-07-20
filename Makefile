CC = gcc
CFLAGS = -O3 -Wall

.PHONY: all clean

all: displaylink-responder displaylink-dumpscreen

displaylink-responder: decoder.c descriptors.c responder.c worker.c
	$(CC) $(CFLAGS) -pthread -o displaylink-responder decoder.c descriptors.c responder.c worker.c

displaylink-dumpscreen: dumpscreen.c
	$(CC) $(CFLAGS) -o displaylink-dumpscreen dumpscreen.c -lpng 

clean:
	rm displaylink-dumpscreen displaylink-responder
