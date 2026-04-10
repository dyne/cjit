#include "app/extract_assets.h"

#include "adapters/fs/local_asset.h"

ExtractAssetsResponse extract_assets_route(CJITState *cjit, const ExtractAssetsRequest *request)
{
    ExtractAssetsResponse response;
    RuntimeSession session;
    AssetPort assets = local_asset_port;
    char *resolved_path = NULL;
    assets.context = cjit;
    session.compiler_handle = cjit->TCC;
    session.tmpdir = cjit->tmpdir;
    session.tempdir_is_fresh = cjit->fresh;
    session.setup_complete = cjit->done_setup;
    session.execution_complete = cjit->done_exec;
    if (!assets.extract_runtime_assets(assets.context, &session,
                                       request->destination_path, &resolved_path).ok) {
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
    response.destination_path = resolved_path;
    return response;
}
