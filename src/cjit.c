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

#include <sys/types.h>
#include <sys/stat.h>

#include <cflag.h>
#include <libtcc.h>
#include <sys/wait.h>
#include <unistd.h>

// from embed-libtcc1 generated from lib/tinycc/libtcc1.a
extern char *libtcc1;
extern unsigned int libtcc1_len;
extern char *musl_libc;
extern unsigned int musl_libc_len;

// from file.c
extern long  file_size(const char *filename);
extern char* file_load(const char *filename);
extern bool write_to_file(char *path, char *filename, char *buf, unsigned int len);
extern bool rm_recursive(char *path);

// from io.c
extern void _out(const char *fmt, ...);
extern void _err(const char *fmt, ...);
// TCC callback
void handle_error(void *n, const char *m) {
  (void)n;
  _err("%s",m);
}

static int cjit_exec(TCCState *TCC, const char *ep, int argc, char **argv)
{
  pid_t pid;
  int res = 1;
  int (*_ep)(int, char**);
  _ep = tcc_get_symbol(TCC, ep);
  if (!_ep) {
    _err("Symbol not found in source: %s","main");
    return -1;
  }
  _err("Execution start\n---");
  pid = fork();
  if (pid == 0) {
      res = _ep(argc, argv);
      exit(res);
  } else {
      int status;
      int ret;
      ret = waitpid(pid, &status, WUNTRACED | WCONTINUED);
      if (ret != pid){
          _err("Wait error in source: %s","main");
      }
      if (WIFEXITED(status)) {
          res = WEXITSTATUS(status);
          _err("Process has returned %d", res);
      } else if (WIFSIGNALED(status)) {
          res = WTERMSIG(status);
          _err("Process terminated with signal %d", WTERMSIG(status));
      } else if (WIFSTOPPED(status)) {
          res = WSTOPSIG(status);
          _err("Process has returned %d", WSTOPSIG(status));
      } else if (WIFSTOPPED(status)) {
          res = WSTOPSIG(status);
          _err("Process stopped with signal", WSTOPSIG(status));
      } else {
          _err("wait: unknown status: %d", status);
      }
  }
  return res;
}

static int cjit_cli(TCCState *TCC)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int res = 0;
    const char intro[]="#include <stdio.h>\n#include <stdlib.h>\nint main(int argc, char **argv) {\n";
    char *code = malloc(sizeof(intro));
    if (!code) {
        _err("Memory allocation error");
        return 2;
    }
    // don't add automatic main preamble if in a pipe
    if (isatty(fileno(stdin)))
        strcpy(code, intro);
    else
        _err("Not running from a terminal though.\n");

    while (1) {
        // don't print prompt if we are in a pipe 
        if (isatty(fileno(stdin)))
            printf("cjit> ");
        fflush(stdout);
        read = getline(&line, &len, stdin);
        if (read == -1) {
            /* This is CTRL + D */
            code = realloc(code, strlen(code) + 4);
            if (!code) {
                _err("Memory allocation error");
                res = 2;
                break;
            }
            free(line);
            line = NULL;
            strcat(code, "\n}\n");

            // run the code from main
#ifdef VERBOSE_CLI
            _err("Compiling code\n");
            _err("-----------------------------------\n");
            _err("%s\n", code);
            _err("-----------------------------------\n");
#endif
            if (tcc_compile_string(TCC, code) < 0) {
                _err("Code runtime error in source\n");
                res = 1;
                break;
            }
            if (tcc_relocate(TCC) < 0) {
                _err("Code relocation error in source\n");
                res = 1;
                break;
            }
#ifdef VERBOSE_CLI
            _err("Running code\n");
            _err("-----------------------------------\n");
#endif
            res = cjit_exec(TCC, "main", 0, NULL);
            free(code);
            code = NULL;
            break;
        }
        code = realloc(code, strlen(code) + len + 1);
        if (!code) {
            _err("Memory allocation error");
            res = 2;
            break;
        }
        strcat(code, line);
        free(line);
        line = NULL;
    }
    return res;
}

int main(int argc, char **argv) {
  TCCState *TCC;
  const char *syntax = "[options] code.c";
  const char *include_path = 0x0;
  const char *libs_path = 0x0;
  const char *progname = "cjit";
  static bool verbose = false;
  static bool version = false;
  char tmptemplate[] = "/tmp/CJIT-exec.XXXXXX";
  char *tmpdir = NULL;
  int res = 1;


  static const struct cflag options[] = {
    CFLAG(bool, "verbose", 'v', &verbose, "Verbosely show progress"),
    CFLAG(bool, "version", 'V', &version, "Show build version"),
    CFLAG_HELP,
    CFLAG_END
  };
  cflag_apply(options, syntax, &argc, &argv);
  const char *code_path = argv[0];
  _err("CJIT %s",VERSION);
  TCC = tcc_new();
  if (!TCC) {
    _err("Could not initialize tcc");
    exit(1);
  }
  // error handler callback for TCC
  tcc_set_error_func(TCC, stderr, handle_error);

  // initialize the tmpdir for execution
  tmpdir = mkdtemp(tmptemplate);
  if(!tmpdir) {
    _err("Error creating temp dir %s: %s",tmptemplate,strerror(errno));
    goto endgame;
  }
  // _err("tempdir: %s",tmpdir);
  tcc_set_lib_path(TCC,tmpdir);
  tcc_add_library_path(TCC,tmpdir);

  //// TCC DEFAULT PATHS
  tcc_add_include_path(TCC,"/usr/include/x86_64-linux-musl"); // devuan
  // tcc_add_include_path(TCC,"src"); // devuan
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

  // simple temporary exports for hello world
  // tcc_add_symbol(TCC, "exit", &return);
  tcc_add_symbol(TCC, "stdout", &stdout);
  tcc_add_symbol(TCC, "stderr", &stderr);
  tcc_add_symbol(TCC, "fprintf", &fprintf);

  if(! write_to_file(tmpdir,"libtcc1.a",&libtcc1,libtcc1_len) )
    goto endgame;
  if(! write_to_file(tmpdir,"libc.so",&musl_libc,musl_libc_len) )
    goto endgame;

  if (argc == 0) {
      printf("No input file: running in interactive mode\n");
      res = cjit_cli(TCC);
      goto endgame;
  }
  _err("Source to execute: %s",code_path);
  char *code = file_load(code_path);
  if(!code) {
    _err("File not found: %s",code_path);
    goto endgame;
  }
  if (tcc_compile_string(TCC, code) == -1) return 1;
  free(code); // safe: bytecode compiled is in TCC now
  _err("Compilation successful");

  // relocate the code
  if (tcc_relocate(TCC) < 0) {
    _err("TCC relocation error");
    goto endgame;
  }

  res = cjit_exec(TCC, "main", argc, argv);

  endgame:
  // free TCC
  tcc_delete(TCC);
  if(tmpdir) {
    // _err("remove tmpdir");
    rm_recursive(tmpdir);
  }
  _err("---\nExecution completed");
  exit(res);
}
