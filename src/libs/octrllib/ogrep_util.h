/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#include "EditCtrl.h"
#include "check_kanji.h"

struct _thr_grep_st {
	CEditCtrl	*edit_ctrl;
	TCHAR		*search_text;
	TCHAR		*file_type;
	TCHAR		*search_folder;
	BOOL		distinct_lwr_upr;
	BOOL		distinct_width_ascii;
	BOOL		regexp;
	BOOL		search_sub_folder;
	int			kanji_code;
	HWND		hwnd;
	int			search_cnt;
	TCHAR		*msg_buf;
	int			ret_v;
};

unsigned int _stdcall GrepThr(void *lpvThreadParam);

int Grep(CEditCtrl *edit_ctrl, TCHAR *search_text, TCHAR *file_type, 
	TCHAR *search_folder, BOOL distinct_lwr_upr, BOOL distinct_width_ascii, BOOL regexp, BOOL search_sub_folder,
	int *search_cnt, int kanji_code, HWND hwnd, TCHAR *msg_buf);
