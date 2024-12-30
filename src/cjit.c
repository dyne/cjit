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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h> // _err/_out
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h> // getpid/write
#include <fcntl.h> // open(2)
#include <inttypes.h>
#include <sys/stat.h> // fstat(2)

#define MAX_PATH 260 // rather short paths
#define MAX_STRING 20480 // max 20KiB strings

#define tcc(cjit) (TCCState*)cjit->TCC
#define setup if(!cjit->done_setup)cjit_setup(cjit)
#define debug(fmt,par) if(!cjit->quiet)_err(fmt,par)
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
	cjit->TCC = (void*)tcc_new();
	if (!cjit->TCC) {
		_err("CJIT: Could not initialize tinyCC");
		free(cjit);
		return(NULL);
	}
	// set default execution in memory
	cjit->tcc_output = TCC_OUTPUT_MEMORY;
	// call the generated function to populate the tmpdir
	if(!extract_assets(cjit)) {
		_err("Error extracting assets in temp dir: %s",
		     strerror(errno));
		return(NULL);
	}
	// error handler callback for TCC
	tcc_set_error_func(tcc(cjit), stderr, cjit_tcc_handle_error);
	return(cjit);
}

static bool cjit_setup(CJITState *cjit) {
	// set output in memory for just in time execution
	if(cjit->done_setup) {
		_err("Warning: cjit_setup called twice or more times");
		return(true);
	}
	tcc_set_output_type(tcc(cjit), cjit->tcc_output);
#if defined(LIBC_MUSL)
	tcc_add_libc_symbols(tcc(cjit));
#endif
	if(getenv("CFLAGS")) {
		char *extra_cflags = NULL;
		extra_cflags = getenv("CFLAGS");
		_err("CFLAGS: %s",extra_cflags);
		tcc_set_options(tcc(cjit), extra_cflags);
	}
#if defined(_WIN32)
	// add symbols for windows compatibility
	tcc_add_symbol(tcc(cjit), "usleep", &win_compat_usleep);
	tcc_add_symbol(tcc(cjit), "getline", &win_compat_getline);
#endif
	// When using SDL2 these defines are needed
	tcc_define_symbol(tcc(cjit),"SDL_DISABLE_IMMINTRIN_H",NULL);
	tcc_define_symbol(tcc(cjit),"SDL_MAIN_HANDLED",NULL);

	// where is libtcc1.a found
	tcc_add_library_path(tcc(cjit), cjit->tmpdir);

	// tcc_set_lib_path(TCC,tmpdir); // this overrides all?

	tcc_add_sysinclude_path(tcc(cjit), cjit->tmpdir);
	tcc_add_sysinclude_path(tcc(cjit), ".");
	tcc_add_sysinclude_path(tcc(cjit), "include");
	tcc_add_library_path(tcc(cjit), ".");

#if defined(_WIN32)
	{
		// windows system32 libraries
		//tcc_add_library_path(TCC, "C:\\Windows\\System32")
		// 64bit
		tcc_add_library_path(tcc(cjit), "C:\\Windows\\SysWOW64");
		// tinycc win32 headers
		char *tpath = malloc(strlen(cjit->tmpdir)+32);
		strcpy(tpath,cjit->tmpdir);
		strcat(tpath,"/tinycc_win32/winapi");
		tcc_add_sysinclude_path(tcc(cjit), tpath);
		free(tpath);
		// windows SDK headers
		char *sdkpath = malloc(512);
		if( get_winsdkpath(sdkpath,511) ) {
			int pathend = strlen(sdkpath);
			strcpy(&sdkpath[pathend],"\\um"); // um/GL
			tcc_add_sysinclude_path(tcc(cjit), sdkpath);
			strcpy(&sdkpath[pathend],"\\shared"); // winapifamili.h etc.
			tcc_add_sysinclude_path(tcc(cjit), sdkpath);
		}
		free(sdkpath);
	}
#endif
	cjit->done_setup = true;
	return(true);
}

bool cjit_status(CJITState *cjit) {
	_err("CJIT %s (c) 2024-2025 Dyne.org foundation",VERSION);
	_err("Build system: %s",PLATFORM);
#if defined(TCC_TARGET_I386)
        _err("Target platform: i386 code generator");
#elif defined(TCC_TARGET_X86_64)
        _err("Target platform: x86-64 code generator");
#elif defined(TCC_TARGET_ARM)
        _err("Target platform: ARMv4 code generator");
#elif defined(TCC_TARGET_ARM64)
        _err("Target platform: ARMv8 code generator");
#elif defined(TCC_TARGET_C67)
        _err("Target platform: TMS320C67xx code generator");
#elif defined(TCC_TARGET_RISCV64)
        _err("Target platform: risc-v code generator");
#else
        _err("Target platform: No target is defined");
#endif
#if   defined(TCC_TARGET_PE) && defined(TCC_TARGET_X86_64)
	_err("Target system: WIN64");
#elif defined(TCC_TARGET_PE) && defined(TCC_TARGET_I386)
	_err("Target system: WIN32");
#elif defined(TCC_TARGET_MACHO)
	_err("Target system: Apple/OSX");
#elif defined(TARGETOS_Linux)
	_err("Target system: GNU/Linux");
#elif defined(TARGETOS_BSD)
	_err("Target system: BSD");
#endif
#if !(defined TCC_TARGET_PE || defined TCC_TARGET_MACHO)
	_err("ELF interpreter: %s",CONFIG_TCC_ELFINTERP);
#endif
	return true;
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

static int detect_bom(const char *filename,size_t *filesize) {
	uint8_t bom[3];
	int res;
	int fd = open(filename, O_RDONLY | O_BINARY);
	if(fd<0) {
		_err("%s: error opening file: %s",__func__,filename);
		_err("%s",strerror(errno));
		return -1;
	}
	struct stat st;
	if (fstat(fd, &st) == -1) {
		_err("%s: error analyzing file: %s",__func__,filename);
		_err("%s",strerror(errno));
		close(fd);
		return -1;
	}
	*filesize = st.st_size;
	res = read(fd,bom,3);
	close(fd);
	if (res!=3) {
		_err("%s: error reading file: %s",__func__,filename);
		_err("%s",strerror(errno));
		return -1;
	}
	// _err("%s bom: %x %x %x",filename,bom[0],bom[1],bom[2]);
	if (bom[0] == 0xFF && bom[1] == 0xFE) {
		return 1; // UTF-16 LE
	} else if (bom[0] == 0xFE && bom[1] == 0xFF) {
		return 2; // UTF-16 BE
	} else if (bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) {
		return 3; // UTF-8
	} else {
		return 0; // No BOM
	}
}

bool cjit_add_buffer(CJITState *cjit, const char *buffer) {
	setup;
	tcc_compile_string(tcc(cjit),buffer);
	debug("+B %p",buffer);
}

bool cjit_add_source(CJITState *cjit, const char *path) {
	setup;
	size_t length;
	int res = detect_bom(path,&length);
	if(res<0) {
		_err("Cannot open file: %s",path);
		_err("Execution aborted.");
		return false;
	} else if(res>0) {
		_err("UTF BOM detected in file: %s",path);
		_err("Encoding is not yet supported, execution aborted.");
		return false;
	}
	FILE *file = fopen(path, "rb");
	if (!file) {
		_err("%s: fopen error: ", __func__, strerror(errno));
		return false;
	}
	char *spath = (char*)malloc((strlen(path)+16)*sizeof(char));
	if (!spath) {
		_err("%s: malloc error: %s",__func__, strerror(errno));
		fclose(file);
		return false;
	}
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
	size_t dirname;
	cwk_path_get_dirname(path,&dirname);
	if(dirname) {
		char *tmp = malloc(dirname+1);
		strncpy(tmp,path,dirname);
		tmp[dirname] = 0x0;
		tcc_add_include_path(tcc(cjit),tmp);
		free(tmp);
	}
	tcc_compile_string(tcc(cjit),contents);
	free(contents);
	debug("+S %s",path);
	return true;
}

bool cjit_add_file(CJITState *cjit, const char *path) {
	setup;
	int is_source = has_source_extension(path);
	if(is_source == 0) { // no extension, we still add
		if(tcc_add_file(tcc(cjit), path)<0) {
			_err("%s: tcc_add_file error: %s",__func__,path);
			return false;
		}
		return true;
	}
	if(is_source>0) {
		if(!cjit_add_source(cjit, path)) {
			_err("%s: error: %s",__func__,path);
			return false;
		}
	} else {
		if(tcc_add_file(tcc(cjit), path)<0) {
			_err("%s: tcc_add_file error: %s",__func__,path);
			return false;
		}
	}
	debug("+F %s",path);
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
	setup;
	tcc_add_file(tcc(cjit), path);
	if(cjit->output_filename) {
		if(!cjit->quiet)
			_err("Compiling: %s -> %s",path,
			     cjit->output_filename);
		tcc_output_file(tcc(cjit),
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
		tcc_output_file(tcc(cjit),tmp);
		free(tmp);
	}
	return true;
}

// link all setup and create an executable file
int cjit_link(CJITState *cjit) {
	if(!cjit->done_setup) {
		_err("%s: no source code found",__func__);
		return 1;
	}
	if(!cjit->output_filename) {
		_err("%s: no output file configured (-o)",__func__);
		return 1;
	}
	return( tcc_output_file(tcc(cjit),cjit->output_filename));
}

int cjit_exec(CJITState *cjit, int argc, char **argv) {
	if(!cjit->done_setup) {
		_err("%s: no source code found",__func__);
		return 1;
	}
	if(cjit->done_exec) {
		_err("%s: CJIT already executed once",__func__);
		return 1;
	}
	int res = 1;
	int (*_ep)(int, char**);
	// relocate the code (link symbols)
	if (tcc_relocate(tcc(cjit)) < 0) {
		_err("%s: TCC linker error",__func__);
		_err("Library functions missing.");
		return -1;
	}
	_ep = tcc_get_symbol(tcc(cjit), cjit->entry?cjit->entry:"main");
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
	if(cjit->TCC) tcc_delete(tcc(cjit));
	free(cjit);
}


// wrappers to make TCC opaque
void cjit_set_output(CJITState *cjit, int output) {
	if(output>5 || output<1)
		_err("%s: invalid output: %i",__func__,output);
	else
		cjit->tcc_output = output;
}
void cjit_define_symbol(CJITState *cjit, const char *sym, const char *value) {
	tcc_define_symbol(tcc(cjit),sym,value);
	if(!cjit->quiet)_err("+D %s",sym,value?value:"");
}
void cjit_add_include_path(CJITState *cjit, const char *path) {
	tcc_add_include_path(tcc(cjit), path);
	if(!cjit->quiet)_err("+I %s",path);
}
// TODO: temporary, to be reimplemented in linker.c
void cjit_add_library_path(CJITState *cjit, const char *path) {
	tcc_add_library_path(tcc(cjit), path);
	if(!cjit->quiet)_err("+L %s",path);
}
// TODO: temporary, to be reimplemented in linker.c
void cjit_add_library(CJITState *cjit, const char *path) {
	tcc_add_library(tcc(cjit), path);
	if(!cjit->quiet)_err("+l %s",path);
}
void cjit_set_tcc_options(CJITState *cjit, const char *opts) {
	tcc_set_options(tcc(cjit),opts);
	if(!cjit->quiet)_err("+O %s",opts);
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
