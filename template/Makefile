
all: kernel.img

include ../kernel/common.mk
include ../uspi/pkgconfig.mk

OBJS = \
	../kernel/start.o \
	../kernel/syscalls.o \
	main.o

CFLAGS += -DHAVE_USPI

-include $(DEPDIR)/*.Po

kernel.img: $(OBJS) Makefile ../kernel/raspberry.ld $(LIBS_DEP)
	$(CXX) $(LDFLAGS) $(OBJS) $(LIBS)
	$(OBJCOPY) kernel.elf -O binary kernel.img
	rm kernel.elf

clean:
	rm -f *.o kernel.elf kernel.img kernel.map
	rm -rf $(DEPDIR)
