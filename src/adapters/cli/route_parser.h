#ifndef CJIT_ADAPTERS_CLI_ROUTE_PARSER_H
#define CJIT_ADAPTERS_CLI_ROUTE_PARSER_H

#include <stdbool.h>

#include "cjit.h"

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

#endif
