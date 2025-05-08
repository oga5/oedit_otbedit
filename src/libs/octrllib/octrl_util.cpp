/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "octrl_util.h"

#include <stdarg.h>

#include <algorithm>

static char debug_file_name[1024] = "";

void octrl_set_debug_filename(const char *file_name)
{
	strcpy(debug_file_name, file_name);
	FILE *fp = fopen(debug_file_name, "w");
	if(fp != NULL) fclose(fp);
}

void octrl_debug_msg(const char *format, ...)
{
	va_list		ap;
	va_start(ap, format);

	if(strlen(debug_file_name) != 0) {
		FILE *fp = fopen(debug_file_name, "a");
		if(fp != NULL) {
			vfprintf(fp, format, ap);
			fclose(fp);
		}
	}

	va_end(ap);
}

void CRegData::clear()
{
	if(m_reg_data) oreg_free(m_reg_data); 
	m_reg_data = NULL;
}

BOOL CRegData::Compile2(const TCHAR *pattern, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp)
{
	int regexp_opt = 0;
	if(!b_distinct_lwr_upr) regexp_opt |= OREGEXP_OPT_IGNORE_CASE;
	if(!b_distinct_width_ascii) regexp_opt |= OREGEXP_OPT_IGNORE_WIDTH_ASCII;

	clear();
	if(b_regexp) {
		m_reg_data = oreg_comp2(pattern, regexp_opt);
	} else {
		m_reg_data = oreg_comp_str2(pattern, regexp_opt);
	}
	return (m_reg_data != NULL);
}

BOOL CRegData::Compile(const TCHAR *pattern, BOOL b_distinct_lwr_upr, BOOL b_regexp)
{
	return Compile2(pattern, b_distinct_lwr_upr, TRUE, b_regexp);
}

struct enum_font_size_st {
	HDC			hdc;
	int			logpixelsy;
	FontSizeList *font_size_list;
};

static int CALLBACK EnumFontSizeExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType, LPARAM lParam)
{
	struct enum_font_size_st *pst = (struct enum_font_size_st *)lParam;
	int iCalcFontSize = (int)((double)((double)(lpntme->ntmTm.tmHeight) - lpntme->ntmTm.tmInternalLeading) * 72 / (double)(pst->logpixelsy) + 0.5);
	
	if((FontType & TRUETYPE_FONTTYPE) || !((FontType & TRUETYPE_FONTTYPE) || (FontType & RASTER_FONTTYPE))) {
		int font_size_arr[] = {8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72};
		if(pst->font_size_list->size() == 0) {
			for(int i = 0; i < sizeof(font_size_arr)/sizeof(font_size_arr[0]); i++) {
				pst->font_size_list->push_back(font_size_arr[i]);
			}
		}
	} else {
		if(std::find(pst->font_size_list->begin(), pst->font_size_list->end(), iCalcFontSize) == pst->font_size_list->end()) {
			pst->font_size_list->push_back(iCalcFontSize);
		}
	}

	return 1;
}

BOOL EnumFontSize(HDC hdc, const TCHAR *font_name, FontSizeList *font_size_list)
{
	font_size_list->clear();

	LOGFONT	lf;
	memset(&lf, 0, sizeof(LOGFONT));
	_tcscpy(lf.lfFaceName, font_name);
	lf.lfCharSet = 128;

	struct enum_font_size_st st;
	st.hdc = hdc;
	st.font_size_list = font_size_list;
	st.logpixelsy = GetDeviceCaps(hdc, LOGPIXELSY);
	
	EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontSizeExProc, (LPARAM)&st, 0);

	std::sort(font_size_list->begin(), font_size_list->end());
/*
	for(FontSizeListIter it = font_size_list->begin(); it != font_size_list->end(); it++) {
		TRACE("%d\n", *it);
	}
*/
	return TRUE;
}

int GetLargeFontSize(HDC hdc, const TCHAR *font_name, int cur_size)
{
	FontSizeList	font_size_list;
	if(!EnumFontSize(hdc, font_name, &font_size_list)) return cur_size;

	for(FontSizeListIter it = font_size_list.begin(); it != font_size_list.end(); it++) {
		if(*it == cur_size) {
			it++;
			if(it != font_size_list.end()) return *it;
			return cur_size;
		}
	}
	return cur_size;
}

int GetSmallFontSize(HDC hdc, const TCHAR *font_name, int cur_size)
{
	FontSizeList	font_size_list;
	if(!EnumFontSize(hdc, font_name, &font_size_list)) return cur_size;

	int prev_size = cur_size;
	for(FontSizeListIter it = font_size_list.begin(); it != font_size_list.end(); it++) {
		if(*it == cur_size) return prev_size;
		prev_size = *it;
	}
	return cur_size;
}

static int CALLBACK EnumFontCnt(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType, LPARAM lParam)
{
	int *pc = (int *)lParam;
	(*pc)++;
	return 0;
}

BOOL IsFontExist(const TCHAR *font_name)
{
	LOGFONT    lf;
	memset(&lf, 0, sizeof(LOGFONT));
	_tcscpy(lf.lfFaceName, font_name);
	lf.lfCharSet = 128;

	int cnt = 0;
	HDC dc = ::GetDC(NULL);
	EnumFontFamiliesEx(dc, &lf, (FONTENUMPROC)EnumFontCnt, (LPARAM)&cnt, 0);
	::ReleaseDC(NULL, dc);

	if(cnt) return TRUE;
	return FALSE;
}
