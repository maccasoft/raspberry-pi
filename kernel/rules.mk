# Copyright (c) 2014 Marco Maccaferri and Others
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

TOOLCHAIN := arm-none-eabi-
 
CC := $(TOOLCHAIN)gcc
CXX := $(TOOLCHAIN)g++
LD := $(TOOLCHAIN)ld
AS := $(TOOLCHAIN)as
AR := $(TOOLCHAIN)ar
OBJCOPY := $(TOOLCHAIN)objcopy

PREFIX := /opt/raspberry-pi
DEPDIR := .deps

USPI := 1

ASFLAGS = --warn -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=softfp
CFLAGS = -Wall -O2 -ffreestanding -marm -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=softfp -fsigned-char -I$(PREFIX)/include -D__RASPBERRY_PI__
CPPFLAGS = $(CFLAGS) -fno-exceptions -fno-unwind-tables -fno-rtti
LDFLAGS = -T $(PREFIX)/raspberry.ld -nostartfiles -fno-exceptions -fno-unwind-tables -fno-rtti -Wl,-Map=kernel.map -o kernel.elf -L$(PREFIX)/lib

ifeq ($(USPI),1)
CFLAGS += -DHAVE_USPI
endif

LIBS := -lkernel
LIBS_DEP := $(PREFIX)/lib/libkernel.a

ifneq ("$(strip $(USE_SDL2_IMAGE) $(USE_LIBPNG) $(USE_LIBZ))", "")
LIBS := -lz $(LIBS)
LIBS_DEP += $(PREFIX)/lib/libz.a
endif

ifneq ("$(strip $(USE_SDL2_MIXER) $(USE_LIBVORBIS))", "")
LIBS := -lvorbis -logg $(LIBS)
LIBS_DEP += $(PREFIX)/lib/libogg.a $(PREFIX)/lib/libvorbis.a
endif

ifneq ("$(strip $(USE_SDL2_IMAGE) $(USE_LIBPNG))", "")
LIBS := -lpng $(LIBS)
LIBS_DEP += $(PREFIX)/lib/libpng.a
endif

ifneq ("$(strip $(USE_SDL2) $(USE_SDL2_IMAGE) $(USE_SDL2_MIXER))", "")
LIBS := -lSDL2 $(LIBS)
LIBS_DEP += $(PREFIX)/lib/libSDL2.a
endif

ifneq ("$(strip $(USE_SDL2_IMAGE))", "")
LIBS := -lSDL2_image $(LIBS)
LIBS_DEP += $(PREFIX)/lib/libSDL2_image.a
endif

ifneq ("$(strip $(USE_SDL2_MIXER))", "")
LIBS := -lSDL2_mixer $(LIBS)
LIBS_DEP += $(PREFIX)/lib/libSDL2_mixer.a
endif

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
