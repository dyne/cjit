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

#ifdef SELFHOST
extern const char *cjit_source;
extern const unsigned int cjit_source_len;
#endif

extern char *load_stdin();

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
	"CJIT %s by Dyne.org\n"
	"\n"
	"Synopsis: cjit [options] files(*) -- app arguments\n"
	"  (*) can be any source (.c) or built object (dll, dylib, .so)\n"
	"Options and values (+) mandatory (-) default (=) optional:\n"
	" -h \t print this help\n"
	" -v \t print version information\n"
	" -q \t stay quiet and only print errors and output\n"
	" -C \t set interpreter/compiler flags (-) env var CFLAGS\n"
	" -c \t compile a single source file, do not execute\n"
	" -o exe\t do not run, compile an executable (+path)\n"
	" -D sym\t define a macro value (+) symbol or key=value\n"
	" -I dir\t also search folder (+) dir for header files\n"
	" -l lib\t link the shared library (+) lib\n"
	" -L dir\t add folder (+) dir to library search paths\n"
	" -e fun\t run starting from entry function (-) main\n"
	" -p pid\t write execution process ID to (+) pid\n"
	" --verb\t don't go quiet, verbose logs\n"
#if !defined(SHAREDTCC)
	" --xass\t just extract runtime assets (=) to path\n"
#endif
#if defined(SELFHOST)
	" --src\t  extract source code to cjit_source\n"
#endif
	" --xtgz\t extract all files in a USTAR (+) tar.gz\n";

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
	  { "verb", ko_no_argument, 101 },
#if defined(SELFHOST)
	  { "src",  ko_no_argument, 311 },
#endif
#if !defined(SHAREDTCC)
	  { "xass", ko_optional_argument, 401 },
#endif
	  { "xtgz", ko_required_argument, 501 },
	  { NULL, 0, 0 }
  };
  ketopt_t opt = KETOPT_INIT;
  // tolerated and ignored: -f -W -O -g -U -E -S -M
  while ((c = ketopt(&opt, argc, argv, 1, "qhvD:L:l:C:I:e:p:co:f:W:O:gU:ESM:m:a:", longopts)) >= 0) {
	  if(c == 'q') {
		  if(!CJIT->verbose)
			  CJIT->quiet = true;
	  } else if (c == 'v') {
		  cjit_status(CJIT);
		  cjit_free(CJIT);
		  exit(0); // print and exit
	  } else if (c=='h' || c==100) { // help
		  _err(cli_help,VERSION);
		  cjit_free(CJIT);
		  exit(0); // print and exit
	  } else if (c==101 ) { // verb
		  CJIT->quiet = false;
		  CJIT->verbose = true;
	  } else if (c == 'D') { // define
		  int _res;
		  _res = parse_value(opt.arg);
		  if(_res==0) { // -Dsym (no key=value)
			  cjit_define_symbol(CJIT, opt.arg, NULL);
		  } else if(_res>0) { // -Dkey=value
			  cjit_define_symbol(CJIT, opt.arg, &opt.arg[_res]);
		  } else { // invalid char
			  _err("Invalid char used in -D define symbol: %s", opt.arg);
			  cjit_free(CJIT);
			  exit(1);
		  }
	  } else if (c == 'c') { // don't link or execute, just compile to .o
		  cjit_set_output(CJIT, OBJ);
		  CJIT->quiet = true;
		  CJIT->output_obj = true;
	  } else if (c == 'o') { // override output filename
		  if(CJIT->output_filename) free(CJIT->output_filename);
		  CJIT->output_filename = malloc(strlen(opt.arg)+1);
		  strcpy(CJIT->output_filename,opt.arg);
		  if(!CJIT->output_obj) // don't overwrite output_obj mode
			  cjit_set_output(CJIT, EXE);
	  } else if (c == 'L') { // library path
		  if(CJIT->verbose)_err("arg lib path: %s",opt.arg);
		  cjit_add_library_path(CJIT, opt.arg);
	  } else if (c == 'l') { // library link
		  if(CJIT->verbose)_err("arg lib: %s",opt.arg);
		  cjit_add_library(CJIT, opt.arg);
	  } else if (c == 'C') { // cflags compiler options
		  if(CJIT->verbose)_err("arg cflags: %s",opt.arg);
		  cjit_set_tcc_options(CJIT->TCC, opt.arg);
	  } else if (c == 'I') { // include paths in cflags
		  if(CJIT->verbose)_err("arg inc: %s",opt.arg);
		  cjit_add_include_path(CJIT, opt.arg);
	  } else if (c == 'e') { // entry point (default main)
		  if(!CJIT->quiet)_err("entry function: %s",opt.arg);
		  if(CJIT->entry) free(CJIT->entry);
		  CJIT->entry = malloc(strlen(opt.arg)+1);
		  strcpy(CJIT->entry,opt.arg);
	  } else if (c == 'p') { // write pid to file
		  if(strcmp(opt.arg,"edantic")!=0) { // ignore -pedantic
			  if(!CJIT->quiet)_err("pid file: %s",opt.arg);
			  if(CJIT->write_pid) free(CJIT->write_pid);
			  CJIT->write_pid = malloc(strlen(opt.arg)+1);
			  strcpy(CJIT->write_pid,opt.arg);
		  }
	  } else if (c =='a') { // emulate -ar
		  if(*opt.arg=='r') {
			  CJIT->call_ar = true;
			  _err("gcc emulation: call ar");
		  }
#if defined(SELFHOST)
	  } else if (c == 311) { // --src
		  char cwd[PATH_MAX];
		  getcwd(cwd, sizeof(cwd));
		  _err("Extracting CJIT's own source to %s/cjit_source",cwd);
		  muntargz_to_path(cwd,(char*)&cjit_source,cjit_source_len);
		  cjit_free(CJIT);
#if defined(POSIX)
		  // restore executable bit on test suite, fixes make check
		  chmod("cjit_source/test/bats/bin/bats", 0755);
		  chmod("cjit_source/test/bats/libexec/bats-core/bats", 0755);
		  chmod("cjit_source/test/bats/libexec/bats-core/bats-exec-suite", 0755);
		  chmod("cjit_source/test/bats/libexec/bats-core/bats-format-pretty", 0755);
		  chmod("cjit_source/test/bats/libexec/bats-core/bats-exec-suite", 0755);
		  chmod("cjit_source/test/bats/libexec/bats-core/bats-gather-tests", 0755);
		  chmod("cjit_source/test/bats/libexec/bats-core/bats-preprocess", 0755);
		  chmod("cjit_source/test/bats/libexec/bats-core/bats-exec-file", 0755);
		  chmod("cjit_source/test/bats/libexec/bats-core/bats-exec-test", 0755);
#endif
		  exit(0);
#endif
#if !defined(SHAREDTCC)
	  } else if (c == 401) { // --xass
		  if(opt.arg) {
			  _err("Extracting runtime assets to:",opt.arg);
			  extract_assets(CJIT,opt.arg);
		  } else {
			  extract_assets(CJIT,NULL);
		  }
		  _out(CJIT->tmpdir);
		  cjit_free(CJIT);
		  exit(0);
#endif
	  } else if (c == 501) { // --xtgz
		  cjit_free(CJIT);
		  unsigned int len = 0;
		  _err("Extract contents of: %s",opt.arg);
		  const uint8_t *targz = (const uint8_t*)
			  file_load(opt.arg, &len);
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

#if 0
  // If no arguments then start the REPL
  if (argc == 0 ) {
    _err("No input file: interactive mode");
    CJIT->live = true;
  }
  if(CJIT->live) {
    if (!isatty(fileno(stdin))) {
      _err("Live mode only available in terminal (tty not found)");
      goto endgame;
    }
    cjit_setup(CJIT);
    res = cjit_cli_tty(CJIT);
    goto endgame;
  }
  // end of REPL
  /////////////////////////////////////
#endif

  // number of args at the left hand of arg separator, or all of them
  int left_args = arg_separator? arg_separator: argc;

  char *stdin_code = NULL;
  if(opt.ind >= argc) {
#if defined(_WIN32)
	  _err("No files specified on commandline");
	  goto endgame;
#endif
	  ////////////////////////////
	  // Processs code from STDIN
	  if(!CJIT->quiet)_err("No files specified on commandline, reading code from stdin");
	  stdin_code = load_stdin(); // allocated returned buffer, needs free
	  if(!stdin_code) {
		  _err("Error reading from standard input");
		  goto endgame;
	  }
	  if(!cjit_add_buffer(CJIT,stdin_code)) {
		  _err("Code runtime error in stdin");
		  free(stdin_code);
		  goto endgame;
	  }
	  free(stdin_code);
	  // end of STDIN
	  ////////////////

  } else if(CJIT->tcc_output==OBJ) {
	  /////////////////////////////
	  // Compile one .c file to .o
	  if(left_args - opt.ind != 1) {
		  _err("Compiling to object files supports only one file argument");
		  goto endgame;
	  }
	  //if(!CJIT->quiet)_err("Compile: %s",argv[opt.ind]);
	  res = cjit_compile_file(CJIT, argv[opt.ind]) ?0:1; // 0 on success
	  // if(CJIT->output_filename) {
		  // TODO: output to explicitly configured filename

	  goto endgame;
	  ////////////////////////////

  } else if(CJIT->call_ar) {
	  if(opt.ind >= argc) {
		  _err("No files specified on commandline");
		  goto endgame;
	  }
	  // TODO: opt.ind-1 here should be "/bin/ar" or whatever being called
	  // also note that there is no path resolution
	  // and also need to support env AR for manual configuration
	  res = execve("/bin/ar", &argv[opt.ind-1], NULL);
	  if(res<0)_err("Error in ar subcall: %s",strerror(errno));
	  goto endgame;
  } else if(opt.ind < left_args) {
	  // process files on commandline before separator
	  if(CJIT->verbose)_err("Source code:");
	  for (i = opt.ind; i < left_args; ++i) {
		  const char *code_path = argv[i];
		  if(CJIT->verbose)_err("%c %s",(*code_path=='-'?'|':'+'),
				       (*code_path=='-'?"standard input":code_path));
		  if(*code_path=='-') { // stdin explicit
#if defined(_WIN32)
			  _err("Code from standard input not supported on Windows");
			  goto endgame;
#endif
			  stdin_code = load_stdin(); // allocated returned buffer, needs free
			  if(!stdin_code) {
				  _err("Error reading from standard input");
				  goto endgame;
			  }
			  if(!cjit_add_buffer(CJIT,stdin_code)) {
				  _err("Code runtime error in stdin");
				  free(stdin_code);
				  goto endgame;
			  } else free(stdin_code);
		  } else { // load any file path
			  cjit_add_file(CJIT, code_path);
		  }
	  }
  }

  /////////////////////////
  // compile to executable
  if(CJIT->output_filename) {
	  _err("Create executable: %s", CJIT->output_filename);
	  if(cjit_link(CJIT)<0) {
		  _err("Error in linker compiling to file: %s",
		       CJIT->output_filename);
		  res = 1;
	  } else res = 0;
  } else {
	  // number of args at the left hand of arg separator, or all
	  // of them
	  int right_args = argc-left_args+1;//arg_separator? argc-arg_separator : 0;
	  char **right_argv = &argv[left_args-1];//arg_separator?&argv[arg_separator]:0
	  res = cjit_exec(CJIT, right_args, right_argv);
  }
  endgame:
  // free TCC
  cjit_free(CJIT);
  exit(res);
}
