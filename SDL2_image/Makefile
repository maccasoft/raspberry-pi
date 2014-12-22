
TOOLCHAIN := arm-none-eabi-
 
CC := $(TOOLCHAIN)gcc
CXX := $(TOOLCHAIN)g++
LD := $(TOOLCHAIN)ld
AS := $(TOOLCHAIN)as
AR := $(TOOLCHAIN)ar
OBJCOPY := $(TOOLCHAIN)objcopy

DEPDIR := .deps
DEFS = -D_REENTRANT -D__RASPBERRY_PI__ -DLOAD_BMP -DLOAD_GIF -DLOAD_PNG -DHAVE_HIDDEN -DSDL_BYTEORDER=SDL_LIL_ENDIAN

PREFIX := /opt/raspberry-pi

ASFLAGS = --warn -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=softfp
CFLAGS = -O3 -I../SDL2/include -I../libpng -ffreestanding -marm -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=softfp $(DEFS)
CPPFLAGS = $(CFLAGS) -fno-exceptions -fno-unwind-tables -fno-rtti

all: deps libSDL2_image.a

.PHONY: deps

OBJS = IMG.o IMG_bmp.o IMG_gif.o IMG_jpg.o IMG_png.o IMG_tif.o IMG_webp.o

-include $(DEPDIR)/*.Po

libSDL2_image.a: $(OBJS) Makefile
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
	-mkdir -p $(PREFIX)/include/SDL2
	cp -f SDL_image.h $(PREFIX)/include/SDL2
	-mkdir -p $(PREFIX)/lib
	cp -f libSDL2_image.a $(PREFIX)/lib

clean:
	rm -f *.o libSDL2_image.a
	rm -rf $(DEPDIR)
