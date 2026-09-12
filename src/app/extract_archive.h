#ifndef CJIT_APP_EXTRACT_ARCHIVE_H
#define CJIT_APP_EXTRACT_ARCHIVE_H

#include "domain/requests.h"
#include "domain/responses.h"
#include "app/slice_dependencies.h"

ExtractArchiveResponse extract_archive_route(const ExtractArchiveRequest *request);
ExtractArchiveResponse extract_archive_with_dependencies(const ExtractArchiveRequest *request,
                                                         const SliceDependencies *dependencies);

#endif
