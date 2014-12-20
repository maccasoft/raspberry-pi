
all: kernel.img

include ../kernel/common.mk

include ../libvorbis/pkgconfig.mk
include ../libpng/pkgconfig.mk
include ../SDL2/pkgconfig.mk
include ../SDL2_image/pkgconfig.mk
include ../SDL2_mixer/pkgconfig.mk

OBJS = \
	data/enemies.o data/map.o \
	graphics/blueparch.o graphics/fonts.o graphics/history.o graphics/intro.o graphics/parchment2.o \
	graphics/parchment4.o graphics/parchment6.o graphics/redparch.o graphics/ending.o graphics/gameover.o \
	graphics/intromd.o graphics/parchment1.o graphics/parchment3.o graphics/parchment5.o graphics/parchment.o \
	graphics/tiles.o \
	sounds/AreaIChurchN.o sounds/doorfx.o sounds/HangmansTree.o sounds/MainTitleN.o sounds/mechanismn.o sounds/shoot.o \
	sounds/AreaIICavesV2N.o sounds/EvilFightN.o sounds/Item.o sounds/ManhuntN.o sounds/onedeathn.o sounds/slash.o \
	sounds/AreaIIIHellN.o sounds/GameOverV2N.o sounds/jump.o sounds/ManhuntwoodN.o sounds/PrayerofHopeN.o \
	src/drawing.o src/ending.o src/enemies.o src/game.o src/gameover.o src/history.o src/jean.o src/loading.o src/startscreen.o \
	src/main.o

CFLAGS += -finline-functions -funswitch-loops -fpredictive-commoning -fgcse-after-reload -ftree-vectorize

-include $(DEPDIR)/*.Po

kernel.img: $(OBJS) Makefile ../kernel/raspberry.ld $(LIBS_DEP)
	$(CXX) $(LDFLAGS) $(OBJS) $(LIBS)
	$(OBJCOPY) kernel.elf -O binary kernel.img
	rm kernel.elf

%.o: %.png
	$(LD) -r -b binary -o $@ $<

%.o: %.ogg
	$(LD) -r -b binary -o $@ $<

%.o: %.txt
	$(LD) -r -b binary -o $@ $<

clean:
	rm -f $(OBJS) kernel.elf kernel.img kernel.map
	rm -rf $(DEPDIR)
