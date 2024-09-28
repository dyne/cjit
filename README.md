[![CJIT logo](docs/cjit-logo.png)](https://dyne.org/cjit)

# CJIT executes C code just in time

This is a C interpreter (and compiler) based on tinycc and capable of
executing C code with no need to compile it. It also supports a
curated selection of functions from included libraries.

When using CJIT there is no need for any toolchain, library, headers
or other files, only its executable interpreter is needed.

The idea of JIT compilation and execution for a C-like language is
inspired by Terry Davis, the author of TempleOS, and Fabrice Bellard,
the author of FFMpeg and TinyCC, whose in-memory compiler
implementation is used inside CJIT.

[![software by Dyne.org](https://files.dyne.org/software_by_dyne.png)](http://www.dyne.org)

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
