load bats_setup

@test "macOS builds and links a dylib from whitespace paths" {
    case "$(uname -s)" in Darwin) ;; *) skip "macOS native contract only applies on Darwin" ;; esac
    work="${TMP}/native dylib"
    mkdir -p "${work}"
    printf '%s\n' 'int native_answer(void) { return 42; }' > "${work}/answer.c"
    cc -dynamiclib -o "${work}/libanswer.dylib" "${work}/answer.c"
    printf '%s\n' 'int native_answer(void); int main(void) { return native_answer() == 42 ? 0 : 1; }' > "${work}/main.c"
    output="${work}/program"
    run ${CJIT} -q -L"${work}" -lanswer -o "${output}" "${work}/main.c"
    assert_success
    [ -x "${output}" ]
    run file "${output}"
    assert_success
    assert_output --partial 'Mach-O'
}

@test "macOS linker failure leaves no output file" {
    case "$(uname -s)" in Darwin) ;; *) skip "macOS native contract only applies on Darwin" ;; esac
    output="${TMP}/missing native/program"
    run ${CJIT} -q -o "${output}" test/hello.c -lmissing_cjit_native_library
    assert_failure
    [ ! -e "${output}" ]
}
