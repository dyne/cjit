#include "adapters/cli/route_parser.h"

#include <stddef.h>

ParsedRoute parse_cli_route(CJITState *cjit, int argc, char **argv,
                            int opt_ind, int arg_separator)
{
    ParsedRoute parsed;

    parsed.left_args = arg_separator ? arg_separator : argc;
    parsed.source_count = parsed.left_args - opt_ind;
    if (parsed.source_count < 0) {
        parsed.source_count = 0;
    }
    parsed.sources = parsed.source_count > 0 ? (const char **)&argv[opt_ind] : NULL;
    parsed.app_argc = argc - parsed.left_args + 1;
    parsed.app_argv = &argv[parsed.left_args - 1];

    if (opt_ind >= argc && cjit->print_status) {
        parsed.route = CLI_ROUTE_PRINT_STATUS;
    } else if (cjit->tcc_output == OBJ) {
        parsed.route = CLI_ROUTE_COMPILE_OBJECT;
    } else if (cjit->output_filename) {
        parsed.route = CLI_ROUTE_BUILD_EXECUTABLE;
    } else {
        parsed.route = CLI_ROUTE_EXECUTE;
    }

    return parsed;
}
