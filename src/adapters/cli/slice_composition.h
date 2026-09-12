#ifndef CJIT_ADAPTERS_CLI_SLICE_COMPOSITION_H
#define CJIT_ADAPTERS_CLI_SLICE_COMPOSITION_H

#include "app/slice_dependencies.h"
#include "cjit.h"

/* CLI composition root for the concrete route adapters. */
SliceDependencies cjit_default_slice_dependencies(CJITState *cjit);

#endif
