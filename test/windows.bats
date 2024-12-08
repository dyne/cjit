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
      assert_line 'UTF BOM detected in file: test/hello-bom-utf8.c'
      assert_line 'Encoding is not yet supported, execution aborted.'
}
