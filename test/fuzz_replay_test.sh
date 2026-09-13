#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -Eeuo pipefail

replay=test/fuzz/replay.bin
rm -f "$replay"

make fuzz-replay
if [[ -e "$replay" ]]; then
    printf 'Fuzz replay left its generated runner in the checkout\n' >&2
    exit 1
fi

if make fuzz-replay FUZZ_CORPUS=/definitely/missing/cjit-fuzz-seed; then
    printf 'Fuzz replay masked a missing-seed failure\n' >&2
    exit 1
fi
if [[ -e "$replay" ]]; then
    printf 'Failed fuzz replay left its generated runner in the checkout\n' >&2
    exit 1
fi
