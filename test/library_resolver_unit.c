#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "adapters/platform/library_resolver_posix.h"
#include "adapters/platform/library_resolver_windows.h"
#include "cjit.h"
#include "support/string_list.h"

static int failures;

void _err(const char *format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    vfprintf(stderr, format, arguments);
    fputc('\n', stderr);
    va_end(arguments);
}

void _out(const char *format, ...)
{
    (void)format;
}

void dynarray_add(void *items, int *count, void *data)
{
    (void)items;
    (void)count;
    free(data);
}

void dynarray_reset(void *items, int *count)
{
    (void)items;
    (void)count;
}

char *tcc_strdup(const char *value)
{
    return strdup(value);
}

char *pstrcpy(char *destination, size_t size, const char *source)
{
    if (size != 0) {
        snprintf(destination, size, "%s", source);
    }
    return destination;
}

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        failures++;
    }
}

static void path_join(char *destination, size_t size, const char *left,
                      const char *right)
{
    snprintf(destination, size, "%s/%s", left, right);
}

static void write_file(const char *path, const char *contents)
{
    FILE *file = fopen(path, "wb");

    if (!file) {
        perror(path);
        exit(2);
    }
    fputs(contents, file);
    fclose(file);
}

static void expect_list(const StringList *list, size_t count,
                        const char *first, const char *second)
{
    expect(string_list_count(list) == count, "list has expected entry count");
    if (count > 0) {
        expect(strcmp(string_list_get(list, 0), first) == 0,
               "first entry has expected value");
    }
    if (count > 1) {
        expect(strcmp(string_list_get(list, 1), second) == 0,
               "second entry has expected value");
    }
}

int main(void)
{
    char root[] = "/tmp/cjit-resolver-unit-XXXXXX";
    char conf_dir[512], include_dir[512], conf[512], child[512];
    char libraries_dir[512], first_dir[512], second_dir[512];
    char alpha[512], beta[512], link[512], script[512], broken[512], priority[512], mixed[512];
    StringList *paths = string_list_new();
    StringList *libraries = string_list_new();
    StringList *resolved = string_list_new();

    expect(mkdtemp(root) != NULL, "creates isolated resolver fixture root");
    path_join(conf_dir, sizeof(conf_dir), root, "conf");
    path_join(include_dir, sizeof(include_dir), conf_dir, "parts");
    mkdir(conf_dir, 0700);
    mkdir(include_dir, 0700);
    path_join(conf, sizeof(conf), conf_dir, "ld.so.conf");
    path_join(child, sizeof(child), include_dir, "20-extra.conf");
    write_file(child, " /fixture/third\n/fixture/first\n");
    {
        char contents[1024];
        snprintf(contents, sizeof(contents),
                 "# comment\n /fixture/first  # trailing\n\t/fixture/second\t\n"
                 "include %s/*.conf\nnot-a-path\n", include_dir);
        write_file(conf, contents);
    }
    expect(read_ldsoconf(paths, conf), "ld.so configuration parses fixture and include");
    expect_list(paths, 3, "/fixture/first", "/fixture/second");
    expect(strcmp(string_list_get(paths, 2), "/fixture/third") == 0,
           "included paths preserve deterministic order and remove duplicates");
    expect(!read_ldsoconf(paths, "/definitely/missing/cjit-ld.so.conf"),
           "missing direct configuration reports failure");
    string_list_free(&paths);

    paths = string_list_new();
    path_join(libraries_dir, sizeof(libraries_dir), root, "libraries");
    mkdir(libraries_dir, 0700);
    path_join(alpha, sizeof(alpha), libraries_dir, "libalpha.so");
    path_join(beta, sizeof(beta), libraries_dir, "libbeta.so");
    path_join(link, sizeof(link), libraries_dir, "liblink.so");
    path_join(script, sizeof(script), libraries_dir, "libscript.so");
    path_join(broken, sizeof(broken), libraries_dir, "libbroken.so");
    write_file(alpha, "\177ELF fixture");
    write_file(beta, "\177ELF fixture");
    expect(symlink("libalpha.so", link) == 0, "creates relative symlink fixture");
    write_file(script, "/* fixture */ INPUT ( libalpha.so -lbeta )\n");
    write_file(broken, "GROUP ( libalpha.so\n");
    string_list_add(paths, libraries_dir);
    string_list_add(libraries, "script");
    string_list_add(libraries, "missing");
    { int count = posix_resolve_library_lists(libraries, paths, resolved);
    expect(count == 2,
           "ld script resolves relative and -l entries while missing library is ignored");
    }
    expect(string_list_count(resolved) == 2,
           "ld script adds both relative and -l fixture libraries");
    expect(strcmp(string_list_get(resolved, 0), alpha) == 0,
           "ld script preserves the first resolved path");
    string_list_add(libraries, "link");
    expect(posix_resolve_library_lists(libraries, paths, resolved) == 5,
           "symlink requests resolve to their concrete library path");
    string_list_add(libraries, "script");
    expect(posix_resolve_library_lists(libraries, paths, resolved) == 10,
           "repeated requests intentionally append independently owned paths");
    string_list_add(libraries, "broken");
    expect(posix_resolve_library_lists(libraries, paths, resolved) == 15,
           "malformed ld scripts add no partial library result");
    {
        CJITState state = {0};
        LibraryResolverPort port = posix_library_resolver_port;
        LibraryResolverRequest request = {
            .library_count = (int)string_list_count(libraries),
            .libraries = libraries,
            .search_path_count = (int)string_list_count(paths),
            .search_paths = paths
        };
        LibraryResolverResponse response = {0};

        state.libs = string_list_new();
        state.libpaths = string_list_new();
        state.reallibs = string_list_new();
        string_list_add(state.libs, "alpha");
        string_list_add(state.libpaths, libraries_dir);
        request.libraries = state.libs;
        request.search_paths = state.libpaths;
        port.context = &state;
        expect(port.resolve(port.context, &request, &response).ok,
               "POSIX port accepts request lists independently of state inputs");
        expect(response.resolved_paths == state.reallibs && response.resolved_count == 1,
               "resolver response borrows the context-owned resolved path list");
        string_list_free(&state.libs);
        string_list_free(&state.libpaths);
        string_list_free(&state.reallibs);
    }
    string_list_free(&libraries);
    string_list_free(&resolved);

    libraries = string_list_new();
    resolved = string_list_new();
    string_list_free(&paths);
    paths = string_list_new();
    path_join(first_dir, sizeof(first_dir), root, "first dir");
    path_join(second_dir, sizeof(second_dir), root, "second dir");
    mkdir(first_dir, 0700);
    mkdir(second_dir, 0700);
    path_join(priority, sizeof(priority), first_dir, "priority.dll");
    path_join(mixed, sizeof(mixed), second_dir, "MiXeD.DLL");
    write_file(priority, "dll");
    write_file(mixed, "dll");
    string_list_add(paths, first_dir);
    string_list_add(paths, second_dir);
    string_list_add(libraries, "priority");
    string_list_add(libraries, "MiXeD.DLL");
    string_list_add(libraries, "absent");
    expect(windows_resolve_library_lists(libraries, paths, resolved) == 2,
           "Windows resolver uses ordered paths, normalizes extension, and ignores missing DLLs");
    expect_list(resolved, 2, priority, mixed);

    string_list_free(&paths);
    string_list_free(&libraries);
    string_list_free(&resolved);
    unlink(alpha); unlink(beta); unlink(link); unlink(script); unlink(broken); unlink(priority); unlink(mixed);
    unlink(child); unlink(conf);
    rmdir(first_dir); rmdir(second_dir); rmdir(libraries_dir); rmdir(include_dir);
    rmdir(conf_dir); rmdir(root);
    return failures != 0;
}
