#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "adapters/compiler/tinycc_adapter.h"
#include "adapters/platform/runtime_platform.h"
#include "cjit.h"
#include "libtcc.h"

static int deletes;
static int output_type_calls;
static int options_calls;

TCCState *tcc_new(void) { return (TCCState *)1; }
void tcc_delete(TCCState *state) { (void)state; deletes++; }
void tcc_set_error_func(TCCState *state, void *opaque, TCCErrorFunc *callback)
{ (void)state; (void)opaque; (void)callback; }
int tcc_set_options(TCCState *state, const char *options)
{ (void)state; (void)options; options_calls++; return 0; }
int tcc_add_include_path(TCCState *state, const char *path)
{ (void)state; (void)path; return 0; }
void tcc_define_symbol(TCCState *state, const char *symbol, const char *value)
{ (void)state; (void)symbol; (void)value; }
int tcc_add_file(TCCState *state, const char *path)
{ (void)state; (void)path; return 0; }
int tcc_compile_string(TCCState *state, const char *source)
{ (void)state; (void)source; return 0; }
int tcc_set_output_type(TCCState *state, int output)
{ (void)state; (void)output; output_type_calls++; return 0; }
int tcc_add_library_path(TCCState *state, const char *path)
{ (void)state; (void)path; return 0; }

char *new_abspath(const char *path) { (void)path; return NULL; }
void cjit_platform_setup_runtime(CJITState *state) { (void)state; }
void cjit_platform_add_library_path(CJITState *state, const char *path)
{ (void)state; (void)path; }
void cjit_platform_print_status(const CJITState *state) { (void)state; }
LibraryResolverPort cjit_platform_library_resolver(void)
{ LibraryResolverPort port = {0}; return port; }
const CompilerPort tinycc_compiler_port = {0};

static int failures;
static void expect(int condition, const char *message)
{ if (!condition) { fprintf(stderr, "FAIL: %s\n", message); failures++; } }

int main(void)
{
    CJITState *state;
    CJITResult result;
    int original_output;

    cjit_free(NULL);
    state = cjit_new();
    expect(state != NULL, "new state initializes all owned resources");
    if (!state) return 1;
    expect(state->tcc_output == TCC_OUTPUT_MEMORY, "memory is the default output mode");
    expect(!state->done_setup, "new state has not been prepared");

    result = cjit_prepare(NULL);
    expect(!result.ok && result.code == CJIT_RESULT_INVALID_REQUEST,
           "prepare rejects missing state");
    result = cjit_add_buffer_result(state, NULL);
    expect(!result.ok && result.code == CJIT_RESULT_INVALID_REQUEST,
           "buffer wrapper rejects missing source");
    result = cjit_add_source_result(state, NULL);
    expect(!result.ok && result.code == CJIT_RESULT_INVALID_REQUEST,
           "source wrapper rejects missing path");
    result = cjit_add_file_result(state, NULL);
    expect(!result.ok && result.code == CJIT_RESULT_INVALID_REQUEST,
           "file wrapper rejects missing path");

    result = cjit_prepare(state);
    expect(result.ok && state->done_setup, "prepare succeeds once");
    expect(output_type_calls == 1, "prepare configures output once");
    result = cjit_prepare(state);
    expect(result.ok && output_type_calls == 1, "prepare is idempotent");
    expect(options_calls == 0, "no ambient compiler options are required");

    original_output = state->tcc_output;
    cjit_set_output(state, 0);
    expect(state->tcc_output == original_output, "invalid low output mode is ignored");
    cjit_set_output(state, 6);
    expect(state->tcc_output == original_output, "invalid high output mode is ignored");
    cjit_set_output(state, TCC_OUTPUT_OBJ);
    expect(state->tcc_output == TCC_OUTPUT_OBJ, "valid output mode is retained");

    cjit_free(state);
    expect(deletes == 1, "state cleanup releases the compiler context once");
    return failures != 0;
}
