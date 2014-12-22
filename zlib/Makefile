
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
CFLAGS = -O3 -ffreestanding -marm -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=softfp -D_LARGEFILE64_SOURCE=1 -DHAVE_HIDDEN -I. 
CPPFLAGS = $(CFLAGS) -fno-exceptions -fno-unwind-tables -fno-rtti

all: libz.a

OBJS = adler32.o crc32.o  deflate.o infback.o inffast.o inflate.o inftrees.o trees.o zutil.o \
       compress.o uncompr.o \
       gzclose.o gzlib.o gzread.o gzwrite.o

-include $(DEPDIR)/*.Po

libz.a: $(OBJS) Makefile
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
	-mkdir -p $(PREFIX)/include
	cp -f zlib.h zconf.h $(PREFIX)/include
	-mkdir -p $(PREFIX)/lib
	cp -f libz.a $(PREFIX)/lib

clean:
	rm -f *.o libz.a
	rm -rf $(DEPDIR)
