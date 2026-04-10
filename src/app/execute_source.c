#include "app/execute_source.h"

#include <stdlib.h>
#include <string.h>

extern char *load_stdin();

static ExecuteResponse make_error(CJITResultCode code, int exit_status, const char *message)
{
    ExecuteResponse response;
    response.result.code = code;
    response.result.exit_status = exit_status;
    response.result.ok = false;
    response.result.message = message;
    return response;
}

static ExecuteResponse make_success(int exit_status)
{
    ExecuteResponse response;
    response.result.code = CJIT_RESULT_OK;
    response.result.exit_status = exit_status;
    response.result.ok = true;
    response.result.message = NULL;
    return response;
}

ExecuteResponse execute_source(CJITState *cjit, const ExecuteRequest *request)
{
    char *stdin_code = NULL;
    int i;

    if (request->source_count == 0) {
#if defined(_WIN32)
        return make_error(CJIT_RESULT_INVALID_REQUEST, 1,
                          "No files specified on commandline");
#else
        if (!cjit->quiet) {
            _err("No files specified on commandline, reading code from stdin");
        }
        stdin_code = load_stdin();
        if (!stdin_code) {
            return make_error(CJIT_RESULT_IO_ERROR, 1,
                              "Error reading from standard input");
        }
        if (!cjit_add_buffer(cjit, stdin_code)) {
            free(stdin_code);
            return make_error(CJIT_RESULT_COMPILER_ERROR, 1,
                              "Code runtime error in stdin");
        }
        free(stdin_code);
#endif
    } else {
        if (cjit->verbose) {
            _err("Source code:");
        }
        for (i = 0; i < request->source_count; ++i) {
            const char *code_path = request->sources[i];
            if (cjit->verbose) {
                _err("%c %s", (*code_path == '-' ? '|' : '+'),
                     (*code_path == '-' ? "standard input" : code_path));
            }
            if (*code_path == '-') {
#if defined(_WIN32)
                return make_error(CJIT_RESULT_INVALID_REQUEST, 1,
                                  "Code from standard input not supported on Windows");
#else
                stdin_code = load_stdin();
                if (!stdin_code) {
                    return make_error(CJIT_RESULT_IO_ERROR, 1,
                                      "Error reading from standard input");
                }
                if (!cjit_add_buffer(cjit, stdin_code)) {
                    free(stdin_code);
                    return make_error(CJIT_RESULT_COMPILER_ERROR, 1,
                                      "Code runtime error in stdin");
                }
                free(stdin_code);
                stdin_code = NULL;
#endif
            } else {
                if (!cjit_add_file(cjit, code_path)) {
                    return make_error(CJIT_RESULT_COMPILER_ERROR, 1,
                                      "Error loading source input");
                }
            }
        }
    }

    return make_success(cjit_exec(cjit, request->app_argc, request->app_argv));
}
