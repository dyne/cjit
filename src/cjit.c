/* CJIT https://dyne.org/cjit
 *
 * Copyright (C) 2024 Dyne.org foundation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <cjit.h>
#include <libtcc.h>
#include <cwalk.h>
//#include <xfs.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h> // _err/_out
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h> // getpid/write

#define MAX_PATH 260 // rather short paths
#define MAX_STRING 20480 // max 20KiB strings

// declared at bottom
void _out(const char *fmt, ...);
void _err(const char *fmt, ...);

// from win-compat.c
extern void    win_compat_usleep(unsigned int microseconds);
extern ssize_t win_compat_getline(char **lineptr, size_t *n, FILE *stream);
extern bool    get_winsdkpath(char *dst, size_t destlen);

static bool cjit_mkdtemp(CJITState *cjit) {
	char tempDir[MAX_PATH];
#if defined(WINDOWS)
	char tempPath[MAX_PATH];
	char filename [64];
	snprintf(filename,63,"CJIT-%s",VERSION);
	// Get the temporary path
	if (GetTempPath(MAX_PATH, tempPath) == 0) {
		_err("Failed to get temporary path");
		return false;
	}
	PathCombine(tempDir, tempPath, filename);
	// return already if found existing
	DWORD attributes = GetFileAttributes(tempDir);
	if (attributes == INVALID_FILE_ATTRIBUTES) {
		// The path does not exist
		cjit->fresh = true;
	} else if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
		// The path exists and is a directory
		cjit->fresh = false;
	} else {
		_err("Temp dir is a file, cannot overwrite: %s",tempDir);
		// The path exists but is not a directory
		return(false);
	}
	if(cjit->fresh) {
		// Create the temporary directory
		if (CreateDirectory(tempDir, NULL) == 0) {
			_err("Failed to create temporary dir: %s",tempDir);
			return false;
		}
	}
#else // POSIX
	snprintf(tempDir,259,"/tmp/cjit-%s",VERSION);
	struct stat info;
	if (stat(tempDir, &info) != 0) {
		// stat() failed; the path does not exist
		cjit->fresh = true;
	} else if (info.st_mode & S_IFDIR) {
		// The path exists and is a directory
		cjit->fresh = false;
	} else {
		_err("Temp dir is a file, cannot overwrite: %s",tempDir);
		// The path exists but is not a directory
		return(false);
	}
	if(cjit->fresh) mkdir(tempDir,0755);
#endif
	cjit->tmpdir = malloc(strlen(tempDir)+1);
	strcpy(cjit->tmpdir, tempDir);
	return(true);
}

static void cjit_tcc_handle_error(void *n, const char *m) {
  (void)n;
  _err("%s",m);
}

CJITState* cjit_new() {
	CJITState *cjit = NULL;
	cjit = malloc(sizeof(CJITState));
	memset(cjit,0x0,sizeof(CJITState));
	// quiet is by default on when cjit's output is redirected
	// errors will still be printed on stderr
	cjit->quiet = isatty(fileno(stdout))?false:true;
	//////////////////////////////////////
	// initialize the tmpdir for execution
	if(!cjit_mkdtemp(cjit)) {
		_err("Error creating CJIT temporary execution directory");
		return(NULL);
	}
	// instantiate TCC before extracting assets because
	// extract_assets() will also add include paths using TCC
	cjit->TCC = tcc_new();
	if (!cjit->TCC) {
		_err("CJIT: Could not initialize tinyCC");
		free(cjit);
		return(NULL);
	}
	// call the generated function to populate the tmpdir
	if(!extract_assets(cjit)) {
		_err("Error extracting assets in temp dir: %s",
		     strerror(errno));
		return(NULL);
	}
	// error handler callback for TCC
	tcc_set_error_func(cjit->TCC, stderr, cjit_tcc_handle_error);
	return(cjit);
}

bool cjit_setup(CJITState *cjit) {
	// set output in memory for just in time execution
	if(cjit->done_setup) return(true);
	tcc_set_output_type(cjit->TCC,
			    cjit->tcc_output?
			    cjit->tcc_output:
			    TCC_OUTPUT_MEMORY);
#if defined(LIBC_MUSL)
	tcc_add_libc_symbols(cjit->TCC);
#endif
	if(getenv("CFLAGS")) {
		char *extra_cflags = NULL;
		extra_cflags = getenv("CFLAGS");
		_err("CFLAGS: %s",extra_cflags);
		tcc_set_options(cjit->TCC, extra_cflags);
	}
#if defined(_WIN32)
	// add symbols for windows compatibility
	tcc_add_symbol(cjit->TCC, "usleep", &win_compat_usleep);
	tcc_add_symbol(cjit->TCC, "getline", &win_compat_getline);
#endif
	// When using SDL2 these defines are needed
	tcc_define_symbol(cjit->TCC,"SDL_DISABLE_IMMINTRIN_H",NULL);
	tcc_define_symbol(cjit->TCC,"SDL_MAIN_HANDLED",NULL);

	// where is libtcc1.a found
	tcc_add_library_path(cjit->TCC, cjit->tmpdir);

	// tcc_set_lib_path(TCC,tmpdir); // this overrides all?

	tcc_add_sysinclude_path(cjit->TCC, cjit->tmpdir);
	tcc_add_sysinclude_path(cjit->TCC, ".");
	tcc_add_sysinclude_path(cjit->TCC, "include");
	tcc_add_library_path(cjit->TCC, ".");

#if defined(_WIN32)
	{
		// windows system32 libraries
		//tcc_add_library_path(TCC, "C:\\Windows\\System32")
		// 64bit
		tcc_add_library_path(cjit->TCC, "C:\\Windows\\SysWOW64");
		// tinycc win32 headers
		char *tpath = malloc(strlen(cjit->tmpdir)+32);
		strcpy(tpath,cjit->tmpdir);
		strcat(tpath,"/tinycc_win32/winapi");
		tcc_add_sysinclude_path(cjit->TCC, tpath);
		free(tpath);
		// windows SDK headers
		char *sdkpath = malloc(512);
		if( get_winsdkpath(sdkpath,511) ) {
			int pathend = strlen(sdkpath);
			strcpy(&sdkpath[pathend],"\\um"); // um/GL
			tcc_add_sysinclude_path(cjit->TCC, sdkpath);
			strcpy(&sdkpath[pathend],"\\shared"); // winapifamili.h etc.
			tcc_add_sysinclude_path(cjit->TCC, sdkpath);
		}
		free(sdkpath);
	}
#endif
	cjit->done_setup = true;
	return(true);
}

static int has_source_extension(const char *path) {
	char *ext;
	size_t extlen;
	bool is_source;
	is_source = cwk_path_get_extension(path,(const char**)&ext,&extlen);
	//_err("%s: extension: %s",__func__,ext);
	if(!is_source) // no extension at all
		return 0;
	if(extlen==2 && (ext[1]=='c' || ext[1]=='C')) is_source = true;
	else if(extlen==3
	   && (ext[1]=='c' || ext[1]=='C')
	   && (ext[2]=='c' || ext[2]=='C')) is_source = true;
	else if(extlen==4
	   && (ext[1]=='c' || ext[1]=='C')
	   && (ext[2]=='x' || ext[2]=='X')
	   && (ext[3]=='x' || ext[3]=='X')) is_source = true;
	else is_source = false;
	return (is_source? 1 : -1);
}

bool cjit_add_file(CJITState *cjit, const char *path) {
	// _err("%s",__func__);
	int is_source = has_source_extension(path);
	if(is_source == 0) { // no extension, we still add
		cjit_setup(cjit);
		if(tcc_add_file(cjit->TCC, path)<0) {
			_err("%s: tcc_add_file error",__func__);
			return false;
		}
		return true;
	}
	if(is_source>0) {
		// _err("%s: is source(%i): %s",__func__, is_source, path);
		long length = file_size(path);
		if (length == -1) return false;
		FILE *file = fopen(path, "rb");
		if (!file) {
			_err("%s: fopen error: ", __func__, strerror(errno));
			return false;
		}
		char *spath = (char*)malloc((strlen(path)+16)*sizeof(char));
		sprintf(spath,"#line 1 \"%s\"\n",path);
		size_t spath_len = strlen(spath);
		char *contents =
			(char*)malloc
			((spath_len + length + 1)
			* sizeof(char));
		if (!contents) {
			_err("%s: malloc error: %s",__func__, strerror(errno));
			free(spath);
			fclose(file);
			return false;
		}
		strcpy(contents,spath);
		free(spath);
		fread(contents+spath_len, 1, length, file);
		contents[length+spath_len] = 0x0;
		fclose(file);
		cjit_setup(cjit);
		size_t dirname;
		cwk_path_get_dirname(path,&dirname);
		if(dirname) {
			char *tmp = malloc(dirname+1);
			strncpy(tmp,path,dirname);
			tmp[dirname] = 0x0;
			tcc_add_include_path(cjit->TCC,tmp);
			free(tmp);
		}
		tcc_compile_string(cjit->TCC,contents);
		free(contents);
	} else {
		cjit_setup(cjit);
		if(tcc_add_file(cjit->TCC, path)<0) {
			_err("%s: tcc_add_file error",__func__);
			return false;
		}
	}
	return true;
}

bool cjit_compile_file(CJITState *cjit, const char *path) {
	int is_source = has_source_extension(path);
	// _err("basename: %s",tmp);
	if( is_source == 0) {
		_err("%s: filename has no extension: %s",
		     __func__,path);
		return false;
	}
	if( is_source < 0) {
		_err("%s: filename has wrong extension: %s",
		     __func__,path);
		return false;
	}
	cjit_setup(cjit);
	tcc_add_file(cjit->TCC, path);
	if(cjit->output_filename) {
		if(!cjit->quiet)
			_err("Compiling: %s -> %s",path,
			     cjit->output_filename);
		tcc_output_file(cjit->TCC,
				cjit->output_filename);
	} else {
		char *ext;
		size_t extlen;
		char *restrict tmp;
		const char *basename;
		size_t len;
		cwk_path_get_basename((char*)path, &basename, &len);
		tmp = malloc(len+2);
		strncpy(tmp,basename,len+1);
		cwk_path_get_extension(tmp,(const char**)&ext,&extlen);
		strcpy(ext,".o");
		if(!cjit->quiet)
			_err("Compiling: %s -> %s",path,tmp);
		tcc_output_file(cjit->TCC,tmp);
		free(tmp);
	}
	return true;
}

int cjit_exec(CJITState *cjit, int argc, char **argv) {
	if(cjit->done_exec) {
		_err("%s: CJIT already executed once",__func__);
		return 1;
	}

  int res = 1;
  int (*_ep)(int, char**);
  // relocate the code (link symbols)
  if (tcc_relocate(cjit->TCC) < 0) {
    _err("TCC symbol relocation error (some library missing?)");
    return -1;
  }
  _ep = tcc_get_symbol(cjit->TCC, cjit->entry?cjit->entry:"main");
  if (!_ep) {
    _err("Symbol not found in source: %s",cjit->entry?cjit->entry:"main");
    return -1;
  }
#if defined(WINDOWS)
  if(cjit->write_pid) {
	  pid_t pid = getpid();
	  FILE *fd = fopen(cjit->write_pid, "w");
	  if(!fd) {
		  _err("Cannot create pid file %s: %s",
		       cjit->write_pid, strerror(errno));
		  return -1;
	  }
	  fprintf(fd,"%d\n",pid);
	  fclose(fd);
  }
  cjit->done_exec = true;
  res = _ep(argc, argv);
  return(res);
#else // we assume anything else but WINDOWS has fork()
  pid_t pid;
  cjit->done_exec = true;
  pid = fork();
  if (pid == 0) {
	  res = _ep(argc, argv);
	  exit(res);
  } else {
	  if(cjit->write_pid) {
		  // pid_t pid = getpid();
		  FILE *fd = fopen(cjit->write_pid, "w");
		  if(!fd) {
			  _err("Cannot create pid file %s: %s",
			       cjit->write_pid, strerror(errno));
			  return -1;
		  }
		  fprintf(fd,"%d\n",pid);
		  fclose(fd);
	  }
	  int status;
	  int ret;
	  ret = waitpid(pid, &status, WUNTRACED | WCONTINUED);
	  if (ret != pid){
		  _err("Wait error in source: %s","main");
	  }
	  if (WIFEXITED(status)) {
		  res = WEXITSTATUS(status);
		  //_err("Process has returned %d", res);
	  } else if (WIFSIGNALED(status)) {
		  res = WTERMSIG(status);
		  _err("Process terminated with signal %d", WTERMSIG(status));
	  } else if (WIFSTOPPED(status)) {
		  res = WSTOPSIG(status);
		  //_err("Process has returned %d", WSTOPSIG(status));
	  } else if (WIFSTOPPED(status)) {
		  res = WSTOPSIG(status);
		  _err("Process stopped with signal", WSTOPSIG(status));
	  } else {
		  _err("wait: unknown status: %d", status);
	  }
  }
  return res;
#endif // cjit_exec with fork()
}

void cjit_free(CJITState *cjit) {
	if(cjit->tmpdir) free(cjit->tmpdir);
	if(cjit->write_pid) free(cjit->write_pid);
	if(cjit->entry) free(cjit->entry);
	if(cjit->output_filename) free(cjit->output_filename);
	if(cjit->TCC) tcc_delete(cjit->TCC);
	free(cjit);
}


// stdout message free from context
void _out(const char *fmt, ...) {
  char msg[MAX_STRING+4];
  va_list args;
  va_start(args, fmt);
  int len = vsnprintf(msg, MAX_STRING, fmt, args);
  va_end(args);
  msg[len] = '\n';
  msg[len+1] = 0x0; //safety
#if defined(__EMSCRIPTEN__)
  EM_ASM_({Module.print(UTF8ToString($0))}, msg);
#else
  write(fileno(stdout), msg, len+1);
#endif
}

// error message free from context
void _err(const char *fmt, ...) {
  char msg[MAX_STRING+4];
  int len;
  va_list args;
  va_start(args, fmt);
  len = vsnprintf(msg, MAX_STRING, fmt, args);
  va_end(args);
  msg[len] = '\n';
  msg[len+1] = 0x0;
#if defined(__EMSCRIPTEN__)
  EM_ASM_({Module.printErr(UTF8ToString($0))}, msg);
#elif defined(__ANDROID__)
  __android_log_print(ANDROID_LOG_ERROR, "ZEN", "%s", msg);
#else
  write(fileno(stderr),msg,len+1);
#endif
}
