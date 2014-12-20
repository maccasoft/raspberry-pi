
all: libkernel.a

-include common.mk

OBJS = \
	start.o \
	audio.o \
	console.o \
	emmc.o \
	fb.o \
	ff.o \
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

clean:
	rm -f $(OBJS) start.o syscalls.o libkernel.a
	rm -rf $(DEPDIR)
