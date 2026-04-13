load bats_setup

@test "Hello World!" {
    skip_if_systcc_execute_is_unavailable
    run ${CJIT} -q test/hello.c
    assert_success
    assert_output 'Hello World!'
}

@test "Pass pre-processor defines" {
    skip_if_systcc_execute_is_unavailable
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
    skip_if_systcc_execute_is_unavailable
    run ${CJIT} -q test/multifile/*.c
    assert_success
    assert_line --partial 'hello from myfunc'
    assert_line --partial 'hello from myfunc2'
    assert_line --partial 'hello from myfunc3'
}

@test "Pass arguments to executed source" {
    skip_if_systcc_execute_is_unavailable
    run ${CJIT} -q test/cargs.c -- a b c
    assert_success
    assert_line --partial '0: test/cargs.c'
    assert_line --partial '1: a'
    assert_line --partial '2: b'
    assert_line --partial '3: c'
}

@test "Execute code from explicit stdin" {
    skip_if_systcc_execute_is_unavailable
    skip_if_windows_stdin_is_unsupported
    run bash -lc "printf '%s\n' '#include <stdio.h>' 'int main(void) { puts(\"stdin ok\"); return 0; }' | '${CJIT}' -q -"
    assert_success
    assert_output 'stdin ok'
}

@test "Execute source preserves non-zero exit status" {
    skip_if_systcc_execute_is_unavailable
    skip_if_windows_stdin_is_unsupported
    run bash -lc "printf '%s\n' 'int main(void) { return 7; }' | '${CJIT}' -q -"
    [ "$status" -eq 7 ]
}

@test "Execute source avoids legacy temp-file collision when TMPDIR is set" {
    skip_if_systcc_execute_is_unavailable
    version="$(git -C "${R}" describe --tags 2>/dev/null || git -C "${R}" rev-parse --short HEAD 2>/dev/null || printf dev)"
    version="$(printf '%s' "${version}" | cut -d- -f1)"
    legacy_path="/tmp/cjit-${version}"
    custom_tmp="${TMP}/custom-tmp"
    mkdir -p "${custom_tmp}"
    rm -rf "${legacy_path}"
    printf '%s\n' 'block legacy flat temp path' > "${legacy_path}"
    run env TMPDIR="${custom_tmp}" "${CJIT}" -q test/hello.c
    rm -f "${legacy_path}"
    assert_success
    assert_output 'Hello World!'
    [ -d "${custom_tmp}/cjit/${version}" ]
}

@test "Execute source refreshes incomplete cached runtime assets" {
    skip_if_systcc_execute_is_unavailable
    version="$(git -C "${R}" describe --tags 2>/dev/null || git -C "${R}" rev-parse --short HEAD 2>/dev/null || printf dev)"
    version="$(printf '%s' "${version}" | cut -d- -f1)"
    custom_tmp="${TMP}/repair-cache"
    runtime_dir="${custom_tmp}/cjit/${version}"
    mkdir -p "${custom_tmp}"

    run env TMPDIR="${custom_tmp}" "${CJIT}" -q test/hello.c
    assert_success
    assert_output 'Hello World!'

    : > "${runtime_dir}/include/stdarg.h"
    [ ! -s "${runtime_dir}/include/stdarg.h" ]

    run env TMPDIR="${custom_tmp}" "${CJIT}" -q test/hello.c
    assert_success
    assert_output 'Hello World!'
    [ -s "${runtime_dir}/include/stdarg.h" ]
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

@test "Compile driver ignores make dependency flags" {
    run ${CJIT} -MMD -MP -MF ${TMP}/hello.d -c test/hello.c -o ${TMP}/hello.o
    assert_success
    [ -f "${TMP}/hello.o" ]
}

@test "Argument separator preserves app flags" {
    skip_if_systcc_execute_is_unavailable
    run ${CJIT} -q test/cargs.c -- --verb -q
    assert_success
    assert_line --partial '1: --verb'
    assert_line --partial '2: -q'
}

@test "Extract runtime assets route" {
    if [ -n "${SYSTCC:-}" ]; then
        skip "embedded runtime assets are unavailable with shared libtcc builds"
    fi
    run ${CJIT} --xass ${TMP}/assets
    assert_success
    [ -n "${output}" ]
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
