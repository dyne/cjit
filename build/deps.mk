
.c.o:
	$(cc) \
	$(cflags) \
	-c $< -o $@ \
	-DVERSION=\"${VERSION}\" \
	-DCURRENT_YEAR=\"${CURRENT_YEAR}\"

src/embed-musl-libc.c:
	bash build/embed-musl-libc.sh

src/embed-libtcc1.c:
	$(info Embedding libtcc1: ${embed_libtcc1})
	bash build/embed-libtcc1.sh ${embed_libtcc1}

src/embed-headers.c:
	$(info Embedding tinycc headers)
	bash build/embed-headers.sh

lib/tinycc/libtcc.a:
	cd lib/tinycc && ./configure ${tinycc_config}
	${MAKE} -C lib/tinycc libtcc.a
	${MAKE} -C lib/tinycc libtcc1.a

clean:
	${MAKE} -C lib/tinycc clean distclean
	${MAKE} -C src clean
