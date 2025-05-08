/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "StdAfx.h"
#include "CommonUtil.h"
#include "GrepUtil.h"
#include "fileutil.h"
#include "file_winutil.h"
#include "TagJumpCommon.h"

void SetBootOption(const TCHAR *file_name, int row, const TCHAR *mode)
{
	HWND hwnd = OeditCheckFileIsOpen(file_name);
	if(hwnd == NULL) return;

	struct _set_option_st option_data;

	_tcscpy(option_data.file_name, file_name);
	option_data.row = row;
	_tcscpy(option_data.mode, mode);

	COPYDATASTRUCT	copy_data;
	copy_data.dwData = WM_COPY_DATA_SET_OPTION;
	copy_data.cbData = sizeof(struct _set_option_st);
	copy_data.lpData = &option_data;

	::SendMessage(hwnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&copy_data);
}

BOOL TagJump(const TCHAR *tag_data)
{
	CString file_name;
	int		row;

	if(GetTagData(tag_data, file_name, row) == FALSE) return FALSE;
	file_name = GetFullPath(file_name);
	if(is_file_exist(file_name) == FALSE) return FALSE;
	
	HWND	wnd;
	wnd = OeditCheckFileIsOpen(file_name);
	if(wnd == NULL) {
		OeditOpenFile(file_name);
	}
	
	SetBootOption(file_name, row, _T(""));

	wnd = OeditCheckFileIsOpen(file_name);
	if(wnd == NULL) return FALSE;
	
	::SetForegroundWindow(wnd);

	return TRUE;
}

