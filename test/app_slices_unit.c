#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app/build_executable.h"
#include "app/compile_object.h"
#include "app/execute_source.h"
#include "app/extract_archive.h"
#include "app/extract_assets.h"
#include "app/print_status.h"

typedef struct Fake {
    int begin, end, file, buffer, compile, link, execute, stdin_reads, status, assets, archive;
    int fail_at, program_status;
    char events[64];
    size_t event_count;
} Fake;

static void event(Fake *fake, char value)
{
    fake->events[fake->event_count++] = value;
    fake->events[fake->event_count] = '\0';
}

void _err(const char *format, ...)
{
    (void)format;
}

static CJITResult result(int ok)
{
    return ok ? cjit_result_ok() : cjit_result_error(CJIT_RESULT_IO_ERROR, 9, "fake");
}
static CJITResult begin(void *context, RuntimeSession *session)
{ Fake *f = context; (void)session; f->begin++; event(f, 'B'); return result(1); }
static void end(void *context, RuntimeSession *session)
{ Fake *f = context; (void)session; f->end++; event(f, 'E'); }
static CJITResult file(void *context, RuntimeSession *session, const char *path)
{ Fake *f = context; (void)session; (void)path; event(f, 'F'); return result(++f->file != f->fail_at); }
static CJITResult buffer(void *context, RuntimeSession *session, const char *contents)
{ Fake *f = context; (void)session; (void)contents; event(f, 'U'); return result(++f->buffer != f->fail_at); }
static CJITResult compile(void *context, RuntimeSession *session, const char *path)
{ Fake *f = context; (void)session; (void)path; f->compile++; event(f, 'C'); return result(f->fail_at != -1); }
static CJITResult link(void *context, RuntimeSession *session)
{ Fake *f = context; (void)session; f->link++; event(f, 'L'); return result(f->fail_at != -2); }
static CJITResult execute(void *context, RuntimeSession *session, int argc, char **argv, int *status)
{ Fake *f = context; (void)session; (void)argc; (void)argv; f->execute++; event(f, 'X'); *status = f->program_status; return f->fail_at == -3 ? result(0) : cjit_result_make(CJIT_RESULT_OK, *status, true, NULL); }
static CJITResult stdin_read(void *context, char **contents, size_t *length)
{ Fake *f = context; (void)length; f->stdin_reads++; event(f, 'R'); if (f->fail_at == -4) return result(0); *contents = strdup("int main(void){return 0;}"); return result(1); }
static CJITResult assets(void *context, RuntimeSession *session, const char *destination, char **resolved)
{ Fake *f = context; (void)session; (void)destination; f->assets++; if (f->fail_at == -5) return result(0); *resolved = strdup("owned-result"); return result(1); }
static CJITResult archive(void *context, const char *input, const char *destination)
{ Fake *f = context; (void)input; (void)destination; f->archive++; return result(f->fail_at != -6); }
static void status(void *context) { Fake *f = context; f->status++; event(f, 'S'); }

static SliceDependencies dependencies(Fake *fake)
{
    SliceDependencies d = { 0 };
    d.compiler.context = fake; d.compiler.begin_session = begin; d.compiler.end_session = end;
    d.compiler.add_source_file = file; d.compiler.add_source_buffer = buffer;
    d.compiler.compile_object = compile; d.compiler.link_executable = link; d.compiler.execute_program = execute;
    d.filesystem.context = fake; d.filesystem.read_stdin = stdin_read;
    d.assets.context = fake; d.assets.extract_runtime_assets = assets; d.assets.extract_archive_to_path = archive;
    d.print_status = status; d.status_context = fake;
    return d;
}
static int expect(int condition, const char *name)
{ if (!condition) fprintf(stderr, "failed: %s\n", name); return !condition; }
static int expect_events(const Fake *fake, const char *expected, const char *name)
{ return expect(strcmp(fake->events, expected) == 0, name); }

int main(void)
{
    CJITState state = { 0 };
    Fake f = { 0 };
    SliceDependencies d = dependencies(&f);
    const char *two[] = { "one.c", "two.c" };
    const char *stdin_source[] = { "-" };
    char *app[] = { "one.c", NULL };
    ExecuteRequest execute_request = { { 0 }, 2, two, 1, app };
    CompileObjectRequest compile_request = { { 0 }, "one.c" };
    BuildExecutableRequest build_request = { { 0 }, 2, two };
    ExtractAssetsRequest assets_request = { "destination" };
    ExtractArchiveRequest archive_request = { "archive.tgz" };
    StatusRequest status_request = { false };
    ExecuteResponse execute_response;
    CompileObjectResponse compile_response;
    BuildExecutableResponse build_response;
    ExtractAssetsResponse assets_response;
    ExtractArchiveResponse archive_response;
    int failures = 0;

    execute_response = execute_source_with_dependencies(&state, &execute_request, &d);
    failures += expect(execute_response.result.ok && f.begin == 1 && f.file == 2 && f.execute == 1 && f.end == 1, "execute success counts");
    failures += expect_events(&f, "BFFXE", "execute success order");
    memset(&f, 0, sizeof(f)); f.fail_at = 2;
    execute_response = execute_source_with_dependencies(&state, &execute_request, &d);
    failures += expect(!execute_response.result.ok && execute_response.result.code == CJIT_RESULT_COMPILER_ERROR && f.end == 1 && f.execute == 0, "execute input failure cleanup");
    failures += expect_events(&f, "BFFE", "execute input failure order");
    memset(&f, 0, sizeof(f)); execute_request.source_count = 0;
    execute_response = execute_source_with_dependencies(&state, &execute_request, &d);
#if defined(_WIN32)
    failures += expect(!execute_response.result.ok && execute_response.result.code == CJIT_RESULT_INVALID_REQUEST && f.stdin_reads == 0 && f.end == 1, "Windows rejects no-file stdin");
    failures += expect_events(&f, "BE", "Windows no-file stdin cleanup");
#else
    failures += expect(execute_response.result.ok && f.stdin_reads == 1 && f.buffer == 1 && f.end == 1, "stdin execution ownership");
    failures += expect_events(&f, "BRUXE", "zero-source stdin order");
#endif
    memset(&f, 0, sizeof(f)); execute_request.source_count = 1; execute_request.sources = stdin_source;
    execute_response = execute_source_with_dependencies(&state, &execute_request, &d);
#if defined(_WIN32)
    failures += expect(!execute_response.result.ok && execute_response.result.code == CJIT_RESULT_INVALID_REQUEST && f.stdin_reads == 0 && f.end == 1, "Windows rejects explicit stdin");
    failures += expect_events(&f, "BE", "Windows explicit stdin cleanup");
#else
    failures += expect(execute_response.result.ok && f.stdin_reads == 1 && f.buffer == 1, "explicit stdin source");
    failures += expect_events(&f, "BRUXE", "explicit stdin order");
#endif
    execute_request.source_count = 2; execute_request.sources = two;
    memset(&f, 0, sizeof(f)); f.program_status = 42;
    execute_response = execute_source_with_dependencies(&state, &execute_request, &d);
    failures += expect(execute_response.result.ok && execute_response.result.exit_status == 42, "nonzero program status preserved");
    failures += expect_events(&f, "BFFXE", "nonzero program status order");
    memset(&f, 0, sizeof(f)); f.fail_at = -3; f.program_status = 42;
    execute_response = execute_source_with_dependencies(&state, &execute_request, &d);
    failures += expect(!execute_response.result.ok && f.execute == 1 && f.end == 1, "execute failure result propagation");
    failures += expect_events(&f, "BFFXE", "execute failure order");
    memset(&f, 0, sizeof(f)); f.fail_at = -4; execute_request.source_count = 0;
    execute_response = execute_source_with_dependencies(&state, &execute_request, &d);
#if defined(_WIN32)
    failures += expect(!execute_response.result.ok && execute_response.result.code == CJIT_RESULT_INVALID_REQUEST && f.stdin_reads == 0 && f.end == 1, "Windows no-file input never reads stdin");
    failures += expect_events(&f, "BE", "Windows no-file stdin error cleanup");
#else
    failures += expect(!execute_response.result.ok && execute_response.result.code == CJIT_RESULT_IO_ERROR && f.end == 1, "stdin read failure cleanup");
    failures += expect_events(&f, "BRE", "stdin read failure order");
#endif
    execute_request.source_count = 2;
    memset(&f, 0, sizeof(f)); compile_request.source_path = NULL;
    compile_response = compile_object_with_dependencies(&state, &compile_request, &d);
    failures += expect(!compile_response.result.ok && f.begin == 1 && f.end == 1, "compile missing source cleanup");
    failures += expect_events(&f, "BE", "compile missing source order");
    memset(&f, 0, sizeof(f)); compile_request.source_path = "one.c"; compile_request.options.print_status = true;
    compile_response = compile_object_with_dependencies(&state, &compile_request, &d);
    failures += expect(compile_response.result.ok && f.compile == 1 && f.status == 1 && f.end == 1 && compile_response.output_path == NULL, "compile default output");
    failures += expect_events(&f, "SBCE", "compile status and success order");
    memset(&f, 0, sizeof(f)); compile_request.options.print_status = false; compile_request.options.output_path = "custom.o";
    compile_response = compile_object_with_dependencies(&state, &compile_request, &d);
    failures += expect(compile_response.result.ok && strcmp(compile_response.output_path, "custom.o") == 0, "compile custom output");
    failures += expect_events(&f, "BCE", "compile custom output order");
    memset(&f, 0, sizeof(f)); f.fail_at = -1;
    compile_response = compile_object_with_dependencies(&state, &compile_request, &d);
    failures += expect(!compile_response.result.ok && compile_response.result.code == CJIT_RESULT_COMPILER_ERROR && f.end == 1, "compile adapter failure cleanup");
    failures += expect_events(&f, "BCE", "compile adapter failure order");
    memset(&f, 0, sizeof(f)); f.fail_at = 1;
    build_response = build_executable_with_dependencies(&state, &build_request, &d);
    failures += expect(!build_response.result.ok && build_response.result.code == CJIT_RESULT_COMPILER_ERROR && f.link == 0 && f.end == 1, "build source failure cleanup");
    failures += expect_events(&f, "BFE", "build source failure order");
    memset(&f, 0, sizeof(f)); f.fail_at = -2;
    build_response = build_executable_with_dependencies(&state, &build_request, &d);
    failures += expect(!build_response.result.ok && build_response.result.code == CJIT_RESULT_LINK_ERROR && f.end == 1, "build link failure cleanup");
    failures += expect_events(&f, "BFFLE", "build link failure order");
    memset(&f, 0, sizeof(f));
    build_response = build_executable_with_dependencies(&state, &build_request, &d);
    failures += expect(build_response.result.ok && build_response.output_path == NULL && f.file == 2 && f.link == 1 && f.end == 1, "build default output");
    failures += expect_events(&f, "BFFLE", "build success order");
    memset(&f, 0, sizeof(f)); build_request.options.output_path = "custom-program";
    build_response = build_executable_with_dependencies(&state, &build_request, &d);
    failures += expect(build_response.result.ok && strcmp(build_response.output_path, "custom-program") == 0, "build custom output");
    failures += expect_events(&f, "BFFLE", "build custom output order");
    memset(&f, 0, sizeof(f));
    assets_response = extract_assets_with_dependencies(&state, &assets_request, &d);
    failures += expect(assets_response.result.ok && f.assets == 1 && strcmp(assets_response.destination_path, "owned-result") == 0, "asset returned path ownership");
    free((void *)assets_response.destination_path);
    memset(&f, 0, sizeof(f)); f.fail_at = -5;
    assets_response = extract_assets_with_dependencies(&state, &assets_request, &d);
    failures += expect(!assets_response.result.ok && assets_response.destination_path == NULL, "asset failure");
    memset(&f, 0, sizeof(f));
    archive_response = extract_archive_with_dependencies(&archive_request, &d);
    failures += expect(archive_response.result.ok && f.archive == 1, "archive success");
    memset(&f, 0, sizeof(f)); f.fail_at = -6;
    archive_response = extract_archive_with_dependencies(&archive_request, &d);
    failures += expect(!archive_response.result.ok && archive_response.result.code == CJIT_RESULT_IO_ERROR && f.archive == 1, "archive failure");
    memset(&f, 0, sizeof(f));
    print_status_with_dependencies(&state, &status_request, &d);
    failures += expect(f.status == 1, "status dependency");
    return failures ? 1 : 0;
}
