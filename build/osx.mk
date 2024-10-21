include build/init.mk

cc := clang

all: deps cjit.command

cjit.command: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldflags} ${ldadd}

.c.o:
	$(cc) \
	$(cflags) \
	-c $< -o $@ \
	-DVERSION=\"${VERSION}\" \
	-DCURRENT_YEAR=\"${CURRENT_YEAR}\"

deps: lib/tinycc/libtcc.a src/embed-libtcc1.c src/embed-headers.c

## Custom deps targets for osx due to different sed

lib/tinycc/libtcc.a:
	cd lib/tinycc && ./configure ${tinycc_config}
	${MAKE} -C lib/tinycc libtcc.a
	${MAKE} -C lib/tinycc libtcc1.a

src/embed-libtcc1.c:
	$(info Embedding libtcc1)
	sh build/embed-libtcc1.sh lib/tinycc/libtcc1.a$
	sed -i'' -e 's/unsigned char lib_tinycc_libtcc1_a/const unsigned char libtcc1/' src/embed-libtcc1.c
	sed -i'' -e 's/unsigned int lib_tinycc_libtcc1_a_len/const unsigned int libtcc1_len/' src/embed-libtcc1.c

src/embed-headers.c:
	$(info Embedding tinycc headers)
	bash build/embed-headers.sh win
	sed -i'' -e 's/unsigned char/const char/' src/embed-headers.c
	sed -i'' -e 's/unsigned int/const unsigned int/' src/embed-headers.c
