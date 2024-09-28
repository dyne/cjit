# CJIT executes C code just in time

This is a C interpreter (and compiler) based on tinycc and capable of
executing C code with no need to compile it. It also supports a
curated selection of functions from included libraries.

When using CJIT there is no need for any toolchain, only its
executable is needed.

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

CJIT is distributed under the GNU General Public License v3 or later

tinycc is copyright (C) 2001-2004 by Fabrice Bellard

tinycc is distributed under the GNU Lesser General Public License
