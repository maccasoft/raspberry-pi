
all: libkernel.a

OBJS = \
	audio.o \
	console.o \
	emmc.o \
	fb.o \
	ff.o \
	platform.o \
	usb.o \
	wiring.o \

-include common.mk
-include $(DEPDIR)/*.Po

libkernel.a: $(OBJS) start.o syscalls.o Makefile raspberry.ld
	$(AR) rcs -o $@ $(OBJS)

clean:
	rm -f $(OBJS) libkernel.a
	rm -rf $(DEPDIR)
