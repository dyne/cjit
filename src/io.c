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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>

#define MAX_STRING 20480 // max 20KiB strings

static bool no_info = false;
static bool no_output = false;
static bool no_errors = false;

bool get_no_info() {
  return no_info;
}

void set_no_info_output(bool val) {
  no_info = val;
}

bool get_no_output() {
  return no_output;
}

void set_no_output(bool val) {
  no_output = val;
}

bool get_no_error_output() {
  return no_errors;
}

void set_no_error_output(bool val) {
  no_errors = val;
}

// stdout message free from context
void _info(const char *fmt, ...) {
  if (no_info) return;

  va_list args;
  va_start(args, fmt);
  char msg[MAX_STRING+4];
  int len = vsnprintf(msg, MAX_STRING, fmt, args);
  va_end(args);
  msg[len] = '\n';
  msg[len+1] = 0x0; //safety
#if defined(__EMSCRIPTEN__)
  EM_ASM_({Module.print(UTF8ToString($0))}, msg);
#else
  write(STDOUT_FILENO, msg, len+1);
#endif
}

// stdout message free from context
void _out(const char *fmt, ...) {
  if (no_output) return;

  va_list args;
  va_start(args, fmt);
  char msg[MAX_STRING+4];
  int len = vsnprintf(msg, MAX_STRING, fmt, args);
  va_end(args);
  msg[len] = '\n';
  msg[len+1] = 0x0; //safety
#if defined(__EMSCRIPTEN__)
  EM_ASM_({Module.print(UTF8ToString($0))}, msg);
#else
  write(STDOUT_FILENO, msg, len+1);
#endif
}

// error message free from context
void _err(const char *fmt, ...) {
  if (no_errors) return;

  char msg[MAX_STRING+4];
  int len;
  va_list args;
  va_start(args, fmt);
  len = vsnprintf(msg, MAX_STRING, fmt, args);
  va_end(args);
  msg[len] = '\n';
  msg[len+1] = 0x0;
#if defined(__EMSCRIPTEN__)
  EM_ASM_({Module.printErr(UTF8ToString($0))}, msg);
#elif defined(__ANDROID__)
  __android_log_print(ANDROID_LOG_ERROR, "ZEN", "%s", msg);
#else
  write(STDERR_FILENO,msg,len+1);
#endif
}
