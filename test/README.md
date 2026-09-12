# CJIT test layers

Build `cjit` before running the Bats suites.  Use the smallest layer that
exercises a change, then run `make check` before handing off a runtime or CLI
change.

| Layer | Purpose | Command |
| --- | --- | --- |
| Unit | Deterministic C support and parser logic with no compiler runtime or host filesystem dependency. | `make check-unit` |
| Component | Maintained libraries exercised through small temporary harnesses. | `./test/bats/bin/bats test/muntar.bats` |
| E2E | Public CLI routes, arguments, files, and output. | `./test/bats/bin/bats test/cli.bats` |
| Platform | Platform-specific behavior and resolver policy. | `./test/bats/bin/bats test/linux.bats` or `test/windows.bats` |
| Smoke | Host-library integration that cannot be made fixture-driven yet. | The labeled OpenSSL, ncurses, and system resolver cases in `test/linux.bats` |

`make check-ci` runs the unit, CLI, applicable platform, Windows-gated, and
archive/component suites. `make check` additionally runs the Linux dmon suite
when `.build_done_linux` is present. The initial baseline was 34 Bats cases on
Linux with one Windows-only case skipped; case counts are observations, not a
permanent contract.

## Test data and platform rules

- Bats-generated C files, objects, executables, archives, and fixtures belong
  below `BATS_TEST_TMPDIR`; quote every generated path and keep at least one
  whitespace-path case in the suite.
- Checked-in inputs live below `test/` or `examples/`. Do not create test output
  in the checkout root.
- Linux-only and dmon tests run only after the corresponding platform build
  marker exists. Windows tests self-skip on other hosts. Shared-libtcc builds
  skip in-memory execution and embedded-assets cases that they cannot support.
- Prefer synthetic fixtures for deterministic parser/resolver behavior. Keep a
  host-dependent check only as an explicitly labeled smoke test.

## Instrumented runs

`make coverage` builds an instrumented Linux binary, runs `make check-ci`, and
prints a maintained-source summary for every maintained source compiled by the
Linux target, including `src/cjit-ar.c`. It excludes generated assets/embed
sources and the vendored TinyCC tree. `src/io.c` is not linked by the Linux
target (it is currently compiled only by the muntar component harness), and
`src/win-compat.c` is Windows-only; neither can be represented by this Linux
measurement. The command then removes its instrumented objects so a normal
build cannot reuse them. `make coverage-clean` removes profiles on demand;
`make coverage` invokes it after reporting.
Coverage is an initial measurement with no threshold. `make debug-asan` builds
an AddressSanitizer binary; run `make check-unit` and the closest Bats suite
against that binary before wider sanitizer validation.
