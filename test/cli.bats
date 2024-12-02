load bats_setup

@test "Hello World!" {
    run ${CJIT} -q test/hello.c
    assert_success
    assert_output 'Hello World!'
}

@test "Pass pre-processor defines" {
	run ${CJIT} -q test/cflags.c -DALLOWED
    assert_success
    assert_output 'Success.'
	run ${CJIT} -q test/cflags.c -DALLOWED=1
    assert_success
    assert_output 'Success.'
	run ${CJIT} -q test/cflags.c
    assert_failure
    assert_line --index 0 'test/cflags.c:8: error: #error Running this program is not allowed. Please compile with -DALLOWED=1'
}

@test "Execute multiple files" {
	run ${CJIT} -q test/multifile/*
    assert_success
    assert_line 'hello from myfunc'
    assert_line 'hello from myfunc2'
    assert_line 'hello from myfunc3'

}

@test "Pass arguments to executed source" {
	run ${CJIT} -q test/cargs.c -- a b c
    assert_success
    assert_line '0: test/cargs.c'
    assert_line '1: a'
    assert_line '2: b'
    assert_line '3: c'

}
