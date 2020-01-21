CC ?= gcc
CFLAGS += -std=c99 -Wall -O3
LDFLAGS += -lm
MAKE ?= make
PREFIX ?= /usr/local

DIR_DEPS =
LIB_DEPS =

all: jpeg-recompress jpeg-compare jpeg-hash

include find_mozjpeg.mk
ifeq ($(MOZJPEG_FOUND),0)
include mozjpeg.mk
endif

LIBIQA = src/iqa/build/release/libiqa.a

$(LIBIQA):
	cd src/iqa; RELEASE=1 $(MAKE)

LIB_DEPS += $(LIBIQA)

jpeg-recompress: jpeg-recompress.c src/util.o src/edit.o src/smallfry.o $(LIB_DEPS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

jpeg-compare: jpeg-compare.c src/util.o src/hash.o src/edit.o src/smallfry.o $(LIB_DEPS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

jpeg-hash: jpeg-hash.c src/util.o src/hash.o $(LIB_DEPS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c %.h | $(DIR_DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

test: test/test.c src/util.o src/edit.o src/hash.o $(LIB_DEPS)
	$(CC) $(CFLAGS) -o test/$@ $^ $(LDFLAGS)
	./test/$@

install: all
	mkdir -p $(PREFIX)/bin
	cp jpeg-archive $(PREFIX)/bin/
	cp jpeg-recompress $(PREFIX)/bin/
	cp jpeg-compare $(PREFIX)/bin/
	cp jpeg-hash $(PREFIX)/bin/

clean:
	rm -rf \
		jpeg-recompress \
		jpeg-compare \
		jpeg-hash \
		test/test \
		src/*.o \
		src/iqa/build

.PHONY: test install clean
