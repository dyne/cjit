# AGENTS.md

## Purpose

`cjit` is a small C interpreter and compiler built around the vendored TinyCC
tree in `lib/tinycc`. The `cjit` command can compile and execute C in memory,
compile one source to an object, build an executable, resolve host shared
libraries, report its runtime configuration, and extract embedded assets or a
tar.gz archive. A companion `cjit-ar` executable and the `cjit -ar` compatibility
mode provide archive-tool behavior.

This is the repository map and maintenance contract for a new session. Paths in
this file are repository-relative so the guide remains valid in any checkout.

## Read This First

For most changes, read only the direct owner and its nearest test:

1. `README.md` for the user-facing overview.
2. `src/main.c` and `src/adapters/cli/route_parser.c` for CLI behavior.
3. The matching file in `src/app/` for route orchestration.
4. The port in `src/ports/` and concrete adapter in `src/adapters/` for IO,
   compiler, or platform behavior.
5. The closest file in `test/`, then `GNUmakefile` for the exact build/test target.

Do not start by reading `lib/tinycc`; it is a large vendored dependency. Enter it
only when evidence shows the bug is inside TinyCC rather than CJIT's adapter.

## Architecture as It Exists

The repository is midway through a procedural-to-VSA/REPR/hexagonal migration.
The new boundaries are real, but `CJITState` and legacy functions still connect
most of them.

### End-to-end control flow

The normal CLI path is:

```text
argv
  -> src/main.c                         parse/mutate CLI options on CJITState
  -> adapters/cli/route_parser.c        choose route and construct request
  -> app/<route>.c                      orchestrate the use case
  -> ports/*.h                          intended dependency boundary
  -> adapters/{compiler,fs,platform}/   TinyCC, filesystem, process, libraries
  -> domain response + CJITResult
  -> adapters/cli/render_response.c     print route errors/output
  -> process exit status
```

`src/main.c` also has three compatibility paths which bypass normal route
dispatch: `-ar`, the special `conftest.c` build, and self-host-only `--src`.

### Domain and ports

- `src/domain/requests.h`: request structs for the six normal CLI routes.
- `src/domain/responses.h`: route response structs.
- `src/domain/error.h`: `CJITResultCode` and `CJITResult`; route exit status is
  distinct from the result category.
- `src/domain/runtime_session.h`: the small opaque session view used at port
  boundaries.
- `src/ports/compiler_port.h`: compiler/session operations.
- `src/ports/filesystem_port.h`: stdin, files, paths, encoding, and tempdir IO.
- `src/ports/asset_port.h`: embedded asset and tar.gz extraction.
- `src/ports/library_resolver_port.h`: logical-library-to-file resolution.

Important limitation: application slices currently copy global concrete ports
(`tinycc_compiler_port`, `local_filesystem_port`, `local_asset_port`) rather than
receiving ports as dependencies. Several declared port methods are not yet used.
Do not assume the slices can already be isolated with fakes. Process execution
remains a tested runtime-platform adapter because it is inherently host-specific.

### Application slices

- `src/app/execute_source.c`: file/stdin ingestion and in-memory execution.
- `src/app/compile_object.c`: exactly one source to an object file.
- `src/app/build_executable.c`: one or more inputs to an executable.
- `src/app/print_status.c`: compiler/runtime status.
- `src/app/extract_assets.c`: embedded runtime extraction.
- `src/app/extract_archive.c`: tar.gz extraction into the current directory.

Each route follows request -> endpoint -> response, but the endpoints still take
`CJITState *` and therefore are not domain-pure.

### Adapters

- `src/adapters/cli/route_parser.[ch]`: route precedence, source/app argument
  partitioning, and request construction. `ketopt.h` is the local option parser;
  the option loop itself is still in `src/main.c`.
- `src/adapters/cli/render_response.[ch]`: response-to-terminal rendering.
- `src/adapters/compiler/tinycc_adapter.[ch]`: compile-object,
  build-executable, relocation, entrypoint lookup, and execution plumbing.
- `src/adapters/fs/local_filesystem.[ch]`: runtime-cache creation/repair and the
  filesystem port. Low-level file helpers remain in `src/file.c`.
- `src/adapters/fs/local_asset.[ch]`: embedded assets and archive extraction.
- `src/adapters/platform/runtime_platform.[ch]`: host setup, fork/wait versus
  in-process execution, PID-file handling, and target status.
- `src/adapters/platform/library_resolver_posix.[ch]`: `ld.so.conf`, library
  search, symlinks, and GNU ld script parsing.
- `src/adapters/platform/library_resolver_windows.[ch]`: DLL search.
- `src/adapters/platform/build_platform.h`: build/target macros shared across
  core and platform adapters.

### Legacy core and support

- `src/cjit.[ch]`: owns `CJITState`, TinyCC construction/destruction, lazy
  runtime setup, source ingestion/BOM rejection, status, compatibility wrappers,
  and terminal output. This is still the main migration seam.
- `src/file.[ch]`: low-level file, stdin, absolute-path, and write helpers.
- `src/support/string_list.[ch]`: owned-string wrapper over `XArray`.
- `src/support/source_files.[ch]`: extension classification.
- `src/array.[ch]`: generic `XArray` implementation; new runtime code should use
  `StringList` where it is storing strings.
- `src/support/cwalk.[ch]`: vendored-style low-level path manipulation support.
- `src/win-compat.c`: Windows implementations needed by compiled programs and
  Windows SDK discovery.
- `src/cjit-ar.c`: TinyCC-derived archive tool implementation.

### Archive component

`lib/muntarfs/` is maintained code, not generated or part of TinyCC. It contains
the public bundle API (`muntarfs.h`), tar reader/extractor (`muntar.c`), gzip and
deflate implementation (`tinfgzip.c`, `tinflate.c`), runtime wrapper, and pack
script. Keep it reusable and independent of the CJIT application layer.

## Routes and Invariants

Use these names consistently:

- `execute-source`: default; accepts source, object, or shared-library inputs.
- `compile-object`: `-c`; requires exactly one source; `-o` may set its name.
- `build-executable`: `-o` without `-c`; builds and does not execute.
- `print-status`: `-v` with no source input.
- `extract-assets`: `--xass[=path]` in bundled-TinyCC builds.
- `extract-archive`: `--xtgz path`, extracting into the current directory.
- `archive-tool`: `cjit-ar` or the `cjit -ar` compatibility path.

Preserve these behaviors unless a change explicitly targets them:

- `cjit_new()` must successfully create the TinyCC context.
- Setup is lazy and idempotent: `cjit_prepare()` must run before compilation,
  linking, status resolution, or execution that needs runtime paths/assets.
- Bundled builds extract versioned assets under a platform temp root. On POSIX
  the default is `${TMPDIR:-/tmp}/cjit/$VERSION`; an incomplete cache is removed
  and rebuilt.
- System/shared-libtcc (`SHAREDTCC`) builds do not expose embedded assets and may
  not support the in-memory execution cases supported by bundled builds.
- A runtime session executes at most once (`done_exec`).
- POSIX execution forks and returns the child exit/signal status. Windows writes
  the current PID and invokes the entrypoint in-process.
- With no files, POSIX reads stdin; Windows rejects the fallback. Explicit `-`
  has the same platform distinction.
- Arguments after `--` belong to the compiled application and must survive
  unchanged, including strings that look like CJIT flags.
- UTF-8 and UTF-16 BOM source files are deliberately rejected.
- `-c` accepts one source only. No-extension inputs and non-source inputs go
  through `tcc_add_file` in normal execute/build flows.
- Library requests (`-l`) and resolved library paths are different concepts.
  POSIX resolution includes GNU ld scripts; Windows searches DLL paths.

## Direction for New Work

Continue toward a small C-native combination of vertical slices, REPR, and
hexagonal boundaries:

- Put route orchestration in `src/app/<use_case>.*`, not in `main.c` or `cjit.c`.
- Put user intent and outcomes in explicit request/response/result structs.
- Pass ports into application code when making a slice testable; avoid adding a
  new global adapter lookup.
- Move direct `tcc_*` calls toward `src/adapters/compiler/`.
- Move filesystem and tempdir work toward `src/adapters/fs/`.
- Move fork/wait/PID and platform branching behind a process/platform adapter.
- Keep platform `#ifdef`s out of application slices where practical.
- Name modules by use case or adapter intent; do not add generic helper buckets.
- Keep headers narrow and ownership explicit. A caller must be able to tell who
  frees returned strings and who owns referenced request data.
- Prefer `CJITResult` over new boolean/integer error conventions, but preserve
  real program exit statuses at the CLI boundary.

The migration is improving when one behavior can be understood from one slice
and one adapter, `main.c` only handles transport concerns, `CJITState` exposes
less infrastructure state, and tests can substitute ports without TinyCC or host
filesystem setup.

## Change Map

Start at the smallest relevant surface:

| Behavior | Primary owner | Nearest tests |
|---|---|---|
| Option parsing, ignored compiler flags, `--` | `src/main.c`, `src/adapters/cli/route_parser.c` | `test/cli.bats` |
| Execute source/stdin/app args | `src/app/execute_source.c`, compiler and platform adapters | `test/cli.bats`, `test/dmon.bats` |
| Compile one object | `src/app/compile_object.c`, TinyCC adapter | `test/cli.bats` |
| Build executable | `src/app/build_executable.c`, TinyCC adapter | `test/cli.bats` |
| Status | `src/app/print_status.c`, runtime platform | `test/cli.bats`, `test/linux.bats` |
| Tempdir/runtime cache | `src/adapters/fs/local_filesystem.c`, `src/cjit.c` | `test/cli.bats` |
| POSIX libraries/ld scripts | `src/adapters/platform/library_resolver_posix.c` | `test/linux.bats` |
| Windows DLLs/compatibility | Windows resolver, runtime platform, `src/win-compat.c` | `test/windows.bats` |
| Embedded assets/tar.gz | local asset adapter, `lib/muntarfs/` | `test/cli.bats`, `test/muntar.bats` |
| Archive tool | `src/cjit-ar.c` | no dedicated regression test yet |
| Pure source classification | `src/support/source_files.c` | `test/source_files_unit.c` |

## Generated and Vendored Files

Do not hand-edit generated embed output:

- `src/assets.c`, `src/assets.h`
- `src/embed_*`

Change `build/init-assets.sh`, `build/embed-asset-path.sh`,
`build/embed-source.sh`, or source assets instead. Build targets regenerate these
files and also leave `.build_done_linux`, `.build_done_win`, or
`.build_done_osx`, which conditionally enable platform tests.

Treat `lib/tinycc/`, `src/adapters/cli/ketopt.h`, and
`src/support/cwalk.[ch]` as imported code. Keep local changes to them exceptional
and justified. `lib/muntarfs/` is ordinary maintained source.

## Build

The top-level build file is `GNUmakefile` (there is no ordinary `Makefile`). GNU
Make finds it automatically.

```bash
make linux CC=clang       # default Linux maintainer build; vendored/static TCC
make linux                # same path using CC or the environment default
make meson                # system libtcc/libtcc-dev; copies meson/cjit to ./cjit
make apple-osx
make win-mingw
make win-msvc
make win-wsl
make debug-asan
make debug-gdb
make self-host
```

`build/init.mk` owns the common source list, flags, and embedded-asset recipes.
Platform makefiles in `build/` select target-specific sources and dependencies.
`build/meson.build` is the shared-libtcc build and must be kept in sync when
adding maintained source files.

## Test

Build first. `test/bats_setup` locates `./cjit`, `./cjit.exe`, or
`./cjit.command` and detects shared-libtcc limitations.

```bash
make check-unit           # direct C unit binaries discovered by GNU Make
make check-ci             # unit + CLI + platform + muntar; omits dmon
make check                # complete local suite, including dmon on Linux

./test/bats/bin/bats test/cli.bats
./test/bats/bin/bats test/linux.bats
./test/bats/bin/bats test/windows.bats
./test/bats/bin/bats test/muntar.bats
./test/bats/bin/bats test/dmon.bats
```

Run the closest Bats file while iterating, then `make check` before finishing a
runtime or CLI change. `test/windows.bats` is always invoked but gates its
Windows-only case at runtime. Linux library and dmon tests require
`.build_done_linux`. CI builds/tests bundled Linux, system-libtcc Debian,
MinGW, MSVC, and macOS; Linux CI also rebuilds with `CC=cjit` and reruns the CI
suite.

The direct unit convention, test-layer selection, fixture isolation rules, and
instrumented-run commands are documented in `test/README.md`. Most other tests
are black-box CLI tests; `test/muntar.bats` compiles small C harnesses and is
component/integration coverage rather than isolated unit coverage.
That document also defines Linux coverage scope, including `cjit-ar` and its
generated, vendored, component-only, and Windows-only exclusions.

When adding coverage:

- Extend the nearest Bats file for observable CLI behavior.
- Put deterministic, dependency-free C tests behind `check-unit`.
- Use temporary directories from Bats; do not write fixtures into the checkout.
- Assert exit status, output/error text, and filesystem artifact/content when all
  are part of the contract.
- Avoid host-library tests for parser logic that can be exercised against a
  synthetic directory or fixture.
- Exercise both success and failure paths, especially cleanup and partial output.
- Use `debug-asan` plus the relevant suite for ownership/lifetime changes.

## Known Coverage Priorities

Future test work should prioritize:

1. Unit tests for route selection/request construction, `StringList`, file/path
   helpers, and runtime-cache decisions.
2. Injectable fake ports for application-slice success/failure sequencing and
   guaranteed `end_session` cleanup.
3. Fixture-driven POSIX ld-script and Windows DLL resolution tests independent
   of host packages.
4. Malformed/truncated gzip and tar inputs, path traversal, duplicate entries,
   empty archives, and extraction IO failures in `lib/muntarfs`.
5. CLI failure contracts: missing/syntax-error sources, missing entrypoint,
   invalid/missing options, unwritable outputs/PID files, and bad archives.
6. `cjit-ar`/`-ar`, `conftest.c`, self-host `--src`, environment `CFLAGS`, `-I`,
   custom `-e`, and quiet/verbose behavior.
7. Explicit platform assertions for POSIX signal propagation and Windows
   no-file/stdin rejection, plus shared-libtcc route behavior.

## Documentation Contract

Keep `src/main.c` help, `README.md`, `docs/cjit.1`, and the tutorial semantics in
sync. User documentation should distinguish normal routes, special compatibility
modes, bundled versus shared libtcc, stdin behavior, `--`, output naming, and
platform limitations.

## Maintenance Rules

- Preserve unrelated work in a dirty tree.
- Prefer focused changes; do not add dependencies unless explicitly requested.
- Do not hide new behavior in generic utilities or broaden `main.c`/`cjit.c`.
- Update the closest existing test with every observable behavior change.
- Check both GNU Make and Meson source lists when adding or moving a `.c` file.
- Do not claim a full suite passed unless the matching binary was built and the
  command was run in the current checkout.
