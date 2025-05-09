/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "filenames.h"
#include "fileutil.h"

#include "str_inline.h"

CFileNameOption::CFileNameOption(const TCHAR *file_name)
{
	m_file_name = file_name;
	m_row = -1;
}

void CBootParameter::AddFileName(const TCHAR *file_name)
{
	if(_tcscmp(file_name, _T("")) == 0) return;

	CString str_file_name;
	TCHAR	long_name[_MAX_PATH * 2];

	if(IsShortcut(file_name)) {
		if(GetFullPathFromShortcut(file_name, &str_file_name)) {
			file_name = str_file_name;
		}
	}

	if(GetLongPath(file_name, long_name) != FALSE) {
		m_file_name_opt_arr.Add(new CFileNameOption(long_name));
	} else {
		m_file_name_opt_arr.Add(new CFileNameOption(file_name));
	}
}

CBootParameter::~CBootParameter()
{
	for(; m_file_name_opt_arr.GetSize() != 0;) {
		CFileNameOption *data = (CFileNameOption *)m_file_name_opt_arr.GetAt(0);
		m_file_name_opt_arr.RemoveAt(0);
		delete data;
	}
}

CString CBootParameter::ParseFileName(const TCHAR *&p1)
{
	CString result;

	for(; *p1 == ' '; p1++) ;	// スペースを読み飛ばす

	if(*p1 == '"') {
		p1++;
		const TCHAR *p2 = _tcschr(p1, '"');
		if(p2 == NULL) {
			result = p1;
			p1 = p1 + _tcslen(p1);
		} else {
			ptrdiff_t pd = p2 - p1;
			result.Format(_T("%.*s"), (int)pd, p1);
			p1 = p2 + 1;
		} 
	} else {
		const TCHAR *p2 = _tcsstr(p1 + 2, _T("\\\\"));
		const TCHAR *p3 = _tcschr(p1 + 2, ':');
		const TCHAR *p4 = _tcsstr(p1 + 2, _T(" /"));
		const TCHAR *p5 = _tcschr(p1 + 2, '"');

		if(p2 == NULL && p3 == NULL && p4 == NULL && p5 == NULL) {
			result = p1;
			p1 = p1 + _tcslen(p1);
		} else {
			const TCHAR *p_name_end = NULL;
			if(p2 != NULL) p_name_end = p2 - 1;
			if((p_name_end == NULL) || (p3 != NULL && (p3 - 2) < p_name_end)) p_name_end = p3 - 2;
			if((p_name_end == NULL) || (p4 != NULL && p4 < p_name_end)) p_name_end = p4;
			if((p_name_end == NULL) || (p5 != NULL && (p5 - 1) < p_name_end)) p_name_end = p5 - 1;

			ptrdiff_t pd = p_name_end - p1;
			result.Format(_T("%.*s"), (int)pd, p1);
			p1 = p_name_end + 1;
		}
	}

	for(; *p1 == ' '; p1++) ;	// スペースを読み飛ばす

	result.Replace(_T("/"), _T("\\"));	// パスの'/'を'\'に変換する (I-O DataのmAgicマネージャ対応)
	result.TrimLeft();
	result.TrimRight();

	make_full_path(result.GetBuffer(_MAX_PATH));
	result.ReleaseBuffer();

	return result;
}

CString CBootParameter::ParseOptionData(const TCHAR *&p1)
{
	CString result;

	for(; *p1 == ' '; p1++) ;	// スペースを読み飛ばす

	if(*p1 == '"') {
		p1++;
		const TCHAR *p2 = _tcschr(p1, '"');
		if(p2 == NULL) {
			result = p1;
			p1 = p1 + _tcslen(p1);
		} else {
			ptrdiff_t pd = p2 - p1;
			result.Format(_T("%.*s"), (int)pd, p1);
			p1 = p2 + 1;
		} 
	} else {
		// 次のスペースまで
		const TCHAR *p2 = _tcschr(p1, ' ');

		if(p2 == NULL) {
			result = p1;
			p1 = p1 + _tcslen(p1);
		} else {
			ptrdiff_t pd = p2 - p1;
			result.Format(_T("%.*s"), (int)pd, p1);
			p1 = p2 + 1;
		} 
	}

	for(; *p1 == ' '; p1++) ;	// スペースを読み飛ばす

	result.TrimLeft();
	result.TrimRight();

	result.ReleaseBuffer();

	return result;
}

BOOL CBootParameter::ParseOption(const TCHAR *&p1, TCHAR *msg_buf)
{
	if(*p1 != '/') return FALSE;
	p1++;

	switch(*p1) {
	case 'r':
		{
			p1++;
			for(; *p1 == ' '; p1++) ;	// スペースを読み飛ばす

			int row = 0;
			for(;;) {
				if(*p1 == ' ' || *p1 == '\0') break;
				if(!inline_isdigit(*p1)) {
					if(msg_buf != NULL) _stprintf(msg_buf, _T("起動オプションエラー：/rのあとは数字の指定が必要です"));
					return FALSE;
				}

				row = row * 10 + (*p1 - '0');
				p1++;
			}
			if(row > 0) row--;

			if(GetFileCnt() != 0) {
				CString row_str;
				row_str.Format(_T("%d"), row);
				CFileNameOption *file = (CFileNameOption *)m_file_name_opt_arr.GetAt(GetFileCnt() - 1);
				file->SetOption(_T("r"), row_str);
			}
		}
		break;
	case 'd':
		{
			p1++;
			CString data;
			data = ParseFileName(p1);
			m_option.SetAt(_T("d"), data);
		}
		break;
	case 'm':
		{
			p1++;
			CString data = ParseOptionData(p1);
			if(GetFileCnt() != 0) {
				CFileNameOption *file = (CFileNameOption *)m_file_name_opt_arr.GetAt(GetFileCnt() - 1);
				file->SetOption(_T("m"), data);
			} else {
				m_option.SetAt(_T("m"), data);
			}
		}
		break;
	default:
		{
			CString key;
			key.Format(_T("%c"), *p1);
			p1++;
			CString data;
			data = ParseOptionData(p1);
			m_option.SetAt(key, data);
		}
		break;
	}

	for(; *p1 == ' '; p1++) ;	// スペースを読み飛ばす

	return TRUE;
}

CString CBootParameter::GetOption(const TCHAR *key)
{
	CString data;
	if(m_option.Lookup(key, data)) return data;
	return _T("");
}


BOOL CBootParameter::InitParameter(const TCHAR *cmd_line, TCHAR *msg_buf, BOOL file_name_flg)
{
	CString file_name;
	const TCHAR	*p1;

	p1 = cmd_line;

	for(; *p1 == ' '; p1++) ;	// スペースを読み飛ばす

	for(; *p1 != '\0';) {
		if(*p1 == '/') { // 起動オプション
			if(ParseOption(p1, msg_buf) == FALSE) return FALSE;
		} else {
			if(file_name_flg) {
				file_name = ParseFileName(p1);
				AddFileName(file_name);
			} else {
				file_name = ParseOptionData(p1);
				AddFileName(file_name);
			}
		}

		for(; *p1 == ' '; p1++) ;	// スペースを読み飛ばす
	}

	return TRUE;
}

CBootParameter::CBootParameter()
{
}

