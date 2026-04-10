#ifndef CJIT_APP_EXECUTE_SOURCE_H
#define CJIT_APP_EXECUTE_SOURCE_H

#include "cjit.h"
#include "domain/requests.h"
#include "domain/responses.h"

/**
 * Execute the current request using the existing CJIT runtime.
 *
 * This is the first extraction step away from `main.c`: the CLI still
 * builds the request, but the use-case now owns source ingestion and
 * runtime execution sequencing.
 */
ExecuteResponse execute_source(CJITState *cjit, const ExecuteRequest *request);

#endif
