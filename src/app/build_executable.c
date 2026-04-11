#include "app/build_executable.h"

#include <stddef.h>

#include "adapters/compiler/tinycc_adapter.h"

static BuildExecutableResponse make_build_response(CJITResultCode code, int exit_status,
                                                   bool ok, const char *message,
                                                   const char *output_path)
{
    BuildExecutableResponse response;
    response.result = cjit_result_make(code, exit_status, ok, message);
    response.output_path = output_path;
    return response;
}

BuildExecutableResponse build_executable(CJITState *cjit, const BuildExecutableRequest *request)
{
    int i;
    RuntimeSession session;
    CompilerPort compiler = tinycc_compiler_port;
    compiler.context = cjit;
    compiler.begin_session(compiler.context, &session);

    for (i = 0; i < request->source_count; ++i) {
        if (!compiler.add_source_file(compiler.context, &session, request->sources[i]).ok) {
            return make_build_response(CJIT_RESULT_COMPILER_ERROR, 1, false,
                                       "Error loading source input", request->options.output_path);
        }
    }
    if (!compiler.link_executable(compiler.context, &session).ok) {
        return make_build_response(CJIT_RESULT_LINK_ERROR, 1, false,
                                   "Error in linker compiling to file",
                                   request->options.output_path);
    }
    compiler.end_session(compiler.context, &session);
    return make_build_response(CJIT_RESULT_OK, 0, true, NULL,
                               request->options.output_path);
}
