#ifndef CJIT_ADAPTERS_COMPILER_TINYCC_ADAPTER_H
#define CJIT_ADAPTERS_COMPILER_TINYCC_ADAPTER_H

#include "ports/compiler_port.h"

typedef struct TCCState TCCState;

extern const CompilerPort tinycc_compiler_port;

/* TinyCC calls used by the adapter.  Production uses the built-in table;
 * tests may install a complete temporary table to exercise terminal errors. */
typedef struct TinyccAdapterApi {
    int (*add_file)(TCCState *state, const char *path);
    int (*output_file)(TCCState *state, const char *path);
    int (*relocate)(TCCState *state, void *memory);
    void *(*get_symbol)(TCCState *state, const char *name);
} TinyccAdapterApi;

void tinycc_adapter_set_api_for_test(const TinyccAdapterApi *api);
void tinycc_adapter_reset_api_for_test(void);

#endif
