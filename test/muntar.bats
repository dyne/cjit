load bats_setup

@test "muntar list contents" {
      tar -Hustar -cvf examples.tar ${R}/examples
      xxd -i examples.tar > examples.c
      >&3 tail examples.c
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
    assert_line --partial "cjit/examples/"
    assert_line --partial "cjit/examples/donut.c"
}
