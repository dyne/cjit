#include "adapters/compiler/tinycc_adapter.h"

#include <stdlib.h>
#include <string.h>

#include "adapters/platform/library_resolver_posix.h"
#include "adapters/platform/library_resolver_windows.h"
#include "adapters/platform/runtime_platform.h"
#include "cjit.h"
#include "cwalk.h"
#include "libtcc.h"
#include "support/string_list.h"

static CJITState *state_from_context(void *context)
{
    return (CJITState *)context;
}

static int has_source_extension(const char *path)
{
    char *ext;
    size_t extlen;
    bool is_source;

    is_source = cwk_path_get_extension(path, (const char **)&ext, &extlen);
    if (!is_source) {
        return 0;
    }
    if (extlen == 2 && (ext[1] == 'c' || ext[1] == 'C')) {
        is_source = true;
    } else if (extlen == 3
               && (ext[1] == 'c' || ext[1] == 'C')
               && (ext[2] == 'c' || ext[2] == 'C')) {
        is_source = true;
    } else if (extlen == 4
               && (ext[1] == 'c' || ext[1] == 'C')
               && (ext[2] == 'x' || ext[2] == 'X')
               && (ext[3] == 'x' || ext[3] == 'X')) {
        is_source = true;
    } else {
        is_source = false;
    }
    return is_source ? 1 : -1;
}

static CJITResult relocate(void *context, RuntimeSession *session);
static CJITResult resolve_symbol(void *context, RuntimeSession *session,
                                 const char *symbol_name, void **symbol);

static int resolve_libraries(CJITState *cjit)
{
    LibraryResolverPort resolver;
    LibraryResolverRequest request;
    LibraryResolverResponse response;

    request.library_count = (int)string_list_count(cjit->libs);
    request.libraries = NULL;
    request.search_path_count = (int)string_list_count(cjit->libpaths);
    request.search_paths = NULL;
#if defined(WINDOWS)
    resolver = windows_library_resolver_port;
#else
    resolver = posix_library_resolver_port;
#endif
    resolver.context = cjit;
    if (!resolver.resolve(resolver.context, &request, &response).ok) {
        return 0;
    }
    return response.resolved_count;
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
    int is_source = has_source_extension(path);
    TCCState *compiler_handle = (TCCState *)session->compiler_handle;

    if (is_source == 0 || is_source < 0) {
        return cjit_result_error(CJIT_RESULT_INVALID_REQUEST, 1, "Compile to object failed");
    }
    if (!cjit_add_file(cjit, path)) {
        return cjit_result_error(CJIT_RESULT_COMPILER_ERROR, 1, "Compile to object failed");
    }
    if (cjit->output_filename) {
        if (tcc_output_file(compiler_handle, cjit->output_filename) < 0) {
            return cjit_result_error(CJIT_RESULT_COMPILER_ERROR, 1, "Compile to object failed");
        }
    } else {
        char *ext;
        char *tmp;
        const char *basename;
        size_t extlen;
        size_t len;

        cwk_path_get_basename((char *)path, &basename, &len);
        tmp = malloc(len + 2);
        strncpy(tmp, basename, len + 1);
        cwk_path_get_extension(tmp, (const char **)&ext, &extlen);
        strcpy(ext, ".o");
        if (tcc_output_file(compiler_handle, tmp) < 0) {
            free(tmp);
            return cjit_result_error(CJIT_RESULT_COMPILER_ERROR, 1, "Compile to object failed");
        }
        free(tmp);
    }
    return cjit_result_ok();
}

static CJITResult link_executable(void *context, RuntimeSession *session)
{
    CJITState *cjit = state_from_context(context);
    int found;
    TCCState *compiler_handle = (TCCState *)session->compiler_handle;

    if (!cjit->done_setup) {
        return cjit_result_error(CJIT_RESULT_LINK_ERROR, 1, "No source code found");
    }
    if (!cjit->output_filename) {
        return cjit_result_error(CJIT_RESULT_LINK_ERROR, 1, "No output file configured");
    }
    found = resolve_libraries(cjit);
    for (int i = 0; i < found; ++i) {
        char *resolved_path = string_list_get(cjit->reallibs, i);
        if (resolved_path) {
            tcc_add_file(compiler_handle, resolved_path);
        }
    }
    if (tcc_output_file(compiler_handle, cjit->output_filename) < 0) {
        return cjit_result_error(CJIT_RESULT_LINK_ERROR, 1, "Error in linker compiling to file");
    }
    return cjit_result_ok();
}

static CJITResult execute_program(void *context, RuntimeSession *session,
                                  int argc, char **argv, int *exit_status)
{
    CJITState *cjit = state_from_context(context);
    int found;
    int (*entrypoint)(int, char **);
    CJITResult result;

    if (!cjit->done_setup) {
        return cjit_result_error(CJIT_RESULT_INVALID_REQUEST, 1, "No source code found");
    }
    if (cjit->done_exec) {
        return cjit_result_error(CJIT_RESULT_EXEC_ERROR, 1, "CJIT already executed once");
    }

    found = resolve_libraries(cjit);
    for (int i = 0; i < found; ++i) {
        char *resolved_path = string_list_get(cjit->reallibs, i);
        if (resolved_path) {
            tcc_add_file((TCCState *)session->compiler_handle, resolved_path);
        }
    }

    result = relocate(context, session);
    if (!result.ok) {
        return result;
    }
    result = resolve_symbol(context, session, cjit->entry ? cjit->entry : "main",
                            (void **)&entrypoint);
    if (!result.ok) {
        return result;
    }

    *exit_status = cjit_platform_exec(cjit, entrypoint, argc, argv);
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
