#!/bin/sh

lib=lib/tinycc/libtcc1.a
dst=src/embed-libtcc1.c

[ -r $lib ] || {
  >&2 echo "Error not found: $lib"
  exit 1
}

command -v xxd > /dev/null || {
  >&2 echo "Error not found: xxd binary not installed"
  exit 1
}

echo "// $lib" > $dst
xxd -i $lib >> $dst
sed -i 's/unsigned char lib_tinycc_libtcc1_a/const unsigned char libtcc1/' $dst
sed -i 's/unsigned int lib_tinycc_libtcc1_a_len/const unsigned int libtcc1_len/' $dst
