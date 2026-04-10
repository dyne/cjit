#ifndef CJIT_APP_PRINT_STATUS_H
#define CJIT_APP_PRINT_STATUS_H

#include "cjit.h"
#include "domain/requests.h"
#include "domain/responses.h"

StatusResponse print_status(CJITState *cjit, const StatusRequest *request);

#endif
