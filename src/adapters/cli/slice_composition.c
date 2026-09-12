#include "adapters/cli/slice_composition.h"

#include "adapters/compiler/tinycc_adapter.h"
#include "adapters/fs/local_asset.h"
#include "adapters/fs/local_filesystem.h"
#include "app/build_executable.h"
#include "app/compile_object.h"
#include "app/execute_source.h"
#include "app/extract_archive.h"
#include "app/extract_assets.h"
#include "app/print_status.h"

static void print_status_adapter(void *context)
{
    cjit_status((CJITState *)context);
}

SliceDependencies cjit_default_slice_dependencies(CJITState *cjit)
{
    SliceDependencies dependencies;
    dependencies.compiler = tinycc_compiler_port;
    dependencies.filesystem = local_filesystem_port;
    dependencies.assets = local_asset_port;
    dependencies.compiler.context = cjit;
    dependencies.filesystem.context = cjit;
    dependencies.assets.context = cjit;
    dependencies.print_status = print_status_adapter;
    dependencies.status_context = cjit;
    return dependencies;
}

ExecuteResponse execute_source(CJITState *cjit, const ExecuteRequest *request)
{ SliceDependencies d = cjit_default_slice_dependencies(cjit); return execute_source_with_dependencies(cjit, request, &d); }
CompileObjectResponse compile_object(CJITState *cjit, const CompileObjectRequest *request)
{ SliceDependencies d = cjit_default_slice_dependencies(cjit); return compile_object_with_dependencies(cjit, request, &d); }
BuildExecutableResponse build_executable(CJITState *cjit, const BuildExecutableRequest *request)
{ SliceDependencies d = cjit_default_slice_dependencies(cjit); return build_executable_with_dependencies(cjit, request, &d); }
StatusResponse print_status(CJITState *cjit, const StatusRequest *request)
{ SliceDependencies d = cjit_default_slice_dependencies(cjit); return print_status_with_dependencies(cjit, request, &d); }
ExtractAssetsResponse extract_assets_route(CJITState *cjit, const ExtractAssetsRequest *request)
{ SliceDependencies d = cjit_default_slice_dependencies(cjit); return extract_assets_with_dependencies(cjit, request, &d); }
ExtractArchiveResponse extract_archive_route(const ExtractArchiveRequest *request)
{ SliceDependencies d = cjit_default_slice_dependencies(NULL); return extract_archive_with_dependencies(request, &d); }
