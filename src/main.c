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
#include <muntarfs.h>
#include <app/execute_source.h>
#include <app/compile_object.h>
#include <app/build_executable.h>
#include <app/print_status.h>
#include <app/extract_assets.h>
#include <app/extract_archive.h>
#include <adapters/cli/route_parser.h>
#include <adapters/cli/render_response.h>

#ifdef SELFHOST
extern const char *cjit_source;
extern const unsigned int cjit_source_len;
#endif

extern char *load_stdin();

#ifndef CJIT_WITHOUT_AR
extern int cjit_ar(CJITState *CJIT, int argc, char **argv);
#endif

// defined below
static char** remove_args(int* argc,  char** argv,
						  const char** to_remove, int remove_count);
static int handle_archive_mode(CJITState *cjit, int argc, char **argv);
static int handle_conftest_mode(CJITState *cjit, const char *source_path);

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
	" -v \t print runtime and compiler status information\n"
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

const char *ignored_args[] = {
	"-s",
	"-static-libgcc",
	"-shared",
	"-O",
	"-O:",
	"-f:",
	"-W:",
	"-M:",
	"-g:"
};

int main(int argc, char **argv) {
  CJITState *CJIT = cjit_new();
  if(!CJIT) exit(1);

  int arg_separator = 0;
  int res = 1;
  int i, c;
  CliRoute forced_route = CLI_ROUTE_NONE;
  const char *forced_route_path = NULL;

#ifndef CJIT_WITHOUT_AR
  if(argv[1] && strlen(argv[1])==3 && strcmp(argv[1],"-ar")==0) {
	  int res = handle_archive_mode(CJIT, argc, argv);
	  cjit_free(CJIT);
	  exit(res);
  }
#endif

  if(argv[1] && strlen(argv[1])==10 && strcmp(argv[1],"conftest.c")==0) {
	  int res = handle_conftest_mode(CJIT, argv[1]);
	  cjit_free(CJIT);
	  exit(res);
  }

  // clean up argv from ignored args and update argc
  int ignored_count = sizeof(ignored_args) / sizeof(ignored_args[0]);
  char** clean_argv = remove_args(&argc, argv, ignored_args, ignored_count);

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
  while ((c = ketopt(&opt, argc, clean_argv, 1, "qhvD:L:l:C:I:e:p:co:ga:", longopts)) >= 0) {
	  if(c == 'q') {
		  if(!CJIT->verbose)
			  CJIT->quiet = true;
	  } else if (c == 'v') {
		  CJIT->print_status = true;
	  } else if (c=='h' || c==100) { // help
		  _err(cli_help,VERSION);
		  res = 0;
		  goto endgame;
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
			  res = 1;
			  goto endgame;
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
		  cjit_set_tcc_options(CJIT, opt.arg);
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
#if defined(SELFHOST)
	  } else if (c == 311) { // --src
		  char cwd[PATH_MAX];
		  getcwd(cwd, sizeof(cwd));
		  _err("Extracting CJIT's own source to %s/cjit_source",cwd);
		  muntarfs_extract_targz_to_path(cwd, (const uint8_t *)&cjit_source, cjit_source_len);
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
		  res = 0;
		  goto endgame;
#endif
#if !defined(SHAREDTCC)
	  } else if (c == 401) { // --xass
		  forced_route = CLI_ROUTE_EXTRACT_ASSETS;
		  forced_route_path = opt.arg;
		  break;
#endif
	  } else if (c == 501) { // --xtgz
		  forced_route = CLI_ROUTE_EXTRACT_ARCHIVE;
		  forced_route_path = opt.arg;
		  break;
	  }
	  else if (c == '?') _err("unknown opt: -%c\n", opt.opt? opt.opt : ':');
	  else if (c == ':') _err("missing arg: -%c\n", opt.opt? opt.opt : ':');
	  else if (c == '-') { // -- separator
		  arg_separator = opt.ind+1; break;
	  }
  }
  if(!CJIT->quiet)
	_err("cjit version %s (c) 2024-2025 Dyne.org foundation",&VERSION[1]);

  {
	  ParsedRoute parsed = parse_cli_route(CJIT, argc, clean_argv, opt.ind, arg_separator,
											  forced_route, forced_route_path);

	  if(parsed.route == CLI_ROUTE_EXTRACT_ASSETS) {
		  ExtractAssetsRequest request;
		  ExtractAssetsResponse response;
		  request = build_extract_assets_request(&parsed);
		  if(request.destination_path) {
			  _err("Extracting runtime assets to: %s", request.destination_path);
		  }
		  response = extract_assets_route(CJIT, &request);
		  render_extract_assets_response(CJIT, &response);
		  res = response.result.exit_status;
		  goto endgame;
	  } else if(parsed.route == CLI_ROUTE_EXTRACT_ARCHIVE) {
		  ExtractArchiveRequest request;
		  ExtractArchiveResponse response;
		  request = build_extract_archive_request(&parsed);
		  _err("Extract contents of: %s", request.archive_path);
		  response = extract_archive_route(&request);
		  render_extract_archive_response(NULL, &response);
		  res = response.result.exit_status;
		  goto endgame;
	  } else if(parsed.route == CLI_ROUTE_PRINT_STATUS) {
		  StatusRequest request;
		  StatusResponse response;
		  request = build_status_request(CJIT);
		  response = print_status(CJIT, &request);
		  render_status_response(CJIT, &response);
		  res = response.result.exit_status;
		  goto endgame;
	  } else if(parsed.route == CLI_ROUTE_COMPILE_OBJECT) {
	  CompileObjectRequest request;
	  CompileObjectResponse response;
	  request = build_compile_object_request(CJIT, &parsed);
	  response = compile_object(CJIT, &request);
	  render_compile_object_response(CJIT, &response);
	  res = response.result.exit_status;
	  goto endgame;
	  }

  if(parsed.route == CLI_ROUTE_BUILD_EXECUTABLE) {
	  BuildExecutableRequest request;
	  BuildExecutableResponse response;
	  _err("Create executable: %s", CJIT->output_filename);
	  request = build_build_executable_request(CJIT, &parsed);
	  response = build_executable(CJIT, &request);
	  render_build_executable_response(CJIT, &response);
	  res = response.result.exit_status;
  } else {
	  ExecuteRequest request;
	  ExecuteResponse response;
	  request = build_execute_request(CJIT, &parsed);
	  response = execute_source(CJIT, &request);
	  render_execute_response(CJIT, &response);
	  res = response.result.exit_status;
  }
  }
  endgame:
  // release buffer instantiated by remove_args
  free(clean_argv);
  // free TCC
  cjit_free(CJIT);
  exit(res);
}

char** remove_args(int* argc, char** argv,
				   const char** to_remove, int remove_count) {
    if (*argc == 0 || argv == NULL
		|| to_remove == NULL || remove_count == 0) {
        return argv;
    }
    bool* keep = calloc(*argc, sizeof(bool));
    if (!keep) return NULL;
    // First pass: mark all arguments to keep (initially all true)
    for (int i = 0; i < *argc; i++) {
        keep[i] = true;
    }
    int new_argc = *argc;
    // Second pass: process removal patterns
    for (int i = 0; i < *argc; i++) {
        if (!keep[i]) continue;  // Already marked for removal
        for (int j = 0; j < remove_count; j++) {
            const char* arg = argv[i];
            const char* pattern = to_remove[j];
            size_t pattern_len = strlen(pattern);
            // Case 1: Exact match
            if (strcmp(arg, pattern) == 0) {
                keep[i] = false;
                new_argc--;
                break;
            }
            // Case 2: Colon-terminated pattern (e.g., "option:")
            if (pattern[pattern_len - 1] == ':') {
                // Check if current argument starts with the pattern (without colon)
                if (strncmp(arg, pattern, pattern_len - 1) == 0) {
                    // Check if it's in the form "option:value"
                    if (isalnum(arg[pattern_len-1])) {
                        keep[i] = false;  // Remove the whole "option:value"
                        new_argc--;
                        break;
                    }
                    // OR if it's in the form "option value" (next argument)
                    else if (i + 1 < *argc && arg[pattern_len - 1] == '\0') {
                        keep[i] = false;    // Remove the option
                        keep[i + 1] = false; // Remove the value
                        new_argc -= 2;
                        break;
                    }
                }
            }
        }
    }
    // Allocate new argv array
    char** new_argv = malloc((new_argc + 1) * sizeof(char*));
    if (!new_argv) {
        free(keep);
        return NULL;
    }
    // Copy kept arguments to new argv
    for (int i = 0, j = 0; i < *argc; i++) {
        if (keep[i]) {
            new_argv[j++] = argv[i];
        }
    }
    new_argv[new_argc] = NULL;  // NULL-terminate
    free(keep);
    *argc = new_argc;
    return new_argv;
}

/**
 * Runs the archive compatibility mode used to mimic `ar`.
 */
static int handle_archive_mode(CJITState *cjit, int argc, char **argv) {
#ifndef CJIT_WITHOUT_AR
	return cjit_ar(cjit, argc - 1, argv + 1);
#else
	(void)cjit;
	(void)argc;
	(void)argv;
	return 1;
#endif
}

/**
 * Builds the autoconf `conftest.c` probe into `a.out`.
 */
static int handle_conftest_mode(CJITState *cjit, const char *source_path) {
	int res = 0;
	_err("Detected conftest");
	cjit->output_filename = "a.out";
	cjit_set_output(cjit, EXE);
	cjit_add_file(cjit, source_path);
	if(cjit_link(cjit)<0) {
		_err("Error in linker compiling to file: %s",
		     cjit->output_filename);
		res = 1;
	}
	cjit->output_filename = NULL;
	return res;
}
