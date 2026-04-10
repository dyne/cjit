#ifndef CJIT_PORTS_PROCESS_PORT_H
#define CJIT_PORTS_PROCESS_PORT_H

#include "domain/error.h"

/**
 * Process/runtime execution operations.
 *
 * The symbol pointer is opaque to the application layer and comes from
 * the compiler adapter after relocation.
 */
typedef struct ProcessPort {
    void *context;
    CJITResult (*write_pid_file)(void *context, const char *path, int pid);
    CJITResult (*execute_entrypoint)(void *context, void *entrypoint,
                                     int argc, char **argv, int *exit_status);
} ProcessPort;

#endif
