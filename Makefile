# makefile
OUT=bin/hexec
CC=gcc
CFLAGS=-g

override CFILES := $(shell cd src && find . -name '*.c')
override SOURCES := $(shell find src -name '*.c')
override OBJ := $(SOURCES:.c=.c.o)
override MODULEDIR := "/home/abdulhadi5692/hxe/modules"

.PHONY: all
all: preDefinedJob $(OUT)

preDefinedJob:
	tools/pregenDefine src/gen/autogen.h $(MODULEDIR)
$(OUT): $(OBJ)
	mkdir -p "$$(dirname $@)"
	$(CC) -g $(OBJ) -o $(OUT)


src/%.c.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -rf $(OUT)
	rm -rf $(OBJ)
	rm -rf a.out
	rm -rf o.map