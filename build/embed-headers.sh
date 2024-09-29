#!/bin/sh

includes=lib/tinycc/include
dst=src/embed-headers.c

tmptar=`mktemp`.tar
tar cf $tmptar ${includes}

command -v xxd > /dev/null || {
  >&2 echo "Error not found: xxd binary not installed"
  exit 1
}

echo "// ${includes}" > $dst
xxd -i $tmptar >> $dst
tmpvar=`echo $tmptar | sed 's/\//_/g; s/\./_/g'`
>&2 echo "$tmpvar"
sed -i 's/unsigned char '"$tmpvar"'/const unsigned char tinycc_headers/' $dst
sed -i 's/unsigned int '"$tmpvar"'/const unsigned int tinycc_headers_len/' $dst
