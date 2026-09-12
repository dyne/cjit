#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "adapters/platform/runtime_platform.h"
#include "adapters/platform/library_resolver_posix.h"
#include "adapters/platform/library_resolver_windows.h"
#include "cjit.h"
#include "libtcc.h"

static int failures;
static int entry_status;

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        failures++;
    }
}

static int entrypoint(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    return entry_status;
}

static int signal_entrypoint(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    raise(SIGTERM);
    return 99;
}

void _err(const char *format, ...)
{
    (void)format;
}

int tcc_add_symbol(TCCState *state, const char *name, const void *value)
{
    (void)state;
    (void)name;
    (void)value;
    return 0;
}

int tcc_add_library_path(TCCState *state, const char *path)
{
    (void)state;
    (void)path;
    return 0;
}

int tcc_add_sysinclude_path(TCCState *state, const char *path)
{
    (void)state;
    (void)path;
    return 0;
}

int string_list_add(StringList *list, const char *value)
{
    (void)list;
    (void)value;
    return 1;
}

bool read_ldsoconf(StringList *list, const char *path)
{
    (void)list;
    (void)path;
    return true;
}

bool read_ldsoconf_dir(StringList *list, const char *path)
{
    (void)list;
    (void)path;
    return true;
}

const LibraryResolverPort posix_library_resolver_port = {0};
const LibraryResolverPort windows_library_resolver_port = {0};

int main(void)
{
    char *argv[] = { "program", NULL };
    char pid_path[] = "/tmp/cjit-platform-unit.pid";
    char invalid_path[] = "/tmp/cjit-platform-unit-missing/pid";
    char contents[64] = {0};
    FILE *pid_file;
    CJITState state = {0};
    int result;

    unlink(pid_path);
    entry_status = 7;
    state.write_pid = pid_path;
    result = cjit_platform_exec(&state, entrypoint, 1, argv);
    expect(result == 7, "POSIX child exit status is propagated");
    expect(state.done_exec, "execution is marked complete before forking");
    pid_file = fopen(pid_path, "r");
    expect(pid_file != NULL, "child PID is written after a successful fork");
    if (pid_file) {
        expect(fgets(contents, sizeof(contents), pid_file) != NULL, "PID file has a value");
        expect(strtol(contents, NULL, 10) > 0, "PID file contains a positive child PID");
        fclose(pid_file);
    }
    unlink(pid_path);

    state = (CJITState){0};
    result = cjit_platform_exec(&state, signal_entrypoint, 1, argv);
    expect(result == SIGTERM, "POSIX signal termination is reported as its signal number");
    expect(state.done_exec, "signalled execution remains single-use");

    state = (CJITState){0};
    state.write_pid = invalid_path;
    result = cjit_platform_exec(&state, entrypoint, 1, argv);
    expect(result == -1, "unwritable PID file fails the execution contract");
    expect(state.done_exec, "PID failure still consumes the execution session");
    return failures != 0;
}
