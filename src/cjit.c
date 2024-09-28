/* CJIT https://dyne.org/cjit
 *
 * Copyright (C) 2024 Dyne.org foundation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <cflag.h>
#include <libtcc.h>

// from file.c
extern long  file_size(const char *filename);
extern char* file_load(const char *filename);
// from io.c
extern void _out(const char *fmt, ...);
extern void _err(const char *fmt, ...);
// TCC callback
void handle_error(void *n, const char *m) {
  (void)n;
  _err("%s",m);
}

int main(int argc, char **argv) {
  TCCState *TCC;
  const char *syntax = "[options] code.c";
  const char *include_path = 0x0;
  const char *libs_path = 0x0;
  const char *progname = "cjit";
  static bool verbose = false;
  static bool version = false;
  int res = 1;

  static const struct cflag options[] = {
    CFLAG(bool, "verbose", 'v', &verbose, "Verbosely show progress"),
    CFLAG(bool, "version", 'V', &version, "Show build version"),
    CFLAG_HELP,
    CFLAG_END
  };
  cflag_apply(options, syntax, &argc, &argv);
  if(!argv[0]) {
    cflag_usage(options, progname, syntax, stderr);
    exit(1);
  }
  const char *code_path = argv[0];
  _err("cjit to execute code: %s",code_path);
  TCC = tcc_new();
  if (!TCC) {
    _err("Could not initialize tcc");
    exit(1);
  }
  // error handler callback for TCC
  tcc_set_error_func(TCC, stderr, handle_error);
  //// TCC DEFAULT PATHS
  tcc_set_lib_path(TCC,"lib/tinycc"); // inside zenroom source
  tcc_add_library_path(TCC,"/lib/x86_64-linux-musl"); // devuan default
  tcc_add_include_path(TCC,"/usr/include/x86_64-linux-musl"); // devuan
  tcc_add_include_path(TCC,"src"); // devuan
  if(include_path) {
    _err("Path to headers included: %s",include_path);
    tcc_add_include_path(TCC,include_path);
  }
  if(libs_path) {
    _err("Path to libraries linked: %s",libs_path);
    tcc_add_library_path(TCC,libs_path);
  }
  // set output in memory for just in time execution
  tcc_set_output_type(TCC, TCC_OUTPUT_MEMORY);
  char *code = file_load(code_path);
  if(!code) {
    _err("File not found: %s",code_path);
    goto endgame;
  }
  if (tcc_compile_string(TCC, code) == -1) return 1;
  free(code); // safe: bytecode compiled is in TCC now
  _err("Source compiled successfully");

  // simple temporary exports for hello world
  tcc_add_symbol(TCC, "exit", &exit);
  tcc_add_symbol(TCC, "stdout", &stdout);
  tcc_add_symbol(TCC, "stderr", &stderr);
  tcc_add_symbol(TCC, "fprintf", &fprintf);

  // relocate the code
  if (tcc_relocate(TCC) < 0) {
    _err("TCC relocation error");
    goto endgame;
  }

  // get entry symbol
  int (*_main)(int, char**);
  _main = tcc_get_symbol(TCC, "main");
  if (!_main) {
    _err("Symbol not found in source: %s","main");
    goto endgame;
  }

  // run the code
  res = _main(argc, argv);

  endgame:
  // free TCC
  tcc_delete(TCC);

  exit(res);
}
