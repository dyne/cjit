# AGENTS.md

## Purpose

`cjit` is a small C interpreter and live compiler built around vendored TinyCC in `lib/tinycc`.
It can:

- compile and run C in memory
- compile one source file to an object
- build an executable without running it
- load shared libraries from the host system
- extract embedded runtime assets and tar.gz archives

This file is the maintainer guide for both humans and LLMs. It should be sufficient without reading any planning files.

## Current Reality

Today the codebase is still mostly procedural and centered on `CJITState`.

Main files:

- [src/main.c](/home/jrml/devel/cjit/src/main.c): CLI parsing and top-level dispatch
- [src/cjit.c](/home/jrml/devel/cjit/src/cjit.c): runtime lifecycle, TinyCC setup, source ingestion, linking, execution
- [src/cjit.h](/home/jrml/devel/cjit/src/cjit.h): `CJITState` and public runtime functions
- [src/file.c](/home/jrml/devel/cjit/src/file.c): file/stdin/path helpers
- [src/support/cwalk.c](/home/jrml/devel/cjit/src/support/cwalk.c): low-level path manipulation support used by filesystem and platform adapters
- [src/adapters/cli/ketopt.h](/home/jrml/devel/cjit/src/adapters/cli/ketopt.h): local CLI option parsing dependency
- [src/adapters/platform/build_platform.h](/home/jrml/devel/cjit/src/adapters/platform/build_platform.h): compile-time host and target platform definitions used by core and platform adapters
- [src/adapters/platform/library_resolver_posix.c](/home/jrml/devel/cjit/src/adapters/platform/library_resolver_posix.c): POSIX library resolution and GNU ld script handling
- [src/adapters/platform/library_resolver_windows.c](/home/jrml/devel/cjit/src/adapters/platform/library_resolver_windows.c): Windows DLL resolution
- [lib/muntarfs/muntarfs.h](/home/jrml/devel/cjit/lib/muntarfs/muntarfs.h): bundle extraction surface used by CJIT
- [lib/muntarfs/muntar.c](/home/jrml/devel/cjit/lib/muntarfs/muntar.c): tar extraction and archive reader
- [lib/muntarfs/tinflate.c](/home/jrml/devel/cjit/lib/muntarfs/tinflate.c): low-level deflate implementation
- [lib/muntarfs/tinfgzip.c](/home/jrml/devel/cjit/lib/muntarfs/tinfgzip.c): gzip wrapper over the inflater
- [src/win-compat.c](/home/jrml/devel/cjit/src/win-compat.c): Windows compatibility helpers

Important current behavior:

- `main()` still builds `CJITState` and parses argv, but route request construction now lives in `src/adapters/cli/route_parser.c`.
- `cjit_setup()` is lazy and must happen before compile/link/execute flows.
- non-`SHAREDTCC` builds depend on extracted embedded assets.
- the TinyCC adapter now owns the execute, compile-object, and build-executable flows.
- POSIX execution still forks before running the compiled entrypoint.
- Windows execution still runs in-process.
- stdin execution is supported on POSIX and not supported as the no-file fallback on Windows.
- `-c` currently supports only one source file.
- UTF BOM source files are explicitly rejected.
- source files, requested libraries, library search paths, and resolved library paths now use the `StringList` support wrapper instead of raw `XArray` calls in most runtime code.

## Target Architecture

All new work should move the project toward:

- VSA: slice per use-case
- REPR: request/endpoint/response per CLI route
- Hex: ports/adapters for IO and third-party dependencies
- light DDD: explicit ubiquitous language and invariants

This should stay minimal and C-native. Do not over-engineer it.

### Core use-cases

Use these names consistently:

- `execute-source`
- `compile-object`
- `build-executable`
- `print-status`
- `extract-assets`
- `extract-archive`
- `archive-tool`

### Core vocabulary

- `request`: user intent at the application boundary
- `response`: endpoint result rendered by the CLI
- `session`: mutable runtime/compiler state
- `library request`: logical `-l` input
- `resolved library`: concrete path found by platform logic
- `route`: one CLI action mapped to one request and one response

### Target module layout

Use this direction for refactors:

- `src/app/`: use-case orchestration
- `src/domain/`: requests, responses, errors, core invariants
- `src/adapters/cli/`: argv parsing, dispatch, rendering
- `src/adapters/compiler/`: TinyCC integration
- `src/adapters/fs/`: file/path/tempdir/asset/archive IO
- `src/adapters/platform/`: process and platform-specific library resolution
- `src/support/`: small low-level reusable support only
- `lib/muntarfs/`: standalone bundle pack/extract component and archive implementation for embedded tar/tar.gz assets

Current code is not fully there yet. New changes should avoid making `src/main.c` and `src/cjit.c` even broader.

## Change Map

Use the smallest relevant surface first.

### CLI or argument behavior

Current start point:

- [src/main.c](/home/jrml/devel/cjit/src/main.c)

Target destination:

- `src/adapters/cli/`

### Runtime execute behavior

Current start point:

- [src/cjit.c](/home/jrml/devel/cjit/src/cjit.c)

Target destination:

- `src/app/execute_source.*`

### Compile-to-object behavior

Current start point:

- [src/main.c](/home/jrml/devel/cjit/src/main.c)
- [src/cjit.c](/home/jrml/devel/cjit/src/cjit.c)

Target destination:

- `src/app/compile_object.*`

### Build-executable behavior

Current start point:

- [src/main.c](/home/jrml/devel/cjit/src/main.c)
- [src/cjit.c](/home/jrml/devel/cjit/src/cjit.c)

Target destination:

- `src/app/build_executable.*`

### TinyCC integration

Current start point:

- [src/adapters/compiler/tinycc_adapter.c](/home/jrml/devel/cjit/src/adapters/compiler/tinycc_adapter.c)
- [src/cjit.c](/home/jrml/devel/cjit/src/cjit.c) for legacy compatibility wrappers

Target destination:

- `src/adapters/compiler/`

All direct `tcc_*` calls should eventually live there.

### Files, paths, tempdirs, assets

Current start point:

- [src/file.c](/home/jrml/devel/cjit/src/file.c)
- [src/cjit.c](/home/jrml/devel/cjit/src/cjit.c)
- [lib/muntarfs/muntar.c](/home/jrml/devel/cjit/lib/muntarfs/muntar.c)
- [lib/muntarfs/tinflate.c](/home/jrml/devel/cjit/lib/muntarfs/tinflate.c)
- [lib/muntarfs/tinfgzip.c](/home/jrml/devel/cjit/lib/muntarfs/tinfgzip.c)
- [lib/muntarfs/muntarfs_runtime.c](/home/jrml/devel/cjit/lib/muntarfs/muntarfs_runtime.c)

Target destination:

- `src/adapters/fs/`
- `lib/muntarfs/` for reusable tar/tar.gz bundle operations

### Platform library resolution

Current start point:

- [src/adapters/platform/library_resolver_posix.c](/home/jrml/devel/cjit/src/adapters/platform/library_resolver_posix.c)
- [src/adapters/platform/library_resolver_windows.c](/home/jrml/devel/cjit/src/adapters/platform/library_resolver_windows.c)

Target destination:

- `src/adapters/platform/library_resolver_posix.*`
- `src/adapters/platform/library_resolver_windows.*`

## Invariants To Preserve

- `cjit_new()` must create the TinyCC context successfully before most work can proceed.
- `cjit_setup()` must complete before compile/link/execute flows that need includes, assets, and library paths.
- runtime assets are required for bundled TinyCC builds.
- `cjit_exec()` is single-use per runtime session.
- argv handling around `--` must preserve application arguments.
- the current CLI/tested behaviors are the preservation baseline unless intentionally changed.

## Generated Files

Do not hand-edit generated embed outputs unless the task explicitly requires it:

- `src/assets.c`
- `src/assets.h`
- `src/embed_*`

If behavior depends on them, change the generator scripts or the source assets instead.

`lib/muntarfs` is not generated. Its public header and runtime wrapper are ordinary maintained source files.

## Build

Common commands:

```bash
make
make linux
make linux CC=clang
make debug-asan
make debug-gdb
make apple-osx
make win-native
make win-wsl
make meson
```

Notes:

- default Linux maintainer path is `make linux CC=clang`
- `make meson` uses system `tcc/libtcc-dev`
- build steps generate embedded files under `src/`

## Test

Main suite:

```bash
make check
```

CI subset:

```bash
make check-ci
```

Targeted runs:

```bash
./test/bats/bin/bats test/cli.bats
./test/bats/bin/bats test/linux.bats
./test/bats/bin/bats test/windows.bats
./test/bats/bin/bats test/muntar.bats
./test/bats/bin/bats test/dmon.bats
```

Rules:

- build first, then test
- run the smallest relevant Bats file while iterating
- run `make check` before finishing runtime or CLI changes
- Linux-only tests depend on `.build_done_linux`

## Current Regression Surface

The Bats suite currently protects:

- basic CLI execution
- compile-to-object and build-executable flows
- stdin execution
- `--` app-argument handling
- Linux library resolution and linker-script handling
- Windows BOM rejection and compatibility behavior
- archive extraction helpers
- the `dmon` filesystem monitor test helper

If you change any of those surfaces, update or extend the nearest existing Bats file instead of creating a parallel test style.

## Documentation Expectations

The code refactor should improve docs too.

The README should eventually answer:

- what CJIT does
- canonical usage routes
- build/test workflow
- platform caveats
- where maintainers should start for common changes

The usage/reference docs should clearly cover:

- execute source
- execute from stdin
- compile object
- build executable
- link system libraries
- asset/archive extraction
- behavior of `--`
- platform differences and limitations

Keep CLI help, README examples, and manpage semantics aligned.

## LLM Maintenance Rules

- Start with the direct owner of the behavior, not the whole repo.
- Prefer local changes over broad rewrites.
- Do not add dependencies unless explicitly asked.
- Avoid new generic “helper” modules. Name modules by intent.
- Push IO and platform branching toward adapters, not application logic.
- Prefer explicit requests, responses, and result structs over hidden mutation.
- Keep headers small and specific.

## Human Review Heuristics

The refactor is improving the codebase if:

- one behavior is understandable from one slice plus one adapter
- fewer `#ifdef`s appear in application code
- fewer `XArray` details leak across module boundaries
- TinyCC calls become centralized
- docs point directly to the right file family for common changes

## Short Start

If entering the repo cold:

1. Read [README.md](/home/jrml/devel/cjit/README.md).
2. Read [src/main.c](/home/jrml/devel/cjit/src/main.c), [src/cjit.c](/home/jrml/devel/cjit/src/cjit.c), and [test/cli.bats](/home/jrml/devel/cjit/test/cli.bats).
3. Treat current code as procedural, but move new work toward the target VSA/REPR/Hex layout.
4. Validate with the closest Bats file, then `make check`.
