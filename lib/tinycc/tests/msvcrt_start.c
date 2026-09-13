/* ------------------------------------------------------------- */
/* minimal startup with runtime linker to msvcrt */

#if 0

#define REDIR_ALL \
 REDIR(__set_app_type)\
 REDIR(__getmainargs)\
 REDIR(_controlfp)\
 REDIR(_vsnprintf)\
 REDIR(exit)\
 \
 REDIR(puts)\
 REDIR(printf)\
 REDIR(putchar)\
 REDIR(strtod)\
 REDIR(memset)\
 REDIR(strcpy)\
 REDIR(strlen)\
 REDIR(malloc)\
 REDIR(free)\

#if defined __i386__ && !defined __TINYC__
# define __leading_underscore 1
#endif

#ifdef __leading_underscore
# define _(s) "_"#s
#else
# define _(s) #s
#endif

#define REDIR(s) void *s;
static struct { REDIR_ALL } all_ptrs;
#undef REDIR

#define REDIR(s) #s"\0"
static const char all_names[] = REDIR_ALL;
#undef REDIR

#if __aarch64__
  #if defined __TINYC__
  # define ALIGN ".align 8"
  #else
  # define ALIGN ".align 3" /* .align is power of 2 on non-ELF platforms */
  #endif
# define REDIR(s) \
    __asm__("\n"_(s)":"); \
    __asm__(".int 0x58000090"); /* ldr x16, [pc, #16] */ \
    __asm__(".int 0xf9400210"); /* ldr x16, [x16] */ \
    __asm__(".int 0xd61f0200"); /* br x16 */ \
    __asm__(".int 0xd503201f"); /* nop for alignment */ \
    __asm__(".quad all_ptrs + (. - all_jmps - 16) / 24 * 8"); \
    __asm__(".global "_(s));

    __asm__("\t.text\n\t"ALIGN"\nall_jmps:");
    REDIR_ALL
#else
# define REDIR(s) \
    __asm__("\n"_(s)":");\
    __asm__("jmp *%0"::"m"(all_ptrs.s));\
    __asm__(".global "_(s));

    static void all_jmps() { REDIR_ALL }
#endif
#undef REDIR

#if 0
# include <windows.h>
#else
# if __i386__
#  define STDCALL __declspec(stdcall)
# else
#  define STDCALL
# endif
# define DWORD long unsigned
# define HMODULE void*
# define HANDLE void*
S TDCALL HMODULE LoadLibraryA(const char *);
S TDCALL HMODULE GetProcAddress(HMODULE , char*);
S TDCALL void ExitProcess(int);
S TDCALL int WriteFile(HANDLE, const void*, DWORD, DWORD*, void*);
S TDCALL HANDLE GetStdHandle(DWORD);
S TDCALL int FlushFileBuffers(HANDLE);
# define STD_ERROR_HANDLE -12
#endif

static void eput(const char *s)
{
    DWORD n_out;
    int n = 0;
    while (s[n])
        ++n;
    WriteFile(GetStdHandle(STD_ERROR_HANDLE), s, n, &n_out, 0);
}

static void rt_reloc()
{
    const char *s = all_names;
    void **p = (void**)&all_ptrs;
    void *dll = LoadLibraryA("msvcrt.dll");
    do {
        char buf[100], *d = buf;
        *p = (void*)GetProcAddress(dll, (char*)s);
        *d++ = '_'; do *d++ = *s; while (*s++);
        if (0 == *p)
            *p = (void*)GetProcAddress(dll, buf);
        if (0 == *p) {
            eput("MSVCRT_START.C: RUNTIME RELOCATION ERROR: '");
            eput(buf+1);
            eput("'\n");
            ExitProcess(-1);
        }
        ++p;
    } while (*s);
}

#else
# define rt_reloc()
#endif

int main(int argc, char **argv, char **env);
void exit(int);
void __set_app_type(int apptype);

typedef struct { int newmode; } _startupinfo;
int __getmainargs(int *pargc, char ***pargv, char ***penv, int globb, _startupinfo*);

void _controlfp(unsigned a, unsigned b);
#define _MCW_PC 0x00030000 // Precision Control
#define _PC_53 0x00010000 // 53 bits

int __argc;
char **__argv;
char **environ;
_startupinfo start_info = {0};

void mainCRTStartup(void)
{
    rt_reloc();
#if defined __i386__ || defined __x86_64__
    _controlfp(_PC_53, _MCW_PC);
#endif
    __set_app_type(1);
    __getmainargs(&__argc, &__argv, &environ, 0, &start_info);
    exit(main(__argc, __argv, environ));
}

#include <stdarg.h>
#define size_t __SIZE_TYPE__
int printf(const char *, ...);
int _vsnprintf(char *, size_t, const char *, va_list);

/* undefined on windows-11-arm64 */
int vprintf(const char *format, va_list ap)
{
    char buf[1000];
    _vsnprintf(buf, sizeof buf, format, ap);
    return printf("%s", buf);
}

void __main() {} /* for gcc */
void _pei386_runtime_relocator(void) {} /* for gcc */
void __chkstk(unsigned n) {} /* for clang */
