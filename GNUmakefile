# Just a start at a simple build system targeting only musl static

cc := musl-gcc

all: deps

deps:
	cd lib/tinycc && ./configure \
		--config-musl --cc=musl-gcc --enable-static --extra-cflags=-static \
		--extra-ldflags=-static --debug \
	&& ${MAKE} libtcc.a libtcc1.a

clean:
	${MAKE} -C lib/tinycc clean distclean
