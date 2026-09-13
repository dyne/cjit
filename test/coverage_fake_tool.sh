#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -Eeuo pipefail

source_file=${!#}
if [[ "${COVERAGE_FAKE_EMPTY:-}" == 1 ]]; then
    exit 0
fi
if [[ "${COVERAGE_FAKE_OMIT:-}" == "$source_file" ]]; then
    exit 0
fi
printf "File '%s'\nLines executed:50.00%% of 2\n" "$source_file"
