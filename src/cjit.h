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
	char *tmpdir;
	char *write_pid; // filename to write the pid of execution
	bool dmon;
	TCCState *TCC;
};
typedef struct CJITState CJITState;

// from embedded.c - generated at build time
extern bool extract_embeddings(CJITState *CJIT);

// implemented in repl.c
extern int cjit_exec(TCCState *TCC, CJITState *CJIT,
		     const char *ep, int argc, char **argv);

extern bool free_cjit(CJITState *CJIT);

/////////////
// from file.c
extern int   detect_bom(const char *filename);
extern long  file_size(const char *filename);
extern char* file_load(const char *filename);
extern char *load_stdin();
extern char* dir_load(const char *path);
extern bool write_to_file(const char *path, const char *filename,
			  const char *buf, unsigned int len);
// from io.c
extern void _out(const char *fmt, ...);
extern void _err(const char *fmt, ...);

extern int cjit_cli_tty(TCCState *TCC);
#ifdef KILO_SUPPORTED
extern int cjit_cli_kilo(TCCState *TCC);
#endif

#endif
