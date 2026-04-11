#include "adapters/platform/runtime_platform.h"

#include <stdlib.h>
#include <string.h>

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
