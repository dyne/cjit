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
    run ${CJIT} -q -C -DALLOWED=1 test/cflags.c
    assert_success
    assert_output 'Success.'
    run ${CJIT} -q test/cflags.c
    assert_failure
    assert_output --partial 'Please compile with -DALLOWED=1'
}

## This and the following test fail when using Debian's libtcc1 for
## execution, maybe because object files aren't supported , as it
## fails in tcc_add_file() calls inside cjit_add_file()
@test "Compile to object and execute" {
    set +e
    test -z $SYSTCC && {
        set -e
        run ${CJIT} -c test/hello.c
        assert_success
        run ${CJIT} hello.o
        assert_success
        assert_output 'Hello World!'
    }
}
@test "Compile to custom object and execute" {
    set +e
    test -z $SYSTCC && {
        set -e
        run ${CJIT} -o world.o -c test/hello.c
        assert_success
        run ${CJIT} world.o
        assert_success
        assert_output 'Hello World!'
    }
}

@test "Compile and link to executable and run" {
      run ${CJIT} -o world test/hello.c
      assert_success
      chmod +x ./world
      run ./world
      assert_success
      assert_output 'Hello World!'
}

@test "Execute multiple files" {
    run ${CJIT} -q test/multifile/*
    assert_success
    assert_line --partial 'hello from myfunc'
    assert_line --partial 'hello from myfunc2'
    assert_line --partial 'hello from myfunc3'
}

@test "Pass arguments to executed source" {
    run ${CJIT} -q test/cargs.c -- a b c
    assert_success
    assert_line --partial '0: test/cargs.c'
    assert_line --partial '1: a'
    assert_line --partial '2: b'
    assert_line --partial '3: c'
}

@test "Execute code from explicit stdin" {
    run bash -lc "printf '%s\n' '#include <stdio.h>' 'int main(void) { puts(\"stdin ok\"); return 0; }' | '${CJIT}' -q -"
    assert_success
    assert_output 'stdin ok'
}

@test "Execute source preserves non-zero exit status" {
    run bash -lc "printf '%s\n' 'int main(void) { return 7; }' | '${CJIT}' -q -"
    [ "$status" -eq 7 ]
}

@test "Status mode works without source input" {
    run ${CJIT} -v
    assert_success
    assert_line --partial 'Build system:'
    assert_line --partial 'Target platform:'
}

@test "Compile to object rejects multiple source files" {
    run ${CJIT} -c test/hello.c test/cflags.c
    assert_failure
    assert_line --partial 'Compiling to object files supports only one file argument'
}

@test "Compile to object prints status from its route" {
    run ${CJIT} -v -c test/hello.c
    assert_success
    assert_line --partial 'Build system:'
}

@test "Argument separator preserves app flags" {
    run ${CJIT} -q test/cargs.c -- --verb -q
    assert_success
    assert_line --partial '1: --verb'
    assert_line --partial '2: -q'
}

@test "Extract runtime assets route" {
    run ${CJIT} --xass ${TMP}/assets
    assert_success
    assert_output --regexp '^/tmp/cjit-'
    [ -d "${output}" ]
}

@test "Extract archive route" {
    mkdir -p "${TMP}/bundle-src"
    printf '%s\n' 'bundle ok' > "${TMP}/bundle-src/hello.txt"
    run ${R}/lib/muntarfs/muntarfs-pack.sh "${TMP}/bundle-src" "${TMP}/bundle" bundle
    assert_success
    mkdir -p "${TMP}/bundle-out"
    pushd "${TMP}/bundle-out" >/dev/null
    run ${CJIT} --xtgz "${TMP}/bundle.tar.gz"
    popd >/dev/null
    assert_success
    [ -f "${TMP}/bundle-out/bundle/hello.txt" ]
}
