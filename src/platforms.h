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
  #define PLATFORM "GNU/Linux"
  #endif
  #if defined(__APPLE__)
  #define APPLE
  #define PLATFORM "Apple/OSX"
  #endif
  #if defined(__DragonFly__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
  #define BSD
  #define PLATFORM "BSD"
  #endif
  #if defined(__ANDROID__)
  #define ANDROID
  #define PLATFORM "Android"
  #endif
  #if defined(__EMSCRIPTEN__)
  #define EMSCRIPTEN
  #define PLATFORM "WASM"
  #endif
  #if defined(__BEOS__) || defined(__HAIKU__)
  #define BEOS
  #define PLATFORM "BEOS"
  #endif
  #if defined(__HAIKU__)
  #define HAIKU
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

#endif
