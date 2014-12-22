
TOOLCHAIN := arm-none-eabi-
 
CC := $(TOOLCHAIN)gcc
CXX := $(TOOLCHAIN)g++
LD := $(TOOLCHAIN)ld
AS := $(TOOLCHAIN)as
AR := $(TOOLCHAIN)ar
OBJCOPY := $(TOOLCHAIN)objcopy

DEPDIR := .deps
USPI := 1

PREFIX := /opt/raspberry-pi

ASFLAGS = --warn -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=softfp
CFLAGS = -Wall -O2 -ffreestanding -marm -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=softfp -fsigned-char -I../kernel -D__RASPBERRY_PI__
CPPFLAGS = $(CFLAGS) -fno-exceptions -fno-unwind-tables -fno-rtti
LDFLAGS = -T ../kernel/raspberry.ld -nostartfiles -fno-exceptions -fno-unwind-tables -fno-rtti -Wl,-Map=kernel.map -o kernel.elf

ifeq ($(USPI),1)
CFLAGS += -DHAVE_USPI
endif

LDFLAGS += -L../kernel
LIBS += -lkernel
LIBS_DEP += ../kernel/libkernel.a

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
