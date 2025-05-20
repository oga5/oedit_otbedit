/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"

#include "stdafx.h"
#include "ogrep_util.h"

#include "octrl_msg.h"
#include "octrl_util.h"
#include "oregexp.h"

__inline BOOL CheckCancel(HWND hwnd, TCHAR *msg_buf)
{
	if(hwnd == NULL) return FALSE;

	for(; SendMessage(hwnd, CND_WM_QUERY_CANCEL, 0, 0) == 2;) Sleep(500);
	if(SendMessage(hwnd, CND_WM_QUERY_CANCEL, 0, 0) == 1) {
		if(msg_buf != NULL) {
			_stprintf(msg_buf, _T("検索を中断しました\n"));
		}
		return TRUE;
	}
	return FALSE;
}

unsigned int _stdcall GrepThr(void *lpvThreadParam)
{
	struct _thr_grep_st *grep_st = (struct _thr_grep_st *)lpvThreadParam;

	grep_st->search_cnt = 0;

	grep_st->ret_v = Grep(grep_st->edit_ctrl, grep_st->search_text, grep_st->file_type,
		grep_st->search_folder, grep_st->distinct_lwr_upr, grep_st->distinct_width_ascii, grep_st->regexp,
		grep_st->search_sub_folder, &grep_st->search_cnt, grep_st->kanji_code,
		grep_st->hwnd, grep_st->msg_buf);

	if(grep_st->hwnd != NULL) {
		if(CheckCancel(grep_st->hwnd, grep_st->msg_buf) == TRUE) {
			grep_st->ret_v = 1;
		}
		PostMessage(grep_st->hwnd, CND_WM_EXIT, grep_st->ret_v, 0);
	}

	if(grep_st->ret_v == 0 && grep_st->msg_buf != NULL) {
		_stprintf(grep_st->msg_buf, _T("%d個，検索されました。\n"), grep_st->search_cnt);
	}

	return grep_st->ret_v;
}

static int GrepFile(CEditCtrl *edit_ctrl, TCHAR *search_text, TCHAR *file_name,
	BOOL distinct_lwr_upr, BOOL distinct_width_ascii, BOOL regexp, int *search_cnt, int kanji_code,
	HWND hwnd, TCHAR *msg_buf, HREG_DATA reg_data)
{
	CEditData	edit_data;
	int			line_type = LINE_TYPE_CR_LF;
	int			ret_v;
	POINT		pt;
	CString		msg;

	if(hwnd != NULL) {
		msg.Format(_T("ファイル: %s"), file_name);
		SendMessage(hwnd, CND_WM_STATIC, 2, (LPARAM)msg.GetBuffer(0));
	}

	try {
		CFile		file(file_name, CFile::modeRead|CFile::shareDenyNone);
		CArchive	ar(&file, CArchive::load);
		edit_data.load_file(ar, kanji_code, line_type);
		ar.Close();
		file.Close();
	}
	catch(CFileException* e) {
		TCHAR	msg[1024];
		e->GetErrorMessage(msg, 1024);
		_stprintf(msg_buf, _T("Error %s: %s"), file_name, msg);

		// 別スレッドでEditCtrlにペーストしないようにする
		// Paste途中でOnPaintが走るとエラーになる
		edit_ctrl->SendMessage(EC_WM_PASTE_TEXT, (WPARAM)msg_buf);

		e->Delete();

		return 0;
	}

	edit_data.set_cur(0, 0);

	for(;;) {
		ret_v = edit_data.search_text_regexp(&pt, 1, 
			FALSE, NULL, NULL, reg_data, NULL);
		if(ret_v == -1) break;

		msg.Format(_T("%s(%d): %s\n"), file_name, pt.y + 1, edit_data.get_disp_row_text(pt.y));
		// 別スレッドでEditCtrlにペーストしないようにする
		// Paste途中でOnPaintが走るとエラーになる
		edit_ctrl->SendMessage(EC_WM_PASTE_TEXT, (WPARAM)msg.GetBuffer(0));

		if(search_cnt != NULL) (*search_cnt)++;

		if(edit_data.get_row_cnt() <= pt.y + 1) break;
		edit_data.set_cur(pt.y + 1, 0);

		if(CheckCancel(hwnd, msg_buf) == TRUE) return 1;
	}

	return 0;
}

int Grep(CEditCtrl *edit_ctrl, TCHAR *search_text, TCHAR *file_type, 
	TCHAR *search_folder, BOOL distinct_lwr_upr, BOOL distinct_width_ascii, BOOL regexp, BOOL search_sub_folder,
	int *search_cnt, int kanji_code, HWND hwnd, TCHAR *msg_buf)
{
	HANDLE				hFind;
	WIN32_FIND_DATA		find_data;
	TCHAR				find_path[_MAX_PATH];
	int					ret_v = 0;
	TCHAR				file_name[_MAX_PATH];
	CString				msg;
	CRegData			reg_data;
	CRegData			file_filter;
	BOOL				all_file_flg = FALSE;

	// file_typeが空か*.*が含まれている場合は全ファイル対象
	if(file_type == NULL || _tcslen(file_type) == 0 ||
		oregexp(_T("(?:^|;)\\*\\.\\*(?:;|$)"), file_type, NULL, NULL) == OREGEXP_FOUND) {
		all_file_flg = TRUE;
	} else {
		/* 
			file_typeの書式を正規表現に変換する (文字列を以下の順に置換する)
				先頭に\を追加
				末尾に$を追加
				; -> $|\
				. -> \.
				* -> .+
				\.+を削除
		*/
		CString file_type_regstr;
		file_type_regstr.Format(_T("^%s$"), file_type);
		file_type_regstr.Replace(_T(";"), _T("$|^"));
		file_type_regstr.Replace(_T("."), _T("\\."));
		file_type_regstr.Replace(_T("*"), _T(".+"));
		file_type_regstr.Replace(_T("^.+"), _T(""));
		if(!file_filter.Compile(file_type_regstr.GetBuffer(0), FALSE)) {
			_stprintf(msg_buf,
				_T("ファイルの種類の指定が不正です\n")
				_T("\n")
				_T("ファイルの種類の設定例\n")
				_T("  (1)全てのファイルを検索対象にする場合 -> *.*\n")
				_T("  (2)拡張子を指定する場合 -> *.txt\n")
				_T("  (3)ファイル名の一部を指定する場合 -> 200704*.txt\n")
				_T("  (4)複数の条件を指定する場合は;で区切ります -> *.txt;*.log\n"));
			return 3;
		}
	}

	if(!reg_data.Compile2(search_text, distinct_lwr_upr, distinct_width_ascii, regexp)) {
		_stprintf(msg_buf, _T("不正な正規表現です"));
		return 2;
	}

	if(hwnd != NULL) {
		msg.Format(_T("フォルダ: %s"), search_folder);
		SendMessage(hwnd, CND_WM_STATIC, 1, (LPARAM)msg.GetBuffer(0));
	}

	_stprintf(find_path, _T("%s\\*.*"), search_folder);

	hFind = FindFirstFile(find_path, &find_data);
	if(hFind == INVALID_HANDLE_VALUE) return 0;

	for(;;) {
		if(CheckCancel(hwnd, msg_buf) == TRUE) {
			FindClose(hFind);
			return 1;
		}

		if(_tcscmp(find_data.cFileName, _T(".")) == 0 || _tcscmp(find_data.cFileName, _T("..")) == 0) {
			if(FindNextFile(hFind, &find_data) == FALSE) break;
			continue;
		}

		_stprintf(file_name, _T("%s\\%s"), search_folder, find_data.cFileName);
		if((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			if(search_sub_folder) {
				ret_v = Grep(edit_ctrl, search_text, file_type, file_name, 
					distinct_lwr_upr, distinct_width_ascii, regexp, search_sub_folder, search_cnt, kanji_code,
					hwnd, msg_buf);
				if(hwnd != NULL) {
					msg.Format(_T("フォルダ: %s"), search_folder);
					SendMessage(hwnd, CND_WM_STATIC, 1, (LPARAM)msg.GetBuffer(0));
				}
			}
		} else {
			if(all_file_flg || 
				oreg_exec_str2(file_filter.GetRegData(), find_data.cFileName) == OREGEXP_FOUND) {
				ret_v = GrepFile(edit_ctrl, search_text, file_name,
					distinct_lwr_upr, distinct_width_ascii, regexp, search_cnt, kanji_code,
					hwnd, msg_buf, reg_data.GetRegData());
			}
		}
		if(ret_v != 0) {
			FindClose(hFind);
			return ret_v;
		}
		if(FindNextFile(hFind, &find_data) == FALSE) break;
	}

	FindClose(hFind);

	return 0;
}

