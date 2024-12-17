/*
MIT License
Copyright (c) 2019 win32ports
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#ifndef __SYS_TIME_H_D9C1BEC0_4C79_4302_8B29_322C4497B27F__
#define __SYS_TIME_H_D9C1BEC0_4C79_4302_8B29_322C4497B27F__

#ifndef _WIN32

#pragma message("this sys/time.h implementation is for Windows only!")

#else /* _WIN32 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if !defined(_WINSOCK2API_) && !defined(_WINSOCKAPI_)
#include <Winsock2.h> /* timeval */
#endif /* !defined(_WINSOCK2API_) && !defined(_WINSOCKAPI_) */

#include <stdint.h>

struct timezone
{
	int tz_minuteswest;
	int tz_dsttime;
};

static int gettimeofday(struct timeval *tp, struct timezone *tzp)
{
	typedef void (__stdcall * pfnGetSystemTimePreciseAsFileTime)(LPFILETIME lpSystemTimeAsFileTime);
	HMODULE hKernel32 = NULL;
	pfnGetSystemTimePreciseAsFileTime fnGetSystemTimePreciseAsFileTime = NULL;
	FILETIME time;
	hKernel32 = GetModuleHandleW(L"kernel32.dll");
#if defined(__GNUC__) && (__GNUC__ == 8)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
	if (hKernel32)
		fnGetSystemTimePreciseAsFileTime = (pfnGetSystemTimePreciseAsFileTime) GetProcAddress(hKernel32, "GetSystemTimePreciseAsFileTime");
#if defined(__GNUC__) && (__GNUC__ == 8)
#pragma GCC diagnostic pop
#endif

	if (fnGetSystemTimePreciseAsFileTime)
		fnGetSystemTimePreciseAsFileTime(&time);
	else
		GetSystemTimeAsFileTime(&time);

	uint64_t time64 = ((uint64_t)time.dwHighDateTime << 32) | time.dwLowDateTime;
	time64 = (time64 / 10 - 11644473600ULL * 1000000ULL);

	if (tp)
	{
		tp->tv_sec = (long) (time64 / 1000000ULL);
		tp->tv_usec = (long) (time64 % 1000000ULL);
	}
	if (tzp)
	{
		/* The use of the timezone structure is obsolete; the tz argument should normally be specified as NULL. */
		TIME_ZONE_INFORMATION tzi;
		GetTimeZoneInformation(&tzi);

		tzp->tz_minuteswest = tzi.Bias;
		tzp->tz_dsttime = 0;
	}

	/* The gettimeofday() function returns 0 and no value is reserved to indicate an error. */
	return 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _WIN32 */

#endif /* __SYS_TIME_H_D9C1BEC0_4C79_4302_8B29_322C4497B27F__ */
