/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef __FILE_WINUTIL_H_INCLUDED__
#define __FILE_WINUTIL_H_INCLUDED__

BOOL GetFileVersion2(TCHAR *filename, int *v1, int *v2, int *v3, int *v4);
BOOL GetFileVersion(TCHAR *filename, CString *str, int format = 0);
CString GetAppVersion();
BOOL ViewBrowser(TCHAR *app, TCHAR *app_path, TCHAR *file, BOOL new_window);
HWND FindWindowLoop(TCHAR *window_name);

void OpenHelpFile(const TCHAR *file_name);

CString GetFullPath(const TCHAR *file_name);

CDocument *GetActiveDocument();

BOOL SetDllDir(const TCHAR *dir_name);

#endif __FILE_WINUTIL_H_INCLUDED__
