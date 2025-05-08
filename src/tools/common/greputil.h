/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */


#ifndef _GREPUTIL_H_INCLUDED
#define _GREPUTIL_H_INCLUDED

#define OEDIT_CLASS_NAME		_T("OEDIT_CLASS_NAME")
#define WM_COPY_DATA_GET_FILE_NAME		1001
#define WM_COPY_DATA_SET_SEARCH_DATA	1002

struct _set_search_data_st {
	TCHAR		search_text[1024];
	BOOL		distinct_lwr_upr;
	BOOL		distinct_width_ascii;
	BOOL		regexp;
	int			row;
};

HWND OeditCheckFileIsOpen(const TCHAR *file_name);
void OeditOpenFile(const TCHAR *file_name);


#define OTBEDIT_VIEW_WINDOWNAME	_T("OTBEDIT_VIEW_WINDOWNAME")
#define WM_COPY_DATA_OPEN_FILE			1003
#define WM_COPY_DATA_ACTIVE_FILE		1004

struct _set_search_data_st_otbedit {
	TCHAR		file_name[_MAX_PATH];
	TCHAR		search_text[1024];
	BOOL		distinct_lwr_upr;
	BOOL		distinct_width_ascii;
	BOOL		regexp;
	int			row;
};

HWND OtbeditCheckFileIsOpen(HWND wnd, const TCHAR *file_name);
void OtbeditOpenFile(const TCHAR *file_name);

#endif _GREPUTIL_H_INCLUDED