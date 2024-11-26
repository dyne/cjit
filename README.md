[![CJIT logo](https://repository-images.githubusercontent.com/864503934/7d36d2ce-bbd6-4adf-863b-3e37b35216e1)](https://dyne.org/cjit)

CJIT is a C interpreter based on tinyCC that compiles C code in-memory and runs it live. It is released as a small, all-in-one executable that can do a lot, including call functions from any installed library on Linux, Windows, and MacOSX.

More info on [Dyne.org/CJIT](https://dyne.org/cjit).

## Downloads

We provide ready to execute binary builds on [github releases](https://github.com/dyne/cjit/releases).

Beware windows defender will warn you that there is a virus in the file.

There isn't, this is the [0.6.2 release analysis on VirusTotal](https://www.virustotal.com/gui/file/77054b14b5960eaa655bb5c3d5f4f1ddd3ddbd9756136f029074bbef83e168fd/).

## Quick start

Paste this into your terminal
```bash
curl -sLo cjit https://github.com/dyne/cjit/releases/latest/download/cjit-$(uname)-$(uname -m)
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
```
You can now play around with `hello.c` and write your own C code, which can be simply executed with this command:
```
./cjit hello.c
```
If you install `cjit` into your path then you can execute scripts directly:
```
#!/usr/bin/env cjit
```
this hash-bang on first line is supported!

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
 win-native       🪟 Build cjit.exe for WIN64 on Windows Server
 apple-osx        🍎 Build cjit.command for Apple/OSX using clang static
 _
 ------           __ Debugging targets
 debug-asan       🔬 Build using the address sanitizer to detect memory leaks
 _
 ------           __ Testing targets
 check            🧪 Run all tests using the currently built binary ./cjit
 _
 clean            🧹 Clean the source from all built objects
```

## License

CJIT is copyright (C) 2024 by the Dyne.org foundation

CJIT is distributed under the GNU General Public License v3

TinyCC is copyright (C) 2001-2004 by Fabrice Bellard

TinyCC is distributed under the GNU Lesser General Public License

For more information on licensing please refer to the Reuse report and
license texts included in LICENSES/.

[![software by Dyne.org](https://files.dyne.org/software_by_dyne.png)](http://www.dyne.org)
