# `cjit(1)` - Just-In-Time interpreter for C

CJIT, January 2025

## Synopsis

```text
cjit [options] <files> [-- app arguments]
```

- `[options]` are prefixed by single or double dash and may require an argument.
- `<files>` can be one or more paths to any source (`.c`), object (`.o`) or libraries (`.dll`, `.dylib`, `.so`).
- `[-- app arguments]` means all arguments following a double dash are passed as-is to the running application.

## Description

`CJIT` is a lightweight C interpreter that lets you run C code
instantly, without needing to build it first. In addition to
just-in-time execution, it can do everything a C compiler does,
including call functions from any installed library, and generate
executables. It is also designed to be a drop-in replacement for
`gcc(1)` and `clang(1)`, for instance using `CC=cjit` as an
environment setting.

## Options

- `-h`  
  Displays a summary of the command-line options available with `cjit`.
  It is useful for users who need a quick reference on how to use the
  tool. This manual is meant to complete that knowledge with more
  in-depth information.

- `-v`  
  Prints the version number of `cjit`. It is helpful for verifying the
  specific version you are working with, especially when troubleshooting
  or reporting issues: always include the version output. It can also be
  quickly added to any command line combination to show the internal
  state of `cjit`, for instance include and library paths configured,
  `cflags`, and linked libraries.

- `-q`  
  Suppresses all non-essential output, providing a quieter operation.
  Only critical errors are displayed. This option is turned on by
  default when CJIT is launched inside a script (no tty).

- `-C <...flags...>`  
  Use this option to specify custom flags for the interpreter or
  compiler. If not set, `cjit` uses the flags defined in the environment
  variable `CFLAGS`.

- `-c`  
  Only compiles a single provided source file, without executing it, to
  produce an object file. This option will generate a new pre-compiled
  object file named like the given source, changing its extension from
  `.c` to `.o`, unless a new name is specified using `-o`.

- `-o <filename>`  
  Specifies an output filename. When included on the command line it
  switches CJIT to build mode and compiles all the source files into an
  executable but does not run the resulting program. You must provide
  the path for the output executable.

- `-D key[=value]`  
  Defines a macro key, and optionally its value, for the preprocessor,
  just like a `#define` directive included inside the source. You can
  specify a simple symbol or use `key=value` to define a macro with a
  specific value.

- `-I <path>`  
  Adds a directory path to the list of paths searched for header files.
  This is particularly useful if your project includes headers that are
  not in the standard system directories. For more information see the
  paths section.

- `-l <name>`  
  Links a specific shared library. On Windows the name is that of a DLL
  file without its extension. On UNIX systems, including GNU/Linux and
  BSD, provide the library name without the `lib` prefix, for example
  use `-lssl` for `libssl.so`.

- `-L <path>`  
  Adds a directory path to the library search paths. This is helpful
  when your project depends on libraries that are located in
  non-standard directories. For more information see the paths section.

- `-e <function>`  
  Specifies a different entry function than the default `main`
  function. It is useful if your program has multiple potential entry
  points, or if you want to try a different one at your own risk.

- `-p <path>`  
  Writes the process ID of the executing program to the specified file.
  This is useful for managing and monitoring the running process.

- `--verb`  
  Enables verbose logging, which provides more detailed information
  about the actions CJIT is performing. It is useful for debugging and
  understanding the compilation and execution process.

- `--xass [path]`  
  Extracts runtime assets required by CJIT to run your program. If a
  path is specified, the assets are extracted to that location;
  otherwise, they are extracted to the default directory, which is
  located in `AppData\Local\Temp\CJIT-vN.N.N` on Windows and in
  `/tmp/cjit-vN.N.N` on POSIX systems.

- `--xtgz <path>`  
  Extracts all files from a specified `USTAR` format tar.gz archive.
  This is useful for setting up project dependencies or resources
  packaged in an archive. For instance it is used to set up the
  `cjit-demo.tar.gz` tutorial assets by the script found at
  <https://dyne.org/cjit/demo>.

## Author

This manual is Copyright (c) 2025 by the Dyne.org foundation.

Written by Denis Roio <https://jaromil.dyne.org>.

## Licensing

`CJIT` is licensed under the GNU General Public License version 3
(`GPLv3`) or any later version published by the Free Software
Foundation.

The GPLv3 grants you four freedoms:

- Use: you may use this software for any purpose.
- Study and modify: you may study how CJIT works, and modify it to make
  it do what you wish.
- Distribute: you may redistribute copies of CJIT so that others can
  benefit from it.
- Distribute modified versions: you may distribute your modifications if
  you grant others the same freedom.

This is a human-readable summary and not a substitute for the license.
For the full text of the GPLv3 visit
<https://www.gnu.org/licenses/gpl-3.0.html>. Components included in
CJIT are copyrighted and licensed by their respective vendors, and all
are compatible with the GPLv3. A list of component licenses is provided
in CJIT's source code inside the `LICENSES/` folder and detailed by the
`REUSE.toml` file.

This manpage is licensed under the Creative Commons
Attribution-ShareAlike 4.0 International License (`CC BY-SA 4.0`) or
any later version. To view a copy of this license, visit
<http://creativecommons.org/licenses/by-sa/4.0/>.

## Availability

The most recent version of CJIT source code and up-to-date
documentation is made available from its website at
<https://dyne.org/cjit>.

## See Also

`tcc(1)`
