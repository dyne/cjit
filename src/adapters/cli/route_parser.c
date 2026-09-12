#include "adapters/cli/route_parser.h"

#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define CLI_DEFINE_MAX_LENGTH 1024

int cli_parse_define_value(char *definition)
{
    size_t index;
    size_t equal_index = 0;
    bool found_equal = false;

    if (!definition) {
        return -1;
    }
    for (index = 0; definition[index] != '\0'; ++index) {
        unsigned char character = (unsigned char)definition[index];
        if (index >= CLI_DEFINE_MAX_LENGTH || (!isalnum(character) && character != '_' && character != '=')) {
            return -1;
        }
        if (character == '=') {
            if (found_equal) {
                return -1;
            }
            found_equal = true;
            equal_index = index;
        }
    }
    if (!found_equal) {
        return 0;
    }
    definition[equal_index] = '\0';
    return (int)equal_index + 1;
}

static bool cli_ignored_pattern_matches(const char *argument, const char *pattern,
                                        bool *takes_next_value)
{
    size_t length = strlen(pattern);

    *takes_next_value = false;
    if (length == 0) {
        return false;
    }
    if (pattern[length - 1] != ':') {
        return strcmp(argument, pattern) == 0;
    }
    --length;
    if (strncmp(argument, pattern, length) != 0) {
        return false;
    }
    if (argument[length] == '\0') {
        *takes_next_value = true;
        return true;
    }
    return isalnum((unsigned char)argument[length]);
}

char **cli_remove_ignored_arguments(int *argc, char **argv,
                                    const char *const *patterns,
                                    int pattern_count)
{
    int source_count;
    int output_count = 0;
    bool after_separator = false;
    char **filtered;

    if (!argc || !argv || *argc < 1 || !patterns || pattern_count < 0) {
        return NULL;
    }
    source_count = *argc;
    filtered = malloc(((size_t)source_count + 1) * sizeof(*filtered));
    if (!filtered) {
        return NULL;
    }
    for (int index = 0; index < source_count; ++index) {
        bool remove = false;
        bool takes_next_value = false;
        if (index > 0 && !after_separator) {
            for (int pattern_index = 0; pattern_index < pattern_count; ++pattern_index) {
                if (cli_ignored_pattern_matches(argv[index], patterns[pattern_index], &takes_next_value)) {
                    remove = true;
                    break;
                }
            }
        }
        if (remove) {
            if (takes_next_value && index + 1 < source_count) {
                ++index;
            }
            continue;
        }
        filtered[output_count++] = argv[index];
        if (strcmp(argv[index], "--") == 0) {
            after_separator = true;
        }
    }
    filtered[output_count] = NULL;
    *argc = output_count;
    return filtered;
}

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
                            int opt_ind, int arg_separator,
                            CliRoute forced_route, const char *forced_path)
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
    parsed.asset_destination_path = NULL;
    parsed.archive_path = NULL;

    if (forced_route == CLI_ROUTE_EXTRACT_ASSETS) {
        parsed.route = forced_route;
        parsed.asset_destination_path = forced_path;
    } else if (forced_route == CLI_ROUTE_EXTRACT_ARCHIVE) {
        parsed.route = forced_route;
        parsed.archive_path = forced_path;
    } else if (opt_ind >= argc && cjit->print_status) {
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

ExtractAssetsRequest build_extract_assets_request(const ParsedRoute *parsed)
{
    ExtractAssetsRequest request;

    request.destination_path = parsed->asset_destination_path;
    return request;
}

ExtractArchiveRequest build_extract_archive_request(const ParsedRoute *parsed)
{
    ExtractArchiveRequest request;

    request.archive_path = parsed->archive_path;
    return request;
}
