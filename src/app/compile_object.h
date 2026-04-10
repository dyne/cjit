#ifndef CJIT_APP_COMPILE_OBJECT_H
#define CJIT_APP_COMPILE_OBJECT_H

#include "cjit.h"
#include "domain/requests.h"
#include "domain/responses.h"

/**
 * Compile one source file to an object file.
 */
CompileObjectResponse compile_object(CJITState *cjit, const CompileObjectRequest *request);

#endif
