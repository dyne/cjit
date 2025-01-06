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

#ifndef _CJIT_H_
#define _CJIT_H_

#include <platforms.h>
#include <stdbool.h>

#if !defined(PATH_MAX)
#define PATH_MAX 1024
#endif
#if !defined(MAX_PATH)
#define MAX_PATH PATH_MAX
#endif

// passed to cjit_exec with CJIT execution parameters
struct CJITState {
	void *TCC; // the tinyCC context
	char *tmpdir; // path to execution temporary directory
	char *write_pid; // filename to write the pid of execution
	char *entry; // entry point, default "main" if left NULL
	bool live; // live coding mode
	bool quiet; // print less to stderr
	bool verbose; // print more to stderr
	bool fresh; // tempdir is freshly created and needs to be populated
	int tcc_output; //
	// #define TCC_OUTPUT_MEMORY   1 /* output will be run in memory */
	// #define TCC_OUTPUT_EXE      2 /* executable file */
	// #define TCC_OUTPUT_DLL      4 /* dynamic library */
	// #define TCC_OUTPUT_OBJ      3 /* object file */
	// #define TCC_OUTPUT_PREPROCESS 5 /* only preprocess */
	char *output_filename; // output in case of compilation mode
	bool done_setup;
	bool done_exec;
	// INTERNAL
	// sources and libs used and paths to libs
	void *sources;
	void *libs;
	void *libpaths;
	void *reallibs;
};
typedef struct CJITState CJITState;

extern CJITState* cjit_new();
// extern bool cjit_setup(CJITState *cjit);
extern bool cjit_status(CJITState *cjit);

// setup functions to add source and libs
extern bool cjit_add_file(CJITState *cjit, const char *path);
extern bool cjit_add_source(CJITState *cjit, const char *path);
extern bool cjit_add_buffer(CJITState *cjit, const char *buffer);

// end game functions
extern int cjit_link(CJITState *cjit); // link and create an executable file
extern int cjit_exec(CJITState *cjit, int argc, char **argv); // exec in mem
extern bool cjit_compile_file(CJITState *cjit, const char *_path); // compile a single file to a bytecode object

extern void cjit_free(CJITState *CJIT);


#define MEM 1 /* output will be run in memory */
#define EXE 2 /* executable file */
#define OBJ 3 /* object file */
#define DLL 4 /* dynamic library */
#define PRE 5 /* only preprocess */
void cjit_set_output(CJITState *cjit, int output);

void cjit_define_symbol(CJITState *cjit, const char *sym, const char *value);
void cjit_add_include_path(CJITState *cjit, const char *path);
void cjit_add_library_path(CJITState *cjit, const char *path);
void cjit_add_library(CJITState *cjit, const char *path);
void cjit_set_tcc_options(CJITState *cjit, const char *opts);

/////////////
// from embedded.c - generated at build time
extern bool extract_assets(CJITState *CJIT, const char *optional_path);
extern bool cjit_mkdtemp(CJITState *cjit, const char *optional_path);
/////////////
// from file.c
extern char* file_load(const char *filename, unsigned int *len);

// terminal printing functions
extern void _out(const char *fmt, ...);
extern void _err(const char *fmt, ...);

/////////////
// from repl.c
extern int cjit_cli_tty(CJITState *cjit);

#endif
