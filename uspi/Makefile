#
# Makefile
#
# USPi - An USB driver for Raspberry Pi written in C
# Copyright (C) 2014  R. Stange <rsta2@o2online.de>
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
#

TOOLCHAIN := arm-none-eabi-
 
CC := $(TOOLCHAIN)gcc
CXX := $(TOOLCHAIN)g++
LD := $(TOOLCHAIN)ld
AS := $(TOOLCHAIN)as
AR := $(TOOLCHAIN)ar
OBJCOPY := $(TOOLCHAIN)objcopy

DEPDIR := .deps

ASFLAGS = --warn -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=softfp
CFLAGS = -Wall -O2 -ffreestanding -marm -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=softfp -Iinclude -DNDEBUG
CPPFLAGS = $(CFLAGS) -fno-exceptions -fno-unwind-tables -fno-rtti

all: libuspi.a

OBJS = \
	lib/devicenameservice.o \
	lib/dwhcidevice.o \
	lib/dwhciframeschednper.o \
	lib/dwhciframeschednsplit.o \
	lib/dwhciframeschedper.o \
	lib/dwhciregister.o \
	lib/dwhcirootport.o \
	lib/dwhcixferstagedata.o \
	lib/keymap.o \
	lib/macaddress.o \
	lib/smsc951x.o \
	lib/string.o \
	lib/usbconfigparser.o \
	lib/usbdevice.o \
	lib/usbdevicefactory.o \
	lib/usbendpoint.o \
	lib/usbgamepad.o \
	lib/usbkeyboard.o \
	lib/usbmassdevice.o \
	lib/usbmouse.o \
	lib/usbrequest.o \
	lib/usbstandardhub.o \
	lib/uspilibrary.o \
	lib/util.o \
	lib/synchronize.o \
	lib/uspios.o

-include $(DEPDIR)/lib/*.Po

libuspi.a: $(OBJS) Makefile
	$(AR) rcs $@ $(OBJS)

%.o: %.c
	@mkdir -p $(DEPDIR)/$(@D)
	$(CC) $(CFLAGS) -std=c99 -MD -MP -MF $(DEPDIR)/$*.Tpo -c -o $@ $<
	@mv -f $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Po

%.o: %.cpp
	@mkdir -p $(DEPDIR)/$(@D)
	$(CXX) $(CPPFLAGS) -MD -MP -MF $(DEPDIR)/$*.Tpo -c -o $@ $<
	@mv -f $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Po

clean:
	rm -f $(OBJS) libuspi.a
	rm -rf $(DEPDIR)
