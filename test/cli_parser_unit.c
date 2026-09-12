#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adapters/cli/route_parser.h"

static int failures;

static void expect_define(const char *input, int expected_result,
                          const char *expected_symbol, const char *expected_value)
{
    char buffer[1100];
    int result;

    strcpy(buffer, input);
    result = cli_parse_define_value(buffer);
    if (result != expected_result || strcmp(buffer, expected_symbol) != 0 ||
        (expected_value && strcmp(buffer + result, expected_value) != 0)) {
        fprintf(stderr, "define %s: result %d, symbol %s\n", input, result, buffer);
        ++failures;
    }
}

static void expect_filtered(char **argv, int argc, const char *const *patterns,
                            int pattern_count, int expected_argc,
                            const char *const *expected)
{
    char **filtered = cli_remove_ignored_arguments(&argc, argv, patterns, pattern_count);
    int index;

    if (!filtered || argc != expected_argc) {
        fprintf(stderr, "filter count: expected %d, got %d\n", expected_argc, argc);
        ++failures;
    } else {
        for (index = 0; index < argc; ++index) {
            if (strcmp(filtered[index], expected[index]) != 0) {
                fprintf(stderr, "filter argument %d: expected %s, got %s\n",
                        index, expected[index], filtered[index]);
                ++failures;
            }
        }
        if (filtered[argc] != NULL) {
            fprintf(stderr, "filter result is not null terminated\n");
            ++failures;
        }
    }
    free(filtered);
}

int main(void)
{
    static const char *const patterns[] = { "-s", "-MF:", "-M:" };
    static const char *const empty_expected[] = { "cjit" };
    static const char *const mixed_expected[] = { "cjit", "source.c", "--", "-MF", "app.d", "-MMD" };
    static const char *const repeated_expected[] = { "cjit", "source.c" };
    char *empty_argv[] = { "cjit" };
    char *mixed_argv[] = { "cjit", "-MMD", "-MF", "build.d", "-s", "source.c", "--", "-MF", "app.d", "-MMD" };
    char *repeated_argv[] = { "cjit", "-MMD", "-MMD", "-MFout.d", "-MF", "other.d", "source.c" };
    char long_symbol[1026];

    expect_define("SYMBOL", 0, "SYMBOL", NULL);
    expect_define("KEY=VALUE", 4, "KEY", "VALUE");
    expect_define("CONFIG_TRIPLET=\"x86_64-linux-gnu\"", 15,
                  "CONFIG_TRIPLET", "\"x86_64-linux-gnu\"");
    expect_define("=VALUE", 1, "", "VALUE");
    expect_define("KEY=", 4, "KEY", "");
    expect_define("KEY=VALUE=AGAIN", -1, "KEY=VALUE=AGAIN", NULL);
    expect_define("BAD-NAME", -1, "BAD-NAME", NULL);
    memset(long_symbol, 'a', 1024);
    long_symbol[1024] = '\0';
    expect_define(long_symbol, 0, long_symbol, NULL);
    long_symbol[1024] = 'a';
    long_symbol[1025] = '\0';
    expect_define(long_symbol, -1, long_symbol, NULL);

    expect_filtered(empty_argv, 1, patterns, 3, 1, empty_expected);
    expect_filtered(mixed_argv, 10, patterns, 3, 6, mixed_expected);
    expect_filtered(repeated_argv, 7, patterns, 3, 2, repeated_expected);
    return failures ? 1 : 0;
}
