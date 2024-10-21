#!/bin/sh

lib=/lib/x86_64-linux-musl/libc.so
dst=src/embed-musl-libc.c

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
# sed_inplace() { if [[ "$OSTYPE" == "darwin"* ]]; then sed -i'' "$*"; else sed -i $*; fi }
# sed_inplace -e 's/unsigned char _lib_x86_64_linux_musl_libc_so/const unsigned char musl_libc/' $dst
# sed_inplace -e 's/unsigned int _lib_x86_64_linux_musl_libc_so_len/const unsigned int musl_libc_len/' $dst
