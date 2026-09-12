#ifndef CJIT_APP_SLICE_DEPENDENCIES_H
#define CJIT_APP_SLICE_DEPENDENCIES_H

#include "ports/asset_port.h"
#include "ports/compiler_port.h"
#include "ports/filesystem_port.h"

/*
 * Dependencies are borrowed for the duration of one route invocation.  The
 * caller owns both the port contexts and any paths returned by an asset port.
 */
typedef struct SliceDependencies {
    CompilerPort compiler;
    FilesystemPort filesystem;
    AssetPort assets;
    void (*print_status)(void *context);
    void *status_context;
} SliceDependencies;

#endif
