#include "adapters/platform/runtime_platform.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cwalk.h"
#include "elflinker.h"
#include "libtcc.h"
#include "support/string_list.h"

#if defined(WINDOWS)
extern void win_compat_usleep(unsigned int microseconds);
extern ssize_t win_compat_getline(char **lineptr, size_t *n, FILE *stream);
extern bool get_winsdkpath(char *dst, size_t destlen);
#endif

void cjit_platform_setup_runtime(CJITState *cjit)
{
#if defined(WINDOWS)
    size_t plen;
    char *tpath;

    tcc_add_symbol((TCCState *)cjit->TCC, "usleep", &win_compat_usleep);
    tcc_add_symbol((TCCState *)cjit->TCC, "getline", &win_compat_getline);

    string_list_add(cjit->libpaths, "C:\\Windows\\SysWOW64");
    tcc_add_library_path((TCCState *)cjit->TCC, "C:\\Windows\\SysWOW64");

    plen = strlen(cjit->tmpdir) + strlen("/tinycc_win32/winapi") + 8;
    tpath = malloc(plen);
    cwk_path_join(cjit->tmpdir, "/tinycc_win32/winapi", tpath, plen);
    tcc_add_sysinclude_path((TCCState *)cjit->TCC, tpath);
    free(tpath);

    {
        char *sdkpath = malloc(512);
        if (get_winsdkpath(sdkpath, 511)) {
            plen = strlen(sdkpath) + 16;
            tpath = malloc(plen);
            cwk_path_join(sdkpath, "/um", tpath, plen);
            tcc_add_sysinclude_path((TCCState *)cjit->TCC, tpath);
            cwk_path_join(sdkpath, "/shared", tpath, plen);
            tcc_add_sysinclude_path((TCCState *)cjit->TCC, tpath);
            free(tpath);
        }
        free(sdkpath);
    }
#elif defined(UNIX)
    read_ldsoconf(cjit->libpaths, "/etc/ld.so.conf");
    read_ldsoconf_dir(cjit->libpaths, "/etc/ld.so.conf.d");
#else
    (void)cjit;
#endif
}

/**
 * Writes the process id file used by the file watcher integration.
 */
static int write_pid_file(CJITState *cjit, pid_t pid)
{
    FILE *fd;

    if (!cjit->write_pid) {
        return 0;
    }

    fd = fopen(cjit->write_pid, "w");
    if (!fd) {
        fail(cjit->write_pid);
        _err("Cannot create pid file");
        return -1;
    }

    fprintf(fd, "%d\n", pid);
    fclose(fd);
    return 0;
}

int cjit_platform_exec(CJITState *cjit, int (*entrypoint)(int, char **),
                       int argc, char **argv)
{
#if defined(WINDOWS)
    if (write_pid_file(cjit, getpid()) < 0) {
        return -1;
    }
    cjit->done_exec = true;
    return entrypoint(argc, argv);
#else
    int res = 1;
    pid_t pid;

    cjit->done_exec = true;
    pid = fork();
    if (pid == 0) {
        res = entrypoint(argc, argv);
        exit(res);
    }

    if (write_pid_file(cjit, pid) < 0) {
        return -1;
    }

    {
        int status;
        int ret;

        ret = waitpid(pid, &status, WUNTRACED | WCONTINUED);
        if (ret != pid) {
            _err("Wait error in source: %s", cjit->entry);
        }
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        if (WIFSIGNALED(status)) {
            _err("Process terminated with signal %d", WTERMSIG(status));
            return WTERMSIG(status);
        }
        if (WIFSTOPPED(status)) {
            return WSTOPSIG(status);
        }
        _err("wait: unknown status: %d", status);
        return res;
    }
#endif
}

void cjit_platform_add_library_path(CJITState *cjit, const char *path)
{
#if defined(UNIX)
    string_list_add(cjit->libpaths, path);
#elif !defined(SHAREDTCC)
    tcc_add_library_path((TCCState *)cjit->TCC, path);
#else
    (void)cjit;
    (void)path;
#endif
}
