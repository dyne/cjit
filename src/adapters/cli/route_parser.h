#ifndef CJIT_ADAPTERS_CLI_ROUTE_PARSER_H
#define CJIT_ADAPTERS_CLI_ROUTE_PARSER_H

#include <stdbool.h>

#include "cjit.h"
#include "domain/requests.h"

typedef enum CliRoute {
    CLI_ROUTE_EXECUTE = 0,
    CLI_ROUTE_COMPILE_OBJECT,
    CLI_ROUTE_BUILD_EXECUTABLE,
    CLI_ROUTE_PRINT_STATUS
} CliRoute;

typedef struct ParsedRoute {
    CliRoute route;
    int left_args;
    int source_count;
    int app_argc;
    char **app_argv;
    const char **sources;
} ParsedRoute;

ParsedRoute parse_cli_route(CJITState *cjit, int argc, char **argv,
                            int opt_ind, int arg_separator);
CJITCommonOptions build_cli_common_options(const CJITState *cjit);
StatusRequest build_status_request(const CJITState *cjit);
CompileObjectRequest build_compile_object_request(const CJITState *cjit,
                                                  const ParsedRoute *parsed);
BuildExecutableRequest build_build_executable_request(const CJITState *cjit,
                                                      const ParsedRoute *parsed);
ExecuteRequest build_execute_request(const CJITState *cjit,
                                     const ParsedRoute *parsed);

#endif
