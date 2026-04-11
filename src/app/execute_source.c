#include "app/execute_source.h"

#include <stdlib.h>
#include <string.h>

#include "adapters/compiler/tinycc_adapter.h"
#include "adapters/fs/local_filesystem.h"

static ExecuteResponse make_error(CJITResultCode code, int exit_status, const char *message)
{
    ExecuteResponse response;
    response.result = cjit_result_error(code, exit_status, message);
    return response;
}

ExecuteResponse execute_source(CJITState *cjit, const ExecuteRequest *request)
{
    char *stdin_code = NULL;
    int i;
    int exit_status = 0;
    CJITResult result;
    RuntimeSession session;
    CompilerPort compiler = tinycc_compiler_port;
    FilesystemPort filesystem = local_filesystem_port;
    compiler.context = cjit;
    filesystem.context = cjit;
    compiler.begin_session(compiler.context, &session);

    if (request->source_count == 0) {
#if defined(_WIN32)
        return make_error(CJIT_RESULT_INVALID_REQUEST, 1,
                          "No files specified on commandline");
#else
        if (!cjit->quiet) {
            _err("No files specified on commandline, reading code from stdin");
        }
        if (!filesystem.read_stdin(filesystem.context, &stdin_code, NULL).ok) {
            return make_error(CJIT_RESULT_IO_ERROR, 1, "Error reading from standard input");
        }
        if (!compiler.add_source_buffer(compiler.context, &session, stdin_code).ok) {
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
                if (!filesystem.read_stdin(filesystem.context, &stdin_code, NULL).ok) {
                    return make_error(CJIT_RESULT_IO_ERROR, 1,
                                      "Error reading from standard input");
                }
                if (!compiler.add_source_buffer(compiler.context, &session, stdin_code).ok) {
                    free(stdin_code);
                    return make_error(CJIT_RESULT_COMPILER_ERROR, 1,
                                      "Code runtime error in stdin");
                }
                free(stdin_code);
                stdin_code = NULL;
#endif
            } else {
                if (!compiler.add_source_file(compiler.context, &session, code_path).ok) {
                    return make_error(CJIT_RESULT_COMPILER_ERROR, 1,
                                      "Error loading source input");
                }
            }
        }
    }

    result = compiler.execute_program(compiler.context, &session,
                                      request->app_argc, request->app_argv, &exit_status);
    compiler.end_session(compiler.context, &session);
    {
        ExecuteResponse response;
        response.result = result;
        return response;
    }
}
