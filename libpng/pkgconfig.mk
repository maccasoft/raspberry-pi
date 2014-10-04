
CFLAGS += -I../libpng -I../zlib

LIBS += -L../libpng -lpng -L../zlib -lz
LIBS_DEP += ../libpng/libpng.a ../zlib/libz.a
