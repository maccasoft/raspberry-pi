
all:
	make -C kernel
	make -C zlib
	make -C libpng
	make -C libogg
	make -C libvorbis
	make -C SDL2
	make -C SDL2_image
	make -C SDL2_mixer
	make -C template
	make -C template_sdl
	make -C abbaye

install:
	make -C kernel -k install
	make -C zlib -k install
	make -C libpng -k install
	make -C libogg -k install
	make -C libvorbis -k install
	make -C SDL2 -k install
	make -C SDL2_image -k install
	make -C SDL2_mixer -k install

clean:
	make -C kernel -k clean
	make -C zlib -k clean
	make -C libpng -k clean
	make -C libogg -k clean
	make -C libvorbis -k clean
	make -C SDL2 -k clean
	make -C SDL2_image -k clean
	make -C SDL2_mixer -k clean
	make -C template -k clean
	make -C template_sdl -k clean
	make -C abbaye -k clean
