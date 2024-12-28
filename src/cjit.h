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
#include <libtcc.h>

// passed to cjit_exec with CJIT execution parameters
struct CJITState {
	TCCState *TCC; // the tinyCC context
	char *tmpdir; // path to execution temporary directory
	char *write_pid; // filename to write the pid of execution
	char *entry; // entry point, default "main" if left NULL
	bool live; // live coding mode
	bool quiet; // print less to stderr
	bool fresh; // tempdir is freshly created and needs to be populated
	int tcc_output; //
	// #define TCC_OUTPUT_MEMORY   1 /* output will be run in memory */
	// #define TCC_OUTPUT_EXE      2 /* executable file */
	// #define TCC_OUTPUT_DLL      4 /* dynamic library */
	// #define TCC_OUTPUT_OBJ      3 /* object file */
	// #define TCC_OUTPUT_PREPROCESS 5 /* only preprocess */
	char *output_filename; // output in case of compilation mode

};
typedef struct CJITState CJITState;

extern CJITState* cjit_new();

int cjit_compile_obj(CJITState *cjit, const char *_path);

extern int cjit_exec(CJITState *cjit, int argc, char **argv);

extern void cjit_free(CJITState *CJIT);

/////////////
// from embedded.c - generated at build time
extern bool extract_assets(CJITState *CJIT);

/////////////
// from file.c
extern int   detect_bom(const char *filename);
extern long  file_size(const char *filename);
extern char* file_load(const char *filename, unsigned int *len);
extern char *load_stdin();
extern char* dir_load(const char *path);
extern bool write_to_file(const char *path, const char *filename,
			  const char *buf, unsigned int len);

// terminal printing functions
extern void _out(const char *fmt, ...);
extern void _err(const char *fmt, ...);

/////////////
// from repl.c
extern int cjit_cli_tty(CJITState *cjit);
#ifdef KILO_SUPPORTED
extern int cjit_cli_kilo(CJITState *cjit);
#endif

#endif
