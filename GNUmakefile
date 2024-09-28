# Just a start at a simple build system targeting only musl static
BRANCH := $(shell git symbolic-ref HEAD | cut -d/ -f3-)
COMMIT := $(shell git rev-parse --short HEAD)
VERSION := $(shell git describe --tags | cut -d- -f1)
CURRENT_YEAR := $(shell date +%Y)

cc := musl-gcc
cflags := -static
# cflags += -fstack-protector-all -D_FORTIFY_SOURCE=2 -fno-strict-overflow
cflags += -Og -ggdb -DDEBUG=1 -Wall -Wextra -pedantic
cflags += -Isrc -Ilib/tinycc -DARCH=\"MUSL\"

SOURCES := src/io.o src/file.o src/cflag.o \
	src/cjit.o src/embed-libtcc1.o

ldadd := lib/tinycc/libtcc.a

all: deps cjit

cjit: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldadd}

deps: lib/tinycc/libtcc.a

lib/tinycc/libtcc.a:
	cd lib/tinycc && ./configure \
		--config-musl --cc=musl-gcc --enable-static --extra-cflags=-static \
		--extra-ldflags=-static --debug \
	&& ${MAKE} libtcc.a libtcc1.a
	sh build/embed-libtcc1.sh

clean:
	${MAKE} -C lib/tinycc clean distclean
	${MAKE} -C src clean

.c.o:
	$(cc) \
	$(cflags) \
	-c $< -o $@ \
	-DVERSION=\"${VERSION}\" \
	-DCURRENT_YEAR=\"${CURRENT_YEAR}\" \
	-DCOMMIT=\"${COMMIT}\" \
	-DBRANCH=\"${BRANCH}\" \
	-DCFLAGS="${cflags}"
