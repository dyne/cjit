#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -Eeuo pipefail
temporary=$(mktemp -d); trap 'rm -rf "$temporary"' EXIT
for distro in ubuntu-24.04 ubuntu-22.04; do mkdir -p "$temporary/release-linux-$distro"; printf x > "$temporary/release-linux-$distro/cjit-linux-x86_64-$distro"; done
mkdir -p "$temporary/release-windows-x86_64"; printf x > "$temporary/release-windows-x86_64/cjit-windows-x86_64.exe"; printf x > "$temporary/release-windows-x86_64/cjit-installer-windows-x86_64.exe"
mkdir -p "$temporary/release-windows-arm64"; printf x > "$temporary/release-windows-arm64/cjit-windows-arm64.exe"; printf x > "$temporary/release-windows-arm64/cjit-ar-windows-arm64.exe"
for os in macos-14 macos-15 macos-26; do mkdir -p "$temporary/release-osx-$os-arm64"; printf x > "$temporary/release-osx-$os-arm64/cjit-darwin-arm64-$os"; done
./test/release_artifact_set.sh "$temporary"
truncate -s 0 "$temporary/release-linux-ubuntu-22.04/cjit-linux-x86_64-ubuntu-22.04"
if ./test/release_artifact_set.sh "$temporary"; then printf 'Accepted empty required artifact\n' >&2; exit 1; fi
printf x > "$temporary/release-linux-ubuntu-22.04/cjit-linux-x86_64-ubuntu-22.04"
truncate -s 0 "$temporary/release-windows-arm64/cjit-windows-arm64.exe"
if ./test/release_artifact_set.sh "$temporary"; then printf 'Accepted empty Windows ARM64 artifact\n' >&2; exit 1; fi
printf x > "$temporary/release-windows-arm64/cjit-windows-arm64.exe"
truncate -s 0 "$temporary/release-osx-macos-15-arm64/cjit-darwin-arm64-macos-15"
if ./test/release_artifact_set.sh "$temporary"; then printf 'Accepted empty macOS artifact\n' >&2; exit 1; fi
printf x > "$temporary/release-osx-macos-15-arm64/cjit-darwin-arm64-macos-15"
mkdir -p "$temporary/release-osx-macos-15-x86_64"
printf x > "$temporary/release-osx-macos-15-x86_64/cjit-darwin-x86_64-macos-15"
if ./test/release_artifact_set.sh "$temporary"; then printf 'Accepted duplicate macOS artifact directory\n' >&2; exit 1; fi
