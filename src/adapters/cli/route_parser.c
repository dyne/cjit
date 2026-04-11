#include "adapters/cli/route_parser.h"

#include <stddef.h>

/**
 * Copies the CLI-owned common options into a route request payload.
 */
CJITCommonOptions build_cli_common_options(const CJITState *cjit)
{
    CJITCommonOptions options;

    options.quiet = cjit->quiet;
    options.verbose = cjit->verbose;
    options.print_status = cjit->print_status;
    options.entry = cjit->entry;
    options.pid_file = cjit->write_pid;
    options.output_path = cjit->output_filename;
    return options;
}

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

StatusRequest build_status_request(const CJITState *cjit)
{
    StatusRequest request;

    request.verbose = cjit->verbose;
    return request;
}

CompileObjectRequest build_compile_object_request(const CJITState *cjit,
                                                  const ParsedRoute *parsed)
{
    CompileObjectRequest request;

    request.options = build_cli_common_options(cjit);
    request.source_path = (parsed->source_count == 1) ? parsed->sources[0] : NULL;
    return request;
}

BuildExecutableRequest build_build_executable_request(const CJITState *cjit,
                                                      const ParsedRoute *parsed)
{
    BuildExecutableRequest request;

    request.options = build_cli_common_options(cjit);
    request.source_count = parsed->source_count;
    request.sources = parsed->sources;
    return request;
}

ExecuteRequest build_execute_request(const CJITState *cjit,
                                     const ParsedRoute *parsed)
{
    ExecuteRequest request;

    request.options = build_cli_common_options(cjit);
    request.source_count = parsed->source_count;
    request.sources = parsed->sources;
    request.app_argc = parsed->app_argc;
    request.app_argv = parsed->app_argv;
    return request;
}
