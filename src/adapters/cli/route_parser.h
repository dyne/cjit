#ifndef CJIT_ADAPTERS_CLI_ROUTE_PARSER_H
#define CJIT_ADAPTERS_CLI_ROUTE_PARSER_H

#include <stdbool.h>

#include "cjit.h"
#include "domain/requests.h"

typedef enum CliRoute {
    CLI_ROUTE_NONE = -1,
    CLI_ROUTE_EXECUTE = 0,
    CLI_ROUTE_COMPILE_OBJECT,
    CLI_ROUTE_BUILD_EXECUTABLE,
    CLI_ROUTE_PRINT_STATUS,
    CLI_ROUTE_EXTRACT_ASSETS,
    CLI_ROUTE_EXTRACT_ARCHIVE
} CliRoute;

typedef struct ParsedRoute {
    CliRoute route;
    int left_args;
    int source_count;
    int app_argc;
    char **app_argv;
    const char **sources;
    const char *asset_destination_path;
    const char *archive_path;
} ParsedRoute;

/*
 * Validates a -D argument and, on success, splits its optional single '=' in
 * place.  The input remains unchanged on failure.  A return value of zero is
 * a symbol-only definition, a positive value is the value offset, and -1 is
 * invalid.  Callers retain ownership of definition.
 */
int cli_parse_define_value(char *definition);

/*
 * Returns a newly allocated, NULL-terminated argv vector containing borrowed
 * argument pointers.  argv[0] and all arguments following "--" are retained.
 * On success *argc is updated; the caller frees only the returned vector.
 */
char **cli_remove_ignored_arguments(int *argc, char **argv,
                                    const char *const *patterns,
                                    int pattern_count);

/* ParsedRoute and all request builders borrow pointers from argv and cjit. */
ParsedRoute parse_cli_route(CJITState *cjit, int argc, char **argv,
                            int opt_ind, int arg_separator,
                            CliRoute forced_route, const char *forced_path);
CJITCommonOptions build_cli_common_options(const CJITState *cjit);
StatusRequest build_status_request(const CJITState *cjit);
CompileObjectRequest build_compile_object_request(const CJITState *cjit,
                                                  const ParsedRoute *parsed);
BuildExecutableRequest build_build_executable_request(const CJITState *cjit,
                                                      const ParsedRoute *parsed);
ExecuteRequest build_execute_request(const CJITState *cjit,
                                     const ParsedRoute *parsed);
ExtractAssetsRequest build_extract_assets_request(const ParsedRoute *parsed);
ExtractArchiveRequest build_extract_archive_request(const ParsedRoute *parsed);

#endif
