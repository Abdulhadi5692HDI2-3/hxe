# makefile
OUT=hexec
CC=gcc
CFLAGS=-g

override CFILES := $(shell cd src && find -L * -type f -name '*.c' | LC_ALL=C sort)
override OBJ := $(CFILES:.c=.c.o)

.PHONY: all
all: preDefinedJob bin/$(OUT)

preDefinedJob:
	tools/pregenDefine src/gen/autogen.h
bin/$(OUT): $(OBJ)
	mkdir -p "$$(dirname $@)"
	$(CC) -g $(OBJ) -o $(OUT)


%.c.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -rf bin/$(OUT) $(OBJ)
	rm -rf bin
	rm -rf hexec
	rm -rf a.out
	rm -rf o.map