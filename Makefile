CC ?= gcc
CFLAGS += -std=c99 -Wall -O3
LDFLAGS += -lm
MAKE ?= make
AUTORECONF ?= autoreconf
CMAKE ?= cmake
PREFIX ?= /usr/local

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	OS = Unixy
else ifeq ($(UNAME_S),Darwin)
	OS = Unixy
else ifeq ($(UNAME_S),FreeBSD)
	OS = Unixy
else
	OS = Windows
endif

ifeq ($(OS),Unixy)
	LIBJPEG = src/mozjpeg/.libs/libjpeg.a
else
	LIBJPEG = src/mozjpeg/libjpeg.a
endif
CFLAGS += -Isrc/mozjpeg

LIBIQA=src/iqa/build/release/libiqa.a

all: jpeg-recompress jpeg-compare jpeg-hash

src/mozjpeg:
	git clone -b v3.3.1 --single-branch https://github.com/mozilla/mozjpeg.git $@

$(LIBJPEG): src/mozjpeg
ifeq ($(OS),Unixy)
	cd $< && \
		$(AUTORECONF) -fiv && \
		./configure --with-jpeg8 && \
		$(MAKE)
else
	cd $< && \
		$(CMAKE) -G "MSYS Makefiles" \
			-DCMAKE_C_COMPILER=$(CC) \
			-DCMAKE_MAKE_PROGRAM=$(MAKE) \
			-DCMAKE_BUILD_TYPE=RELWITHDEBINFO \
			-DWITH_JPEG8=1 . && \
		$(MAKE)
endif
	touch $@

$(LIBIQA):
	cd src/iqa; RELEASE=1 $(MAKE)

jpeg-recompress: jpeg-recompress.c src/util.o src/edit.o src/smallfry.o src/commander.o $(LIBIQA) $(LIBJPEG)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

jpeg-compare: jpeg-compare.c src/util.o src/hash.o src/edit.o src/commander.o src/smallfry.o $(LIBIQA) $(LIBJPEG)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

jpeg-hash: jpeg-hash.c src/util.o src/hash.o src/commander.o $(LIBJPEG)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c %.h src/mozjpeg
	$(CC) $(CFLAGS) -c -o $@ $<

test: test/test.c src/util.o src/edit.o src/hash.o $(LIBJPEG)
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
		src/iqa/build \
		src/mozjpeg

.PHONY: test install clean
