/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "StdAfx.h"
#include "fileutil.h"
#include "str_inline.h"

BOOL GetTagData(const TCHAR *tag_data, CString &file_name, int &row)
{
	const TCHAR *p1 = tag_data;

	for(; *p1 == ' ' || *p1 == '\t'; p1++) ;		// スペースを読み飛ばす

	const TCHAR *p2 = p1;
	if(p2[1] == ':') {	// フルパス
		p2 += 2;
	}
	
	const TCHAR *p3 = _tcschr(p2, ':');
	const TCHAR *p4 = _tcschr(p2, '(');

	const TCHAR *p_name_end = NULL;
	if(p3 != NULL) p_name_end = p3;
	if((p_name_end == NULL) || (p4 != NULL && p4 < p3)) p_name_end = p4;
	if(p_name_end == NULL) p_name_end = p1 + _tcslen(p1);

	for(; p_name_end;) {
		file_name.Format(_T("%.*s"), p_name_end - p1, p1);
		file_name.TrimLeft();
		file_name.TrimRight();

		if(file_name.GetLength() >= _MAX_PATH) return FALSE;
		make_full_path(file_name.GetBuffer(_MAX_PATH));
		file_name.ReleaseBuffer();

		if(is_file_exist(file_name)) break;
		if(p4 != NULL) p4 = _tcschr(p4 + 1, '(');
		p_name_end = p4;
	}
	if(p_name_end == NULL) return FALSE;
	
	row = 0;

	p1 = p_name_end + 1; // skip ':' or '('
	if(!inline_isdigit(*p1)) return TRUE;

	for(; inline_isdigit(*p1); p1++) {
		row = row * 10 + (*p1 - '0');
	}
	row--;
	if(row < 0) return FALSE;

	return TRUE;
}

