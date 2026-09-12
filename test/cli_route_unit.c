#include <stdio.h>
#include <string.h>

#include "adapters/cli/ketopt.h"
#include "adapters/cli/route_parser.h"

static int failures;

static void expect_route(const char *name, CJITState *state, int argc, char **argv,
                         int opt_ind, int separator, CliRoute forced, const char *path,
                         CliRoute expected_route, int expected_sources, int expected_app_argc)
{
    ParsedRoute parsed = parse_cli_route(state, argc, argv, opt_ind, separator, forced, path);
    if (parsed.route != expected_route || parsed.source_count != expected_sources ||
        parsed.app_argc != expected_app_argc) {
        fprintf(stderr, "%s: route %d sources %d app %d\n", name, parsed.route,
                parsed.source_count, parsed.app_argc);
        ++failures;
    }
    if (forced == CLI_ROUTE_EXTRACT_ASSETS && parsed.asset_destination_path != path) {
        ++failures;
    }
    if (forced == CLI_ROUTE_EXTRACT_ARCHIVE && parsed.archive_path != path) {
        ++failures;
    }
}

int main(void)
{
    CJITState state = { 0 };
    char *no_source[] = { "cjit" };
    char *one_source[] = { "cjit", "one.c" };
    char *two_sources[] = { "cjit", "one.c", "two.c" };
    char *with_separator[] = { "cjit", "one.c", "--", "--verb", "-q" };
    ParsedRoute parsed;
    ExecuteRequest execute;
    CompileObjectRequest compile;
    BuildExecutableRequest build;
    StatusRequest status;
    ExtractAssetsRequest assets;
    ExtractArchiveRequest archive;
    ketopt_t options = KETOPT_INIT;
    int option;
    int separator = 0;

    expect_route("default stdin", &state, 1, no_source, 1, 0, CLI_ROUTE_NONE, NULL,
                 CLI_ROUTE_EXECUTE, 0, 1);
    expect_route("default source", &state, 2, one_source, 1, 0, CLI_ROUTE_NONE, NULL,
                 CLI_ROUTE_EXECUTE, 1, 1);
    while ((option = ketopt(&options, 5, with_separator, 1, "q", NULL)) >= 0) {
        if (option == '-') {
            separator = options.ind + 1;
            break;
        }
    }
    expect_route("separator", &state, 5, with_separator, options.ind, separator,
                 CLI_ROUTE_NONE, NULL, CLI_ROUTE_EXECUTE, 1, 3);
    expect_route("assets", &state, 2, one_source, 1, 0, CLI_ROUTE_EXTRACT_ASSETS, "assets",
                 CLI_ROUTE_EXTRACT_ASSETS, 1, 1);
    expect_route("archive", &state, 2, one_source, 1, 0, CLI_ROUTE_EXTRACT_ARCHIVE, "archive.tgz",
                 CLI_ROUTE_EXTRACT_ARCHIVE, 1, 1);
    state.print_status = true;
    expect_route("status only", &state, 1, no_source, 1, 0, CLI_ROUTE_NONE, NULL,
                 CLI_ROUTE_PRINT_STATUS, 0, 1);
    expect_route("status with source", &state, 2, one_source, 1, 0, CLI_ROUTE_NONE, NULL,
                 CLI_ROUTE_EXECUTE, 1, 1);
    state.verbose = true;
    status = build_status_request(&state);
    if (!status.verbose) ++failures;
    state.print_status = false;
    state.tcc_output = OBJ;
    expect_route("compile object", &state, 2, one_source, 1, 0, CLI_ROUTE_NONE, NULL,
                 CLI_ROUTE_COMPILE_OBJECT, 1, 1);
    state.output_filename = "out.o";
    expect_route("compile beats output", &state, 3, two_sources, 1, 0, CLI_ROUTE_NONE, NULL,
                 CLI_ROUTE_COMPILE_OBJECT, 2, 1);
    parsed = parse_cli_route(&state, 3, two_sources, 1, 0, CLI_ROUTE_NONE, NULL);
    compile = build_compile_object_request(&state, &parsed);
    if (compile.source_path != NULL) ++failures;
    state.tcc_output = 0;
    expect_route("build executable", &state, 3, two_sources, 1, 0, CLI_ROUTE_NONE, NULL,
                 CLI_ROUTE_BUILD_EXECUTABLE, 2, 1);
    parsed = parse_cli_route(&state, 3, two_sources, 1, 0, CLI_ROUTE_NONE, NULL);
    build = build_build_executable_request(&state, &parsed);
    if (build.source_count != 2 || strcmp(build.sources[1], "two.c") != 0) ++failures;
    parsed = parse_cli_route(&state, 5, with_separator, options.ind, separator,
                             CLI_ROUTE_NONE, NULL);
    execute = build_execute_request(&state, &parsed);
    if (execute.sources != (const char **)&with_separator[options.ind] ||
        execute.app_argc != 3 || strcmp(execute.app_argv[0], "one.c") != 0 ||
        strcmp(execute.app_argv[2], "-q") != 0) ++failures;
    parsed = parse_cli_route(&state, 2, one_source, 1, 0, CLI_ROUTE_EXTRACT_ASSETS, "assets");
    assets = build_extract_assets_request(&parsed);
    if (assets.destination_path != parsed.asset_destination_path) ++failures;
    parsed = parse_cli_route(&state, 2, one_source, 1, 0, CLI_ROUTE_EXTRACT_ARCHIVE, "archive.tgz");
    archive = build_extract_archive_request(&parsed);
    if (archive.archive_path != parsed.archive_path) ++failures;
    return failures ? 1 : 0;
}
