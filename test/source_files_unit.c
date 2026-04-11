#include <stdio.h>

#include "support/source_files.h"

struct SourcePathCase {
    const char *path;
    int expected;
};

static int assert_case(const struct SourcePathCase *test_case)
{
    int actual = cjit_classify_source_path(test_case->path);
    if (actual == test_case->expected) {
        return 0;
    }
    fprintf(stderr, "source classification failed for %s: expected %d, got %d\n",
            test_case->path, test_case->expected, actual);
    return 1;
}

int main(void)
{
    static const struct SourcePathCase cases[] = {
        { "hello.c", 1 },
        { "hello.C", 1 },
        { "hello.cc", 1 },
        { "hello.cxx", 1 },
        { "hello.h", -1 },
        { "hello.txt", -1 },
        { "hello", 0 },
    };
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        if (assert_case(&cases[i]) != 0) {
            return 1;
        }
    }
    return 0;
}
