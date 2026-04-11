#include "app/compile_object.h"

#include <stddef.h>

#include "adapters/compiler/tinycc_adapter.h"

static CompileObjectResponse make_compile_object_response(CJITResultCode code, int exit_status,
                                                          bool ok, const char *message,
                                                          const char *output_path)
{
    CompileObjectResponse response;
    response.result = cjit_result_make(code, exit_status, ok, message);
    response.output_path = output_path;
    return response;
}

CompileObjectResponse compile_object(CJITState *cjit, const CompileObjectRequest *request)
{
    RuntimeSession session;
    CompilerPort compiler = tinycc_compiler_port;
    compiler.context = cjit;
    compiler.begin_session(compiler.context, &session);

    if (!request->source_path) {
        return make_compile_object_response(CJIT_RESULT_INVALID_REQUEST, 1, false,
                                            "Compiling to object files supports only one file argument",
                                            NULL);
    }
    if (!compiler.compile_object(compiler.context, &session, request->source_path).ok) {
        return make_compile_object_response(CJIT_RESULT_COMPILER_ERROR, 1, false,
                                            "Compile to object failed", request->options.output_path);
    }
    compiler.end_session(compiler.context, &session);
    return make_compile_object_response(CJIT_RESULT_OK, 0, true, NULL,
                                        request->options.output_path);
}
