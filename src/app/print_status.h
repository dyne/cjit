#ifndef CJIT_APP_PRINT_STATUS_H
#define CJIT_APP_PRINT_STATUS_H

#include "cjit.h"
#include "domain/requests.h"
#include "domain/responses.h"
#include "app/slice_dependencies.h"

StatusResponse print_status(CJITState *cjit, const StatusRequest *request);
StatusResponse print_status_with_dependencies(CJITState *cjit, const StatusRequest *request,
                                              const SliceDependencies *dependencies);

#endif
