CMAKE ?= cmake
AUTORECONF ?= autoreconf

DIR_DEPS += src/mozjpeg

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

src/mozjpeg:
	git clone -b v3.3.1 --single-branch https://github.com/mozilla/mozjpeg.git $@
ifeq ($(OS),Unixy)
	cd $@ && \
		$(AUTORECONF) -fiv && \
		./configure --with-jpeg8
else
	cd $@ && \
		$(CMAKE) -G "MSYS Makefiles" \
			-DCMAKE_C_COMPILER=$(CC) \
			-DCMAKE_MAKE_PROGRAM=$(MAKE) \
			-DCMAKE_BUILD_TYPE=RELEASE \
			-DWITH_JPEG8=1 .
endif

$(LIBJPEG): | $(DIR_DEPS)
	cd $| && $(MAKE)

LIB_DEPS += $(LIBJPEG)

mozjpegclean:
	cd src/mozjpeg && $(MAKE) clean

fullclean: clean
	rm -rf src/mozjpeg

.PHONY: fullclean mozjpegclean
