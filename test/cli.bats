load bats_setup

@test "Hello World!" {
    run ${CJIT} hello.c
}

@test "Pass pre-processor defines" {
	run ${CJIT} cflags.c -DALLOWED
	run ${CJIT} cflags.c -DALLOWED
}

@test "Execute multiple files" {
	run ${CJIT} multifile/*
}

@test "Pass arguments to executed source" {
	run ${CJIT} cargs.c -- a b c
}

@test "Correct order of arguments" {
	run ${CJIT} -q cargs.c -- a b c
#    assert_output ''
}
