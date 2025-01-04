load bats_setup

@test "Log ldso.conf.d output and libc.so resolution" {
    ${CJIT} -DVERSION=debug -lc -v 2>&3
}

@test "Linker resolution of libm simple gnu ld script" {
    cat << EOF > ldscript_test.c
#include <stdio.h>
#include <math.h>
int solve_quadratic(double a, double b, double c) {
    double discriminant = b * b - 4 * a * c;
    // Check if the discriminant is non-negative
    if (discriminant < 0) {
        printf("No real roots exist.\n");
        return 1;
    }
    double sqrt_discriminant = sqrt(discriminant);
    double root1 = (-b + sqrt_discriminant) / (2 * a);
    double root2 = (-b - sqrt_discriminant) / (2 * a);
    printf("Roots: %.2f and %.2f\n", root1, root2);
    return 0;
}
int main() {
    double a, b, c;
    // x^2 - 3x + 2 = 0
    a = 1.0;
    b = -3.0;
    c = 2.0;
    // Solve the quadratic equation
    return solve_quadratic(a, b, c);
}
EOF
    run ${CJIT} -DVERSION=debug ldscript_test.c -lm
    assert_success
    assert_output 'Roots: 2.00 and 1.00'
}

@test "Linker resolution of openssl" {
    echo "Hello World!" | base64 > ${TMP}/hello.b64
cat << EOF > base64_hello.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
// Function to decode base64 using OpenSSL
unsigned char *base64_decode(const char *input, int *output_len) {
    BIO *bio, *b64;
    int input_len = strlen(input);
    unsigned char *output = (unsigned char *)malloc((input_len * 3) / 4);
    if (!output) {
        perror("malloc");
        return NULL;    }
    bio = BIO_new_mem_buf(input, input_len);
    if (!bio) {
        perror("BIO_new_mem_buf");
        free(output);
        return NULL;    }
    b64 = BIO_new(BIO_f_base64());
    if (!b64) {
        perror("BIO_new");
        BIO_free(bio);
        free(output);
        return NULL;    }
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    // Do not use newlines to flush buffer
    *output_len = BIO_read(bio, output, input_len);
    if (*output_len < 0) {
        perror("BIO_read");
        BIO_free_all(bio);
        free(output);
        return NULL;    }
    BIO_free_all(bio);
    return output;
}
int main(int argc, char *argv[]) {
    if (argc != 2) return 1;
    char *input = argv[1];
    int decoded_len;
    unsigned char *decoded = base64_decode(input, &decoded_len);
    if (!decoded) {
        fprintf(stderr, "Decoding base64 failed.\n");
        return 1;    }
    fwrite(decoded, 1, decoded_len, stdout);
    printf("\n");
    free(decoded);
    return 0;
}
EOF
    run ${CJIT} -DVERSION=debug base64_hello.c -lssl \
                    -- `cat ${TMP}/hello.b64`
    assert_success
    assert_output 'Hello World!'
}

@test "Linker missing library" {
    run ${CJIT} -DVERSION=debug -l hyeronimus_bartolomeus test/hello.c
    assert_success
    assert_line 'Library not found: libhyeronimus_bartolomeus.so'
}
