
.c.o:
	$(cc) \
	$(cflags) \
	-c $< -o $@ \
	-DVERSION=\"${VERSION}\" \
	-DCURRENT_YEAR=\"${CURRENT_YEAR}\" \
	-DCOMMIT=\"${COMMIT}\" \
	-DBRANCH=\"${BRANCH}\" \
	-DCFLAGS="${cflags}"


src/embed-musl-libc.c:
	sh build/embed-musl-libc.sh

src/embed-libtcc1.c:
	sh build/embed-libtcc1.sh

src/embed-headers.c:
	sh build/embed-headers.sh

lib/tinycc/libtcc.a:
	cd lib/tinycc && ./configure ${tinycc_config} \
	&& ${MAKE} libtcc.a libtcc1.a

clean:
	${MAKE} -C lib/tinycc clean distclean
	${MAKE} -C src clean
