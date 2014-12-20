
all: kernel.img

include ../kernel/common.mk
include ../SDL2/pkgconfig.mk

OBJS = main.o

-include $(DEPDIR)/*.Po

kernel.img: $(OBJS) Makefile ../kernel/raspberry.ld $(LIBS_DEP)
	$(CXX) $(LDFLAGS) -o kernel.elf $(OBJS) $(LIBS)
	$(OBJCOPY) kernel.elf -O binary kernel.img
	rm kernel.elf

clean:
	rm -f $(OBJS) kernel.elf kernel.img kernel.map
	rm -rf $(DEPDIR)
