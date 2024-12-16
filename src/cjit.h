#ifndef _CJIT_H_
#define _CJIT_H_

#include <stdbool.h>
#include <libtcc.h>

// passed to cjit_exec_fork with CJIT execution parameters
struct CJITState {
	char *tmpdir;
	char *write_pid; // filename to write the pid of execution
	bool dmon;
	TCCState *TCC;
};
typedef struct CJITState CJITState;

/////////////
// from file.c
extern int   detect_bom(const char *filename);
extern long  file_size(const char *filename);
extern char* file_load(const char *filename);
extern char *load_stdin();
extern char* dir_load(const char *path);
extern bool write_to_file(const char *path, const char *filename,
			  const char *buf, unsigned int len);

#ifdef LIBC_MINGW32
bool win32_mkdtemp(CJITState *CJIT);
// from win-compat.c
extern void win_compat_usleep(unsigned int microseconds);
extern ssize_t win_compat_getline(char **lineptr, size_t *n, FILE *stream);
#else
bool posix_mkdtemp(CJITState *CJIT);
#endif
// from io.c
extern void _out(const char *fmt, ...);
extern void _err(const char *fmt, ...);
// from repl.c
#ifdef LIBC_MINGW32
extern int cjit_exec_win(TCCState *TCC, CJITState *CJIT,
			 const char *ep, int argc, char **argv);
#else
extern int cjit_exec_fork(TCCState *TCC, CJITState *CJIT,
			  const char *ep, int argc, char **argv);
#endif
extern int cjit_cli_tty(TCCState *TCC);
#ifdef KILO_SUPPORTED
extern int cjit_cli_kilo(TCCState *TCC);
#endif
// from embed-dmon.c
extern char *lib_dmon_dmon_extra_h;
extern unsigned int lib_dmon_dmon_extra_h_len;
extern char *lib_dmon_dmon_h;
extern unsigned int lib_dmon_dmon_h_len;
/////////////

#endif
