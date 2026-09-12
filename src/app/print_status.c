#include "app/print_status.h"

StatusResponse print_status_with_dependencies(CJITState *cjit, const StatusRequest *request,
                                              const SliceDependencies *dependencies)
{
    StatusResponse response;
    (void)request;
    dependencies->print_status(dependencies->status_context);
    response.result = cjit_result_ok();
    return response;
}
