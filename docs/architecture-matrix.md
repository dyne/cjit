# Architecture matrix decision

Linux ARM64 is deferred rather than represented by a build-only or unexecuted
emulation lane.  The current maintained CI runner is x86_64 and supplies
neither `qemu-aarch64` nor an `aarch64-linux-gnu-gcc` toolchain, so it cannot
execute the required meaningful contract (compile-object plus execute-source
or build-executable).

Revisit ARM64 when a native ARM64 GitHub runner or a pinned emulator/toolchain
can run `make check-unit`, compile an object, and execute either a source route
or a built executable.  The lane must publish its executed-suite manifest and
remain within the existing native-platform CI budget; cross-compilation alone
is not evidence of TinyCC relocation/runtime correctness.
