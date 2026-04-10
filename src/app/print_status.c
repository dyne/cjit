#include "app/print_status.h"

StatusResponse print_status(CJITState *cjit, const StatusRequest *request)
{
    StatusResponse response;
    (void)request;
    cjit_status(cjit);
    response.result.code = CJIT_RESULT_OK;
    response.result.exit_status = 0;
    response.result.ok = true;
    response.result.message = NULL;
    return response;
}
