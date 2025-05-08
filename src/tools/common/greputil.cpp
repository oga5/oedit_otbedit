/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "greputil.h"
#include "fileutil.h"


// common

static void BootProgram(const TCHAR *cmd)
{
	PROCESS_INFORMATION		pi;
	STARTUPINFO				si;

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	if(CreateProcess(NULL, (TCHAR *)cmd, NULL, NULL, FALSE,
		0, NULL, NULL, &si, &pi) == FALSE) {
		return;
	}
	WaitForInputIdle(pi.hProcess, INFINITE);

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}

// for oedit

HWND OeditCheckFileIsOpen(const TCHAR *file_name)
{
	HWND	find = NULL;
	for(;; ) {
		find = FindWindowEx(NULL, find, OEDIT_CLASS_NAME, NULL);
		if(find == NULL) break;

		COPYDATASTRUCT	copy_data;
		TCHAR	file_name2[_MAX_PATH] = _T("");
		_tcscpy(file_name2, file_name);
		copy_data.dwData = WM_COPY_DATA_GET_FILE_NAME;
		copy_data.cbData = sizeof(file_name2);
		copy_data.lpData = &file_name2;

		if(SendMessage(find, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&copy_data) == TRUE) return find;
	}

	return NULL;
}

static CString OeditCreateCommand(const TCHAR *file_name)
{
	CString cmd;

	cmd.Format(_T("%soedit.exe \"%s\""), GetAppPath().GetBuffer(0), file_name);

	return cmd;
}

void OeditOpenFile(const TCHAR *file_name)
{
	CString	cmd = OeditCreateCommand(file_name);
	BootProgram(cmd);
}


// for otbedit

HWND OtbeditCheckFileIsOpen(HWND wnd, const TCHAR *file_name)
{
	HWND	find = NULL;
	for(; wnd != NULL; ) {
		find = FindWindowEx(wnd, NULL, NULL, OTBEDIT_VIEW_WINDOWNAME);
		if(find != NULL) {
			COPYDATASTRUCT	copy_data;
			TCHAR	file_name2[_MAX_PATH] = _T("");
			_tcscpy(file_name2, file_name);
			copy_data.dwData = WM_COPY_DATA_GET_FILE_NAME;
			copy_data.cbData = sizeof(file_name2);
			copy_data.lpData = &file_name2;

			if(SendMessage(find, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&copy_data) == TRUE) return find;
		}
		wnd = GetNextWindow(wnd, GW_HWNDNEXT);
	}

	return NULL;
}

static CString OtbeditCreateCommand(const TCHAR *file_name)
{
	CString cmd;

	cmd.Format(_T("%sotbedit.exe \"%s\""), GetAppPath().GetBuffer(0), file_name);

	return cmd;
}

static void OtbeditOpenFileMain(const TCHAR *file_name)
{
	CString	cmd = OtbeditCreateCommand(file_name);
	BootProgram(cmd);
}

void OtbeditOpenFile(const TCHAR *file_name)
{
	HWND	wnd = GetTopWindow(NULL);
	HWND	find = NULL;
	for(; wnd != NULL; ) {
		find = FindWindowEx(wnd, NULL, NULL, OTBEDIT_VIEW_WINDOWNAME);
		if(find != NULL) {
			COPYDATASTRUCT	copy_data;
			TCHAR	file_name2[_MAX_PATH] = _T("");
			_tcscpy(file_name2, file_name);
			copy_data.dwData = WM_COPY_DATA_OPEN_FILE;
			copy_data.cbData = sizeof(file_name2);
			copy_data.lpData = &file_name2;

			if(SendMessage(find, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&copy_data) == TRUE) return;
			break;
		}
		wnd = GetNextWindow(wnd, GW_HWNDNEXT);
	}

	OtbeditOpenFileMain(file_name);
}
