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

### Basic usage

Run one source file:

```bash
./cjit hello.c
```

Compile one source file to an object:

```bash
./cjit -c hello.c
```

Build an executable without running it:

```bash
./cjit -o hello hello.c
```

Pass arguments to the compiled program:

```bash
./cjit app.c -- --name cjit --verbose
```

Inspect the runtime configuration:

```bash
./cjit -v
```

Extract embedded runtime assets:

```bash
./cjit --xass /tmp/cjit-assets
```

Extract a `tar.gz` archive to the current directory:

```bash
./cjit --xtgz bundle.tar.gz
```


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

### Development workflow

Recommended Linux maintainer loop:

```bash
make linux CC=clang
make check
```

Smaller iteration loop while refactoring:

```bash
./test/bats/bin/bats test/cli.bats
./test/bats/bin/bats test/linux.bats
./test/bats/bin/bats test/muntar.bats
```

Notes:

- `make linux CC=clang` is the safest baseline build on Linux
- `make check` expects a working local `./cjit`
- embedded asset files under `src/` are generated during the build

## Manpage

When installed on UNIX systems, CJIT(1) has a manpage! try `man cjit` after installing.

It is also visible [online at dyne.org/docs/cjit](https://dyne.org/docs/cjit/manpage).

This manual gives you insights about the CJIT command-line options.

## 🔬 Internals

CJIT is being refactored toward a simpler maintenance shape:

- VSA: one slice per use-case
- REPR: one request/endpoint/response per CLI route
- Hex: ports/adapters for filesystem, compiler, process, and platform IO

Important directories:

- `src/app/`: use-case slices such as execute, compile-object, build-executable
- `src/domain/`: shared request, response, error, and session contracts
- `src/adapters/cli/`: CLI route parsing and response rendering
- `src/adapters/compiler/`: TinyCC integration
- `src/adapters/fs/`: filesystem and archive adapters
- `src/adapters/platform/`: platform-specific library resolution
- `lib/muntarfs/`: bundle library for packing a directory tree at build time and extracting an embedded tar or tar.gz at runtime

Current runtime model:

1. TinyCC is vendored in `lib/tinycc` and remains the compiler backend.
2. `CJITState` still owns most mutable runtime state, but new work should narrow changes through requests, responses, sessions, ports, and adapters.
3. Runtime assets are embedded at build time and extracted on demand into a temporary directory.
4. `lib/muntarfs` is now the public extraction surface used by CJIT for those embedded bundles.
5. Platform-specific library resolution remains split between POSIX and Windows adapters.

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
