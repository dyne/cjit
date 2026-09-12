#include "app/extract_archive.h"

#include <stdint.h>

ExtractArchiveResponse extract_archive_with_dependencies(const ExtractArchiveRequest *request,
                                                         const SliceDependencies *dependencies)
{
    ExtractArchiveResponse response;
    AssetPort assets = dependencies->assets;
    if (!assets.extract_archive_to_path(assets.context, request->archive_path, ".").ok) {
        response.result = cjit_result_error(CJIT_RESULT_IO_ERROR, 1,
                                            "Failed to extract archive");
        return response;
    }
    response.result = cjit_result_ok();
    return response;
}
