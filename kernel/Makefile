
all: libkernel.a

OBJS = \
	audio.o \
	console.o \
	emmc.o \
	fb.o \
	platform.o \
	wiring.o

-include common.mk
-include $(DEPDIR)/*.Po

libkernel.a: $(OBJS) start.o syscalls.o Makefile raspberry.ld
	$(AR) rcs -o $@ $(OBJS)

clean:
	rm -f *.o libkernel.a
	rm -rf $(DEPDIR)
