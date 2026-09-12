#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_WIN32) && !defined(__MINGW32__)
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#include "adapters/fs/local_filesystem.h"
#include "cjit.h"

/* The cache-policy unit does not exercise the low-level file port. */
char *file_load(const char *path, unsigned int *length)
{
    (void)path;
    (void)length;
    return NULL;
}

char *load_stdin(void)
{
    return NULL;
}

char *new_abspath(const char *path)
{
    (void)path;
    return NULL;
}

bool write_to_file(const char *path, const char *name, const char *contents,
                   unsigned int length)
{
    (void)path;
    (void)name;
    (void)contents;
    (void)length;
    return false;
}

void _err(const char *fmt, ...)
{
    (void)fmt;
}

static int failures;

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        failures++;
    }
}

#if !defined(_WIN32) && !defined(__MINGW32__) && !defined(WINDOWS)
static void make_directory(const char *path)
{
    expect(mkdir(path, 0700) == 0, "directory fixture is created");
}

static void make_file(const char *path, size_t size)
{
    FILE *file = fopen(path, "wb");
    expect(file != NULL, "runtime fixture file is created");
    if (file) {
        if (size != 0) {
            expect(fseek(file, (long)size - 1, SEEK_SET) == 0, "runtime fixture grows");
            expect(fputc(0, file) != EOF, "runtime fixture final byte is written");
        }
        fclose(file);
    }
}

static void populate_complete_cache(const char *root)
{
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/include", root);
    make_directory(path);
    snprintf(path, sizeof(path), "%s/libtcc1.a", root);
    make_file(path, 1024);
    snprintf(path, sizeof(path), "%s/include/stdarg.h", root);
    make_file(path, 64);
    snprintf(path, sizeof(path), "%s/include/stddef.h", root);
    make_file(path, 64);
    snprintf(path, sizeof(path), "%s/include/tccdefs.h", root);
    make_file(path, 1024);
}

int main(void)
{
    char temporary[] = "/tmp/cjit-cache-unit.XXXXXX";
    char root[PATH_MAX];
    char cache_root[PATH_MAX];
    char version_root[PATH_MAX];
    char path[PATH_MAX];
    CJITState state = {0};
    char *previous_tmpdir = getenv("TMPDIR") ? strdup(getenv("TMPDIR")) : NULL;
    static const struct {
        const char *relative_path;
        size_t minimum_size;
    } required[] = {
        { "libtcc1.a", 1024 },
        { "include/stdarg.h", 64 },
        { "include/stddef.h", 64 },
        { "include/tccdefs.h", 1024 },
    };
    size_t i;

    expect(mkdtemp(temporary) != NULL, "temporary root is created");
    if (failures) {
        return 1;
    }
    snprintf(root, sizeof(root), "%s/runtime", temporary);
    make_directory(root);
    expect(!cjit_runtime_cache_is_complete(NULL), "NULL cache root is incomplete");
    expect(!cjit_runtime_cache_is_complete(""), "empty cache root is incomplete");
    expect(!cjit_runtime_cache_is_complete("/definitely/missing/cjit-cache-unit"),
           "missing cache root is incomplete");
    expect(!cjit_runtime_cache_is_complete(root), "empty cache is incomplete");
    populate_complete_cache(root);
    expect(cjit_runtime_cache_is_complete(root), "complete cache is reusable");

    for (i = 0; i < sizeof(required) / sizeof(required[0]); ++i) {
        snprintf(path, sizeof(path), "%s/%s", root, required[i].relative_path);
        unlink(path);
        expect(!cjit_runtime_cache_is_complete(root), "missing required entry refreshes cache");
        make_directory(path);
        expect(!cjit_runtime_cache_is_complete(root), "required directory is not accepted as file");
        rmdir(path);
        make_file(path, required[i].minimum_size - 1);
        expect(!cjit_runtime_cache_is_complete(root), "undersized required entry refreshes cache");
        unlink(path);
        make_file(path, required[i].minimum_size);
        expect(cjit_runtime_cache_is_complete(root), "repaired required entry restores reuse");
    }

    expect(setenv("TMPDIR", temporary, 1) == 0, "TMPDIR fixture is configured");
    expect(cjit_mkdtemp(&state, NULL), "default versioned cache is created");
    expect(state.fresh, "missing versioned cache is marked fresh");
    snprintf(cache_root, sizeof(cache_root), "%s/cjit", temporary);
    snprintf(version_root, sizeof(version_root), "%s/test-runtime", cache_root);
    expect(strcmp(state.tmpdir, version_root) == 0, "TMPDIR selects exact versioned cache");
    free(state.tmpdir);
    state.tmpdir = NULL;

    expect(cjit_mkdtemp(&state, NULL), "incomplete cache is refreshed");
    expect(state.fresh, "incomplete cache is fresh after refresh");
    expect(access(version_root, F_OK) == 0, "refresh leaves only versioned target directory");
    free(state.tmpdir);
    state.tmpdir = NULL;

    snprintf(path, sizeof(path), "%s/collision", temporary);
    make_file(path, 1);
    expect(!cjit_mkdtemp(&state, path), "custom file collision is rejected");
    unlink(path);

    snprintf(path, sizeof(path), "%s/custom dir", temporary);
    expect(cjit_mkdtemp(&state, path), "custom whitespace path is created");
    expect(state.fresh, "new custom path is fresh");
    free(state.tmpdir);
    state.tmpdir = NULL;
    expect(cjit_mkdtemp(&state, path), "existing custom path is reused");
    expect(!state.fresh, "existing custom path is not fresh");
    free(state.tmpdir);

    if (previous_tmpdir) {
        setenv("TMPDIR", previous_tmpdir, 1);
    } else {
        unsetenv("TMPDIR");
    }
    free(previous_tmpdir);
    snprintf(path, sizeof(path), "%s/runtime/include/stdarg.h", temporary);
    unlink(path);
    snprintf(path, sizeof(path), "%s/runtime/include/stddef.h", temporary);
    unlink(path);
    snprintf(path, sizeof(path), "%s/runtime/include/tccdefs.h", temporary);
    unlink(path);
    snprintf(path, sizeof(path), "%s/runtime/include", temporary);
    rmdir(path);
    snprintf(path, sizeof(path), "%s/runtime/libtcc1.a", temporary);
    unlink(path);
    snprintf(path, sizeof(path), "%s/runtime", temporary);
    rmdir(path);
    snprintf(path, sizeof(path), "%s/custom dir", temporary);
    rmdir(path);
    snprintf(path, sizeof(path), "%s/cjit/test-runtime", temporary);
    rmdir(path);
    snprintf(path, sizeof(path), "%s/cjit", temporary);
    rmdir(path);
    rmdir(temporary);
    return failures != 0;
}
#else
static int checks;
static void make_directory(const char *path) { expect(CreateDirectoryA(path, NULL) != 0, "Windows cache directory is created"); }
static void make_file(const char *path, size_t size) { FILE *file = fopen(path, "wb"); expect(file != NULL, "Windows cache file is created"); if (file) { if (size) { expect(fseek(file, (long)size - 1, SEEK_SET) == 0, "Windows cache file grows"); expect(fputc(0, file) != EOF, "Windows cache file writes"); } fclose(file); } }
static void populate_complete_cache(const char *root) { char path[MAX_PATH]; const char *files[] = { "libtcc1.a", "include\\stdarg.h", "include\\stddef.h", "include\\tccdefs.h", "tinycc_win32\\stdio.h", "tinycc_win32\\_mingw.h", "win32ports\\unistd.h" }; size_t i; snprintf(path, sizeof(path), "%s\\include", root); make_directory(path); snprintf(path, sizeof(path), "%s\\tinycc_win32", root); make_directory(path); snprintf(path, sizeof(path), "%s\\win32ports", root); make_directory(path); for (i = 0; i < sizeof(files) / sizeof(files[0]); ++i) { snprintf(path, sizeof(path), "%s\\%s", root, files[i]); make_file(path, 2048); } }

int main(void)
{
    char temporary[MAX_PATH], root[MAX_PATH], path[MAX_PATH];
    CJITState state = {0};
    expect(GetTempPathA(sizeof(temporary), temporary) != 0, "Windows cache temp root is available");
    expect(GetTempFileNameA(temporary, "cjc", 0, root) != 0, "Windows cache temp name is allocated");
    DeleteFileA(root); make_directory(root);
    expect(!cjit_runtime_cache_is_complete(root), "Windows empty cache is incomplete");
    populate_complete_cache(root);
    expect(cjit_runtime_cache_is_complete(root), "Windows complete cache is reusable");
    snprintf(path, sizeof(path), "%s\\include\\stdarg.h", root); DeleteFileA(path);
    expect(!cjit_runtime_cache_is_complete(root), "Windows incomplete cache is rejected");
    make_file(path, 2048); expect(cjit_runtime_cache_is_complete(root), "Windows repaired cache is reusable");
    snprintf(path, sizeof(path), "%s\\custom cache", root);
    expect(cjit_mkdtemp(&state, path) && state.fresh, "Windows whitespace cache is created"); free(state.tmpdir); state.tmpdir = NULL;
    expect(cjit_mkdtemp(&state, path) && !state.fresh, "Windows whitespace cache is reused"); free(state.tmpdir);
    RemoveDirectoryA(path);
    snprintf(path, sizeof(path), "%s\\libtcc1.a", root); DeleteFileA(path);
    snprintf(path, sizeof(path), "%s\\include\\stdarg.h", root); DeleteFileA(path);
    snprintf(path, sizeof(path), "%s\\include\\stddef.h", root); DeleteFileA(path);
    snprintf(path, sizeof(path), "%s\\include\\tccdefs.h", root); DeleteFileA(path);
    snprintf(path, sizeof(path), "%s\\tinycc_win32\\stdio.h", root); DeleteFileA(path);
    snprintf(path, sizeof(path), "%s\\tinycc_win32\\_mingw.h", root); DeleteFileA(path);
    snprintf(path, sizeof(path), "%s\\win32ports\\unistd.h", root); DeleteFileA(path);
    snprintf(path, sizeof(path), "%s\\include", root); RemoveDirectoryA(path);
    snprintf(path, sizeof(path), "%s\\tinycc_win32", root); RemoveDirectoryA(path);
    snprintf(path, sizeof(path), "%s\\win32ports", root); RemoveDirectoryA(path);
    RemoveDirectoryA(root);
    checks = 8;
    return failures != 0 || checks == 0;
}
#endif
