#include "app/print_status.h"

StatusResponse print_status(CJITState *cjit, const StatusRequest *request)
{
    StatusResponse response;
    (void)request;
    cjit_status(cjit);
    response.result = cjit_result_ok();
    return response;
}
