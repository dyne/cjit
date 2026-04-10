#ifndef CJIT_APP_EXTRACT_ASSETS_H
#define CJIT_APP_EXTRACT_ASSETS_H

#include "cjit.h"
#include "domain/requests.h"
#include "domain/responses.h"

ExtractAssetsResponse extract_assets_route(CJITState *cjit, const ExtractAssetsRequest *request);

#endif
