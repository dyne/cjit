# Shared libtcc capability contract

`make meson` builds CJIT against the distribution `libtcc`.  This is not the
same runtime as the bundled build: it has no embedded assets and distro builds
can omit relocation support needed for in-memory execution or adding a compiled
object back to an execution session.

| Route | Bundled TinyCC | Distribution shared libtcc |
| --- | --- | --- |
| `-v` status | supported | supported; identifies System libtcc |
| `-c source.c` | supported | supported |
| `-o program source.c` | supported | supported |
| `--xass` | supported | explicitly unavailable |
| in-memory source/stdin execution | supported | explicitly skipped by tests; package capability dependent |
| execute a prebuilt object | supported | explicitly skipped by tests; Debian libtcc does not support it |

The Debian CI job must run the shared capability Bats contract.  A future
distribution may promote an unsupported route only after adding a positive,
native test for that package/runtime combination.
