#!/usr/bin/env bash

set -e

path="$1"
parent=`dirname ${1}`
name=${2:-`basename ${1}`}
pathname=`basename ${1}`
dst=src/embed_${name}.c

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

# >&2 echo "parent: $parent"
# >&2 echo "name: $name"
# >&2 echo "pathname: $pathname"
# >&2 echo "dest: $dst"

rm -f ${name}.tar.gz
prevpwd=`pwd`
cd ${parent}
[ "$pathname" != "$name" ] && cp -ra "$pathname" "$name"
>&2 echo "Embed ${prevpwd}/${name}.tar.gz"
tar --format ustar -czf ${prevpwd}/${name}.tar.gz "$name"
[ "$pathname" != "$name" ] && rm -rf "$name"
cd -

echo "// Embedded: $path" > $dst
echo "// source generated by cjit/build/embed-path.sh" >> $dst
echo "// `date`" >> $dst
echo "// ${name}" >> $dst
mv ${name}.tar.gz ${name}
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

# xxd already converts dots in underscores
varname=`echo $name | sed 's/\./_/g'`

# generate assets in source for extract_assets(char *tmpdir)
echo >> src/assets.h
echo "extern const char *${varname};" >> src/assets.h
echo "extern const unsigned int ${varname}_len;" >> src/assets.h
echo >> src/assets.h

cat <<EOF >> src/assets.c

// vv ${name} vv
snprintf(incpath,511,"%s/%s",CJIT->tmpdir,"${name}");
if(CJIT->fresh) res = muntargz_to_path(CJIT->tmpdir,(const uint8_t*)&${varname},${varname}_len);
if(res!=0) { _err("Error extracting %s",incpath); return(false); }
cjit_add_include_path(CJIT, incpath);
// ^^ ${name} ^^

EOF
exit 0
