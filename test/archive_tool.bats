load bats_setup

setup() {
    bats_require_minimum_version 1.5.0
    T="$BATS_TEST_DIRNAME"
    TMP="$BATS_TEST_TMPDIR"
    R="$(pwd)"
    load "$T"/test_helper/bats_support/load
    load "$T"/test_helper/bats_assert/load
    CJIT="${R}/cjit"
    [ -x "${CJIT}" ] || CJIT="${R}/cjit.exe"
    [ -x "${CJIT}" ] || CJIT="${R}/cjit.command"
    ARCHIVE_TOOL="${R}/cjit-ar"
    [ -x "${ARCHIVE_TOOL}" ] || ARCHIVE_TOOL="${R}/cjit-ar.exe"
    [ -x "${ARCHIVE_TOOL}" ] || ARCHIVE_TOOL="${R}/cjit-ar.command"
    [ -x "${CJIT}" ] || { >&2 echo "CJIT is not built"; exit 1; }
    [ -x "${ARCHIVE_TOOL}" ] || skip "cjit-ar is not built for this target"
}

make_object() {
    local source="$1"
    local object="$2"
    printf '%s\n' 'int archive_helper(void) { return 42; }' > "${source}"
    run "${CJIT}" -q -c -o "${object}" "${source}"
    assert_success
    [ -s "${object}" ]
}

make_odd_sized() {
    local object="$1"
    if [ "$(( $(wc -c < "${object}") % 2 ))" -eq 0 ]; then
        printf '\0' >> "${object}"
    fi
}

@test "standalone cjit-ar creates, lists, and extracts an object archive" {
    make_object "${TMP}/helper.c" "${TMP}/helper.o"
    make_odd_sized "${TMP}/helper.o"
    archive="${TMP}/libhelper.a"

    run "${ARCHIVE_TOOL}" rcs "${archive}" "${TMP}/helper.o"
    assert_success
    [ -s "${archive}" ]

    run "${ARCHIVE_TOOL}" t "${archive}"
    assert_success
    assert_output 'helper.o'

    mkdir "${TMP}/extract"
    pushd "${TMP}/extract" >/dev/null
    run "${ARCHIVE_TOOL}" x "${archive}"
    popd >/dev/null
    assert_success
    assert_output ''
    cmp "${TMP}/helper.o" "${TMP}/extract/helper.o"
}

@test "cjit -ar compatibility mode matches standalone archive listing" {
    make_object "${TMP}/compat.c" "${TMP}/compat.o"
    archive="${TMP}/libcompat.a"
    run "${ARCHIVE_TOOL}" rcs "${archive}" "${TMP}/compat.o"
    assert_success

    run "${ARCHIVE_TOOL}" t "${archive}"
    assert_success
    standalone_output="${output}"

    run "${CJIT}" -ar t "${archive}"
    assert_success
    assert_output "${standalone_output}"
}

@test "archive tools reject unsupported operations and missing archives" {
    run "${ARCHIVE_TOOL}" -b "${TMP}/missing.a"
    assert_failure
    assert_output --partial 'usage: cjit-ar'

    run "${CJIT}" -ar t "${TMP}/missing.a"
    assert_failure
    assert_output --partial "can't open file"
}

write_ar_member() {
    local archive="$1"
    local member="$2"
    local size="$3"
    local contents="$4"
    printf '%-16s%-12s%-6s%-6s%-8s%-10s`\n' "${member}" 0 0 0 100644 "${size}" >> "${archive}"
    printf '%s' "${contents}" >> "${archive}"
    if [[ "${size}" =~ ^[0-9]+$ ]] && [ $((size % 2)) -ne 0 ]; then
        printf '\n' >> "${archive}"
    fi
}

@test "cjit-ar rejects truncated and invalid archive members without extraction" {
    truncated="${TMP}/truncated.a"
    printf '!<arch>\n' > "${truncated}"
    write_ar_member "${truncated}" "short.o/" 4 x
    mkdir "${TMP}/truncated-out"
    pushd "${TMP}/truncated-out" >/dev/null
    run "${ARCHIVE_TOOL}" x "${truncated}"
    popd >/dev/null
    assert_failure
    assert_output --partial 'truncated member data'
    ! [ -e "${TMP}/truncated-out/short.o" ]

    invalid="${TMP}/invalid-size.a"
    printf '!<arch>\n' > "${invalid}"
    write_ar_member "${invalid}" "bad.o/" nope x
    run "${ARCHIVE_TOOL}" t "${invalid}"
    assert_failure
    assert_output --partial 'invalid member size'
}

@test "cjit-ar rejects unsafe member names and missing member padding" {
    unsafe="${TMP}/unsafe.a"
    printf '!<arch>\n' > "${unsafe}"
    write_ar_member "${unsafe}" "../escape" 1 x
    mkdir "${TMP}/unsafe-out"
    pushd "${TMP}/unsafe-out" >/dev/null
    run "${ARCHIVE_TOOL}" x "${unsafe}"
    popd >/dev/null
    assert_failure
    assert_output --partial 'unsupported member name'
    ! [ -e "${TMP}/escape" ]

    no_padding="${TMP}/no-padding.a"
    printf '!<arch>\n' > "${no_padding}"
    printf '%-16s%-12s%-6s%-6s%-8s%-10s`\n' 'odd.o/' 0 0 0 100644 1 >> "${no_padding}"
    printf x >> "${no_padding}"
    run "${ARCHIVE_TOOL}" t "${no_padding}"
    assert_failure
    assert_output --partial 'truncated member padding'
}
