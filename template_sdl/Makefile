TOOLCHAIN := arm-none-eabi
 
CC := $(TOOLCHAIN)-gcc
CXX := $(TOOLCHAIN)-g++
LD := $(TOOLCHAIN)-ld
AS := $(TOOLCHAIN)-as
AR := $(TOOLCHAIN)-ar
OBJCOPY := $(TOOLCHAIN)-objcopy

DEPDIR := .deps

all: kernel.img

ASFLAGS = --warn -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=softfp
CFLAGS = -Wall -O2 -ffreestanding -marm -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=softfp -D__RASPBERRY_PI__
CPPFLAGS = $(CFLAGS) -fno-exceptions -fno-unwind-tables -fno-rtti
LDFLAGS = -T ../kernel/raspberry.ld -nostartfiles -fno-exceptions -fno-unwind-tables -fno-rtti -Wl,-Map=kernel.map -o kernel.elf

include ../SDL2/pkgconfig.mk
include ../csud/pkgconfig.mk

OBJS = \
	../kernel/start.o \
	../kernel/syscalls.o \
	../kernel/platform.o \
	main.o

-include $(DEPDIR)/*.Po

kernel.img: $(OBJS) Makefile ../kernel/raspberry.ld $(LIBS_DEP)
	$(CXX) $(LDFLAGS) -o kernel.elf $(OBJS) $(LIBS)
	$(OBJCOPY) kernel.elf -O binary kernel.img
	rm kernel.elf

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

%.o: %.png
	$(LD) -r -b binary -o $@ $<

%.o: %.ogg
	$(LD) -r -b binary -o $@ $<

%.o: %.txt
	$(LD) -r -b binary -o $@ $<

clean:
	rm -f *.o kernel.elf kernel.img kernel.map
	rm -rf $(DEPDIR)
