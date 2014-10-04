
CFLAGS += -I../libvorbis/include -I../libogg/include

LIBS += -L../libvorbis -L../libogg -lvorbis -logg
LIBS_DEP += ../libvorbis/libvorbis.a ../libogg/libogg.a
