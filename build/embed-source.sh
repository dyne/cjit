#!/bin/bash

dst=src/embed_source.c

[ -r src/assets.h ] || {
	>&2 echo "Build must generate src/assets.h first"
	exit 1
}

[ -r src/assets.c ] || {
	>&2 echo "Build must generate src/assets.c first"
	exit 1
}

command -v xxd > /dev/null || {
  >&2 echo "Error not found: xxd binary not installed"
  exit 1
}

set -e
#set -x

name=cjit_source

[ -r ${dst} ] || {

	mkdir -p pristine && cd pristine
	rm -rf ${name}*
	if [ -r ../.git/config ]; then
		git clone ../ ${name}
		rm -rf ${name}/.git*
	else
		rsync -rax ../ --exclude pristine ${name}
	fi

	# cleanup to bare minimum
	rm -rf ${name}/lib/tinycc/tests
	rm -rf ${name}/examples
	rm -rf ${name}/docs

	tar --format ustar -cf ../${name}.tar ${name}
	gzip -9 ../${name}.tar
	cd -
	echo "// Embedded: $path" > $dst
	echo "// source generated by cjit/build/embed-source.sh" >> $dst
	echo "// `date`" >> $dst
	echo "// ${name}" >> $dst
	rm -rf ${name}
	mv ${name}.tar.gz ${name}
	file ${name}
	ls -lh ${name}
	xxd -i ${name} >> $dst
	rm -f ${name}

	# must be constant variables in stack
	# sed inplace is not portable
	if [[ "$OSTYPE" == "darwin"* ]]; then
		sed -i'' -e 's/unsigned char/const char/' $dst
		sed -i'' -e 's/unsigned int/const unsigned int/' $dst
	else
		sed -i -e 's/unsigned char/const char/' $dst
		sed -i -e 's/unsigned int/const unsigned int/' $dst
	fi
}
# xxd already converts dots in underscores
varname=`echo $name | sed 's/\./_/g'`

# generate assets in source for extract_assets(char *tmpdir)
echo >> src/assets.h
echo "extern const char *${varname};" >> src/assets.h
echo "extern const unsigned int ${varname}_len;" >> src/assets.h
echo >> src/assets.h

exit 0