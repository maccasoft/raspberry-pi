
all: libkernel.a

OBJS = \
	platform.o \
	fb.o \
	console.o \
	audio.o \
	wiring.o

-include common.mk
-include $(DEPDIR)/*.Po

libkernel.a: $(OBJS) start.o syscalls.o Makefile
	$(AR) rcs -o $@ $(OBJS)

clean:
	rm -f *.o libkernel.a
	rm -rf $(DEPDIR)
