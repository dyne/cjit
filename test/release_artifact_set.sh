#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -Eeuo pipefail
root=${1:?usage: release_artifact_set.sh ARTIFACT_ROOT}
for os in ubuntu-24.04 ubuntu-22.04; do test -s "$root/release-linux-$os/cjit-x86_64-$os"; done
test -s "$root/release-win-mingw-x86_64/cjit.exe"
test -s "$root/release-win-mingw-x86_64/CJIT_install.exe"
for os in macos-14 macos-15 macos-26; do
    directory="$root/release-osx-$os"
    count=$(find "$directory" -maxdepth 1 -type f -name "cjit-Darwin-$os-*" -size +0c | wc -l)
    [[ "$count" -eq 1 ]] || { printf 'Expected one nonempty macOS artifact for %s, found %s\n' "$os" "$count" >&2; exit 1; }
    total=$(find "$directory" -maxdepth 1 -type f | wc -l)
    [[ "$total" -eq 1 ]] || { printf 'Unexpected macOS artifact count for %s: %s\n' "$os" "$total" >&2; exit 1; }
done
printf 'RELEASE_ARTIFACT_SET root=%s macos=3\n' "$root"
