CC = gcc
CFLAGS = -O3 -Wall -Iinclude

RESPONDER_SRC = decoder.c descriptors.c responder.c worker.c
DUMPSCREEN_SRC = dumpscreen.c

.PHONY: all clean

all: dlresp dlresp-dumpscreen

dlresp: $(RESPONDER_SRC:%.c=src/%.c)
	$(CC) $(CFLAGS) -pthread $^ -o $@

dlresp-dumpscreen: $(DUMPSCREEN_SRC:%.c=src/%.c)
	$(CC) $(CFLAGS) -lpng $^ -o $@

clean:
	rm dlresp-dumpscreen dlresp
