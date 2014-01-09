CC ?= gcc
CFLAGS += -std=c99 -Wall -O3
LDFLAGS += -lm
MAKE ?= make

UNAME_S := $(shell uname -s)
UNAME_P := $(shell uname -p)

ifeq ($(UNAME_S),Linux)
	# Linux (e.g. Ubuntu)
	ifeq ($(UNAME_P),x86_64)
		LIBJPEG = /usr/lib/x86_64-linux-gnu/libjpeg.a
	else
		LIBJPEG = /usr/lib/i386-linux-gnu/libjpeg.a
	endif
else
	ifeq ($(UNAME_S),Darwin)
		# Mac OS X
		LIBJPEG = /usr/local/opt/jpeg-turbo/lib/libjpeg.a
	else
		# Windows
		LIBJPEG = C:\libjpeg-turbo-gcc\lib\libjpeg.a
		CFLAGS += -IC:\libjpeg-turbo-gcc\include
		MAKE = mingw32-make
	endif
endif

LIBIQA=src/iqa/build/release/libiqa.a

all: jpeg-recompress jpeg-compare jpeg-hash

$(LIBIQA):
	cd src/iqa; RELEASE=1 $(MAKE)

jpeg-recompress: jpeg-recompress.c src/util.o src/edit.o src/commander.o $(LIBIQA)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBJPEG) $(LDFLAGS)

jpeg-compare: jpeg-compare.c src/util.o src/hash.o src/commander.o $(LIBIQA)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBJPEG) $(LDFLAGS)

jpeg-hash: jpeg-hash.c src/util.o src/hash.o src/commander.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBJPEG) $(LDFLAGS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

test: test.c src/util.o src/edit.o src/hash.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBJPEG) $(LDFLAGS)
	./test

install:
	cp jpeg-recompress /usr/local/bin/
	cp jpeg-compare /usr/local/bin/
	cp jpeg-hash /usr/local/bin/

clean:
	rm -rf jpeg-recompress jpeg-compare jpeg-hash src/*.o src/iqa/build
