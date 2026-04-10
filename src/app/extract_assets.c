#include "app/extract_assets.h"

ExtractAssetsResponse extract_assets_route(CJITState *cjit, const ExtractAssetsRequest *request)
{
    ExtractAssetsResponse response;
    if (!extract_assets(cjit, request->destination_path)) {
        response.result.code = CJIT_RESULT_IO_ERROR;
        response.result.exit_status = 1;
        response.result.ok = false;
        response.result.message = "Failed to extract runtime assets";
        response.destination_path = NULL;
        return response;
    }
    response.result.code = CJIT_RESULT_OK;
    response.result.exit_status = 0;
    response.result.ok = true;
    response.result.message = NULL;
    response.destination_path = cjit->tmpdir;
    return response;
}
