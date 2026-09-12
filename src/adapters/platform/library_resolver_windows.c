/*
 * Library path resolver from lib names on Windows systems
 *
 *  Copyright (c) 2024-2026 Dyne.org
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "adapters/platform/library_resolver_windows.h"

#include "adapters/platform/build_platform.h"

#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#include "cjit.h"
#include "support/string_list.h"

#if defined(_MSC_VER)
#define CJIT_S_ISREG(mode) (((mode) & _S_IFMT) == _S_IFREG)
#else
#define CJIT_S_ISREG(mode) S_ISREG(mode)
#endif

static bool has_dll_extension(const char *name)
{
    size_t length = strlen(name);
    const char *extension;

    if (length < 4) {
        return false;
    }
    extension = name + length - 4;
    return extension[0] == '.' &&
           (extension[1] == 'd' || extension[1] == 'D') &&
           (extension[2] == 'l' || extension[2] == 'L') &&
           (extension[3] == 'l' || extension[3] == 'L');
}

int windows_resolve_library_lists(const StringList *libraries,
                                  const StringList *library_paths,
                                  StringList *resolved)
{
    char tryfile[PATH_MAX];
    int library_index;
    int path_index;

    for (library_index = 0;
         library_index < (int)string_list_count(libraries);
         library_index++) {
        const char *name = string_list_get(libraries, library_index);
        bool found = false;
        bool has_extension = has_dll_extension(name);

        for (path_index = 0;
             path_index < (int)string_list_count(library_paths);
             path_index++) {
            const char *path = string_list_get(library_paths, path_index);
            struct stat st;

            snprintf(tryfile, sizeof(tryfile), "%s/%s%s", path, name,
                     has_extension ? "" : ".dll");
            if (stat(tryfile, &st) == 0 && CJIT_S_ISREG(st.st_mode)) {
                string_list_add(resolved, tryfile);
                found = true;
                break;
            }
        }
        if (!found) {
            _err("Library not found: %s%s", name,
                 has_extension ? "" : ".dll");
        }
    }
    return (int)string_list_count(resolved);
}

#if defined(WINDOWS)

static CJITResult resolve_impl(void *context,
                               const LibraryResolverRequest *request,
                               LibraryResolverResponse *response)
{
    CJITState *cjit;

    cjit = (CJITState *)context;
    (void)request;
    response->resolved_count = windows_resolve_library_lists(request->libraries,
                                                             request->search_paths,
                                                             cjit->reallibs);
    response->resolved_paths = cjit->reallibs;
    return cjit_result_ok();
}

#else

static CJITResult resolve_impl(void *context,
                               const LibraryResolverRequest *request,
                               LibraryResolverResponse *response)
{
    (void)context;
    (void)request;
    response->resolved_count = 0;
    response->resolved_paths = NULL;
    return cjit_result_error(CJIT_RESULT_PLATFORM_ERROR, 1,
                             "Windows library resolver unavailable on this platform");
}

#endif

const LibraryResolverPort windows_library_resolver_port = {
    .context = NULL,
    .resolve = resolve_impl
};
