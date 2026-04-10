#ifndef CJIT_ADAPTERS_CLI_RENDER_RESPONSE_H
#define CJIT_ADAPTERS_CLI_RENDER_RESPONSE_H

#include "cjit.h"
#include "domain/responses.h"

void render_execute_response(CJITState *cjit, const ExecuteResponse *response);
void render_compile_object_response(CJITState *cjit, const CompileObjectResponse *response);
void render_build_executable_response(CJITState *cjit, const BuildExecutableResponse *response);
void render_status_response(CJITState *cjit, const StatusResponse *response);
void render_extract_assets_response(CJITState *cjit, const ExtractAssetsResponse *response);
void render_extract_archive_response(CJITState *cjit, const ExtractArchiveResponse *response);

#endif
