#include <stdio.h>

#include "support/source_files.h"

struct SourcePathCase {
    const char *path;
    int expected;
};

int main(void)
{
    static const struct SourcePathCase cases[] = {
        { NULL, 0 },
        { "", 0 },
        { "dir.with.dot/file.c", 1 },
        { "uppercase.CXX", 1 },
        { "mixed.Cc", 1 },
        { "mixed.cXx", 1 },
        { "trailing.", -1 },
        { ".hidden", -1 },
        { "archive.a", -1 },
        { "header.hpp", -1 },
    };
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        int actual = cjit_classify_source_path(cases[i].path);
        if (actual != cases[i].expected) {
            fprintf(stderr, "source classification failed for %s: expected %d, got %d\n",
                    cases[i].path ? cases[i].path : "(null)", cases[i].expected, actual);
            return 1;
        }
    }
    return 0;
}
