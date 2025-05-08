#include "stdafx.h"
#include "greputil.h"
#include "fileutil.h"

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

	PROCESS_INFORMATION		pi;
	STARTUPINFO				si;

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	if(CreateProcess(NULL, cmd.GetBuffer(0), NULL, NULL, FALSE,
		0, NULL, NULL, &si, &pi) == FALSE) {
		return;
	}
	WaitForInputIdle(pi.hProcess, INFINITE);

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
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
