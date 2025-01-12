/*
 * Library path resolver from lib names on Windows systems
 *
 *  Copyright (c) 2024-2025 Dyne.org
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

#include <platforms.h>
#if defined(WINDOWS)

#include <stdio.h>
#include <sys/stat.h>

#include <cjit.h>
// #include <cwalk.h>
#include <array.h>

#define debug(fmt,par) if(cjit->verbose)_err(fmt,par)

// takes the list of cjit->libpaths and cjit->libs
// then resolves them all looking for ${lib}.dll in paths
int resolve_libs(CJITState *cjit) {
	char tryfile[PATH_MAX];
	int i,ii;
	int libpaths_num, libnames_num;
	char *lpath, *lname;
	bool found = false;
    struct stat st;
	// search in all paths if lib%s.so exists
	// TODO: support --static here
	libpaths_num = XArray_Used(cjit->libpaths);
	libnames_num = XArray_Used(cjit->libs);
	for(i=0;i<libnames_num;i++) {
		found=false;
		lname = XArray_GetData(cjit->libs,i);
		for(ii=0;ii<libpaths_num;ii++) {
			lpath = XArray_GetData(cjit->libpaths,ii);
			snprintf(tryfile,PATH_MAX-2,"%s/%s.dll",lpath,lname);
			debug("resolve_libs try: %s",tryfile);
			if (stat(tryfile, &st) == 0) {
				XArray_AddData(cjit->reallibs,
							   tryfile,strlen(tryfile));
				debug("library found: %s",tryfile);
				found=true;
				break;
			}
		}
		if(!found)
			_err("Library not found: %s.dll",lname);
		// continue anyway (will log missing symbol names)
	}
	return(XArray_Used(cjit->reallibs));
}

#endif // defined(WINDOWS)
