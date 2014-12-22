
TOOLCHAIN := arm-none-eabi-
 
CC := $(TOOLCHAIN)gcc
CXX := $(TOOLCHAIN)g++
LD := $(TOOLCHAIN)ld
AS := $(TOOLCHAIN)as
AR := $(TOOLCHAIN)ar
OBJCOPY := $(TOOLCHAIN)objcopy

DEPDIR := .deps

PREFIX := /opt/raspberry-pi

ASFLAGS = --warn -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=softfp
CFLAGS = -O3 -Iinclude -Isrc -I../kernel -ffreestanding -fvisibility=hidden -marm -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=softfp -D_REENTRANT -D__RASPBERRY_PI__ -DHAVE_USPI
CPPFLAGS = $(CFLAGS) -fno-exceptions -fno-unwind-tables -fno-rtti

all: libSDL2.a

OBJS = $(shell find . -name *.c -print | sed -r 's/\.c+/\.o/g')

-include $(shell find $(DEPDIR) -name *.Po -print)

libSDL2.a: $(OBJS) Makefile
	$(AR) rcs $@ $(OBJS)

%.o: %.c
	@mkdir -p $(DEPDIR)/$(@D)
	$(CC) $(CFLAGS) -std=c99 -MD -MP -MF $(DEPDIR)/$*.Tpo -c -o $@ $<
	@mv -f $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Po

%.o: %.cpp
	@mkdir -p $(DEPDIR)/$(@D)
	$(CXX) $(CPPFLAGS) -MD -MP -MF $(DEPDIR)/$*.Tpo -c -o $@ $<
	@mv -f $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Po

%.o: %.s
	$(AS) $(ASFLAGS) -o $@ $<

install:
	-mkdir -p $(PREFIX)/include/SDL2
	cp -f include/* $(PREFIX)/include/SDL2
	-mkdir -p $(PREFIX)/lib
	cp -f libSDL2.a $(PREFIX)/lib

clean:
	rm -f $(OBJS) libSDL2.a
	rm -rf $(DEPDIR)
