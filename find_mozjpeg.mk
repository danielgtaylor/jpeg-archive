MOZJPEG_FOUND := 0

ifndef MOZJPEG_PREFIX
	ifneq ($(wildcard /opt/mozjpeg/*),)
		MOZJPEG_FOUND = 1
		MOZJPEG_PREFIX = /opt/mozjpeg
	else ifneq ($(wildcard /usr/local/opt/mozjpeg),)
		MOZJPEG_FOUND = 1
		MOZJPEG_PREFIX = /usr/local/opt/mozjpeg
	endif
else
  MOZJPEG_FOUND = 1
endif

ifeq ($(MOZJPEG_FOUND),1)
	CFLAGS += -I$(MOZJPEG_PREFIX)/include
	LDFLAGS += $(wildcard $(MOZJPEG_PREFIX)/lib*/libjpeg.a)
endif
