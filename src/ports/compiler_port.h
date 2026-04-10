#ifndef CJIT_PORTS_COMPILER_PORT_H
#define CJIT_PORTS_COMPILER_PORT_H

#include <stdbool.h>

#include "domain/error.h"
#include "domain/runtime_session.h"

/**
 * Compiler adapter operations.
 *
 * Implementations own the concrete compiler handle lifecycle and expose
 * a narrow contract to application slices.
 */
typedef struct CompilerPort {
    void *context;
    CJITResult (*begin_session)(void *context, RuntimeSession *session);
    CJITResult (*configure_session)(void *context, RuntimeSession *session);
    CJITResult (*set_output_mode)(void *context, RuntimeSession *session, int output_mode);
    CJITResult (*add_source_file)(void *context, RuntimeSession *session, const char *path);
    CJITResult (*add_source_buffer)(void *context, RuntimeSession *session, const char *buffer);
    CJITResult (*add_binary_input)(void *context, RuntimeSession *session, const char *path);
    CJITResult (*define_symbol)(void *context, RuntimeSession *session,
                                const char *name, const char *value);
    CJITResult (*add_include_path)(void *context, RuntimeSession *session, const char *path);
    CJITResult (*add_library_path)(void *context, RuntimeSession *session, const char *path);
    CJITResult (*set_options)(void *context, RuntimeSession *session, const char *options);
    CJITResult (*output_file)(void *context, RuntimeSession *session, const char *path);
    CJITResult (*relocate)(void *context, RuntimeSession *session);
    CJITResult (*resolve_symbol)(void *context, RuntimeSession *session,
                                 const char *symbol_name, void **symbol);
    void (*end_session)(void *context, RuntimeSession *session);
} CompilerPort;

#endif
