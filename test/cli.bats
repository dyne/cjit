load bats_setup

@test "Hello World!" {
    skip_if_systcc_execute_is_unavailable
    run ${CJIT} -q test/hello.c
    assert_success
    assert_output 'Hello World!'
}

@test "Shared libtcc capability contract is explicit" {
    if [ -z "${SYSTCC:-}" ]; then
        skip "contract applies only to the distribution shared-libtcc build"
    fi
    run ${CJIT} -v
    assert_success
    assert_output --partial 'System libtcc'

    object="${TMP}/shared-contract.o"
    run ${CJIT} -q -c -o "${object}" test/hello.c
    assert_success
    [ -f "${object}" ]

    run ${CJIT} --help
    assert_success
    refute_output --partial '--xass'
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

@test "Reject malformed pre-processor defines" {
    run ${CJIT} -q -DKEY=VALUE=AGAIN test/hello.c
    assert_failure
    assert_output --partial 'Invalid char used in -D define symbol'
}

@test "Reject UTF BOM source files on every platform" {
    for source in test/hello-bom-utf8.c test/hello-bom-utf16-be.c test/hello-bom-utf16-le.c; do
        run ${CJIT} -q "${source}"
        assert_failure
        assert_line --partial "UTF BOM detected in file: ${source}"
        assert_line --partial 'Encoding is not yet supported, execution aborted.'
    done
}

@test "Reject missing and syntactically invalid source inputs" {
    run ${CJIT} -q "${TMP}/does-not-exist.c"
    assert_failure
    assert_line --partial 'Error loading source input'

    printf '%s\n' 'int main(void) { this is not valid C; }' > "${TMP}/invalid.c"
    run ${CJIT} -q "${TMP}/invalid.c"
    assert_failure
    assert_line --partial 'Error loading source input'
}

@test "Reject a source without the requested entry symbol" {
    skip_if_systcc_execute_is_unavailable
    printf '%s\n' 'int not_main(void) { return 0; }' > "${TMP}/no-main.c"
    run ${CJIT} -q "${TMP}/no-main.c"
    assert_failure
    assert_line --partial 'Entrypoint symbol not found'
}

@test "Reject unknown and missing-valued CLI options" {
    run ${CJIT} -q --not-a-real-option
    assert_failure
    assert_line --partial 'unknown opt:'

    run ${CJIT} -q -I
    assert_failure
    assert_line --partial 'missing arg:'
}

## This and the following test fail when using Debian's libtcc1 for
## execution, maybe because object files aren't supported , as it
## fails in tcc_add_file() calls inside cjit_add_file()
@test "Compile to object and execute" {
    set +e
    test -z $SYSTCC && {
        set -e
        cp "${T}/hello.c" "${TMP}/hello.c"
        pushd "${TMP}" >/dev/null
        run "${CJIT}" -c hello.c
        assert_success
        [ -f "${TMP}/hello.o" ]
        run "${CJIT}" hello.o
        popd >/dev/null
        assert_success
        assert_output 'Hello World!'
    }
}
@test "Compile to custom object and execute" {
    set +e
    test -z $SYSTCC && {
        set -e
        object="${TMP}/world object.o"
        run "${CJIT}" -o "${object}" -c test/hello.c
        assert_success
        run "${CJIT}" "${object}"
        assert_success
        assert_output 'Hello World!'
    }
}

@test "Compile and link to executable and run" {
      executable="${TMP}/world with space"
      run "${CJIT}" -o "${executable}" test/hello.c
      assert_success
      chmod +x "${executable}"
      run "${executable}"
      assert_success
      assert_output 'Hello World!'
}

@test "Compile outputs honor nested and missing destination directories" {
    mkdir -p "${TMP}/outputs/with space"
    executable="${TMP}/outputs/with space/program"
    run ${CJIT} -q -o "${executable}" test/hello.c
    assert_success
    [ -f "${executable}" ]

    missing="${TMP}/missing-parent/program"
    run ${CJIT} -q -o "${missing}" test/hello.c
    assert_failure
    assert_line --partial 'Error in linker compiling to file'
    [ ! -e "${missing}" ]
}

@test "Build executable links multiple source inputs" {
    executable="${TMP}/multifile-program"
    run ${CJIT} -q -o "${executable}" test/multifile/*.c
    assert_success
    [ -x "${executable}" ]
    run "${executable}"
    assert_success
    assert_line --partial 'hello from myfunc'
}

@test "Build executable links a prebuilt object" {
    if [ -n "${SYSTCC:-}" ]; then
        skip "prebuilt object linking is unavailable with the Debian shared libtcc build"
    fi
    printf '%s\n' 'int helper(void) { return 3; }' > "${TMP}/helper.c"
    printf '%s\n' 'int helper(void);' \
        'int main(void) { return helper() == 3 ? 0 : 1; }' > "${TMP}/main.c"

    run ${CJIT} -q -c -o "${TMP}/helper.o" "${TMP}/helper.c"
    assert_success
    [ -s "${TMP}/helper.o" ]

    program="${TMP}/object-linked-program"
    run ${CJIT} -q -o "${program}" "${TMP}/main.c" "${TMP}/helper.o"
    assert_success
    [ -x "${program}" ]
    run "${program}"
    assert_success
}

@test "Compile to object rejects headers" {
    printf '%s\n' '#define HEADER_ONLY 1' > "${TMP}/only.h"

    run ${CJIT} -q -c "${TMP}/only.h"
    assert_failure
    assert_line --partial 'Compile to object failed'
    [ ! -e "${TMP}/only.o" ]
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

@test "Help and CFLAGS environment input have observable contracts" {
    run ${CJIT} --help
    assert_success
    assert_line --partial 'Synopsis: cjit'

    skip_if_systcc_execute_is_unavailable
    run env CFLAGS=-DALLOWED "${CJIT}" -q test/cflags.c
    assert_success
    assert_line --partial 'CFLAGS: -DALLOWED'
    assert_line --partial 'Success.'
}

@test "Include paths and PID files affect only the requested execution" {
    skip_if_systcc_execute_is_unavailable
    mkdir -p "${TMP}/include"
    printf '%s\n' '#define MESSAGE "include path works"' > "${TMP}/include/message.h"
    printf '%s\n' '#include <stdio.h>' '#include "message.h"' \
        'int main(void) { puts(MESSAGE); return 0; }' > "${TMP}/include-test.c"

    pid_file="${TMP}/cjit.pid"
    run ${CJIT} -q -I "${TMP}/include" -p "${pid_file}" "${TMP}/include-test.c"
    assert_success
    assert_output 'include path works'
    [ -s "${pid_file}" ]
    [[ "$(cat "${pid_file}")" =~ ^[0-9]+$ ]]
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

@test "Build route wins over status when an output path is requested" {
    executable="${TMP}/status-build-program"
    run ${CJIT} -v -o "${executable}" test/hello.c
    assert_success
    assert_line --partial 'Build system:'
    [ -f "${executable}" ]
}

@test "Compile driver ignores make dependency flags" {
    run ${CJIT} -MMD -MP -MF ${TMP}/hello.d -c test/hello.c -o ${TMP}/hello.o
    assert_success
    [ -f "${TMP}/hello.o" ]
}

@test "Compile driver leaves dependency-looking application arguments after separator" {
    skip_if_systcc_execute_is_unavailable
    run ${CJIT} -q test/cargs.c -- -MMD -MF app.d
    assert_success
    assert_line --partial '1: -MMD'
    assert_line --partial '2: -MF'
    assert_line --partial '3: app.d'
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

@test "Asset extraction takes precedence over trailing source input" {
    if [ -n "${SYSTCC:-}" ]; then
        skip "embedded runtime assets are unavailable with shared libtcc builds"
    fi
    destination="${TMP}/assets-priority"
    destination_argument="${destination}"
    case "$(uname -s)" in
        MINGW*|MSYS*|CYGWIN*) destination_argument="$(cygpath -w "${destination}")" ;;
    esac
    run ${CJIT} --xass="${destination_argument}" test/hello.c
    assert_success
    [ -d "${destination}" ]
    [ -f "${destination}/include/stdarg.h" ]
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

@test "Archive extraction takes precedence over trailing source input" {
    mkdir -p "${TMP}/priority-src"
    printf '%s\n' 'priority ok' > "${TMP}/priority-src/marker.txt"
    run ${R}/lib/muntarfs/muntarfs-pack.sh "${TMP}/priority-src" "${TMP}/priority" priority
    assert_success
    mkdir -p "${TMP}/priority-out"
    pushd "${TMP}/priority-out" >/dev/null
    run ${CJIT} --xtgz "${TMP}/priority.tar.gz" test/hello.c
    popd >/dev/null
    assert_success
    [ -f "${TMP}/priority-out/priority/marker.txt" ]
}
