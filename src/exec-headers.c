/* CJIT https://dyne.org/cjit
 *
 * Copyright (C) 2024 Dyne.org foundation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <stdbool.h>

// from tinycc internal lib
#include <libtcc1.h>

// from file.c
extern bool write_to_file(char *path, char *filename, char *buf, unsigned int len);

bool gen_exec_headers(char *tmpdir) {
	if(! write_to_file(tmpdir,"libtcc1.a",(char*)&libtcc1,libtcc1_len) )
		return(false);

#if defined(LIBC_MUSL)
	if(! write_to_file(tmpdir,"libc.so",(char*)&musl_libc,musl_libc_len) )
		return(false);
#endif

	if(!write_to_file(tmpdir,"float.h",(char*)&lib_tinycc_include_float_h,lib_tinycc_include_float_h_len)) return(false);
	if(!write_to_file(tmpdir,"stdalign.h",(char*)&lib_tinycc_include_stdalign_h,lib_tinycc_include_stdalign_h_len)) return(false);
	if(!write_to_file(tmpdir,"stdarg.h",(char*)&lib_tinycc_include_stdarg_h,lib_tinycc_include_stdarg_h_len)) return(false);
	if(!write_to_file(tmpdir,"stdatomic.h",(char*)&lib_tinycc_include_stdatomic_h,lib_tinycc_include_stdatomic_h_len)) return(false);
	if(!write_to_file(tmpdir,"stdbool.h",(char*)&lib_tinycc_include_stdbool_h,lib_tinycc_include_stdbool_h_len)) return(false);
	if(!write_to_file(tmpdir,"stddef.h",(char*)&lib_tinycc_include_stddef_h,lib_tinycc_include_stddef_h_len)) return(false);
	if(!write_to_file(tmpdir,"stdnoreturn.h",(char*)&lib_tinycc_include_stdnoreturn_h,lib_tinycc_include_stdnoreturn_h_len)) return(false);
	if(!write_to_file(tmpdir,"tccdefs.h",(char*)&lib_tinycc_include_tccdefs_h,lib_tinycc_include_tccdefs_h_len)) return(false);
	if(!write_to_file(tmpdir,"tgmath.h",(char*)&lib_tinycc_include_tgmath_h,lib_tinycc_include_tgmath_h_len)) return(false);
	if(!write_to_file(tmpdir,"varargs.h",(char*)&lib_tinycc_include_varargs_h,lib_tinycc_include_varargs_h_len)) return(false);
#if defined(LIBC_MINGW32)
	if(!write_to_file(tmpdir,"assert.h",(char*)&lib_tinycc_win32_include_assert_h,lib_tinycc_win32_include_assert_h_len)) return(false);
	if(!write_to_file(tmpdir,"conio.h",(char*)&lib_tinycc_win32_include_conio_h,lib_tinycc_win32_include_conio_h_len)) return(false);
	if(!write_to_file(tmpdir,"ctype.h",(char*)&lib_tinycc_win32_include_ctype_h,lib_tinycc_win32_include_ctype_h_len)) return(false);
	if(!write_to_file(tmpdir,"direct.h",(char*)&lib_tinycc_win32_include_direct_h,lib_tinycc_win32_include_direct_h_len)) return(false);
	if(!write_to_file(tmpdir,"dirent.h",(char*)&lib_tinycc_win32_include_dirent_h,lib_tinycc_win32_include_dirent_h_len)) return(false);
	if(!write_to_file(tmpdir,"dir.h",(char*)&lib_tinycc_win32_include_dir_h,lib_tinycc_win32_include_dir_h_len)) return(false);
	if(!write_to_file(tmpdir,"dos.h",(char*)&lib_tinycc_win32_include_dos_h,lib_tinycc_win32_include_dos_h_len)) return(false);
	if(!write_to_file(tmpdir,"errno.h",(char*)&lib_tinycc_win32_include_errno_h,lib_tinycc_win32_include_errno_h_len)) return(false);
	if(!write_to_file(tmpdir,"excpt.h",(char*)&lib_tinycc_win32_include_excpt_h,lib_tinycc_win32_include_excpt_h_len)) return(false);
	if(!write_to_file(tmpdir,"fcntl.h",(char*)&lib_tinycc_win32_include_fcntl_h,lib_tinycc_win32_include_fcntl_h_len)) return(false);
	if(!write_to_file(tmpdir,"fenv.h",(char*)&lib_tinycc_win32_include_fenv_h,lib_tinycc_win32_include_fenv_h_len)) return(false);
	if(!write_to_file(tmpdir,"inttypes.h",(char*)&lib_tinycc_win32_include_inttypes_h,lib_tinycc_win32_include_inttypes_h_len)) return(false);
	if(!write_to_file(tmpdir,"io.h",(char*)&lib_tinycc_win32_include_io_h,lib_tinycc_win32_include_io_h_len)) return(false);
	if(!write_to_file(tmpdir,"iso646.h",(char*)&lib_tinycc_win32_include_iso646_h,lib_tinycc_win32_include_iso646_h_len)) return(false);
	if(!write_to_file(tmpdir,"limits.h",(char*)&lib_tinycc_win32_include_limits_h,lib_tinycc_win32_include_limits_h_len)) return(false);
	if(!write_to_file(tmpdir,"locale.h",(char*)&lib_tinycc_win32_include_locale_h,lib_tinycc_win32_include_locale_h_len)) return(false);
	if(!write_to_file(tmpdir,"malloc.h",(char*)&lib_tinycc_win32_include_malloc_h,lib_tinycc_win32_include_malloc_h_len)) return(false);
	if(!write_to_file(tmpdir,"math.h",(char*)&lib_tinycc_win32_include_math_h,lib_tinycc_win32_include_math_h_len)) return(false);
	if(!write_to_file(tmpdir,"mem.h",(char*)&lib_tinycc_win32_include_mem_h,lib_tinycc_win32_include_mem_h_len)) return(false);
	if(!write_to_file(tmpdir,"memory.h",(char*)&lib_tinycc_win32_include_memory_h,lib_tinycc_win32_include_memory_h_len)) return(false);
	if(!write_to_file(tmpdir,"_mingw.h",(char*)&lib_tinycc_win32_include__mingw_h,lib_tinycc_win32_include__mingw_h_len)) return(false);
	if(!write_to_file(tmpdir,"process.h",(char*)&lib_tinycc_win32_include_process_h,lib_tinycc_win32_include_process_h_len)) return(false);
	if(!write_to_file(tmpdir,"sec_api\\conio_s.h",(char*)&lib_tinycc_win32_include_sec_api_conio_s_h,lib_tinycc_win32_include_sec_api_conio_s_h_len)) return(false);
	if(!write_to_file(tmpdir,"sec_api\\crtdbg_s.h",(char*)&lib_tinycc_win32_include_sec_api_crtdbg_s_h,lib_tinycc_win32_include_sec_api_crtdbg_s_h_len)) return(false);
	if(!write_to_file(tmpdir,"sec_api\\io_s.h",(char*)&lib_tinycc_win32_include_sec_api_io_s_h,lib_tinycc_win32_include_sec_api_io_s_h_len)) return(false);
	if(!write_to_file(tmpdir,"sec_api\\mbstring_s.h",(char*)&lib_tinycc_win32_include_sec_api_mbstring_s_h,lib_tinycc_win32_include_sec_api_mbstring_s_h_len)) return(false);
	if(!write_to_file(tmpdir,"sec_api\\search_s.h",(char*)&lib_tinycc_win32_include_sec_api_search_s_h,lib_tinycc_win32_include_sec_api_search_s_h_len)) return(false);
	if(!write_to_file(tmpdir,"sec_api\\stdio_s.h",(char*)&lib_tinycc_win32_include_sec_api_stdio_s_h,lib_tinycc_win32_include_sec_api_stdio_s_h_len)) return(false);
	if(!write_to_file(tmpdir,"sec_api\\stdlib_s.h",(char*)&lib_tinycc_win32_include_sec_api_stdlib_s_h,lib_tinycc_win32_include_sec_api_stdlib_s_h_len)) return(false);
	if(!write_to_file(tmpdir,"sec_api\\stralign_s.h",(char*)&lib_tinycc_win32_include_sec_api_stralign_s_h,lib_tinycc_win32_include_sec_api_stralign_s_h_len)) return(false);
	if(!write_to_file(tmpdir,"sec_api\\string_s.h",(char*)&lib_tinycc_win32_include_sec_api_string_s_h,lib_tinycc_win32_include_sec_api_string_s_h_len)) return(false);
	if(!write_to_file(tmpdir,"sec_api\\sys\\timeb_s.h",(char*)&lib_tinycc_win32_include_sec_api_sys_timeb_s_h,lib_tinycc_win32_include_sec_api_sys_timeb_s_h_len)) return(false);
	if(!write_to_file(tmpdir,"sec_api\\tchar_s.h",(char*)&lib_tinycc_win32_include_sec_api_tchar_s_h,lib_tinycc_win32_include_sec_api_tchar_s_h_len)) return(false);
	if(!write_to_file(tmpdir,"sec_api\\time_s.h",(char*)&lib_tinycc_win32_include_sec_api_time_s_h,lib_tinycc_win32_include_sec_api_time_s_h_len)) return(false);
	if(!write_to_file(tmpdir,"sec_api\\wchar_s.h",(char*)&lib_tinycc_win32_include_sec_api_wchar_s_h,lib_tinycc_win32_include_sec_api_wchar_s_h_len)) return(false);
	if(!write_to_file(tmpdir,"setjmp.h",(char*)&lib_tinycc_win32_include_setjmp_h,lib_tinycc_win32_include_setjmp_h_len)) return(false);
	if(!write_to_file(tmpdir,"share.h",(char*)&lib_tinycc_win32_include_share_h,lib_tinycc_win32_include_share_h_len)) return(false);
	if(!write_to_file(tmpdir,"signal.h",(char*)&lib_tinycc_win32_include_signal_h,lib_tinycc_win32_include_signal_h_len)) return(false);
	if(!write_to_file(tmpdir,"stdint.h",(char*)&lib_tinycc_win32_include_stdint_h,lib_tinycc_win32_include_stdint_h_len)) return(false);
	if(!write_to_file(tmpdir,"stdio.h",(char*)&lib_tinycc_win32_include_stdio_h,lib_tinycc_win32_include_stdio_h_len)) return(false);
	if(!write_to_file(tmpdir,"stdlib.h",(char*)&lib_tinycc_win32_include_stdlib_h,lib_tinycc_win32_include_stdlib_h_len)) return(false);
	if(!write_to_file(tmpdir,"string.h",(char*)&lib_tinycc_win32_include_string_h,lib_tinycc_win32_include_string_h_len)) return(false);
	if(!write_to_file(tmpdir,"sys\\fcntl.h",(char*)&lib_tinycc_win32_include_sys_fcntl_h,lib_tinycc_win32_include_sys_fcntl_h_len)) return(false);
	if(!write_to_file(tmpdir,"sys\\file.h",(char*)&lib_tinycc_win32_include_sys_file_h,lib_tinycc_win32_include_sys_file_h_len)) return(false);
	if(!write_to_file(tmpdir,"sys\\locking.h",(char*)&lib_tinycc_win32_include_sys_locking_h,lib_tinycc_win32_include_sys_locking_h_len)) return(false);
	if(!write_to_file(tmpdir,"sys\\stat.h",(char*)&lib_tinycc_win32_include_sys_stat_h,lib_tinycc_win32_include_sys_stat_h_len)) return(false);
	if(!write_to_file(tmpdir,"sys\\timeb.h",(char*)&lib_tinycc_win32_include_sys_timeb_h,lib_tinycc_win32_include_sys_timeb_h_len)) return(false);
	if(!write_to_file(tmpdir,"sys\\time.h",(char*)&lib_tinycc_win32_include_sys_time_h,lib_tinycc_win32_include_sys_time_h_len)) return(false);
	if(!write_to_file(tmpdir,"sys\\types.h",(char*)&lib_tinycc_win32_include_sys_types_h,lib_tinycc_win32_include_sys_types_h_len)) return(false);
	if(!write_to_file(tmpdir,"sys\\unistd.h",(char*)&lib_tinycc_win32_include_sys_unistd_h,lib_tinycc_win32_include_sys_unistd_h_len)) return(false);
	if(!write_to_file(tmpdir,"sys\\utime.h",(char*)&lib_tinycc_win32_include_sys_utime_h,lib_tinycc_win32_include_sys_utime_h_len)) return(false);
	if(!write_to_file(tmpdir,"tcc\\tcc_libm.h",(char*)&lib_tinycc_win32_include_tcc_tcc_libm_h,lib_tinycc_win32_include_tcc_tcc_libm_h_len)) return(false);
	if(!write_to_file(tmpdir,"tchar.h",(char*)&lib_tinycc_win32_include_tchar_h,lib_tinycc_win32_include_tchar_h_len)) return(false);
	if(!write_to_file(tmpdir,"time.h",(char*)&lib_tinycc_win32_include_time_h,lib_tinycc_win32_include_time_h_len)) return(false);
	if(!write_to_file(tmpdir,"uchar.h",(char*)&lib_tinycc_win32_include_uchar_h,lib_tinycc_win32_include_uchar_h_len)) return(false);
	if(!write_to_file(tmpdir,"vadefs.h",(char*)&lib_tinycc_win32_include_vadefs_h,lib_tinycc_win32_include_vadefs_h_len)) return(false);
	if(!write_to_file(tmpdir,"values.h",(char*)&lib_tinycc_win32_include_values_h,lib_tinycc_win32_include_values_h_len)) return(false);
	if(!write_to_file(tmpdir,"wchar.h",(char*)&lib_tinycc_win32_include_wchar_h,lib_tinycc_win32_include_wchar_h_len)) return(false);
	if(!write_to_file(tmpdir,"wctype.h",(char*)&lib_tinycc_win32_include_wctype_h,lib_tinycc_win32_include_wctype_h_len)) return(false);
	if(!write_to_file(tmpdir,"winapi\\basetsd.h",(char*)&lib_tinycc_win32_include_winapi_basetsd_h,lib_tinycc_win32_include_winapi_basetsd_h_len)) return(false);
	if(!write_to_file(tmpdir,"winapi\\basetyps.h",(char*)&lib_tinycc_win32_include_winapi_basetyps_h,lib_tinycc_win32_include_winapi_basetyps_h_len)) return(false);
	if(!write_to_file(tmpdir,"winapi\\guiddef.h",(char*)&lib_tinycc_win32_include_winapi_guiddef_h,lib_tinycc_win32_include_winapi_guiddef_h_len)) return(false);
	if(!write_to_file(tmpdir,"winapi\\poppack.h",(char*)&lib_tinycc_win32_include_winapi_poppack_h,lib_tinycc_win32_include_winapi_poppack_h_len)) return(false);
	if(!write_to_file(tmpdir,"winapi\\pshpack1.h",(char*)&lib_tinycc_win32_include_winapi_pshpack1_h,lib_tinycc_win32_include_winapi_pshpack1_h_len)) return(false);
	if(!write_to_file(tmpdir,"winapi\\pshpack2.h",(char*)&lib_tinycc_win32_include_winapi_pshpack2_h,lib_tinycc_win32_include_winapi_pshpack2_h_len)) return(false);
	if(!write_to_file(tmpdir,"winapi\\pshpack4.h",(char*)&lib_tinycc_win32_include_winapi_pshpack4_h,lib_tinycc_win32_include_winapi_pshpack4_h_len)) return(false);
	if(!write_to_file(tmpdir,"winapi\\pshpack8.h",(char*)&lib_tinycc_win32_include_winapi_pshpack8_h,lib_tinycc_win32_include_winapi_pshpack8_h_len)) return(false);
	if(!write_to_file(tmpdir,"winapi\\qos.h",(char*)&lib_tinycc_win32_include_winapi_qos_h,lib_tinycc_win32_include_winapi_qos_h_len)) return(false);
	if(!write_to_file(tmpdir,"winapi\\winbase.h",(char*)&lib_tinycc_win32_include_winapi_winbase_h,lib_tinycc_win32_include_winapi_winbase_h_len)) return(false);
	if(!write_to_file(tmpdir,"winapi\\wincon.h",(char*)&lib_tinycc_win32_include_winapi_wincon_h,lib_tinycc_win32_include_winapi_wincon_h_len)) return(false);
	if(!write_to_file(tmpdir,"winapi\\windef.h",(char*)&lib_tinycc_win32_include_winapi_windef_h,lib_tinycc_win32_include_winapi_windef_h_len)) return(false);
	if(!write_to_file(tmpdir,"winapi\\windows.h",(char*)&lib_tinycc_win32_include_winapi_windows_h,lib_tinycc_win32_include_winapi_windows_h_len)) return(false);
	if(!write_to_file(tmpdir,"winapi\\winerror.h",(char*)&lib_tinycc_win32_include_winapi_winerror_h,lib_tinycc_win32_include_winapi_winerror_h_len)) return(false);
	if(!write_to_file(tmpdir,"winapi\\wingdi.h",(char*)&lib_tinycc_win32_include_winapi_wingdi_h,lib_tinycc_win32_include_winapi_wingdi_h_len)) return(false);
	if(!write_to_file(tmpdir,"winapi\\winnls.h",(char*)&lib_tinycc_win32_include_winapi_winnls_h,lib_tinycc_win32_include_winapi_winnls_h_len)) return(false);
	if(!write_to_file(tmpdir,"winapi\\winnt.h",(char*)&lib_tinycc_win32_include_winapi_winnt_h,lib_tinycc_win32_include_winapi_winnt_h_len)) return(false);
	if(!write_to_file(tmpdir,"winapi\\winreg.h",(char*)&lib_tinycc_win32_include_winapi_winreg_h,lib_tinycc_win32_include_winapi_winreg_h_len)) return(false);
	if(!write_to_file(tmpdir,"winapi\\winsock2.h",(char*)&lib_tinycc_win32_include_winapi_winsock2_h,lib_tinycc_win32_include_winapi_winsock2_h_len)) return(false);
	if(!write_to_file(tmpdir,"winapi\\winuser.h",(char*)&lib_tinycc_win32_include_winapi_winuser_h,lib_tinycc_win32_include_winapi_winuser_h_len)) return(false);
	if(!write_to_file(tmpdir,"winapi\\winver.h",(char*)&lib_tinycc_win32_include_winapi_winver_h,lib_tinycc_win32_include_winapi_winver_h_len)) return(false);
	if(!write_to_file(tmpdir,"winapi\\ws2ipdef.h",(char*)&lib_tinycc_win32_include_winapi_ws2ipdef_h,lib_tinycc_win32_include_winapi_ws2ipdef_h_len)) return(false);
	if(!write_to_file(tmpdir,"winapi\\ws2tcpip.h",(char*)&lib_tinycc_win32_include_winapi_ws2tcpip_h,lib_tinycc_win32_include_winapi_ws2tcpip_h_len)) return(false);
#endif
	return(true);
}
