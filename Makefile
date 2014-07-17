CC ?= gcc
CFLAGS += -std=c99 -Wall -O3
LDFLAGS += -lm
MAKE ?= make
PREFIX ?= /usr/local

UNAME_S := $(shell uname -s)
UNAME_P := $(shell uname -p)

ifeq ($(UNAME_S),Linux)
	# Linux (e.g. Ubuntu)
	CFLAGS += -I/opt/libmozjpeg/include
	ifeq ($(UNAME_P),x86_64)
		LIBJPEG = /opt/libmozjpeg/lib64/libjpeg.a
	else
		LIBJPEG = /opt/libmozjpeg/lib/libjpeg.a
	endif
else
	ifeq ($(UNAME_S),Darwin)
		# Mac OS X
		LIBJPEG = /usr/local/opt/mozjpeg/lib/libjpeg.a
		CFLAGS += -I/usr/local/opt/mozjpeg/include
	else
		# Windows
		LIBJPEG = ../mozjpeg/libjpeg.a
		CFLAGS += -I../mozjpeg
		MAKE = mingw32-make
	endif
endif

LIBIQA=src/iqa/build/release/libiqa.a

all: jpeg-recompress jpeg-compare jpeg-hash

$(LIBIQA):
	cd src/iqa; RELEASE=1 $(MAKE)

jpeg-recompress: jpeg-recompress.c src/util.o src/edit.o src/commander.o $(LIBIQA)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBJPEG) $(LDFLAGS)

jpeg-compare: jpeg-compare.c src/util.o src/hash.o src/edit.o src/commander.o $(LIBIQA)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBJPEG) $(LDFLAGS)

jpeg-hash: jpeg-hash.c src/util.o src/hash.o src/commander.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBJPEG) $(LDFLAGS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

test: test.c src/util.o src/edit.o src/hash.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBJPEG) $(LDFLAGS)
	./test

install:
	mkdir -p $(PREFIX)/bin
	cp jpeg-recompress $(PREFIX)/bin/
	cp jpeg-compare $(PREFIX)/bin/
	cp jpeg-hash $(PREFIX)/bin/

clean:
	rm -rf jpeg-recompress jpeg-compare jpeg-hash src/*.o src/iqa/build
