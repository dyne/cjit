setup() {
    bats_require_minimum_version 1.5.0
    T="$BATS_TEST_DIRNAME"
    TMP="$BATS_TEST_TMPDIR"
	R=`pwd`
    load "$T"/test_helper/bats_support/load
    load "$T"/test_helper/bats_assert/load
    load "$T"/test_helper/bats_file/load
    tar -Hustar -cf examples.tar examples
    xxd -i examples.tar > examples.c
    gzip -c examples.tar > examples.tar.gz
    xxd -i examples.tar.gz > examples_gzip.c
    examples_len=`awk '/tar_len/{print $5}' examples.c`
    examples_len=`echo $examples_len | cut -d';' -f1`
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

@test "tinf decompress gzip" {
    cat << EOF > tinf_gunzip.c
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <tinf.h>
extern unsigned char examples_tar_gz[];
extern unsigned int examples_tar_len;
extern unsigned int examples_tar_gz_len;
int main(int argc, char **argv) {
    int res;
    unsigned int destlen = examples_tar_len;
    uint8_t *dest = malloc(destlen);
    res = tinf_gzip_uncompress(dest, &destlen,
                               examples_tar_gz, examples_tar_gz_len);
    if(res != TINF_OK || destlen != examples_tar_len) {
        fprintf(stderr,"Destination length doesn't match source\n");
        fprintf(stderr,"Dest: %u Source: %u\n",destlen, examples_tar_len);
        free(dest);
        exit(res);
    }
    FILE *fp = fopen("examples_uncompressed.tar","w");
    if(!fp) {
        fprintf(stderr,
                "Error open file for write: %s\n",
                strerror(errno));
        free(dest);
        exit(1);
    }
    fwrite(dest,1,destlen,fp);
    fclose(fp);
    free(dest);
    fprintf(stderr,"OK\n");
    exit(0);
}
EOF
    gcc -o tinf_gunzip -I ${R}/src \
        ${R}/src/tinflate.c ${R}/src/tinfgzip.c \
        examples.c examples_gzip.c tinf_gunzip.c
    run ./tinf_gunzip
    assert_success
    assert_output 'OK'
    assert_file_size_equals examples_uncompressed.tar $examples_len
    # check contents using hash
    l=`sha256sum examples_uncompressed.tar | cut -d' ' -f1`
    r=`sha256sum examples.tar | cut -d' ' -f1`
    assert_equal $l $r
}
