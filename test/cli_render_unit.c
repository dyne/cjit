#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "adapters/cli/render_response.h"

static char stdout_capture[256];
static char stderr_capture[256];

void _out(const char *format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    vsnprintf(stdout_capture, sizeof(stdout_capture), format, arguments);
    va_end(arguments);
}

void _err(const char *format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    vsnprintf(stderr_capture, sizeof(stderr_capture), format, arguments);
    va_end(arguments);
}

static int expect_text(const char *expected_stdout, const char *expected_stderr)
{
    if (strcmp(stdout_capture, expected_stdout) != 0 || strcmp(stderr_capture, expected_stderr) != 0) {
        fprintf(stderr, "render output mismatch: stdout=%s stderr=%s\n", stdout_capture, stderr_capture);
        return 1;
    }
    stdout_capture[0] = '\0';
    stderr_capture[0] = '\0';
    return 0;
}

int main(void)
{
    CJITState state = { 0 };
    ExecuteResponse execute = { cjit_result_ok() };
    CompileObjectResponse compile = { cjit_result_error(CJIT_RESULT_COMPILER_ERROR, 1, "compile failed"), "object.o" };
    BuildExecutableResponse build = { cjit_result_error(CJIT_RESULT_LINK_ERROR, 7, "link failed"), "ignored" };
    BuildExecutableResponse build_success = { cjit_result_ok(), "program" };
    ExtractAssetsResponse assets = { cjit_result_ok(), "runtime-path" };
    ExtractAssetsResponse assets_error = { cjit_result_error(CJIT_RESULT_IO_ERROR, 1, "assets failed"), NULL };
    ExtractArchiveResponse archive = { cjit_result_error(CJIT_RESULT_IO_ERROR, 3, "archive failed") };
    StatusResponse status_success = { cjit_result_ok() };
    StatusResponse status_error = { cjit_result_error(CJIT_RESULT_PLATFORM_ERROR, 1, "status failed") };
    int failures = 0;

    render_execute_response(NULL, &execute);
    failures += expect_text("", "");
    render_compile_object_response(NULL, &compile);
    failures += expect_text("", "compile failed");
    state.output_filename = "program";
    render_build_executable_response(&state, &build);
    failures += expect_text("", "link failed: program");
    if (build.result.exit_status != 7) ++failures;
    render_build_executable_response(&state, &build_success);
    failures += expect_text("", "");
    render_extract_assets_response(NULL, &assets);
    failures += expect_text("runtime-path", "");
    render_extract_assets_response(NULL, &assets_error);
    failures += expect_text("", "assets failed");
    render_extract_archive_response(NULL, &archive);
    failures += expect_text("", "archive failed");
    render_status_response(NULL, &status_success);
    failures += expect_text("", "");
    render_status_response(NULL, &status_error);
    failures += expect_text("", "");
    return failures ? 1 : 0;
}
