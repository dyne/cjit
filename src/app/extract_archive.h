#ifndef CJIT_APP_EXTRACT_ARCHIVE_H
#define CJIT_APP_EXTRACT_ARCHIVE_H

#include "domain/requests.h"
#include "domain/responses.h"

ExtractArchiveResponse extract_archive_route(const ExtractArchiveRequest *request);

#endif
