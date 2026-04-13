load bats_setup

@test "Timeb.h inclusion for clock() in Windows" {
# see https://www.reddit.com/r/C_Programming/comments/1h1g4gc/comment/lzc9fta/
# /sys/timeb.h:132: error: include file 'sec_api/sys/timeb_s.h' not found
    skip_if_systcc_execute_is_unavailable
    run ${CJIT} test/win_timeb.c
    assert_success
}

@test "BOM source file UTF8" {
    run ${CJIT} -q test/hello-bom-utf8.c
    assert_failure
    assert_line --partial 'UTF BOM detected in file: test/hello-bom-utf8.c'
    assert_line --partial 'Encoding is not yet supported, execution aborted.'
}

@test "BOM source file UTF16 big endian" {
    run ${CJIT} -q test/hello-bom-utf16-be.c
    assert_failure
    assert_line --partial 'UTF BOM detected in file: test/hello-bom-utf16-be.c'
    assert_line --partial 'Encoding is not yet supported, execution aborted.'
}

@test "BOM source file UTF16 little endian" {
    run ${CJIT} -q test/hello-bom-utf16-le.c
    assert_failure
    assert_line --partial 'UTF BOM detected in file: test/hello-bom-utf16-le.c'
    assert_line --partial 'Encoding is not yet supported, execution aborted.'
}

@test "Windows library resolver searches -L paths" {
    case "$(uname -s)" in
        MINGW*|MSYS*|CYGWIN*)
            ;;
        *)
            skip "Windows library resolver test only applies on Windows"
            ;;
    esac
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
