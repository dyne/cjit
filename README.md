[![CJIT logo](https://repository-images.githubusercontent.com/864503934/7d36d2ce-bbd6-4adf-863b-3e37b35216e1)](https://dyne.org/cjit)

CJIT is a C interpreter based on tinyCC that compiles C code in-memory and runs it live. It is released as a small, all-in-one executable that can do a lot, including call functions from any installed library on Linux, Windows, and MacOSX.

Homepage at [Dyne.org/CJIT](https://dyne.org/cjit).

## 🚀 Quick start

Download [the latest CJIT release](https://github.com/dyne/cjit/releases/) for your system.

Launch CJIT from a terminal console command prompt: one can mix c source files and dynamic libraries as arguments:

```
./cjit.exe mysource.c mylib.dll
```

Take a tour with the [CJIT tutorial](https://dyne.org/docs/cjit).


### 📦 Download the demo

The CJIT demo package comes with running examples

#### 🪟 On Windows

    iex ((New-Object System.Net.WebClient).DownloadString('https://dyne.org/cjit/demo'))

#### 🍎 / 🐧 On Apple/OSX and GNU/Linux

    curl -sL https://dyne.org/cjit/demo.sh | bash

### 📖 and follow the [The CJIT tutorial](https://dyne.org/docs/cjit)


## ⚙️ Build from source

There are various build targets, just type `make` to have a list:
```
✨ Welcome to the CJIT build system
🛟 Usage: make <target>
👇 List of targets:
 _
 ------           __ Production targets
 linux-x86        🐧 Build a dynamically linked cjit using libs found on Linux x86
 win-wsl          🪟 Build cjit.exe for WIN64 on an Ubuntu WSL VM using gcc-mingw-w64
 win-native       🪟 Build cjit.exe for WIN64 on Windows Server
 apple-osx        🍎 Build cjit.command for Apple/OSX using clang static
 _
 ------           __ Debugging targets
 debug-gdb        🔬 Build using the address sanitizer to detect memory leaks
 debug-asan       🔬 Build using the address sanitizer to detect memory leaks
 self-host        💎 Build a CJIT that builts itself (embed its source)
 _
 ------           __ Testing targets
 check            🧪 Run all tests using the currently built binary ./cjit
 check-ci         🧪 Run all tests using the currently built binary ./cjit
 _
 ------           __ Installation targets
 install          🔌 Install the built binaries in PREFIX
 clean            🧹 Clean the source from all built objects
```

## Manpage

When installed on UNIX systems, CJIT(1) has a manpage! try `man cjit` after installing.

It is also visible [online at dyne.org/docs/cjit](https://dyne.org/docs/cjit/manpage).

This manual gives you insights about the CJIT command-line options.

## 🔬 Internals

CJIT is a bit complex inside.

1. It relies on [tinycc](https://bellard.org/tcc/) to compile C code in-memory and run it immediately.
2. It detects automatically the system on which its running and auto-configures to support most features.
3. It embeds all C code and headers in [cjit/assets](https://github.com/dyne/cjit/tree/main/assets) making them available to all running code.
4. To embed them creates a `tar.gz` of assets at build-time and decompresses them at run-time in a temporary dir.
5. It ships a non-exclusive, opinionated selection of libraries useful to quickly script advanced applications in C.

The [CJIT's Frequently Asked Questions](https://dyne.org/docs/cjit/faq/) page may provide more information.

## 📑 Acknowledgements

CJIT is copyright (C) 2024-2025 by the Dyne.org foundation. 
Designed, written and maintained by [Jaromil](https://jaromil.dyne.org).
Free and open source (GNU General Public License v3).

The TinyCC core component is copyright (C) 2001-2004 by Fabrice
Bellard. TinyCC is also free and open source (GNU Lesser General
Public License).

The CJIT manual offers [more information on CJIT licensing](https://dyne.org/docs/cjit/manpage/#licensing).

[![software by Dyne.org](https://files.dyne.org/software_by_dyne.png)](http://www.dyne.org)
