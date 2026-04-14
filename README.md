[![CJIT logo](https://repository-images.githubusercontent.com/864503934/7d36d2ce-bbd6-4adf-863b-3e37b35216e1)](https://dyne.org/cjit)

CJIT is a TinyCC-powered C runner and lightweight compiler frontend.

It can:

- compile and execute one or more C inputs directly from memory
- compile one source file to an object
- build an executable without running it

CJIT is designed for fast iteration, scripting-style execution, and
small deployment footprints.

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

## What CJIT Is Not

CJIT is not a tracing or adaptive JIT in the VM sense. It does not
interpret first and optimize hot paths later.

CJIT uses TinyCC to compile C quickly, often in memory, and can
execute the resulting code immediately.

### 📦 Download the demo

The CJIT demo package comes with running examples

#### 🪟 On Windows

    iex ((New-Object System.Net.WebClient).DownloadString('https://dyne.org/cjit/demo'))

#### 🍎 / 🐧 On Apple/OSX and GNU/Linux

    curl -sL https://dyne.org/cjit/demo.sh | bash

### 📖 and follow the [The CJIT tutorial](https://dyne.org/docs/cjit)


## Manpage

When installed on UNIX systems, CJIT(1) has a manpage! try `man cjit` after installing.

It is also visible [online at dyne.org/docs/cjit](https://dyne.org/docs/cjit/manpage).

This manual gives you insights about the CJIT command-line options.

The [CJIT's Frequently Asked Questions](https://dyne.org/docs/cjit/faq/) page may provide more information.

## 📑 Acknowledgements

CJIT is copyright (C) 2024-2026 by the Dyne.org foundation. 
Designed, written and maintained by [Jaromil](https://jaromil.dyne.org).
Free and open source (GNU General Public License v3).

The TinyCC core component is copyright (C) 2001-2004 by Fabrice
Bellard. TinyCC is also free and open source (GNU Lesser General
Public License).

The CJIT manual offers [more information on CJIT licensing](https://dyne.org/docs/cjit/manpage/#licensing).

[![software by Dyne.org](https://files.dyne.org/software_by_dyne.png)](http://www.dyne.org)
