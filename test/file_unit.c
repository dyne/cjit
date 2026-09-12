#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(WINDOWS)
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "cjit.h"

extern char *load_stdin(void);
extern char *new_abspath(const char *path);
extern bool write_to_file(const char *path, const char *filename, const char *buf,
                          unsigned int len);

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

#if !defined(WINDOWS)
static void write_bytes(const char *path, const void *bytes, size_t length)
{
    FILE *file = fopen(path, "wb");
    expect(file != NULL, "fixture can be created");
    if (file) {
        expect(fwrite(bytes, 1, length, file) == length, "fixture bytes are written");
        fclose(file);
    }
}

int main(void)
{
    char temporary[] = "/tmp/cjit-file-unit.XXXXXX";
    char fixture[PATH_MAX];
    char output[PATH_MAX];
    char *contents;
    char *absolute;
    unsigned int length = 99;
    static const char binary[] = { 'a', '\0', 'b' };
    int input_pipe[2];
    int saved_stdin;
    const char stdin_source[] = "first line\nsecond line that confirms stdin growth\n";

    expect(mkdtemp(temporary) != NULL, "temporary directory is created");
    if (failures) {
        return 1;
    }
    snprintf(fixture, sizeof(fixture), "%s/input.bin", temporary);
    snprintf(output, sizeof(output), "%s/output.bin", temporary);
    write_bytes(fixture, binary, sizeof(binary));

    contents = file_load(fixture, &length);
    expect(contents != NULL, "binary file loads");
    expect(length == sizeof(binary), "binary file reports byte length");
    expect(contents && memcmp(contents, binary, sizeof(binary)) == 0,
           "binary bytes are preserved");
    expect(contents && contents[length] == '\0', "loaded buffer is terminated");
    free(contents);

    length = 99;
    expect(file_load("/definitely/missing/cjit-file-unit", &length) == NULL,
           "missing file is rejected");
    expect(file_load(fixture, NULL) == NULL, "missing length destination is rejected");
    expect(file_load(NULL, &length) == NULL, "missing path is rejected");

    snprintf(fixture, sizeof(fixture), "%s/empty", temporary);
    write_bytes(fixture, "", 0);
    expect(file_load(fixture, &length) == NULL, "empty file is rejected by current contract");

    absolute = new_abspath(".");
    expect(absolute != NULL && absolute[0] == '/', "current directory resolves absolutely");
    free(absolute);
    absolute = new_abspath("relative/../path");
    expect(absolute != NULL && absolute[0] == '/', "relative path resolves absolutely");
    free(absolute);
    expect(new_abspath(NULL) == NULL, "NULL path is rejected");
    expect(new_abspath("") == NULL, "empty path is rejected");

    expect(write_to_file(temporary, "output.bin", binary, sizeof(binary)),
           "binary output is written");
    length = 0;
    contents = file_load(output, &length);
    expect(contents != NULL && length == sizeof(binary), "written output loads");
    expect(contents && memcmp(contents, binary, sizeof(binary)) == 0,
           "written output preserves bytes");
    free(contents);
    expect(!write_to_file("/definitely/missing/cjit-file-unit", "output", "x", 1),
           "unwritable destination is rejected");
    expect(!write_to_file(temporary, "output", NULL, 1), "NULL nonempty buffer is rejected");

    expect(pipe(input_pipe) == 0, "stdin pipe is created");
    if (failures == 0) {
        expect(write(input_pipe[1], stdin_source, sizeof(stdin_source) - 1)
                   == (ssize_t)(sizeof(stdin_source) - 1),
               "stdin fixture is written");
        close(input_pipe[1]);
        saved_stdin = dup(STDIN_FILENO);
        expect(saved_stdin >= 0, "stdin is saved");
        expect(dup2(input_pipe[0], STDIN_FILENO) >= 0, "stdin is redirected");
        close(input_pipe[0]);
        contents = load_stdin();
        expect(contents != NULL, "stdin is loaded");
        expect(contents && strcmp(contents, stdin_source) == 0,
               "stdin lines are concatenated without loss");
        free(contents);
        expect(dup2(saved_stdin, STDIN_FILENO) >= 0, "stdin is restored");
        close(saved_stdin);
    }

    unlink(output);
    unlink(fixture);
    rmdir(temporary);
    return failures != 0;
}
#else
int main(void)
{
    return 0;
}
#endif
