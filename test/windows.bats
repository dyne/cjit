load bats_setup

@test "Timeb.h inclusion for clock() in Windows" {
# see https://www.reddit.com/r/C_Programming/comments/1h1g4gc/comment/lzc9fta/
# /sys/timeb.h:132: error: include file 'sec_api/sys/timeb_s.h' not found
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
