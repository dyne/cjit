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

- [cgit - Windows x86 64bit](https://github.com/dyne/cjit/releases/latest/download/cjit.exe)
- [cgit - Linux ELF x86 64bit](https://github.com/dyne/cjit/releases/latest/download/cjit)

Beware windows defender will warn you that there is a virus in the file.

There isn't, this is the [latest release analysis on VirusTotal](https://www.virustotal.com/gui/file/77054b14b5960eaa655bb5c3d5f4f1ddd3ddbd9756136f029074bbef83e168fd/).

## Quick start

Paste this into your terminal
```sh
curl -sLo cjit https://github.com/dyne/cjit/releases/latest/download/cjit
chmod +x cjit
cat << EOF > hello.c
#!/usr/bin/env cjit
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
If you install `cjit` into your path then you can `chmod +x hello.c` and execute it directly: the hash bang on first line is supported.


## Build from source

There are various build targets, just type `make` to have a list:
```
✨ Welcome to the CJIT build system
🛟 Usage: make <target>
👇 List of targets:
 _
 ------           __ Production targets
 musl-linux       🗿 Build a fully static cjit using musl-libc on Linux
 linux-x86        🐧 Build a dynamically linked cjit using libs found on Linux x86
 win-wsl          🪟 Build cjit.exe for WIN64 on an Ubuntu WSL VM using gcc-mingw-w64
 _
 ------           __ Debugging targets
 linux-asan       🔬 Build using the address sanitizer to detect memory leaks
 _
 ------           __ Testing targets
 check            🧪 Run all tests using the currently built binary ./cjit
 _
 clean            🧹 Clean the source from all built objects
```

## License

CJIT is copyright (C) 2024 by the Dyne.org foundation

CJIT is distributed under the Affero GNU General Public License v3

TinyCC is copyright (C) 2001-2004 by Fabrice Bellard

TinyCC is distributed under the GNU Lesser General Public License

For more information on licensing please refer to the Reuse report and
license texts included in LICENSES/.
