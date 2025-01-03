// Platform detection
#ifndef __PLATFORMS_H__
#define __PLATFORMS_H__

// deactivate warnings about re-definitions
#pragma GCC system_header

#if defined(_WIN32) || defined(__COSMOPOLITAN__)
#define WINDOWS
#define PLATFORM "Windows"
#endif

#if !defined(_WIN32)    /* If it's not Win32, assume POSIX. */
  #define POSIX
  #define PLATFORM "Posix"
  #if defined(__unix__)
  #define UNIX
  #define PLATFORM "UNIX"
  #endif
  #if defined(__linux__)
  #define LINUX
  #define POSIX
  #define UNIX
  #define PLATFORM "GNU/Linux"
  #endif
  #if defined(__APPLE__)
  #define APPLE
  #define POSIX
  #define PLATFORM "Apple/OSX"
  #endif
  #if defined(__DragonFly__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
  #define BSD
  #define POSIX
  #define UNIX
  #define PLATFORM "BSD"
  #endif
  #if defined(__ANDROID__)
  #define ANDROID
  #define POSIX
  #define PLATFORM "Android"
  #endif
  #if defined(__EMSCRIPTEN__)
  #define EMSCRIPTEN
  #define PLATFORM "WASM"
  #endif
  #if defined(__BEOS__) || defined(__HAIKU__)
  #define BEOS
  #define POSIX
  #define PLATFORM "BEOS"
  #endif
  #if defined(__HAIKU__)
  #define HAIKU
  #define POSIX
  #define PLATFORM "Haiku"
  #endif
#endif
#if !defined(PLATFORM)
  #define PLATFORM "Unknown"
#endif

#if defined(WINDOWS)
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#include <windows.h>
#include <shlwapi.h>
#include <rpc.h>
#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "shlwapi.lib")
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#ifndef O_BINARY
# define O_BINARY 0
#endif
#endif

#if defined(LINUX) || defined(POSIX) || defined(UNIX) || defined(BSD) || defined(ANDROID) || defined(EMSCRIPTEN)
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/poll.h>
#endif

////////////////////////////////
// TCC logics copied from tcc.h

/* target selection */
/* #define TCC_TARGET_I386   *//* i386 code generator */
/* #define TCC_TARGET_X86_64 *//* x86-64 code generator */
/* #define TCC_TARGET_ARM    *//* ARMv4 code generator */
/* #define TCC_TARGET_ARM64  *//* ARMv8 code generator */
/* #define TCC_TARGET_C67    *//* TMS320C67xx code generator */
/* #define TCC_TARGET_RISCV64 *//* risc-v code generator */

/* default target is I386 */
#if !defined(TCC_TARGET_I386) && !defined(TCC_TARGET_ARM) && \
    !defined(TCC_TARGET_ARM64) && !defined(TCC_TARGET_C67) && \
    !defined(TCC_TARGET_X86_64) && !defined(TCC_TARGET_RISCV64)
# if defined __x86_64__
#  define TCC_TARGET_X86_64
# elif defined __arm__
#  define TCC_TARGET_ARM
#  define TCC_ARM_EABI
#  define TCC_ARM_VFP
#  define TCC_ARM_HARDFLOAT
# elif defined __aarch64__
#  define TCC_TARGET_ARM64
# elif defined __riscv
#  define TCC_TARGET_RISCV64
# else
#  define TCC_TARGET_I386
# endif
# ifdef _WIN32
#  define TCC_TARGET_PE 1
# endif
# ifdef __APPLE__
#  define TCC_TARGET_MACHO 1
# endif
#endif

/* only native compiler supports -run */
#if defined _WIN32 == defined TCC_TARGET_PE \
    && defined __APPLE__ == defined TCC_TARGET_MACHO
# if defined __i386__ && defined TCC_TARGET_I386
#  define TCC_IS_NATIVE
# elif defined __x86_64__ && defined TCC_TARGET_X86_64
#  define TCC_IS_NATIVE
# elif defined __arm__ && defined TCC_TARGET_ARM
#  define TCC_IS_NATIVE
# elif defined __aarch64__ && defined TCC_TARGET_ARM64
#  define TCC_IS_NATIVE
# elif defined __riscv && defined __LP64__ && defined TCC_TARGET_RISCV64
#  define TCC_IS_NATIVE
# endif
#endif

#if defined TARGETOS_OpenBSD \
    || defined TARGETOS_FreeBSD \
    || defined TARGETOS_NetBSD \
    || defined TARGETOS_FreeBSD_kernel
# define TARGETOS_BSD 1
#elif !(defined TCC_TARGET_PE || defined TCC_TARGET_MACHO)
# define TARGETOS_Linux 1
#endif

#if defined TCC_TARGET_PE || defined TCC_TARGET_MACHO
# define ELF_OBJ_ONLY /* create elf .o but native executables */
#endif

/* system include paths */
#ifndef CONFIG_SYSROOT
# define CONFIG_SYSROOT ""
#endif
#if !defined CONFIG_TCCDIR && !defined _WIN32
# define CONFIG_TCCDIR "/usr/local/lib/tcc"
#endif
#ifndef CONFIG_LDDIR
# define CONFIG_LDDIR "lib"
#endif
#ifdef CONFIG_TRIPLET
# define USE_TRIPLET(s) s "/" CONFIG_TRIPLET
# define ALSO_TRIPLET(s) USE_TRIPLET(s) ":" s
#else
# define USE_TRIPLET(s) s
# define ALSO_TRIPLET(s) s
#endif
#ifndef CONFIG_TCC_SYSINCLUDEPATHS
# if defined TCC_TARGET_PE || defined _WIN32
#  define CONFIG_TCC_SYSINCLUDEPATHS "{B}/include"PATHSEP"{B}/include/winapi"
# else
#  define CONFIG_TCC_SYSINCLUDEPATHS \
        "{B}/include" \
    ":" ALSO_TRIPLET(CONFIG_SYSROOT "/usr/local/include") \
    ":" ALSO_TRIPLET(CONFIG_SYSROOT CONFIG_USR_INCLUDE)
# endif
#endif

/* library search paths */
#ifndef CONFIG_TCC_LIBPATHS
# if defined TCC_TARGET_PE || defined _WIN32
#  define CONFIG_TCC_LIBPATHS "{B}/lib"
# else
#  define CONFIG_TCC_LIBPATHS \
        "{B}" \
    ":" ALSO_TRIPLET(CONFIG_SYSROOT "/usr/" CONFIG_LDDIR) \
    ":" ALSO_TRIPLET(CONFIG_SYSROOT "/" CONFIG_LDDIR) \
    ":" ALSO_TRIPLET(CONFIG_SYSROOT "/usr/local/" CONFIG_LDDIR)
# endif
#endif

/* name of ELF interpreter */
#ifndef CONFIG_TCC_ELFINTERP
# if TARGETOS_FreeBSD
#  define CONFIG_TCC_ELFINTERP "/libexec/ld-elf.so.1"
# elif TARGETOS_FreeBSD_kernel
#  if defined(TCC_TARGET_X86_64)
#   define CONFIG_TCC_ELFINTERP "/lib/ld-kfreebsd-x86-64.so.1"
#  else
#   define CONFIG_TCC_ELFINTERP "/lib/ld.so.1"
#  endif
# elif TARGETOS_DragonFly
#  define CONFIG_TCC_ELFINTERP "/usr/libexec/ld-elf.so.2"
# elif TARGETOS_NetBSD
#  define CONFIG_TCC_ELFINTERP "/usr/libexec/ld.elf_so"
# elif TARGETOS_OpenBSD
#  define CONFIG_TCC_ELFINTERP "/usr/libexec/ld.so"
# elif defined __GNU__
#  define CONFIG_TCC_ELFINTERP "/lib/ld.so"
# elif defined(TCC_TARGET_PE)
#  define CONFIG_TCC_ELFINTERP "-"
# elif defined(TCC_UCLIBC)
#  define CONFIG_TCC_ELFINTERP "/lib/ld-uClibc.so.0" /* is there a uClibc for x86_64 ? */
# elif defined TCC_TARGET_ARM64
#  if defined(TCC_MUSL)
#   define CONFIG_TCC_ELFINTERP "/lib/ld-musl-aarch64.so.1"
#  else
#   define CONFIG_TCC_ELFINTERP "/lib/ld-linux-aarch64.so.1"
#  endif
# elif defined(TCC_TARGET_X86_64)
#  if defined(TCC_MUSL)
#   define CONFIG_TCC_ELFINTERP "/lib/ld-musl-x86_64.so.1"
#  else
#   define CONFIG_TCC_ELFINTERP "/lib64/ld-linux-x86-64.so.2"
#  endif
# elif defined(TCC_TARGET_RISCV64)
#  define CONFIG_TCC_ELFINTERP "/lib/ld-linux-riscv64-lp64d.so.1"
# elif !defined(TCC_ARM_EABI)
#  if defined(TCC_MUSL)
#   if defined(TCC_TARGET_I386)
#     define CONFIG_TCC_ELFINTERP "/lib/ld-musl-i386.so.1"
#    else
#     define CONFIG_TCC_ELFINTERP "/lib/ld-musl-arm.so.1"
#    endif
#  else
#   define CONFIG_TCC_ELFINTERP "/lib/ld-linux.so.2"
#  endif
# endif
#endif

#endif
