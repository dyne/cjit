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

@test "standalone cjit-ar creates, lists, and extracts an object archive" {
    make_object "${TMP}/helper.c" "${TMP}/helper.o"
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
