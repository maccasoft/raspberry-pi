
all: libkernel.a

-include common.mk

OBJS = \
	start.o \
	audio.o \
	console.o \
	emmc.o \
	fb.o \
	fatfs/diskio.o \
	fatfs/ff.o \
	fatfs/option/ccsbcs.o \
	fatfs/option/syscall.o \
	platform.o \
	syscalls.o \
	usb.o \
	wiring.o

ifeq ($(USPI),1)
OBJS += \
	uspi/devicenameservice.o \
	uspi/dwhcidevice.o \
	uspi/dwhciframeschednper.o \
	uspi/dwhciframeschednsplit.o \
	uspi/dwhciframeschedper.o \
	uspi/dwhciregister.o \
	uspi/dwhcirootport.o \
	uspi/dwhcixferstagedata.o \
	uspi/keymap.o \
	uspi/macaddress.o \
	uspi/smsc951x.o \
	uspi/string.o \
	uspi/usbconfigparser.o \
	uspi/usbdevice.o \
	uspi/usbdevicefactory.o \
	uspi/usbendpoint.o \
	uspi/usbgamepad.o \
	uspi/usbkeyboard.o \
	uspi/usbmassdevice.o \
	uspi/usbmouse.o \
	uspi/usbrequest.o \
	uspi/usbstandardhub.o \
	uspi/usbstring.o \
	uspi/uspilibrary.o \
	uspi/util.o \
	uspi/synchronize.o \
	uspi/uspios.o
endif

-include $(DEPDIR)/*.Po

libkernel.a: $(OBJS) start.o syscalls.o Makefile raspberry.ld
	$(AR) rcs -o $@ $(OBJS)

install:
	-mkdir -p $(PREFIX)/include
	cp -f audio.h console.h emmc.h fb.h kernel.h platform.h wiring.h $(PREFIX)/include
	-mkdir -p $(PREFIX)/include/fatfs
	cp -f fatfs/ff.h fatfs/ffconf.h fatfs/integer.h $(PREFIX)/include/fatfs
	cp -f platform.inc $(PREFIX)/include
ifeq ($(USPI),1)
	cp -f uspi.h $(PREFIX)/include
endif
	-mkdir -p $(PREFIX)/lib
	cp -f libkernel.a $(PREFIX)/lib
	cp -f raspberry.ld $(PREFIX)
	cp -f rules.mk $(PREFIX)

clean:
	rm -f $(OBJS) libkernel.a
	rm -rf $(DEPDIR)
