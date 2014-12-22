
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
CFLAGS = -DHAVE_CONFIG_H -I. -I../libogg/include -Iinclude -Ilib -O20 -Wall -Wextra -ffast-math -ffreestanding -marm -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=softfp -D_REENTRANT -fsigned-char -Wdeclaration-after-statement 
CPPFLAGS = $(CFLAGS) -fno-exceptions -fno-unwind-tables -fno-rtti

all: libvorbis.a

OBJS = lib/mdct.o lib/smallft.o lib/block.o lib/envelope.o lib/window.o lib/lsp.o lib/lpc.o lib/analysis.o lib/synthesis.o \
       lib/psy.o lib/info.o lib/floor1.o lib/floor0.o lib/res0.o lib/mapping0.o lib/registry.o lib/codebook.o lib/sharedbook.o \
       lib/lookup.o lib/bitrate.o lib/vorbisfile.o

-include $(shell find $(DEPDIR) -name *.Po -print)

libvorbis.a: $(OBJS) Makefile
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
	-mkdir -p $(PREFIX)/include/vorbis
	cp -f include/vorbis/*.h $(PREFIX)/include/vorbis
	-mkdir -p $(PREFIX)/lib
	cp -f libvorbis.a $(PREFIX)/lib

clean:
	rm -f $(OBJS) libvorbis.a
	rm -rf $(DEPDIR)
