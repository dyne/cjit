CC := musl-gcc

include build/init.mk

cflags := -Wall -static -O2 ${cflags_stack_protect}
cflags += -Isrc -Ilib/tinycc -DLIBC_MUSL -nostdlib
cflags += -DKILO_SUPPORTED -DCJIT_BUILD_MUSL

ldadd := lib/tinycc/libtcc.a \
		/usr/lib/x86_64-linux-musl/crt1.o \
		/usr/lib/x86_64-linux-musl/libc.a

SOURCES += \
	src/kilo.o \
	src/embed_libtcc1.a.o \
	src/embed_include.o \
	src/embed_misc.o \
	src/embed_libc.so.o \
	src/musl-symbols.o \
	src/embed_stb.o

# SOURCES += src/embed-musl-libc.o src/musl-symbols.o src/kilo.o

tinycc_config += --config-musl --enable-static
tynycc_config += --extra-cflags=-static --extra-ldflags=-static

all: lib/tinycc/libtcc.a embed cjit

embed: lib/tinycc/libtcc1.a
	$(info Generating assets)
	bash build/init-assetss.sh
	bash build/embed-asset-path.sh lib/tinycc/libtcc1.a
	bash build/embed-asset-path.sh lib/tinycc/include
	bash build/embed-asset-path.sh assets/misc
	bash build/embed-asset-path.sh /lib/x86_64-linux-musl/libc.so
	bash build/embed-asset-path.sh assets/stb
	@echo                 >> src/assets.c
	@echo "return(true);" >> src/assets.c
	@echo "}"             >> src/assets.c
	@echo          >> src/assets.h
	@echo "#endif" >> src/assets.h

cjit: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldadd}

include build/deps.mk
