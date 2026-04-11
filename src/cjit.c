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
#include <elflinker.h>
#include <adapters/compiler/tinycc_adapter.h>
#include <adapters/platform/runtime_platform.h>
#include <support/string_list.h>
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

#define tcc(cjit) (TCCState*)cjit->TCC
#define debug(fmt,par) if(cjit->verbose)_err(fmt,par)
#define add(buf,s) if ((s) != NULL) string_list_add(cjit->buf, s); else _err("!!! NULL var added to list: %s", #buf)

// declared at bottom
void _out(const char *fmt, ...);
void _err(const char *fmt, ...);

// from file.c
extern char *new_abspath(const char *path);

static void cjit_tcc_handle_error(void *n, const char *m) {
  (void)n;
  _err("%s",m);
}

static int resolve_libraries(CJITState *cjit) {
	LibraryResolverPort resolver;
	LibraryResolverRequest request;
	LibraryResolverResponse response;
	request.library_count = (int)string_list_count(cjit->libs);
	request.libraries = NULL;
	request.search_path_count = (int)string_list_count(cjit->libpaths);
	request.search_paths = NULL;
	resolver = cjit_platform_library_resolver();
	resolver.context = cjit;
	if (!resolver.resolve(resolver.context, &request, &response).ok) {
		return 0;
	}
	return response.resolved_count;
}

CJITState* cjit_new() {
	CJITState *cjit = NULL;
	cjit = malloc(sizeof(CJITState));
	memset(cjit,0x0,sizeof(CJITState));
	// quiet is by default on when cjit's output is redirected
	// errors will still be printed on stderr
	cjit->quiet = isatty(fileno(stdout))?false:true;
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
	// error handler callback for TCC
	tcc_set_error_func(tcc(cjit), stderr, cjit_tcc_handle_error);
	// initialize internal arrays
	cjit->sources  = string_list_new();
	cjit->libs     = string_list_new();
	cjit->libpaths = string_list_new();
	cjit->reallibs = string_list_new();
	return(cjit);
}

void cjit_free(CJITState *cjit) {
	if(cjit->tmpdir) free(cjit->tmpdir);
	if(cjit->write_pid) free(cjit->write_pid);
	if(cjit->entry) free(cjit->entry);
	if(cjit->output_filename) free(cjit->output_filename);
	if(cjit->TCC) tcc_delete(tcc(cjit));
	string_list_free(&cjit->sources);
	string_list_free(&cjit->libs);
	string_list_free(&cjit->libpaths);
	string_list_free(&cjit->reallibs);
	free(cjit);
}

CJITResult cjit_prepare(CJITState *cjit) {
	// set output in memory for just in time execution
	if(cjit->done_setup) {
		return cjit_result_ok();
	}
#if !defined(SHAREDTCC)
	// extract all runtime assets to tmpdir
	if(!extract_assets(cjit,NULL)) {
		fail("error extracting assets in temp dir");
		return cjit_result_error(CJIT_RESULT_IO_ERROR, 1,
					 "Failed to extract runtime assets");
	}
#endif
	if (tcc_set_output_type(tcc(cjit), cjit->tcc_output) < 0) {
		return cjit_result_error(CJIT_RESULT_COMPILER_ERROR, 1,
					 "Failed to set compiler output mode");
	}
#if defined(LIBC_MUSL)
	tcc_add_libc_symbols(tcc(cjit));
#endif
	if(getenv("CFLAGS")) {
		char *extra_cflags = NULL;
		extra_cflags = getenv("CFLAGS");
		_err("CFLAGS: %s",extra_cflags);
		debug(" -C %s",extra_cflags);
		if (tcc_set_options(tcc(cjit), extra_cflags) < 0) {
			return cjit_result_error(CJIT_RESULT_COMPILER_ERROR, 1,
						 "Failed to apply compiler options");
		}
	}
	// When using SDL2 these defines are needed
	tcc_define_symbol(tcc(cjit),"SDL_DISABLE_IMMINTRIN_H",NULL);
	tcc_define_symbol(tcc(cjit),"SDL_MAIN_HANDLED",NULL);

	// where is libtcc1.a found
	// add(libpaths,cjit->tmpdir);
	// tinyCC needs libtcc1.a in library path (not added as file)
#if !defined(SHAREDTCC)
	debug(" -L %s",cjit->tmpdir);
	tcc_add_library_path(tcc(cjit),cjit->tmpdir);
	tcc_add_library_path(tcc(cjit), "."); debug(" -L %s",".");
	tcc_add_sysinclude_path(tcc(cjit), cjit->tmpdir);
#endif
	{ // search libs also in current dir
		char pwd[MAX_PATH];
		// Get the current working directory
		if(getcwd(pwd, MAX_PATH)) {
			add(libpaths,pwd);
		}
	}

	cjit_platform_setup_runtime(cjit);

	cjit->done_setup = true;
	return cjit_result_ok();
}

bool cjit_status(CJITState *cjit) {
	CJITResult result;
	_err("Build system: %s",PLATFORM);
	cjit_platform_print_status(cjit);
	////////////////////////
	// call cjit_setup here
	if (!cjit->done_setup) {
		result = cjit_prepare(cjit);
		if (!result.ok) {
			if (result.message) {
				_err("%s: %s", __func__, result.message);
			}
			return false;
		}
	}
	{
		size_t i;
		size_t used;
		used = string_list_count(cjit->sources);
		if(used) {
			_err("Sources (%u)",used);
			for(i=0;i<used;i++)
				_err("+ %s",string_list_get(cjit->sources,i));
		}
		used = string_list_count(cjit->libpaths);
		if(used) {
			_err("Library paths (%u)",used);
			for(i=0;i<used;i++) {
				char *d = string_list_get(cjit->libpaths,i);
				_err("+ %s",d?d:"(null)");
			}
		}
		used = string_list_count(cjit->libs);
		if(used) {
			_err("Libraries (%u)",used);
			for(i=0;i<used;i++)
				_err("+ %s",string_list_get(cjit->libs,i));
		}
		resolve_libraries(cjit);
		used = string_list_count(cjit->reallibs);
		if(used) {
			_err("Lib files (%u)",used);
			for(i=0;i<used;i++)
				_err("+ %s",string_list_get(cjit->reallibs,i));
		}
	}
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
		fail(filename);
		return -1;
	}
	struct stat st;
	if (fstat(fd, &st) == -1) {
		fail(filename);
		close(fd);
		return -1;
	}
	*filesize = st.st_size;
	res = read(fd,bom,3);
	close(fd);
	if (res!=3) {
		fail(filename);
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

CJITResult cjit_add_buffer_result(CJITState *cjit, const char *buffer) {
	int res;
	CJITResult result;
	result = cjit_prepare(cjit);
	if (!result.ok) {
		return result;
	}
	res = tcc_compile_string(tcc(cjit),buffer);
	debug("+B %p",buffer);
	if (res < 0) {
		return cjit_result_error(CJIT_RESULT_COMPILER_ERROR, 1,
					 "Code runtime error in stdin");
	}
	return cjit_result_ok();
}

bool cjit_add_buffer(CJITState *cjit, const char *buffer) {
	return cjit_add_buffer_result(cjit, buffer).ok;
}

CJITResult cjit_add_source_result(CJITState *cjit, const char *path) {
	CJITResult result;
	size_t length;
	int res = detect_bom(path,&length);
	result = cjit_prepare(cjit);
	if (!result.ok) {
		return result;
	}
	if(res<0) {
		fail(path);
		return cjit_result_error(CJIT_RESULT_IO_ERROR, 1,
					 "Error loading source input");
	} else if(res>0) {
		_err("UTF BOM detected in file: %s",path);
		_err("Encoding is not yet supported, execution aborted.");
		return cjit_result_error(CJIT_RESULT_INVALID_REQUEST, 1,
					 "Encoding is not yet supported, execution aborted.");
	}
	FILE *file = fopen(path, "rb");
	if (!file) {
		fail(path);
		return cjit_result_error(CJIT_RESULT_IO_ERROR, 1,
					 "Error loading source input");
	}
	char *spath = (char*)malloc((strlen(path)+16)*sizeof(char));
	if (!spath) {
		fail(path);
		fclose(file);
		return cjit_result_error(CJIT_RESULT_IO_ERROR, 1,
					 "Error loading source input");
	}
	sprintf(spath,"#line 1 \"%s\"\n",path);
	size_t spath_len = strlen(spath);
	char *contents =
		(char*)malloc
		((spath_len + length + 1)
		 * sizeof(char));
	if (!contents) {
		fail(path);
		free(spath);
		fclose(file);
		return cjit_result_error(CJIT_RESULT_IO_ERROR, 1,
					 "Error loading source input");
	}
	strcpy(contents,spath);
	free(spath);
	if(0== fread(contents+spath_len, 1, length, file)) {
		fail(file);
		fclose(file);
		free(contents);
		return cjit_result_error(CJIT_RESULT_IO_ERROR, 1,
					 "Error loading source input");
	}
	contents[length+spath_len] = 0x0;
	fclose(file);
	{ // if inside a dir then add dir to includes too
		size_t dirname;
		cwk_path_get_dirname(path,&dirname);
		if(dirname) {
			char *tmp = malloc(dirname+1);
			strncpy(tmp,path,dirname);
			tmp[dirname] = 0x0;
			tcc_add_include_path(tcc(cjit),tmp);
			free(tmp);
		}
	}
	res = tcc_compile_string(tcc(cjit),contents);
	free(contents);
	debug("+S %s",path);
	if (res < 0) {
		return cjit_result_error(CJIT_RESULT_COMPILER_ERROR, 1,
					 "Error loading source input");
	}
	return cjit_result_ok();
}

bool cjit_add_source(CJITState *cjit, const char *path) {
	return cjit_add_source_result(cjit, path).ok;
}

CJITResult cjit_add_file_result(CJITState *cjit, const char *path) {
	CJITResult result;
	int is_source = has_source_extension(path);
	result = cjit_prepare(cjit);
	if (!result.ok) {
		return result;
	}
	if(is_source == 0) { // no extension, we still add
		if(tcc_add_file(tcc(cjit), path)<0) {
			_err("%s: error: %s",__func__, path);
			return cjit_result_error(CJIT_RESULT_COMPILER_ERROR, 1,
						 "Error loading source input");
		}
		return cjit_result_ok();
	}
	if(is_source>0) {
		result = cjit_add_source_result(cjit, path);
		if(!result.ok) {
			_err("%s: error: %s",__func__, path);
			return result;
		}
		return cjit_result_ok();
	} else {
		if(tcc_add_file(tcc(cjit), path)<0) {
			_err("%s: error: %s",__func__, path);
			return cjit_result_error(CJIT_RESULT_COMPILER_ERROR, 1,
						 "Error loading source input");
		}
	}
	debug("+F %s",path);
	return cjit_result_ok();
}

bool cjit_add_file(CJITState *cjit, const char *path) {
	return cjit_add_file_result(cjit, path).ok;
}

CJITResult cjit_compile_file_result(CJITState *cjit, const char *path) {
	RuntimeSession session;
	CompilerPort compiler = tinycc_compiler_port;
	CJITResult result = cjit_result_ok();
	compiler.context = cjit;
	result = compiler.begin_session(compiler.context, &session);
	if (!result.ok) {
		return result;
	}
	result = compiler.compile_object(compiler.context, &session, path);
	compiler.end_session(compiler.context, &session);
	return result;
}

bool cjit_compile_file(CJITState *cjit, const char *path) {
	CJITResult result = cjit_compile_file_result(cjit, path);
	if (!result.ok && result.message) {
		_err("%s: %s", __func__, result.message);
	}
	return result.ok;
}

// link all setup and create an executable file
CJITResult cjit_link_result(CJITState *cjit) {
	RuntimeSession session;
	CompilerPort compiler = tinycc_compiler_port;
	CJITResult result = cjit_result_ok();
	compiler.context = cjit;
	result = compiler.begin_session(compiler.context, &session);
	if (!result.ok) {
		return result;
	}
	result = compiler.link_executable(compiler.context, &session);
	compiler.end_session(compiler.context, &session);
	return result;
}

int cjit_link(CJITState *cjit) {
	CJITResult result = cjit_link_result(cjit);
	if (!result.ok && result.message) {
		_err("%s: %s", __func__, result.message);
		return result.exit_status ? result.exit_status : 1;
	}
	return 0;
}

CJITResult cjit_exec_result(CJITState *cjit, int argc, char **argv, int *exit_status) {
	RuntimeSession session;
	CompilerPort compiler = tinycc_compiler_port;
	CJITResult result = cjit_result_ok();
	int local_exit_status = 1;
	compiler.context = cjit;
	result = compiler.begin_session(compiler.context, &session);
	if (!result.ok) {
		return result;
	}
	result = compiler.execute_program(compiler.context, &session, argc, argv, &local_exit_status);
	compiler.end_session(compiler.context, &session);
	if (exit_status) {
		*exit_status = local_exit_status;
	}
	return result;
}

int cjit_exec(CJITState *cjit, int argc, char **argv) {
	CJITResult result;
	int res = 1;
	result = cjit_exec_result(cjit, argc, argv, &res);
	if (!result.ok) {
		if (result.message && strcmp(result.message, "Entrypoint symbol not found") == 0) {
			_err("Symbol not found in source: %s", cjit->entry ? cjit->entry : "main");
		} else if (result.message && strcmp(result.message, "TCC linker error") == 0) {
			_err("%s: %s", __func__, result.message);
			_err("Library functions missing.");
		} else if (result.message) {
			_err("%s: %s", __func__, result.message);
		}
	}
	return res;
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
	if(cjit->verbose)_err("+D %s %s",sym,value?value:"");
}
void cjit_add_include_path(CJITState *cjit, const char *path) {
	char *restrict toadd = new_abspath(path);
	if(!toadd) {
		_err("%s: absolute path error: %s",__func__,path);
		return;
	}
	tcc_add_include_path(tcc(cjit), toadd);
	free(toadd);
	debug(" -I %s",path);
}
// TODO: temporary, to be reimplemented in linker.c
void cjit_add_library_path(CJITState *cjit, const char *path) {
	char *restrict toadd = new_abspath(path);
	if(!toadd) {
		_err("%s: absolute path error: %s",__func__,path);
		return;
	}
	cjit_platform_add_library_path(cjit, toadd);
	debug(" -L %s",toadd);
	free(toadd);
}
// TODO: temporary, to be reimplemented in linker.c
void cjit_add_library(CJITState *cjit, const char *path) {
	add(libs,path);
	debug(" +l %s",path);
}
void cjit_set_tcc_options(CJITState *cjit, const char *opts) {
	tcc_set_options(tcc(cjit),opts);
	debug(" +O %s",opts);
}

#define MAX_STRING 2040

// stdout message free from context
void _out(const char *fmt, ...) {
  char msg[MAX_STRING+4];
  va_list args;
  va_start(args, fmt);
  int len = vsnprintf(msg, MAX_STRING, fmt, args);
  va_end(args);
  msg[len] = '\n';
  msg[len+1] = 0x0; //safety
  if(write(fileno(stdout), msg, len+1) <0)
	  fail("_out()");
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
  if(write(fileno(stderr),msg,len+1) <0)
	fail("_err()");
}
