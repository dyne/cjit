#ifndef CJIT_PORTS_ASSET_PORT_H
#define CJIT_PORTS_ASSET_PORT_H

#include "domain/error.h"
#include "domain/runtime_session.h"

/**
 * Asset and archive extraction operations.
 */
typedef struct AssetPort {
    void *context;
    CJITResult (*extract_runtime_assets)(void *context, RuntimeSession *session,
                                         const char *destination_path, char **resolved_path);
    CJITResult (*extract_archive_to_path)(void *context, const char *archive_path,
                                          const char *destination_path);
} AssetPort;

#endif
