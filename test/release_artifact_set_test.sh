#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -Eeuo pipefail
temporary=$(mktemp -d); trap 'rm -rf "$temporary"' EXIT
for os in ubuntu-24.04 ubuntu-22.04; do mkdir -p "$temporary/release-linux-$os"; printf x > "$temporary/release-linux-$os/cjit-x86_64-$os"; done
mkdir -p "$temporary/release-win-mingw-x86_64"; printf x > "$temporary/release-win-mingw-x86_64/cjit.exe"; printf x > "$temporary/release-win-mingw-x86_64/CJIT_install.exe"
for os in macos-14 macos-15 macos-26; do mkdir -p "$temporary/release-osx-$os"; printf x > "$temporary/release-osx-$os/cjit-Darwin-$os-arm64"; done
./test/release_artifact_set.sh "$temporary"
truncate -s 0 "$temporary/release-linux-ubuntu-22.04/cjit-x86_64-ubuntu-22.04"
if ./test/release_artifact_set.sh "$temporary"; then printf 'Accepted empty required artifact\n' >&2; exit 1; fi
printf x > "$temporary/release-linux-ubuntu-22.04/cjit-x86_64-ubuntu-22.04"
truncate -s 0 "$temporary/release-osx-macos-15/cjit-Darwin-macos-15-arm64"
if ./test/release_artifact_set.sh "$temporary"; then printf 'Accepted empty macOS artifact\n' >&2; exit 1; fi
