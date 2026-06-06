---
layout: home

hero:
  name: "CJIT"
  text: "C, Just in Time!"
  tagline: "A tiny, portable C runner and compiler powered by TinyCC. Write C, run it immediately, or produce objects and executables without setting up a full toolchain."
  image:
    src: /cjit-mascotte.png
    alt: CJIT mascot
  actions:
    - theme: brand
      text: Get started
      link: /tutorial
    - theme: alt
      text: Command reference
      link: /manpage
    - theme: alt
      text: Download CJIT
      link: https://github.com/dyne/cjit/releases

features:
  - title: Run C immediately
    details: Compile C in memory and execute it directly for fast experiments, scripts, and prototypes.
  - title: Build useful artifacts
    details: Compile one source file to an object or link an executable without running the program.
  - title: Bring native libraries
    details: Load shared libraries and make their exported symbols available to your C program.
  - title: Carry one small tool
    details: CJIT bundles TinyCC, its headers, and runtime assets into a portable command-line executable.
---

## Start in seconds

Download the [latest release](https://github.com/dyne/cjit/releases) for your
platform, then run a C source file:

```sh
cjit hello.c
```

Pass arguments to the compiled program after `--`:

```sh
cjit app.c -- --name cjit --verbose
```

Or produce a file without executing it:

```sh
cjit -c hello.c
cjit -o hello hello.c
```

::: tip Next step
Follow the [tutorial](/tutorial) for installation and practical examples, or
open the [command reference](/manpage) for every option.
:::

## What CJIT is

CJIT is a TinyCC-powered C runner and lightweight compiler frontend. It
prioritizes quick iteration and a small deployment footprint across Windows,
macOS, and GNU/Linux.

CJIT is not a tracing or adaptive JIT in the virtual-machine sense. It compiles
C quickly, often in memory, and can execute the resulting native code
immediately.

## Explore

- Build visual programs in [Graphics](/graphics).
- Work with audio in [Sound](/sound).
- Read and write files in [Filesystem](/filesystem).
- Create terminal interfaces in [Terminal UI](/tui).
- Check platform details and common questions in the [FAQ](/faq).

CJIT is free software maintained by
[Jaromil](https://jaromil.dyne.org) and the
[Dyne.org](https://dyne.org) community.
