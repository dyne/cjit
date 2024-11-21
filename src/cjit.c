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
#include <errno.h>
#include <ctype.h>

#ifndef LIBC_MINGW32
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/poll.h>
#endif

#include <cflag.h>
#include <libtcc.h>
#include <unistd.h>

#include <ketopt.h>

/////////////
// from file.c
extern long  file_size(const char *filename);
extern char* file_load(const char *filename);
extern char* dir_load(const char *path);
extern bool rm_recursive(char *path);
#ifdef LIBC_MINGW32
extern char *win32_mkdtemp();
#endif
// from io.c
extern void _out(const char *fmt, ...);
extern void _err(const char *fmt, ...);
// from exec-headers.c
extern bool gen_exec_headers(char *tmpdir);
// from repl.c
#ifdef LIBC_MINGW32
extern int cjit_exec_win(TCCState *TCC, const char *ep, int argc, char **argv);
#else
extern int cjit_exec_fork(TCCState *TCC, const char *ep, int argc, char **argv);
#endif
extern int cjit_cli_tty(TCCState *TCC);
#ifdef REPL_SUPPORTED
extern int cjit_compile_and_run(TCCState *TCC, const char *code, int argc, char **argv, int rn, char **err_msg);
extern int cjit_cli_kilo(TCCState *TCC);
#endif
/////////////

void handle_error(void *n, const char *m) {
  (void)n;
  _err("%s",m);
}

#define MAX_ARG_STRING 1024
int parse_value(char *str) {
  int i = 0;
  int value_pos = 0;
  bool equal_found = false;
  while (str[i] != '\0') {
    if (equal_found && str[i] == '=') {
      return -1; // can't include equal twice
    }
    if (str[i] == '=') {
      str[i]=0x0;
      value_pos = i + 1;
      equal_found = true;
      continue;
    }
    if (!isalnum(str[i]) && str[i] != '_') {
      return -1; // Invalid character found
    }
    i++;
    if(i>MAX_ARG_STRING) {
      return -1; // string too long
    }
  }
  if(equal_found)
    return(value_pos);
  else return(0);
}

int main(int argc, char **argv) {
  TCCState *TCC = NULL;
  // const char *progname = "cjit";
  char tmptemplate[] = "/tmp/CJIT-exec.XXXXXX";
  char *tmpdir = NULL;
  int res = 1;

  _err("CJIT %s by Dyne.org",VERSION);

  TCC = tcc_new();
  if (!TCC) {
    _err("Could not initialize tcc");
    exit(1);
  }

  // get the extra cflags from the CFLAGS env variable
  // they are overridden by explicit command-line options
  if(getenv("CFLAGS")) {
    char *extra_cflags = NULL;
    extra_cflags = getenv("CFLAGS");
    _err("CFLAGS: %s",extra_cflags);
    tcc_set_options(TCC, extra_cflags);
  }

  ketopt_t opt = KETOPT_INIT;
  int i, c;
  while ((c = ketopt(&opt, argc, argv, 1, "vD:L:l:C:I:", NULL)) >= 0) {
    if (c == 'v') {
      _err("Running version: %s\n",VERSION);
      tcc_delete(TCC);
      exit(0); // print version and exit
    } else if (c == 'D') { // define
      int _res;
      _res = parse_value(opt.arg);
      if(_res==0) { // -Dsym (no key=value)
        tcc_define_symbol(TCC, opt.arg, NULL);
      } else if(_res>0) { // -Dkey=value
        tcc_define_symbol(TCC, opt.arg, &opt.arg[_res]);
      } else { // invalid char
        _err("Invalid char used in -D define symbol: %s", opt.arg);
        tcc_delete(TCC);
        exit(1);
      }
    } else if (c == 'L') { // library path
      _err("lib path: %s",opt.arg);
      tcc_add_library_path(TCC,tmpdir);
    } else if (c == 'l') { // library link
      _err("lib: %s",opt.arg);
      tcc_add_library(TCC, opt.arg);
    } else if (c == 'C') { // library link
      _err("cflags: %s",opt.arg);
      tcc_set_options(TCC, opt.arg);
    } else if (c == 'I') { // library link
      tcc_add_include_path(TCC,opt.arg);
    }
    else if (c == '?') _err("unknown opt: -%c\n", opt.opt? opt.opt : ':');
    else if (c == ':') _err("missing arg: -%c\n", opt.opt? opt.opt : ':');
  }

  // initialize the tmpdir for execution
#ifndef LIBC_MINGW32
  tmpdir = mkdtemp(tmptemplate);
#else
  tmpdir = win32_mkdtemp();
#endif
  if(!tmpdir) {
    _err("Error creating temp dir %s: %s",tmptemplate,strerror(errno));
    goto endgame;
  }
  tcc_set_lib_path(TCC,tmpdir);
  tcc_add_library_path(TCC,tmpdir);
  tcc_add_include_path(TCC,tmpdir);

  if( !gen_exec_headers(tmpdir) ) goto endgame;

  // set output in memory for just in time execution
  tcc_set_output_type(TCC, TCC_OUTPUT_MEMORY);

#if defined(LIBC_MUSL)
  tcc_add_libc_symbols(TCC);
#endif

//   if (argc == 0) {
//       printf("No input file: running in interactive mode\n");
// #ifdef REPL_SUPPORTED
//       res = cjit_cli_kilo(TCC);
// #else
//       res = cjit_cli_tty(TCC);
// #endif
//       goto endgame;
//   }
//   _err("Source: %s",code_path);


  _err("Source code:");
  for (i = opt.ind; i < argc; ++i) {
    const char *code_path = argv[i];
    // TODO: load entire directory
// #ifndef LIBC_MINGW32 // POSIX only
//     struct stat st;
//     if (stat(code_path, &st) == -1) {
//       _err("File not found: %s",code_path);
//       goto endgame;
//     }
//     if (S_ISDIR(st.st_mode)) {
//       _err("+ %s (dir)", code_path);
//       tcc_add_include_path(TCC, code_path);
//       code = dir_load(code_path);
//   } else {
//     code = file_load(code_path);
//   }
// #else
//   /* TODO: Add Windows support for directory */
//   code = file_load(code_path);
// #endif
    _err("+ %s",code_path);
    // TODO: check if file exists
    tcc_add_file(TCC, code_path);
  }

// #ifdef REPL_SUPPORTED
//   _err("Start execution\n---------------");
//   res = cjit_compile_and_run(TCC, code, argc, argv, 1, &err_msg);
//   if (err_msg) {
//         _err(err_msg);
//   }
// #else
  // error handler callback for TCC
  tcc_set_error_func(TCC, stderr, handle_error);
  // if (tcc_compile_string(TCC, code) == -1) return 1;
  // free(code); // safe: bytecode compiled is in TCC now
  // code = NULL;
  // _err("Compilation successful");

  // relocate the code
  if (tcc_relocate(TCC) < 0) {
    _err("TCC relocation error");
    goto endgame;
  }

#ifndef LIBC_MINGW32
  res = cjit_exec_fork(TCC, "main", argc, argv);
#else
  res = cjit_exec_win(TCC, "main", argc, argv);
#endif
// #endif // REPL_SUPPORTED

  // if( tcc_run(TCC,argc,argv) == -1) res=1; else res=0;

  endgame:
  // free TCC
  if(TCC) tcc_delete(TCC);
  if(tmpdir) {
    rm_recursive(tmpdir);
  }
  exit(res);
}
