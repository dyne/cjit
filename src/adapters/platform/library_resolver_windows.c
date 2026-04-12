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

#if defined(WINDOWS)

#include <stdio.h>
#include <sys/stat.h>

#include "cjit.h"
#include "support/string_list.h"

#define debug(fmt, par) if (cjit->verbose) _err(fmt, par)

static int windows_resolve_libs(CJITState *cjit)
{
    char tryfile[PATH_MAX];
    int i;
    int ii;
    int libnames_num;
    int libpaths_num;
    bool found;
    char *lname;
    char *lpath;
    struct stat st;

    libpaths_num = (int)string_list_count(cjit->libpaths);
    libnames_num = (int)string_list_count(cjit->libs);
    for (i = 0; i < libnames_num; i++) {
        found = false;
        lname = string_list_get(cjit->libs, i);
        for (ii = 0; ii < libpaths_num; ii++) {
            lpath = string_list_get(cjit->libpaths, ii);
            snprintf(tryfile, PATH_MAX - 2, "%s/%s.dll", lpath, lname);
            debug("resolve_libs try: %s", tryfile);
            if (stat(tryfile, &st) == 0) {
                string_list_add(cjit->reallibs, tryfile);
                debug("library found: %s", tryfile);
                found = true;
                break;
            }
        }
        if (!found) {
            _err("Library not found: %s.dll", lname);
        }
    }
    return (int)string_list_count(cjit->reallibs);
}

static CJITResult resolve_impl(void *context,
                               const LibraryResolverRequest *request,
                               LibraryResolverResponse *response)
{
    CJITState *cjit;

    cjit = (CJITState *)context;
    (void)request;
    response->resolved_count = windows_resolve_libs(cjit);
    response->resolved_paths = NULL;
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
