include build/init.mk

cc := clang
cflags += -DCJIT_BUILD_OSX

SOURCES += \
	src/kilo.o \
	src/embed_libtcc1.a.o \
	src/embed_include.o \
	src/embed_misc.o \
	src/embed_stb.o

all: embed cjit.command

embed: lib/tinycc/libtcc1.a
	$(info Generating assets)
	bash build/init-assets.sh
	bash build/embed-asset-path.sh lib/tinycc/libtcc1.a
	bash build/embed-asset-path.sh lib/tinycc/include
	bash build/embed-asset-path.sh assets/misc
	bash build/embed-asset-path.sh assets/stb
	@echo                 >> src/assets.c
	@echo "return(true);" >> src/assets.c
	@echo "}"             >> src/assets.c
	@echo          >> src/assets.h
	@echo "#endif" >> src/assets.h

cjit.command: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldflags} ${ldadd}

include build/deps.mk
