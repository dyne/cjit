load bats_setup

@test "linker resolution of simple gnu ld scripts" {
    cat << EOF > ldscript_test.c
#include <linker.h>
int main(int argc, char **argv) {
    LDState s1;
    int res;
    res = cjit_load_ldscript(&s1,"/usr/lib/x86_64-linux-gnu/libm.so");
    return(res);
}
EOF
    run ${CJIT} -DVERSION=debug --verb ldscript_test.c \
        -I ${R}/src ${R}/src/cjit.c ${R}/src/linker.c \
        ${R}/lib/tinycc/libtcc.a
    assert_success
}
