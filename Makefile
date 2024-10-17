CC = gcc
CFLAGS = -O3 -Wall -Iinclude

RESPONDER_SRC = decoder.c descriptors.c responder.c worker.c
DUMPSCREEN_SRC = dumpscreen.c

.PHONY: all clean

all: displaylink-responder displaylink-dumpscreen

displaylink-responder: $(RESPONDER_SRC:%.c=src/%.c)
	$(CC) $(CFLAGS) -pthread $^ -o $@

displaylink-dumpscreen: $(DUMPSCREEN_SRC:%.c=src/%.c)
	$(CC) $(CFLAGS) -lpng $^ -o $@

clean:
	rm displaylink-dumpscreen displaylink-responder
