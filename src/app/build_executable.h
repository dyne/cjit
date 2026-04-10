#ifndef CJIT_APP_BUILD_EXECUTABLE_H
#define CJIT_APP_BUILD_EXECUTABLE_H

#include "cjit.h"
#include "domain/requests.h"
#include "domain/responses.h"

/**
 * Build an executable file without running it.
 */
BuildExecutableResponse build_executable(CJITState *cjit, const BuildExecutableRequest *request);

#endif
