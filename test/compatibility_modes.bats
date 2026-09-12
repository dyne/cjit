load bats_setup

@test "conftest.c compatibility route creates a.out without executing it" {
    printf '%s\n' '#include <stdio.h>' 'int main(void) { puts("not executed"); return 0; }' \
        > "${TMP}/conftest.c"
    pushd "${TMP}" >/dev/null
    run "${CJIT}" conftest.c
    popd >/dev/null

    assert_success
    assert_output --partial 'Detected conftest'
    refute_output --partial 'not executed'
    [ -x "${TMP}/a.out" ]
    run "${TMP}/a.out"
    assert_success
    assert_output 'not executed'
}

@test "conftest.c compatibility route propagates compile failure and leaves no executable" {
    printf '%s\n' 'int main(void) { invalid syntax; }' > "${TMP}/conftest.c"
    pushd "${TMP}" >/dev/null
    run "${CJIT}" conftest.c
    popd >/dev/null

    assert_failure
    assert_output --partial 'Error loading source input'
    [ ! -e "${TMP}/a.out" ]
}

@test "self-host source extraction is available only in a SELFHOST build" {
    pushd "${TMP}" >/dev/null
    run "${CJIT}" --src
    popd >/dev/null
    if [[ "${output}" == *"unknown opt:"* ]]; then
        skip "current binary was not built with SELFHOST; make self-host is tested separately"
    fi
    assert_success
    assert_output --partial 'Extracting CJIT'
    [ -r "${TMP}/cjit_source/README.md" ]
    [ -x "${TMP}/cjit_source/test/bats/bin/bats" ]
    [ -x "${TMP}/cjit_source/lib/tinycc/configure" ]
}
