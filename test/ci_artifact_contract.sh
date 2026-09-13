#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Required generic artifacts must reject missing paths instead of publishing an
# apparently successful empty archive.
set -Eeuo pipefail

workflow=${1:-.github/workflows/main.yml}
test -r "$workflow" || { printf 'CI workflow is unreadable: %s\n' "$workflow" >&2; exit 1; }

uploads=$(grep -c 'uses: actions/upload-artifact@' "$workflow" || true)
strict_uploads=$(awk '
    /uses: actions\/upload-artifact@/ { upload = 1; next }
    upload && /if-no-files-found: error/ { count++; upload = 0 }
    upload && /^[[:space:]]*- uses:/ { upload = 0 }
    END { print count + 0 }
' "$workflow")
if [[ "$uploads" -eq 0 || "$uploads" -ne "$strict_uploads" ]]; then
    printf 'Every required generic artifact upload must set if-no-files-found: error (uploads=%s strict=%s)\n' "$uploads" "$strict_uploads" >&2
    exit 1
fi

for marker in 'test -s coverage/maintained.txt' 'test -s cjit-x86_64-' 'Test-Path cjit.exe' 'test -s cjit-Darwin-' './test/release_artifact_set.sh cjit-bin'; do
    grep -Fq -- "$marker" "$workflow" || { printf 'Missing artifact validation: %s\n' "$marker" >&2; exit 1; }
done
for consumed in 'cjit-bin/release-linux-ubuntu-24.04/cjit-x86_64-ubuntu-24.04' 'cjit-bin/release-linux-ubuntu-22.04/cjit-x86_64-ubuntu-22.04' 'cjit-bin/release-win-mingw-x86_64/*' 'cjit-bin/release-osx-macos-14/cjit-Darwin-macos-14-*' 'cjit-bin/release-osx-macos-15/cjit-Darwin-macos-15-*' 'cjit-bin/release-osx-macos-26/cjit-Darwin-macos-26-*'; do
    grep -Fq -- "$consumed" "$workflow" || { printf 'VirusTotal does not consume expected artifact: %s\n' "$consumed" >&2; exit 1; }
done
printf 'CI_ARTIFACT_CONTRACT uploads=%s strict=%s\n' "$uploads" "$strict_uploads"
