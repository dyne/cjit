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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <cjit.h>

#if !defined(_WIN32)
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <poll.h>
#endif

// from io.c
extern void _out(const char *fmt, ...);
extern void _err(const char *fmt, ...);

#if defined(_WIN32)
int cjit_exec_win(TCCState *TCC, CJITState *CJIT, const char *ep, int argc, char **argv) {
  int res = 1;
  int (*_ep)(int, char**);
  _ep = tcc_get_symbol(TCC, ep);
  if (!_ep) {
    _err("Symbol not found in source: %s",ep);
    return -1;
  }
  if(CJIT->write_pid) {
	  pid_t pid = getpid();
	  FILE *fd = fopen(CJIT->write_pid, "w");
	  if(!fd) {
		  _err("Cannot create pid file %s: %s",
		       CJIT->write_pid, strerror(errno));
		  return -1;
	  }
	  fprintf(fd,"%d\n",pid);
	  fclose(fd);
  }
  // _err("Execution start\n---");
  res = _ep(argc, argv);
  return(res);
}

#else // _WIN32
int cjit_exec_fork(TCCState *TCC, CJITState *CJIT, const char *ep, int argc, char **argv) {
  pid_t pid;
  int res = 1;
  int (*_ep)(int, char**);
  _ep = tcc_get_symbol(TCC, ep);
  if (!_ep) {
    _err("Symbol not found in source: %s",ep);
    return -1;
  }
  pid = fork();
  if (pid == 0) {
      res = _ep(argc, argv);
      exit(res);
  } else {
	  if(CJIT->write_pid) {
		  // pid_t pid = getpid();
		  FILE *fd = fopen(CJIT->write_pid, "w");
		  if(!fd) {
			  _err("Cannot create pid file %s: %s",
			       CJIT->write_pid, strerror(errno));
			  return -1;
		  }
		  fprintf(fd,"%d\n",pid);
		  fclose(fd);
	  }
      int status;
      int ret;
      ret = waitpid(pid, &status, WUNTRACED | WCONTINUED);
      if (ret != pid){
          _err("Wait error in source: %s","main");
      }
      if (WIFEXITED(status)) {
          res = WEXITSTATUS(status);
          //_err("Process has returned %d", res);
      } else if (WIFSIGNALED(status)) {
          res = WTERMSIG(status);
          _err("Process terminated with signal %d", WTERMSIG(status));
      } else if (WIFSTOPPED(status)) {
          res = WSTOPSIG(status);
          //_err("Process has returned %d", WSTOPSIG(status));
      } else if (WIFSTOPPED(status)) {
          res = WSTOPSIG(status);
          _err("Process stopped with signal", WSTOPSIG(status));
      } else {
          _err("wait: unknown status: %d", status);
      }
  }
  return res;
}
#endif // _WIN32

#ifdef KILO_SUPPORTED

// from kilo.c
extern void initEditor(void);
extern void editorRefreshScreen(void);
extern void editorSetStatusMessage(const char *fmt, ...);
extern void editorSetCompilerCallback(int (*cb)(void *, char *, int, char **));
extern void editorSetCheckCallback(int (*cb)(void *, char *, char **));
extern void editorSetCompilerContext(void *ctx);
extern void editorProcessKeypress(int fd);
extern int enableRawMode(int fd);
extern void disableRawMode(int fd);
extern int enableGetCharMode(int fd);
extern void disableGetCharMode(int fd);
extern void editorInsertRow(int at, const char *s, size_t len);

static void error_callback(void *ctx, const char *msg) {
    int wfd = *(int *)ctx;
    if ((msg) && (wfd >=0)) {
        write(wfd, msg, strlen(msg));
    }
}

int cjit_compile_and_run(TCCState *TCC, const char *code, int argc, char **argv, int rn, char **err_msg) {
  pid_t pid;
  int res = 0;
  int (*_ep)(int, char**);
  const char main_fn[]="main";
  int err_fds[2];
  int err_r, err_w;
  const char compile_errmsg[]= "Code compilation error in source";
  const char reloc_errmsg[]= "Code relocation error in source";
  const char nomain_errmsg[]= "Symbol 'main' was not found in source";


  *err_msg = NULL;

  if (pipe(err_fds) != 0) {
    _err("Error creating pipe\n");
    return 1;
  }
  err_r = err_fds[0];
  err_w = err_fds[1];

  pid = fork();
  if (pid == 0) {
      close(err_r);
      tcc_set_error_func(TCC, &err_w, error_callback);
      if (tcc_compile_string(TCC, code) < 0) {
          write(err_w, compile_errmsg, strlen(compile_errmsg));
          res = 1;
      }
      if ((res == 0) && (tcc_relocate(TCC) < 0)) {
          write(err_w, reloc_errmsg, strlen(reloc_errmsg));
          res = 1;
      }
      if (res == 0) {
          _ep = tcc_get_symbol(TCC, main_fn);
          if (!_ep) {
              write(err_w, nomain_errmsg, strlen(nomain_errmsg));
              res = 1;
          }
      }

      if ((res == 0) && (rn != 0)) {
          res = _ep(argc, argv);
      }
      close(err_w);
      exit(res);
  } else {
      int status;
      int ret;
      struct pollfd pfd;
      close(err_w);
      pfd.fd = err_r;
      pfd.events = POLLIN;

      while (poll(&pfd, 1, 2000) > 0) {
          ssize_t n = 0;
          char buf[1024];
          memset(buf, 0, 1024);
          n = read(err_r, buf, 1023);
          if (n > 0) {
              if (!*err_msg)
                  *err_msg = strdup(buf);
          } else break;
      }
      close(err_r);

      ret = waitpid(pid, &status, WUNTRACED | WCONTINUED);
      if (ret != pid){
          _err("Wait error in source: %s","main");
          return 1;
      }
      if (WIFEXITED(status)) {
          res = WEXITSTATUS(status);
          //_err("Process has returned %d", res);
      } else if (WIFSIGNALED(status)) {
          res = WTERMSIG(status);
          _err("Process terminated with signal %d", WTERMSIG(status));
      } else if (WIFSTOPPED(status)) {
          res = WSTOPSIG(status);
          //_err("Process has returned %d", WSTOPSIG(status));
      } else if (WIFSTOPPED(status)) {
          res = WSTOPSIG(status);
          _err("Process stopped with signal", WSTOPSIG(status));
      } else {
          _err("wait: unknown status: %d", status);
      }
  }
  return res;
}

/* Called by the editor when the buffer is set to run */
static int cjit_compile_buffer(void *tcs, char *code, int argc, char **argv)
{
    TCCState *TCC = (TCCState *)tcs;
    int res = 0;
    char *err_msg;
    disableRawMode(fileno(stdin));
    /* Clear the screen */
    write(fileno(stdout), "\x1b[2J", 4);
    // run the code from main
    res = cjit_compile_and_run(TCC, code, argc, argv, 1, &err_msg);
    if (err_msg) {
        _err(err_msg);
        free(err_msg);
    }
    enableGetCharMode(fileno(stdin));
    _err("\n\n\n\nPress any key to continue....\n");
    getchar();
    disableGetCharMode(fileno(stdin));
    free(code);

    enableRawMode(fileno(stdin));
    editorRefreshScreen();
    return res;
}

#define ERR_MAX 80
int cjit_check_buffer(void *tcs, char *code, char **err_msg) {
    TCCState *TCC = (TCCState *)tcs;
    int res = 0;
    if(err_msg)
        *err_msg = NULL;
    // run the code from main
    //
    disableRawMode(fileno(stdin));
    res = cjit_compile_and_run(TCC, code, 0, NULL, 0, err_msg);
    if (res != 0) {
        if(*err_msg) {
            if (strlen(*err_msg) > ERR_MAX -1) {
                (*err_msg)[ERR_MAX - 1] = 0;
            }
            char *p = strchr(*err_msg, '\n');
            if (p) *p = 0;
            if (*err_msg) {
                editorSetStatusMessage(*err_msg);
            }

        }
    } else {
        editorSetStatusMessage("No errors.");
    }
    enableRawMode(fileno(stdin));
    return res;
}

int cjit_cli_kilo(TCCState *TCC) {
    char *line = NULL;
    size_t len = 0;
    ssize_t rd;
    int res = 0;
    char *err_msg;
    // don't add automatic main preamble if in a pipe
    if (!isatty(fileno(stdin))) {
        char *code = NULL;
        _err("Not running from a terminal, executing source from STDIN\n");
        do {
	    rd = getline(&line, &len, stdin);
            if (rd > 0) {
                if (!code)
                    code = strdup(line);
                else {
                   code = realloc(code, strlen(code) + rd + 1);
                   if (!code) {
                       _err("Memory error while executing from STDIN\n");
                       return 2;
                   }
                   strcat(code, line);
                }
            }
        } while (rd != -1);
        cjit_compile_and_run(TCC, code, 0, NULL, 1, &err_msg);
        if (err_msg)
            _err(err_msg);
    } else {
        int row = 0;
        int i = 0;
        initEditor();

        const char editor_rows[6][40] = {
            "#include <stdio.h>",
            "#include <stdlib.h>",
            "",
            "int main(int argc, char **argv) {",
            "",
            "}"
        };

        for (i = 0; i < 6; i++) {
            editorInsertRow(row++, editor_rows[i], strlen(editor_rows[i]));
        }

        enableRawMode(fileno(stdin));
        editorSetStatusMessage(
                "HELP: Cx-S = save | Cx-Q = quit | Cx-F = find | Cx-R = run | Cx-E = editor");
        while(1) {
            editorSetCompilerCallback(cjit_compile_buffer);
            editorSetCheckCallback(cjit_check_buffer);
            editorSetCompilerContext(TCC);
            editorRefreshScreen();
            editorProcessKeypress(fileno(stdin));
        }
    }
    return res;
}

#endif // KILO_SUPPORTED


int cjit_cli_tty(TCCState *TCC) {
    char *line = NULL;
    size_t len = 0;
    ssize_t rd;
    int res = 0;
    const char intro[]="#include <stdio.h>\n#include <stdlib.h>\nint main(int argc, char **argv) {\n";
    char *code = malloc(sizeof(intro));
    if (!code) {
        _err("Memory allocation error");
        return 2;
    }
    strcpy(code, intro);
#if defined(_WIN32)
    _err("Missing source code argument");
#else // _WIN32
    while (1) {
      printf("cjit> ");
      fflush(stdout);
      rd = getline(&line, &len, stdin);
      if (rd == -1) {
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
#endif // VERBOSE_CLI
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
#endif // VERBOSE_CLI
#if !defined(_WIN32)
        res = cjit_exec_fork(TCC, NULL, "main", 0, NULL);
#else // _WIN32
        res = cjit_exec_win(TCC, NULL, "main", 0, NULL);
#endif // _WIN32
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
#endif // _WIN32
    return res;
}
