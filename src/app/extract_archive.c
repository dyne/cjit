#include "app/extract_archive.h"

#include <stdint.h>

#include "cjit.h"
#include "muntar.h"

ExtractArchiveResponse extract_archive_route(const ExtractArchiveRequest *request)
{
    ExtractArchiveResponse response;
    unsigned int len = 0;
    const uint8_t *targz;

    targz = (const uint8_t *)file_load(request->archive_path, &len);
    if (!targz || !len) {
        response.result.code = CJIT_RESULT_IO_ERROR;
        response.result.exit_status = 1;
        response.result.ok = false;
        response.result.message = "Failed to load archive";
        return response;
    }
    if (muntargz_to_path(".", targz, len) != 0) {
        response.result.code = CJIT_RESULT_IO_ERROR;
        response.result.exit_status = 1;
        response.result.ok = false;
        response.result.message = "Failed to extract archive";
        return response;
    }
    response.result.code = CJIT_RESULT_OK;
    response.result.exit_status = 0;
    response.result.ok = true;
    response.result.message = NULL;
    return response;
}
