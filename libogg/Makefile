
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
CFLAGS = -DHAVE_CONFIG_H -I. -Iinclude -O20 -Wall -ffreestanding -marm -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=softfp -ffast-math -fsigned-char -g -O2 
CPPFLAGS = $(CFLAGS) -fno-exceptions -fno-unwind-tables -fno-rtti

all: deps libogg.a

.PHONY: deps

OBJS = bitwise.o framing.o

-include $(DEPDIR)/*.Po

libogg.a: $(OBJS) Makefile
	$(AR) rcs $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -std=c99 -MD -MP -MF $(DEPDIR)/$*.Tpo -c -o $@ $<
	@mv -f $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Po

%.o: %.cpp
	$(CXX) $(CPPFLAGS) -MD -MP -MF $(DEPDIR)/$*.Tpo -c -o $@ $<
	@mv -f $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Po

%.o: %.s
	$(AS) $(ASFLAGS) -o $@ $<

deps:
	-@mkdir -p $(DEPDIR)

install:
	-mkdir -p $(PREFIX)/include/ogg
	cp -f include/ogg/*.h $(PREFIX)/include/ogg
	-mkdir -p $(PREFIX)/lib
	cp -f libogg.a $(PREFIX)/lib

clean:
	rm -f *.o libogg.a
	rm -rf $(DEPDIR)
