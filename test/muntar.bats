setup() {
    bats_require_minimum_version 1.5.0
    T="$BATS_TEST_DIRNAME"
    TMP="$BATS_TEST_TMPDIR"
	R=`pwd`
    load "$T"/test_helper/bats_support/load
    load "$T"/test_helper/bats_assert/load
    tar -Hustar -cf examples.tar examples
    xxd -i examples.tar > examples.c
}


@test "muntar list contents" {
      cat << EOF > muntar_list.c
#include <stdio.h>
#include <stdlib.h>      
#include <muntar.h>
extern unsigned char examples_tar[];
extern unsigned int examples_tar_len;
int main(int argc, char **argv) {
    int res;
    mtar_t tar;
    const mtar_header_t *header = NULL;
    res = mtar_load(&tar, "examples_tar", examples_tar, examples_tar_len);
    if(res != MTAR_ESUCCESS) return(res);
    printf("loaded %s\n",tar.name);
    while(!mtar_eof(&tar)) {
        mtar_header(&tar, &header);
        if (header->path[0] != 0)
            printf("%s/", header->path);
            printf("%s (%d bytes)\n", header->name, (uint32_t) header->size);
            mtar_next(&tar);
    }
    exit(0);
}
EOF
    gcc -o muntar_list -I ${R}/src \
        ${R}/src/muntar.c ${R}/src/io.c examples.c muntar_list.c 
    run ./muntar_list
    assert_success
    assert_line --partial "examples/"
    assert_line --partial "examples/donut.c (743 bytes)"
}


@test "muntar extract contents" {
      cat << EOF > muntar_extract.c
#include <stdio.h>
#include <stdlib.h>
#include <muntar.h>
extern unsigned char examples_tar[];
extern unsigned int examples_tar_len;
int main(int argc, char **argv) {
    int res;
    fprintf(stderr,"extract to %s\n",argv[1]);
    res = untar_to_path(argv[1], examples_tar, examples_tar_len);
    exit(res);
}
EOF
    gcc -o muntar_extract -I ${R}/src \
        ${R}/src/muntar.c ${R}/src/io.c examples.c muntar_extract.c
    run ./muntar_extract extracted
    >&3 cat extracted/examples/donut.c
    assert_success
    assert_line --partial "untar_to_path file  extracted/examples/donut.c"
}
