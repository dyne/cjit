/* CJIT https://dyne.org/cjit
 *
 * Copyright (C) 2024 Dyne.org foundation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

// from embed-libtcc1 generated from lib/tinycc/libtcc1.a
extern char *libtcc1;
extern unsigned int libtcc1_len;
extern char *musl_libc;
extern unsigned int musl_libc_len;
// from embed-headers generated from lib/tinycc/include
extern char *lib_tinycc_include_float_h;
extern unsigned int lib_tinycc_include_float_h_len;
extern char *lib_tinycc_include_stdalign_h;
extern unsigned int lib_tinycc_include_stdalign_h_len;
extern char *lib_tinycc_include_stdarg_h;
extern unsigned int lib_tinycc_include_stdarg_h_len;
extern char *lib_tinycc_include_stdatomic_h;
extern unsigned int lib_tinycc_include_stdatomic_h_len;
extern char *lib_tinycc_include_stdbool_h;
extern unsigned int lib_tinycc_include_stdbool_h_len;
extern char *lib_tinycc_include_stddef_h;
extern unsigned int lib_tinycc_include_stddef_h_len;
extern char *lib_tinycc_include_stdnoreturn_h;
extern unsigned int lib_tinycc_include_stdnoreturn_h_len;
extern char *lib_tinycc_include_tccdefs_h;
extern unsigned int lib_tinycc_include_tccdefs_h_len;
extern char *lib_tinycc_include_tgmath_h;
extern unsigned int lib_tinycc_include_tgmath_h_len;
extern char *lib_tinycc_include_varargs_h;
extern unsigned int lib_tinycc_include_varargs_h_len;
#if defined(LIBC_MINGW32)
extern char *lib_tinycc_win32_include__mingw_h;
extern unsigned int lib_tinycc_win32_include__mingw_h_len;
extern char *lib_tinycc_win32_include_assert_h;
extern unsigned int lib_tinycc_win32_include_assert_h_len;
extern char *lib_tinycc_win32_include_conio_h;
extern unsigned int lib_tinycc_win32_include_conio_h_len;
extern char *lib_tinycc_win32_include_ctype_h;
extern unsigned int lib_tinycc_win32_include_ctype_h_len;
extern char *lib_tinycc_win32_include_dir_h;
extern unsigned int lib_tinycc_win32_include_dir_h_len;
extern char *lib_tinycc_win32_include_direct_h;
extern unsigned int lib_tinycc_win32_include_direct_h_len;
extern char *lib_tinycc_win32_include_dirent_h;
extern unsigned int lib_tinycc_win32_include_dirent_h_len;
extern char *lib_tinycc_win32_include_dos_h;
extern unsigned int lib_tinycc_win32_include_dos_h_len;
extern char *lib_tinycc_win32_include_errno_h;
extern unsigned int lib_tinycc_win32_include_errno_h_len;
extern char *lib_tinycc_win32_include_excpt_h;
extern unsigned int lib_tinycc_win32_include_excpt_h_len;
extern char *lib_tinycc_win32_include_fcntl_h;
extern unsigned int lib_tinycc_win32_include_fcntl_h_len;
extern char *lib_tinycc_win32_include_fenv_h;
extern unsigned int lib_tinycc_win32_include_fenv_h_len;
extern char *lib_tinycc_win32_include_inttypes_h;
extern unsigned int lib_tinycc_win32_include_inttypes_h_len;
extern char *lib_tinycc_win32_include_io_h;
extern unsigned int lib_tinycc_win32_include_io_h_len;
extern char *lib_tinycc_win32_include_iso646_h;
extern unsigned int lib_tinycc_win32_include_iso646_h_len;
extern char *lib_tinycc_win32_include_limits_h;
extern unsigned int lib_tinycc_win32_include_limits_h_len;
extern char *lib_tinycc_win32_include_locale_h;
extern unsigned int lib_tinycc_win32_include_locale_h_len;
extern char *lib_tinycc_win32_include_malloc_h;
extern unsigned int lib_tinycc_win32_include_malloc_h_len;
extern char *lib_tinycc_win32_include_math_h;
extern unsigned int lib_tinycc_win32_include_math_h_len;
extern char *lib_tinycc_win32_include_mem_h;
extern unsigned int lib_tinycc_win32_include_mem_h_len;
extern char *lib_tinycc_win32_include_memory_h;
extern unsigned int lib_tinycc_win32_include_memory_h_len;
extern char *lib_tinycc_win32_include_process_h;
extern unsigned int lib_tinycc_win32_include_process_h_len;
extern char *lib_tinycc_win32_include_sec_api_conio_s_h;
extern unsigned int lib_tinycc_win32_include_sec_api_conio_s_h_len;
extern char *lib_tinycc_win32_include_sec_api_crtdbg_s_h;
extern unsigned int lib_tinycc_win32_include_sec_api_crtdbg_s_h_len;
extern char *lib_tinycc_win32_include_sec_api_io_s_h;
extern unsigned int lib_tinycc_win32_include_sec_api_io_s_h_len;
extern char *lib_tinycc_win32_include_sec_api_mbstring_s_h;
extern unsigned int lib_tinycc_win32_include_sec_api_mbstring_s_h_len;
extern char *lib_tinycc_win32_include_sec_api_search_s_h;
extern unsigned int lib_tinycc_win32_include_sec_api_search_s_h_len;
extern char *lib_tinycc_win32_include_sec_api_stdio_s_h;
extern unsigned int lib_tinycc_win32_include_sec_api_stdio_s_h_len;
extern char *lib_tinycc_win32_include_sec_api_stdlib_s_h;
extern unsigned int lib_tinycc_win32_include_sec_api_stdlib_s_h_len;
extern char *lib_tinycc_win32_include_sec_api_stralign_s_h;
extern unsigned int lib_tinycc_win32_include_sec_api_stralign_s_h_len;
extern char *lib_tinycc_win32_include_sec_api_string_s_h;
extern unsigned int lib_tinycc_win32_include_sec_api_string_s_h_len;
extern char *lib_tinycc_win32_include_sec_api_sys_timeb_s_h;
extern unsigned int lib_tinycc_win32_include_sec_api_sys_timeb_s_h_len;
extern char *lib_tinycc_win32_include_sec_api_tchar_s_h;
extern unsigned int lib_tinycc_win32_include_sec_api_tchar_s_h_len;
extern char *lib_tinycc_win32_include_sec_api_time_s_h;
extern unsigned int lib_tinycc_win32_include_sec_api_time_s_h_len;
extern char *lib_tinycc_win32_include_sec_api_wchar_s_h;
extern unsigned int lib_tinycc_win32_include_sec_api_wchar_s_h_len;
extern char *lib_tinycc_win32_include_setjmp_h;
extern unsigned int lib_tinycc_win32_include_setjmp_h_len;
extern char *lib_tinycc_win32_include_share_h;
extern unsigned int lib_tinycc_win32_include_share_h_len;
extern char *lib_tinycc_win32_include_signal_h;
extern unsigned int lib_tinycc_win32_include_signal_h_len;
extern char *lib_tinycc_win32_include_stdint_h;
extern unsigned int lib_tinycc_win32_include_stdint_h_len;
extern char *lib_tinycc_win32_include_stdio_h;
extern unsigned int lib_tinycc_win32_include_stdio_h_len;
extern char *lib_tinycc_win32_include_stdlib_h;
extern unsigned int lib_tinycc_win32_include_stdlib_h_len;
extern char *lib_tinycc_win32_include_string_h;
extern unsigned int lib_tinycc_win32_include_string_h_len;
extern char *lib_tinycc_win32_include_tcc_tcc_libm_h;
extern unsigned int lib_tinycc_win32_include_tcc_tcc_libm_h_len;
extern char *lib_tinycc_win32_include_sys_fcntl_h;
extern unsigned int lib_tinycc_win32_include_sys_fcntl_h_len;
extern char *lib_tinycc_win32_include_sys_file_h;
extern unsigned int lib_tinycc_win32_include_sys_file_h_len;
extern char *lib_tinycc_win32_include_sys_locking_h;
extern unsigned int lib_tinycc_win32_include_sys_locking_h_len;
extern char *lib_tinycc_win32_include_sys_stat_h;
extern unsigned int lib_tinycc_win32_include_sys_stat_h_len;
extern char *lib_tinycc_win32_include_sys_time_h;
extern unsigned int lib_tinycc_win32_include_sys_time_h_len;
extern char *lib_tinycc_win32_include_sys_timeb_h;
extern unsigned int lib_tinycc_win32_include_sys_timeb_h_len;
extern char *lib_tinycc_win32_include_sys_types_h;
extern unsigned int lib_tinycc_win32_include_sys_types_h_len;
extern char *lib_tinycc_win32_include_sys_unistd_h;
extern unsigned int lib_tinycc_win32_include_sys_unistd_h_len;
extern char *lib_tinycc_win32_include_sys_utime_h;
extern unsigned int lib_tinycc_win32_include_sys_utime_h_len;
extern char *lib_tinycc_win32_include_tchar_h;
extern unsigned int lib_tinycc_win32_include_tchar_h_len;
extern char *lib_tinycc_win32_include_time_h;
extern unsigned int lib_tinycc_win32_include_time_h_len;
extern char *lib_tinycc_win32_include_uchar_h;
extern unsigned int lib_tinycc_win32_include_uchar_h_len;
extern char *lib_tinycc_win32_include_vadefs_h;
extern unsigned int lib_tinycc_win32_include_vadefs_h_len;
extern char *lib_tinycc_win32_include_values_h;
extern unsigned int lib_tinycc_win32_include_values_h_len;
extern char *lib_tinycc_win32_include_wchar_h;
extern unsigned int lib_tinycc_win32_include_wchar_h_len;
extern char *lib_tinycc_win32_include_wctype_h;
extern unsigned int lib_tinycc_win32_include_wctype_h_len;
extern char *lib_tinycc_win32_include_winapi_basetsd_h;
extern unsigned int lib_tinycc_win32_include_winapi_basetsd_h_len;
extern char *lib_tinycc_win32_include_winapi_basetyps_h;
extern unsigned int lib_tinycc_win32_include_winapi_basetyps_h_len;
extern char *lib_tinycc_win32_include_winapi_guiddef_h;
extern unsigned int lib_tinycc_win32_include_winapi_guiddef_h_len;
extern char *lib_tinycc_win32_include_winapi_poppack_h;
extern unsigned int lib_tinycc_win32_include_winapi_poppack_h_len;
extern char *lib_tinycc_win32_include_winapi_pshpack1_h;
extern unsigned int lib_tinycc_win32_include_winapi_pshpack1_h_len;
extern char *lib_tinycc_win32_include_winapi_pshpack2_h;
extern unsigned int lib_tinycc_win32_include_winapi_pshpack2_h_len;
extern char *lib_tinycc_win32_include_winapi_pshpack4_h;
extern unsigned int lib_tinycc_win32_include_winapi_pshpack4_h_len;
extern char *lib_tinycc_win32_include_winapi_pshpack8_h;
extern unsigned int lib_tinycc_win32_include_winapi_pshpack8_h_len;
extern char *lib_tinycc_win32_include_winapi_qos_h;
extern unsigned int lib_tinycc_win32_include_winapi_qos_h_len;
extern char *lib_tinycc_win32_include_winapi_winbase_h;
extern unsigned int lib_tinycc_win32_include_winapi_winbase_h_len;
extern char *lib_tinycc_win32_include_winapi_wincon_h;
extern unsigned int lib_tinycc_win32_include_winapi_wincon_h_len;
extern char *lib_tinycc_win32_include_winapi_windef_h;
extern unsigned int lib_tinycc_win32_include_winapi_windef_h_len;
extern char *lib_tinycc_win32_include_winapi_windows_h;
extern unsigned int lib_tinycc_win32_include_winapi_windows_h_len;
extern char *lib_tinycc_win32_include_winapi_winerror_h;
extern unsigned int lib_tinycc_win32_include_winapi_winerror_h_len;
extern char *lib_tinycc_win32_include_winapi_wingdi_h;
extern unsigned int lib_tinycc_win32_include_winapi_wingdi_h_len;
extern char *lib_tinycc_win32_include_winapi_winnls_h;
extern unsigned int lib_tinycc_win32_include_winapi_winnls_h_len;
extern char *lib_tinycc_win32_include_winapi_winnt_h;
extern unsigned int lib_tinycc_win32_include_winapi_winnt_h_len;
extern char *lib_tinycc_win32_include_winapi_winreg_h;
extern unsigned int lib_tinycc_win32_include_winapi_winreg_h_len;
extern char *lib_tinycc_win32_include_winapi_winsock2_h;
extern unsigned int lib_tinycc_win32_include_winapi_winsock2_h_len;
extern char *lib_tinycc_win32_include_winapi_winuser_h;
extern unsigned int lib_tinycc_win32_include_winapi_winuser_h_len;
extern char *lib_tinycc_win32_include_winapi_winver_h;
extern unsigned int lib_tinycc_win32_include_winapi_winver_h_len;
extern char *lib_tinycc_win32_include_winapi_ws2ipdef_h;
extern unsigned int lib_tinycc_win32_include_winapi_ws2ipdef_h_len;
extern char *lib_tinycc_win32_include_winapi_ws2tcpip_h;
extern unsigned int lib_tinycc_win32_include_winapi_ws2tcpip_h_len;
#endif
