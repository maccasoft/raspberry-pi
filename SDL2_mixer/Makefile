
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
CFLAGS = -O3 -I. -I../SDL2/include -I../libogg/include -I../libvorbis/include -ffreestanding -marm -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=softfp -D_REENTRANT -D__RASPBERRY_PI__ -DOGG_MUSIC=1 -DWAV_MUSIC=1
CPPFLAGS = $(CFLAGS) -fno-exceptions -fno-unwind-tables -fno-rtti

all: deps libSDL2_mixer.a

.PHONY: deps

OBJS = \
	dynamic_ogg.o \
	effect_position.o \
	effect_stereoreverse.o \
	effects_internal.o \
	load_aiff.o \
	load_ogg.o \
	load_voc.o \
	mixer.o \
	music_ogg.o \
	music.o \
	wavestream.o

-include $(DEPDIR)/*.Po

libSDL2_mixer.a: $(OBJS) Makefile
	$(AR) rcs -o libSDL2_mixer.a $(OBJS)

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
	-mkdir -p $(PREFIX)/include/SDL2
	cp -f SDL_mixer.h $(PREFIX)/include/SDL2
	-mkdir -p $(PREFIX)/lib
	cp -f libSDL2_mixer.a $(PREFIX)/lib

clean:
	rm -f *.o libSDL2_mixer.a
	rm -rf $(DEPDIR)
