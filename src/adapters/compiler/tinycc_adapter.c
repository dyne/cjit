#include "adapters/compiler/tinycc_adapter.h"

#include <stdlib.h>
#include <string.h>

#include "cjit.h"
#include "libtcc.h"

static CJITState *state_from_context(void *context)
{
    return (CJITState *)context;
}

static CJITResult begin_session(void *context, RuntimeSession *session)
{
    CJITState *cjit = state_from_context(context);
    session->compiler_handle = cjit->TCC;
    session->tmpdir = cjit->tmpdir;
    session->tempdir_is_fresh = cjit->fresh;
    session->setup_complete = cjit->done_setup;
    session->execution_complete = cjit->done_exec;
    return cjit_result_ok();
}

static CJITResult configure_session(void *context, RuntimeSession *session)
{
    CJITState *cjit = state_from_context(context);
    (void)session;
    if (!cjit) {
        return cjit_result_error(CJIT_RESULT_COMPILER_ERROR, 1, "Missing CJIT state");
    }
    return cjit_result_ok();
}

static CJITResult set_output_mode(void *context, RuntimeSession *session, int output_mode)
{
    CJITState *cjit = state_from_context(context);
    (void)session;
    cjit_set_output(cjit, output_mode);
    return cjit_result_ok();
}

static CJITResult add_source_file(void *context, RuntimeSession *session, const char *path)
{
    CJITState *cjit = state_from_context(context);
    (void)session;
    if (!cjit_add_file(cjit, path)) {
        return cjit_result_error(CJIT_RESULT_COMPILER_ERROR, 1, "Error loading source input");
    }
    return cjit_result_ok();
}

static CJITResult add_source_buffer(void *context, RuntimeSession *session, const char *buffer)
{
    CJITState *cjit = state_from_context(context);
    (void)session;
    if (!cjit_add_buffer(cjit, buffer)) {
        return cjit_result_error(CJIT_RESULT_COMPILER_ERROR, 1, "Code runtime error in stdin");
    }
    return cjit_result_ok();
}

static CJITResult add_binary_input(void *context, RuntimeSession *session, const char *path)
{
    return add_source_file(context, session, path);
}

static CJITResult define_symbol(void *context, RuntimeSession *session,
                                const char *name, const char *value)
{
    CJITState *cjit = state_from_context(context);
    (void)session;
    cjit_define_symbol(cjit, name, value);
    return cjit_result_ok();
}

static CJITResult add_include_path(void *context, RuntimeSession *session, const char *path)
{
    CJITState *cjit = state_from_context(context);
    (void)session;
    cjit_add_include_path(cjit, path);
    return cjit_result_ok();
}

static CJITResult add_library_path(void *context, RuntimeSession *session, const char *path)
{
    CJITState *cjit = state_from_context(context);
    (void)session;
    cjit_add_library_path(cjit, path);
    return cjit_result_ok();
}

static CJITResult set_options(void *context, RuntimeSession *session, const char *options)
{
    CJITState *cjit = state_from_context(context);
    (void)session;
    cjit_set_tcc_options(cjit, options);
    return cjit_result_ok();
}

static CJITResult output_file(void *context, RuntimeSession *session, const char *path)
{
    CJITState *cjit = state_from_context(context);
    (void)session;
    if (cjit->output_filename) {
        free(cjit->output_filename);
    }
    cjit->output_filename = NULL;
    if (path) {
        cjit->output_filename = malloc(strlen(path) + 1);
        strcpy(cjit->output_filename, path);
    }
    return cjit_result_ok();
}

static CJITResult compile_object(void *context, RuntimeSession *session, const char *path)
{
    CJITState *cjit = state_from_context(context);
    (void)session;
    if (!cjit_compile_file(cjit, path)) {
        return cjit_result_error(CJIT_RESULT_COMPILER_ERROR, 1, "Compile to object failed");
    }
    return cjit_result_ok();
}

static CJITResult link_executable(void *context, RuntimeSession *session)
{
    CJITState *cjit = state_from_context(context);
    (void)session;
    if (cjit_link(cjit) < 0) {
        return cjit_result_error(CJIT_RESULT_LINK_ERROR, 1, "Error in linker compiling to file");
    }
    return cjit_result_ok();
}

static CJITResult execute_program(void *context, RuntimeSession *session,
                                  int argc, char **argv, int *exit_status)
{
    CJITState *cjit = state_from_context(context);
    (void)session;
    *exit_status = cjit_exec(cjit, argc, argv);
    return cjit_result_make((*exit_status == 0) ? CJIT_RESULT_OK : CJIT_RESULT_EXEC_ERROR,
                            *exit_status, (*exit_status == 0), NULL);
}

static CJITResult relocate(void *context, RuntimeSession *session)
{
    TCCState *compiler_handle;

    (void)context;
    compiler_handle = (TCCState *)session->compiler_handle;
#if defined(SHAREDTCC)
    if (tcc_relocate(compiler_handle, TCC_RELOCATE_AUTO) < 0) {
#else
    if (tcc_relocate(compiler_handle) < 0) {
#endif
        return cjit_result_error(CJIT_RESULT_LINK_ERROR, -1, "TCC linker error");
    }
    return cjit_result_ok();
}

static CJITResult resolve_symbol(void *context, RuntimeSession *session,
                                 const char *symbol_name, void **symbol)
{
    TCCState *compiler_handle;

    (void)context;
    compiler_handle = (TCCState *)session->compiler_handle;
    *symbol = tcc_get_symbol(compiler_handle, symbol_name);
    if (!*symbol) {
        return cjit_result_error(CJIT_RESULT_LINK_ERROR, -1, "Entrypoint symbol not found");
    }
    return cjit_result_ok();
}

static void end_session(void *context, RuntimeSession *session)
{
    CJITState *cjit = state_from_context(context);
    session->compiler_handle = cjit->TCC;
    session->tmpdir = cjit->tmpdir;
    session->tempdir_is_fresh = cjit->fresh;
    session->setup_complete = cjit->done_setup;
    session->execution_complete = cjit->done_exec;
}

const CompilerPort tinycc_compiler_port = {
    .context = NULL,
    .begin_session = begin_session,
    .configure_session = configure_session,
    .set_output_mode = set_output_mode,
    .add_source_file = add_source_file,
    .add_source_buffer = add_source_buffer,
    .add_binary_input = add_binary_input,
    .define_symbol = define_symbol,
    .add_include_path = add_include_path,
    .add_library_path = add_library_path,
    .set_options = set_options,
    .output_file = output_file,
    .compile_object = compile_object,
    .link_executable = link_executable,
    .execute_program = execute_program,
    .relocate = relocate,
    .resolve_symbol = resolve_symbol,
    .end_session = end_session
};
