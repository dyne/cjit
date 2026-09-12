load bats_setup

@test "Windows rejects no-file and explicit stdin execution" {
    skip_if_not_windows

    run bash -lc "printf '%s\\n' 'int main(void) { return 0; }' | '${CJIT}' -q"
    assert_failure
    assert_line --partial 'No files specified on commandline'

    run bash -lc "printf '%s\\n' 'int main(void) { return 0; }' | '${CJIT}' -q -"
    assert_failure
    assert_line --partial 'Code from standard input not supported on Windows'
}

@test "Timeb.h inclusion for clock() in Windows" {
# see https://www.reddit.com/r/C_Programming/comments/1h1g4gc/comment/lzc9fta/
# /sys/timeb.h:132: error: include file 'sec_api/sys/timeb_s.h' not found
    skip_if_not_windows
    skip_if_systcc_execute_is_unavailable
    run ${CJIT} test/win_timeb.c
    assert_success
}

@test "Windows library resolver searches -L paths" {
    skip_if_not_windows
    skip_if_systcc_execute_is_unavailable
    dll_source="${R}/libtcc.dll"
    [ -r "${dll_source}" ] || dll_source="/c/Windows/System32/kernel32.dll"
    [ -r "${dll_source}" ] || dll_source="/c/Windows/SysWOW64/kernel32.dll"
    [ -r "${dll_source}" ]
    mkdir -p "${TMP}/mq path"
    cp "${dll_source}" "${TMP}/mq path/mqm.dll"
    run ${CJIT} --verb -L"${TMP}/mq path" -lmqm test/hello.c
    assert_success
    assert_output --partial 'Hello World!'
    refute_output --partial 'Library not found: mqm.dll'
}
