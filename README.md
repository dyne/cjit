[![CJIT logo](docs/cjit-logo.png)](https://dyne.org/cjit)

# Use C as a scripting language

This is a portable C interpreter (and compiler) based on tinycc and capable of
executing C code as-is at runtime, without a build step. It also supports a
curated selection of functions from included libraries.

When using CJIT there is no need for any toolchain, library, headers
or other files, only its executable interpreter is needed.

The idea of JIT compilation and execution for a C-like language is
inspired by Terry Davis, the author of TempleOS, and Fabrice Bellard,
the author of FFMpeg and TinyCC, whose in-memory compiler
implementation is used inside CJIT.

[![software by Dyne.org](https://files.dyne.org/software_by_dyne.png)](http://www.dyne.org)

## Downloads

We provide ready to execute binary builds as releases on github.

- [cgit - Windows x86 64bit](https://github.com/dyne/cjit/releases/latest/download/cjit.exe) ⚠️ (WIP)
- [cgit - Linux ELF x86 64bit](https://github.com/dyne/cjit/releases/latest/download/cjit)

## Quick start

Paste this into your terminal
```sh
curl -sLo cjit https://github.com/dyne/cjit/releases/latest/download/cjit
chmod +x cjit
cat << EOF > hello.c
#!./cjit
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv) {
  fprintf(stderr,"Hello, World!\n");
  exit(0);
}
EOF
./cjit hello.c
```
You can now play around with `hello.c` and write your own C code, which can be simply executed with this command:
```
./cjit hello.c
```

## Build dependencies

On GNU+Linux systems make sure to have installed:

```
musl musl-dev musl-tools
```

This is the first target chosen for development, others will come soon
with different build-time dependencies: mingw cross, dynamic
executable, etc.

## License

CJIT is copyright (C) 2024 by the Dyne.org foundation

CJIT is distributed under the Affero GNU General Public License v3

TinyCC is copyright (C) 2001-2004 by Fabrice Bellard

TinyCC is distributed under the GNU Lesser General Public License

For more information on licensing please refer to the Reuse report and
license texts included in LICENSES/.
