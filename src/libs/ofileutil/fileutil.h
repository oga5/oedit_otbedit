/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

LRESULT SelectFolder(HWND hWnd, TCHAR *folder, const TCHAR *base_dir, const TCHAR *title);
void make_dirname(TCHAR *dir);
void make_dirname2(TCHAR *dir);
void make_parent_dirname(TCHAR *dir);

CString GetAppPath();
CString GetShortAppPath();
BOOL GetLongPath(const TCHAR *short_name, TCHAR *long_name);
BOOL is_file_exist(const TCHAR *path);
BOOL is_directory_exist(const TCHAR *path);
int make_directory(const TCHAR *dir, TCHAR *msg_buf);
BOOL is_valid_path(const TCHAR *path);
void make_full_path(TCHAR *path);

BOOL check_ext(const TCHAR *file_name, const TCHAR *ext);

BOOL CheckFileType(const TCHAR *file_name, const TCHAR *file_type);

BOOL IsShortcut(const TCHAR *file_name);
BOOL GetFullPathFromShortcut(const TCHAR *file_name, CString *str);

CString GetSafePathName(const TCHAR *file_name);
