
# Add debug information and asserts? 
DEBUG_BUILD ?= y

# Location and prefix of cross compiler, Linux kernel style
CROSS_COMPILE ?=

AS	= $(CROSS_COMPILE)as
LD	= $(CROSS_COMPILE)ld
CC	= $(CROSS_COMPILE)gcc
CPP	= $(CC) -E
AR	= $(CROSS_COMPILE)ar
NM	= $(CROSS_COMPILE)nm
STRIP	= $(CROSS_COMPILE)strip
OBJCOPY	= $(CROSS_COMPILE)objcopy
OBJDUMP	= $(CROSS_COMPILE)objdump


CFLAGS	:= -Wall 
LDFLAGS := 

LIBS_CFLAGS  := $(CFLAGS)
LIBS_LDFLAGS := $(LDLAGS) -shared

ifdef DEBUG_BUILD
LIBS_CFLAGS	+= -g
else
LIBS_CFLAGS	+= -DNDEBUG
endif

TESTS_CFLAGS = $(CFLAGS) -g 
TESTS_LDFLAGS = $(LDFLAGS)

LIB_API_VERSION = 1
LIB_VERSION = 1.0.0

CFILES = coproc.c coproc.h

.PHONY: all clean libs tests dist

all: libs tests

# The regular and automagic version of the library

libs: libcoproc.so.$(LIB_VERSION) 

libcoproc.so.$(LIB_VERSION): $(CFILES) Makefile
	$(CC) -fPIC $(CFILES) $(LIBS_CFLAGS) $(LIBS_LDFLAGS) \
		-Wl,-soname,libcoproc.so.$(LIB_API_VERSION) -o libcoproc.so.$(LIB_VERSION)

# Unit tests.
# Make sure to ldconfig -n `pwd` before running

tests: libcoproc.so.$(LIB_VERSION) test.c Makefile
	$(CC) coproc.c test.c $(TESTS_CFLAGS) $(TESTS_LDFLAGS) -o test 
	
dist: clean
	rm -f libcoproc-$(LIB_VERSION).tar.bz2 && \
	rm -rf libcoproc-$(LIB_VERSION) && \
	mkdir libcoproc-$(LIB_VERSION) && \
	cp *.c *.h Makefile README libcoproc-$(LIB_VERSION) && \
	tar jcvf libcoproc-$(LIB_VERSION).tar.bz2 libcoproc-$(LIB_VERSION) 
	rm -rf libcoproc-$(LIB_VERSION)

clean: 
	rm -f *.o test *~ libcoproc.so.$(LIB_VERSION) 
