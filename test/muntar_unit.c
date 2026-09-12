#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#if defined(_WIN32) || defined(__MINGW32__)
#include <direct.h>
#include <process.h>
#endif

#include "muntar.h"
#include "muntarfs.h"
#include "tinf.h"

static int failures;

#if defined(_WIN32) || defined(__MINGW32__)
#define CJIT_TEST_WINDOWS
#endif

#define CHECK(expression) do { \
    if (!(expression)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expression); failures++; } \
} while (0)

static void octal(char *field, size_t length, uint64_t value)
{
    snprintf(field, length, "%0*llo", (int)length - 1, (unsigned long long)value);
}

static void header(uint8_t *block, const char *name, char type, const void *data, size_t size)
{
    unsigned sum = 0;
    size_t i;
    memset(block, 0, 512);
    strncpy((char *)block, name, 99);
    octal((char *)block + 100, 8, 0644);
    octal((char *)block + 124, 12, size);
    memset(block + 148, ' ', 8);
    block[156] = (uint8_t)type;
    memcpy(block + 257, "ustar", 5);
    memcpy(block + 263, "00", 2);
    for (i = 0; i < 512; i++) sum += block[i];
    snprintf((char *)block + 148, 8, "%06o", sum);
    block[154] = '\0'; block[155] = ' ';
    if (data && size) memcpy(block + 512, data, size);
}

static size_t tar_one(uint8_t *tar, const char *name, char type, const void *data, size_t size)
{
    size_t used = 512 + ((size + 511) & ~(size_t)511);
    header(tar, name, type, data, size);
    memset(tar + used, 0, 1024);
    return used + 1024;
}

static void fix_checksum(uint8_t *block)
{
    unsigned sum = 0;
    size_t i;
    memset(block + 148, ' ', 8);
    for (i = 0; i < 512; i++) sum += block[i];
    snprintf((char *)block + 148, 8, "%06o", sum);
    block[154] = '\0'; block[155] = ' ';
}

static int make_fixture_root(char *root, size_t size, const char *name)
{
#if defined(CJIT_TEST_WINDOWS)
    const char *temporary_directory = getenv("TEMP");

    if (!temporary_directory || !temporary_directory[0]) {
        temporary_directory = ".";
    }
    snprintf(root, size, "%s/%s-%ld", temporary_directory, name,
             (long)_getpid());
    return _mkdir(root) == 0;
#else
    snprintf(root, size, "/tmp/%s-XXXXXX", name);
    return mkdtemp(root) != NULL;
#endif
}

static int remove_fixture_directory(const char *path)
{
#if defined(CJIT_TEST_WINDOWS)
    return _rmdir(path);
#else
    return rmdir(path);
#endif
}

static void test_gzip(void)
{
    static const uint8_t good[] = {0x1f,0x8b,8,0,0,0,0,0,0,3,0xcb,0x48,0xcd,0xc9,0xc9,7,0,0x86,0xa6,0x10,0x36,5,0,0,0};
    uint8_t input[sizeof(good)], output[16];
    unsigned int length;
    size_t i;
    for (i = 0; i < sizeof(good); i++) {
        length = sizeof(output); memcpy(input, good, sizeof(good));
        CHECK(tinf_gzip_uncompress(output, &length, input, (unsigned int)i) != TINF_OK);
    }
    length = 4; CHECK(tinf_gzip_uncompress(output, &length, good, sizeof(good)) == TINF_BUF_ERROR);
    length = sizeof(output); CHECK(tinf_gzip_uncompress(output, &length, good, sizeof(good)) == TINF_OK);
    CHECK(length == 5 && memcmp(output, "hello", 5) == 0);
    memcpy(input, good, sizeof(good)); input[0] = 0; length = sizeof(output);
    CHECK(tinf_gzip_uncompress(output, &length, input, sizeof(input)) != TINF_OK);
    memcpy(input, good, sizeof(good)); input[3] = 0xe0; length = sizeof(output);
    CHECK(tinf_gzip_uncompress(output, &length, input, sizeof(input)) != TINF_OK);
    memcpy(input, good, sizeof(good)); input[sizeof(input) - 8] ^= 1; length = sizeof(output);
    CHECK(tinf_gzip_uncompress(output, &length, input, sizeof(input)) != TINF_OK);
    CHECK(muntarfs_extract_targz_to_path("/tmp", input, sizeof(input)) != MTAR_ESUCCESS);
    memcpy(input, good, sizeof(good));
    input[sizeof(input) - 4] = 0xff; input[sizeof(input) - 3] = 0xff;
    input[sizeof(input) - 2] = 0xff; input[sizeof(input) - 1] = 0x7f;
    CHECK(muntargz_to_path("/tmp", input, sizeof(input)) == TINF_BUF_ERROR);
}

static void test_tar_and_paths(void)
{
    uint8_t tar[2048], broken[2048];
    const char data[] = "ok";
    mtar_t parsed;
    const mtar_header_t *entry;
    char root[512];
    char canary[512];
#if !defined(CJIT_TEST_WINDOWS)
    char outside[512], link[512];
#endif
    size_t length = tar_one(tar, "nested/file.txt", MTAR_TREG, data, sizeof(data) - 1);
    CHECK(mtar_load(&parsed, "valid", tar, length) == MTAR_ESUCCESS);
    CHECK(mtar_header(&parsed, &entry) == MTAR_ESUCCESS && entry->size == 2);
    CHECK(mtar_next(&parsed) == MTAR_ENULLRECORD || mtar_eof(&parsed));
    memcpy(broken, tar, length); broken[148] = 'x';
    CHECK(mtar_load(&parsed, "bad-checksum", broken, length) == MTAR_EINVALIDMODE || mtar_load(&parsed, "bad-checksum", broken, length) == MTAR_EBADCHKSUM);
    CHECK(mtar_load(&parsed, "short", tar, 511) != MTAR_ESUCCESS);
    header(tar, "directory", MTAR_TDIR, NULL, 0);
    header(tar + 512, "empty", MTAR_TREG, NULL, 0);
    memset(tar + 1024, 0, 1024);
    CHECK(mtar_load(&parsed, "metadata", tar, sizeof(tar)) == MTAR_ESUCCESS);
    CHECK(mtar_header(&parsed, &entry) == MTAR_ESUCCESS && entry->type == MTAR_TDIR);
    CHECK(mtar_next(&parsed) == MTAR_ESUCCESS);
    CHECK(mtar_header(&parsed, &entry) == MTAR_ESUCCESS && entry->size == 0 && strcmp(entry->name, "empty") == 0);
    header(tar, "bad-size", MTAR_TREG, NULL, 0);
    memset(tar + 124, '9', 11); tar[135] = '\0'; fix_checksum(tar);
    CHECK(mtar_load(&parsed, "bad-size", tar, 1536) == MTAR_EINVALIDMODE);
    CHECK(make_fixture_root(root, sizeof(root), "cjit-muntar-unit"));
    snprintf(canary, sizeof(canary), "%s/canary", root);
    {
        int result = muntar_to_path(root, tar, length);
        CHECK(result == MTAR_EWRITEFAIL || result == MTAR_EOPENFAIL);
    }
    length = tar_one(tar, "../canary", MTAR_TREG, data, sizeof(data) - 1);
    CHECK(muntar_to_path(root, tar, length) == MTAR_EINVALIDMODE);
    CHECK(access(canary, F_OK) != 0);
    length = tar_one(tar, "./canary", MTAR_TREG, data, sizeof(data) - 1);
    CHECK(muntar_to_path(root, tar, length) == MTAR_EINVALIDMODE);
    length = tar_one(tar, "nested//canary", MTAR_TREG, data, sizeof(data) - 1);
    CHECK(muntar_to_path(root, tar, length) == MTAR_EINVALIDMODE);
    length = tar_one(tar, "C:/canary", MTAR_TREG, data, sizeof(data) - 1);
    CHECK(muntar_to_path(root, tar, length) == MTAR_EINVALIDMODE);
    length = tar_one(tar, "nested\\canary", MTAR_TREG, data, sizeof(data) - 1);
    CHECK(muntar_to_path(root, tar, length) == MTAR_EINVALIDMODE);
#if !defined(CJIT_TEST_WINDOWS)
    CHECK(make_fixture_root(outside, sizeof(outside), "cjit-muntar-outside"));
    snprintf(link, sizeof(link), "%s/linked", root);
    CHECK(symlink(outside, link) == 0);
    length = tar_one(tar, "linked/escaped.txt", MTAR_TREG, data, sizeof(data) - 1);
    CHECK(muntar_to_path(root, tar, length) == MTAR_EWRITEFAIL);
    snprintf(canary, sizeof(canary), "%s/escaped.txt", outside);
    CHECK(access(canary, F_OK) != 0);
    unlink(link);
    remove_fixture_directory(outside);
#endif
    length = tar_one(tar, "/absolute", MTAR_TREG, data, sizeof(data) - 1);
    CHECK(muntar_to_path(root, tar, length) == MTAR_EINVALIDMODE);
    length = tar_one(tar, "safe.txt", MTAR_TREG, data, sizeof(data) - 1);
    CHECK(muntar_to_path(root, tar, length) == MTAR_ESUCCESS);
    CHECK(muntar_to_path(root, tar, length) == MTAR_EWRITEFAIL);
    snprintf(canary, sizeof(canary), "%s/occupied", root);
    CHECK(mkdir(canary, 0700) == 0);
    length = tar_one(tar, "occupied", MTAR_TREG, data, sizeof(data) - 1);
    CHECK(muntar_to_path(root, tar, length) == MTAR_EWRITEFAIL);
    CHECK(rmdir(canary) == 0);
    length = tar_one(tar, "link", MTAR_TSYM, data, 0);
    CHECK(muntar_to_path(root, tar, length) == MTAR_ESUCCESS);
    snprintf(canary, sizeof(canary), "%s/safe.txt", root);
    unlink(canary);
    remove_fixture_directory(root);
}

int main(void)
{
    test_gzip();
    test_tar_and_paths();
    return failures != 0;
}
