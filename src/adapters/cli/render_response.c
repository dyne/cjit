#include "adapters/cli/render_response.h"

void render_execute_response(CJITState *cjit, const ExecuteResponse *response)
{
    (void)cjit;
    if (!response->result.ok && response->result.message) {
        _err("%s", response->result.message);
    }
}

void render_compile_object_response(CJITState *cjit, const CompileObjectResponse *response)
{
    (void)cjit;
    if (!response->result.ok && response->result.message) {
        _err("%s", response->result.message);
    }
}

void render_build_executable_response(CJITState *cjit, const BuildExecutableResponse *response)
{
    if (!response->result.ok && response->result.message) {
        _err("%s: %s", response->result.message,
             cjit->output_filename ? cjit->output_filename : "(null)");
    }
}

void render_status_response(CJITState *cjit, const StatusResponse *response)
{
    (void)cjit;
    (void)response;
}

void render_extract_assets_response(CJITState *cjit, const ExtractAssetsResponse *response)
{
    (void)cjit;
    if (!response->result.ok && response->result.message) {
        _err("%s", response->result.message);
        return;
    }
    if (response->destination_path) {
        _out("%s", response->destination_path);
    }
}

void render_extract_archive_response(CJITState *cjit, const ExtractArchiveResponse *response)
{
    (void)cjit;
    if (!response->result.ok && response->result.message) {
        _err("%s", response->result.message);
    }
}
