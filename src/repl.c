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

// #if !defined(WINDOWS)
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <sys/wait.h>
// #include <poll.h>
// #endif

// TODO: rudimental start at a repl, needs development

int cjit_cli_tty(CJITState *cjit) {
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
#if defined(WINDOWS)
    _err("Missing source code argument");
#else // WINDOWS
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
        if (tcc_compile_string(cjit->TCC, code) < 0) {
          _err("Code runtime error in source\n");
          res = 1;
          break;
        }
        if (tcc_relocate(cjit->TCC) < 0) {
          _err("Code relocation error in source\n");
          res = 1;
          break;
        }
#ifdef VERBOSE_CLI
        _err("Running code\n");
        _err("-----------------------------------\n");
#endif // VERBOSE_CLI

        res = cjit_exec(cjit, 0, NULL);
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
#endif // WINDOWS
    return res;
}
