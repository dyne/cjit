#include "app/extract_assets.h"

ExtractAssetsResponse extract_assets_with_dependencies(CJITState *cjit,
                                                       const ExtractAssetsRequest *request,
                                                       const SliceDependencies *dependencies)
{
    ExtractAssetsResponse response;
    RuntimeSession session;
    AssetPort assets = dependencies->assets;
    char *resolved_path = NULL;
    session.compiler_handle = cjit->TCC;
    session.tmpdir = cjit->tmpdir;
    session.tempdir_is_fresh = cjit->fresh;
    session.setup_complete = cjit->done_setup;
    session.execution_complete = cjit->done_exec;
    if (!assets.extract_runtime_assets(assets.context, &session,
                                       request->destination_path, &resolved_path).ok) {
        response.result = cjit_result_error(CJIT_RESULT_IO_ERROR, 1,
                                            "Failed to extract runtime assets");
        response.destination_path = NULL;
        return response;
    }
    response.result = cjit_result_ok();
    response.destination_path = resolved_path;
    return response;
}
