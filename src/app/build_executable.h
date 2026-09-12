#ifndef CJIT_APP_BUILD_EXECUTABLE_H
#define CJIT_APP_BUILD_EXECUTABLE_H

#include "cjit.h"
#include "domain/requests.h"
#include "domain/responses.h"
#include "app/slice_dependencies.h"

/**
 * Build an executable file without running it.
 */
BuildExecutableResponse build_executable(CJITState *cjit, const BuildExecutableRequest *request);
BuildExecutableResponse build_executable_with_dependencies(CJITState *cjit,
                                                           const BuildExecutableRequest *request,
                                                           const SliceDependencies *dependencies);

#endif
