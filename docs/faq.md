# Frequently Asked Questions about CJIT

Here we try to answer the most asked questions.

## Is CJIT an interpreter, compiler, or JIT?

CJIT is best described as a TinyCC-powered C runner and lightweight
compiler frontend.

It can compile C in memory and execute it immediately, or write
objects and executables to disk. It is not a tracing JIT or adaptive
VM-style JIT, and it is not a classic source interpreter either.

## What's different between `tcc -run` and CJIT?

The main difference is in usability.

CJIT improves three main UX aspects for now:

1. It works as a single executable file which embeds the TinyCC
   compiler, all its headers and its standard library. This way there
   is no need to install anything system wide, check paths and setup
   build folders.

2. It supports adding multiple files into one execution: can accept
   wildcards to ingest anything that is a C source, a pre-compiled
   object or a shared library. The symbols exported by each file will
   be visible to all during the same execution.

3. It finds automatically common system libraries for each target
   platform, avoiding the need to repeat these settings and look for
   the right paths.

We are happy to further improve the developer experience with CJIT,
and your advice is welcome: [open an issue](https://github.com/dyne/cjit/issues)!

## Is CJIT a GCC or Clang replacement?

No, not in the full toolchain sense.

CJIT overlaps with some common compile-and-run workflows and can be
useful in places where one would otherwise reach for `gcc` or `clang`,
but it does not aim to match their full feature set, diagnostics,
optimization pipeline, target coverage, or ecosystem integration.

## What's different between `libgccjit` and CJIT?

CJIT is built as a command-line interpreter using the TinyCC backend
for in-memory compilation. In the future it may also offer `libgccjit`
as backend, as long as it will be possible to embed it all inside a
single executable file, which is a core feature of CJIT's vision for
developer experience.

## Is the CJIT binary fully self-contained?

Not exactly.

The build is distributed as a single executable, but at runtime CJIT
extracts bundled assets to a temporary directory and relies on base
system libraries, plus additional libraries required by C programs to
run.

## Is CJIT an LLM-generated product?

No. CJIT is an open source C tool built around TinyCC.

Like many modern projects, contributors may use AI tools in parts of
the workflow, but CJIT itself is not an "LLM runtime" or AI product.

## Which parts of CJIT are licensed, under what?

Detailed licensing information for CJIT is in the [REUSE metadata
file](https://github.com/dyne/cjit/blob/main/REUSE.toml). We check
correctness of these attributions at every single commit.

All CJIT's original code is licensed using GNU GPL v3 and will be
updated to use future versions of this license published by the Free
Software Foundation. All included licenses are compatible with this.

We grant to everyone the freedom to use, study, modify, and
redistribute modifications of CJIT as long as such modifications are
licensed with one of the licenses already present: MIT, GPL or LGPL.

More information about CJIT's licensing is also found in its [manpage section on LICENSING](https://dyne.org/docs/cjit/manpage/#licensing).

## Where do I send my corrections to this documentation?

You are welcome to [open an issue or a PR to the dyne/docs project](https://github.com/dyne/docs).

The source of the CJIT manual is in the Markdown-formatted files in `src/cjit/docs` inside the repository.

## I have a new question, whom can I ask?

You are welcome to interact in public with Dyne.org hackers over any
of [our channels and social network accounts](https://dyne.org/contact).

If you prefer to interact privately, write a mail to
[info@dyne.org](mailto:info@dyne.org).
