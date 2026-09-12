#include <stdio.h>
#include <string.h>

#include "adapters/compiler/tinycc_adapter.h"
#include "adapters/platform/library_resolver_posix.h"
#include "cjit.h"

int tcc_add_file(TCCState *s, const char *p) { (void)s; (void)p; return 0; }
int tcc_output_file(TCCState *s, const char *p) { (void)s; (void)p; return 0; }
int tcc_relocate(TCCState *s) { (void)s; return 0; }
void *tcc_get_symbol(TCCState *s, const char *p) { (void)s; (void)p; return (void *)1; }

static char events[16]; static int at; static int relocate_result, output_result; static void *symbol_result;
static void event(char c) { events[at++] = c; events[at] = 0; }
static int add_file(TCCState *s, const char *p) { (void)s; (void)p; event('F'); return 0; }
static int output_file(TCCState *s, const char *p) { (void)s; (void)p; event('O'); return output_result; }
static int relocate(TCCState *s, void *m) { (void)s; (void)m; event('R'); return relocate_result; }
static void *symbol(TCCState *s, const char *n) { (void)s; (void)n; event('S'); return symbol_result; }
static CJITResult resolve(void *c, const LibraryResolverRequest *r, LibraryResolverResponse *o)
{ (void)c; (void)r; o->resolved_count = 0; return cjit_result_ok(); }
const LibraryResolverPort posix_library_resolver_port = { .resolve = resolve };
const LibraryResolverPort windows_library_resolver_port = { .resolve = resolve };
int cjit_platform_exec(CJITState *s, int (*e)(int, char **), int a, char **v)
{ (void)s; (void)e; (void)a; (void)v; event('X'); return 7; }
CJITResult cjit_add_file_result(CJITState *s, const char *p) { (void)s; (void)p; return cjit_result_ok(); }
CJITResult cjit_add_buffer_result(CJITState *s, const char *p) { (void)s; (void)p; return cjit_result_ok(); }
void cjit_set_output(CJITState *s, int o) { s->tcc_output = o; }
void cjit_define_symbol(CJITState *s, const char *a, const char *b) { (void)s;(void)a;(void)b; }
void cjit_add_include_path(CJITState *s, const char *p) { (void)s;(void)p; }
void cjit_add_library_path(CJITState *s, const char *p) { (void)s;(void)p; }
void cjit_set_tcc_options(CJITState *s, const char *p) { (void)s;(void)p; }

int main(void)
{
    CJITState state = {0}; RuntimeSession session = { .compiler_handle = (void *)1 };
    TinyccAdapterApi api = { add_file, output_file, relocate, symbol };
    int status = 0; CJITResult r;
    state.done_setup = true; symbol_result = (void *)1;
    tinycc_adapter_set_api_for_test(&api);
    r = tinycc_compiler_port.execute_program(&state, &session, 0, NULL, &status);
    if (r.ok || r.exit_status != 7 || status != 7 || strcmp(events, "RSX")) return 11;
    at = 0; events[0] = 0; relocate_result = -1;
    r = tinycc_compiler_port.execute_program(&state, &session, 0, NULL, &status);
    if (r.ok || r.code != CJIT_RESULT_LINK_ERROR || strcmp(events, "R")) return 12;
    at = 0; events[0] = 0; relocate_result = 0; output_result = -1;
    state.output_filename = "object.o";
    r = tinycc_compiler_port.compile_object(&state, &session, "input.c");
    if (r.ok || r.code != CJIT_RESULT_COMPILER_ERROR || strcmp(events, "O")) return 13;
    at = 0; events[0] = 0; output_result = 0; symbol_result = NULL;
    r = tinycc_compiler_port.execute_program(&state, &session, 0, NULL, &status);
    if (r.ok || r.code != CJIT_RESULT_LINK_ERROR || strcmp(events, "RS")) return 14;
    at = 0; events[0] = 0; state.done_exec = true;
    r = tinycc_compiler_port.execute_program(&state, &session, 0, NULL, &status);
    if (r.ok || r.code != CJIT_RESULT_EXEC_ERROR || events[0] != 0) return 15;
    tinycc_adapter_reset_api_for_test();
    return 0;
}
