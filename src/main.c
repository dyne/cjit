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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>

#include <ketopt.h>
#include <muntar.h>

#define MAX_ARG_STRING 1024
static int parse_value(char *str) {
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

const char cli_help[] =
  "\n"
  "Synopsis: cjit [options] files(*) -- app arguments\n"
  "  (*) can be any source (.c) or built object (dll, dylib, .so)\n"
  "Options:\n"
  " -h \t print this help\n"
  " -v \t print version information\n"
  " -q \t stay quiet and only print errors and output\n"
  " -D sym\t define a macro symbol or key=value\n"
  " -C \t set compiler flags (default from env var CFLAGS)\n"
  " -I dir\t also search folder 'dir' for header files\n"
  " -l lib\t search the library named 'lib' when linking\n"
  " -L dir\t also search inside folder 'dir' for -l libs\n"
  " -e fun\t entry point function (default 'main')\n"
  " -p pid\t write pid of executed program to file\n"
  " --live\t run interactive editor for live coding\n"
  " --temp\t create the runtime temporary dir and exit\n"
  " --utar\t extract all contents from a USTAR tar.gz\n";

int main(int argc, char **argv) {
  CJITState *CJIT = cjit_new();
  if(!CJIT) exit(1);

  int arg_separator = 0;
  int res = 1;
  int i, c;
  // get the extra cflags from the CFLAGS env variable
  // they are overridden by explicit command-line options
  static ko_longopt_t longopts[] = {
	  { "help", ko_no_argument, 100 },
	  { "live", ko_no_argument, 301 },
	  { "temp", ko_no_argument, 401 },
	  { "utar", ko_required_argument, 501 },
	  { NULL, 0, 0 }
  };
  ketopt_t opt = KETOPT_INIT;
  while ((c = ketopt(&opt, argc, argv, 1, "qhvD:L:l:C:I:e:p:", longopts)) >= 0) {
	  if(c == 'q') {
		  CJIT->quiet = true;
	  }
	  if (c == 'v') {
		  _err("CJIT %s by Dyne.org",VERSION);
		  // _err("Running version: %s\n",VERSION);
		  // version is always shown
		  _err("Build: %s",PLATFORM);
#if   defined(CJIT_DEBUG_ASAN)
		  _err("Debug: ASAN");
#elif defined(CJIT_DEBUG_GDB)
		  _err("Debug: GDB");
#else
		  _err("Debug: NONE");
#endif
		  cjit_free(CJIT);
		  exit(0); // print and exit
	  } else if (c=='h' || c==100) { // help
		  _err(cli_help,VERSION);
		  cjit_free(CJIT);
		  exit(0); // print and exit
	  } else if (c == 'D') { // define
		  int _res;
		  _res = parse_value(opt.arg);
		  if(_res==0) { // -Dsym (no key=value)
			  tcc_define_symbol(CJIT->TCC, opt.arg, NULL);
		  } else if(_res>0) { // -Dkey=value
			  tcc_define_symbol(CJIT->TCC, opt.arg, &opt.arg[_res]);
		  } else { // invalid char
			  _err("Invalid char used in -D define symbol: %s", opt.arg);
			  cjit_free(CJIT);
			  exit(1);
		  }
	  } else if (c == 'L') { // library path
		  if(!CJIT->quiet)_err("lib path: %s",opt.arg);
		  tcc_add_library_path(CJIT->TCC, opt.arg);
	  } else if (c == 'l') { // library link
		  if(!CJIT->quiet)_err("lib: %s",opt.arg);
		  tcc_add_library(CJIT->TCC, opt.arg);
	  } else if (c == 'C') { // cflags compiler options
		  if(!CJIT->quiet)_err("cflags: %s",opt.arg);
		  tcc_set_options(CJIT->TCC, opt.arg);
	  } else if (c == 'I') { // include paths in cflags
		  if(!CJIT->quiet)_err("inc: %s",opt.arg);
		  tcc_add_include_path(CJIT->TCC, opt.arg);
	  } else if (c == 'e') { // entry point (default main)
		  if(!CJIT->quiet)_err("entry: %s",opt.arg);
		  if(CJIT->entry) free(CJIT->entry);
		  CJIT->entry = malloc(strlen(opt.arg)+1);
		  strcpy(CJIT->entry,opt.arg);
	  } else if (c == 'p') { // write pid to file
		  if(!CJIT->quiet)_err("pid file: %s",opt.arg);
		  if(CJIT->write_pid) free(CJIT->write_pid);
		  CJIT->write_pid = malloc(strlen(opt.arg)+1);
		  strcpy(CJIT->write_pid,opt.arg);
	  } else if (c == 301) {
#if defined(_WIN32)
		  _err("Live mode not supported in Windows");
#else
		  if(!CJIT->quiet)_err("Live mode activated");
		  CJIT->live = true;
#endif
	  } else if (c == 401) { // --temp
		  fprintf(stdout,"%s\n",CJIT->tmpdir);
		  cjit_free(CJIT);
		  exit(0);
	  } else if (c == 501) { // --utar
		  cjit_free(CJIT);
		  unsigned int len = 0;
		  _err("Extract contents of: %s",opt.arg);
		  char *targz = file_load(opt.arg, &len);
		  if(!targz) exit(1);
		  if(!len) exit(1);
		  muntargz_to_path(".",targz,len);
		  exit(0);
	  }
	  else if (c == '?') _err("unknown opt: -%c\n", opt.opt? opt.opt : ':');
	  else if (c == ':') _err("missing arg: -%c\n", opt.opt? opt.opt : ':');
	  else if (c == '-') { // -- separator
		  arg_separator = opt.ind+1; break;
	  }
  }
  if(!CJIT->quiet) _err("CJIT %s by Dyne.org",VERSION);

  if (argc == 0 ) {
    _err("No input file: live mode!");
    CJIT->live = true;
  }
  if(CJIT->live) {
    if (!isatty(fileno(stdin))) {
      _err("Live mode only available in terminal (tty not found)");
      goto endgame;
    }
#ifdef KILO_SUPPORTED
    res = cjit_cli_kilo(CJIT);
#else
    res = cjit_cli_tty(CJIT);
#endif
    goto endgame;
  }

  // number of args at the left hand of arg separator, or all of them
  int left_args = arg_separator? arg_separator: argc;

  char *stdin_code = NULL;
  if(opt.ind >= argc) {
#if defined(_WIN32)
    _err("No files specified on commandline");
    goto endgame;
#endif
    if(!CJIT->quiet)_err("No files specified on commandline, reading code from stdin");
    stdin_code = load_stdin(); // allocated returned buffer, needs free
    if(!stdin_code) {
      _err("Error reading from standard input");
      goto endgame;
    }
    if( tcc_compile_string(CJIT->TCC,stdin_code) < 0) {
      _err("Code runtime error in stdin");
      free(stdin_code);
      goto endgame;
    }
  } else if(opt.ind < left_args) {
    // process files on commandline before separator
    if(!CJIT->quiet)_err("Source code:");
    for (i = opt.ind; i < left_args; ++i) {
      const char *code_path = argv[i];
      if(!CJIT->quiet)_err("%c %s",(*code_path=='-'?'|':'+'),
           (*code_path=='-'?"standard input":code_path));
      if(*code_path=='-') { // stdin explicit
#if defined(_WIN32)
        _err("Code from standard input not supported on Windows");
        goto endgame;
#endif
        stdin_code = load_stdin(); // allocated returned buffer, needs free
        if(!stdin_code) {
          _err("Error reading from standard input");
        } else if( tcc_compile_string(CJIT->TCC,stdin_code) < 0) {
          _err("Code runtime error in stdin");
          free(stdin_code);
          goto endgame;
        } else free(stdin_code);
      } else { // load any file path
	int res = detect_bom(code_path);
	// returned values:
	// 0  : no BOM, all OK
	// <0 : file not found
	// 1  : BOM found, UTF16-LE
	// 2  : BOM found, UTF16-BE
	// 3  : BOM found, UTF8
	if(res ==0) {
	  tcc_add_file(CJIT->TCC, code_path);
	} else if(res<0) {
		_err("Cannot open file: %s",code_path);
		_err("Execution aborted.");
		goto endgame;
	} else {
	  _err("UTF BOM detected in file: %s",code_path);
	  _err("Encoding is not yet supported, execution aborted.");
	  goto endgame;
	}
      }
    }
  }

  // number of args at the left hand of arg separator, or all of them
  int right_args = argc-left_args+1;//arg_separator? argc-arg_separator : 0;
  char **right_argv = &argv[left_args-1];//arg_separator?&argv[arg_separator]:0
  res = cjit_exec(CJIT, right_args, right_argv);
  endgame:
  // free TCC
  cjit_free(CJIT);
  exit(res);
}
