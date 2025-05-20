/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef __OCTRL_UTIL_H_INCLUDED__
#define __OCTRL_UTIL_H_INCLUDED__

#include "editdata.h"
#include "oregexp.h"
#include <vector>

// 関数を抜けるとき，カーソル位置を元に戻すためのオブジェクト
class CEditDataSaveCurPt
{
private:
	CEditData	*m_edit_data;
	POINT		m_cur_pt;

public:
	CEditDataSaveCurPt(CEditData *edit_data) : m_edit_data(edit_data) { SetCurPt(); }
	~CEditDataSaveCurPt() { ResetPt(); }

	POINT GetPt() { return m_cur_pt; }
	void SetCurPt() { m_cur_pt = m_edit_data->get_cur_pt(); }
	void ResetPt() { m_edit_data->set_cur(m_cur_pt.y, m_cur_pt.x); }
};

class CRegData
{
private:
	HREG_DATA	m_reg_data;

	void clear();

public:
	CRegData() : m_reg_data(NULL) {}
	~CRegData() { clear(); }

	BOOL Compile(const TCHAR *pattern, BOOL b_distinct_lwr_upr, BOOL b_regexp = TRUE);
	BOOL Compile2(const TCHAR *pattern, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp = TRUE);
	HREG_DATA GetRegData() { return m_reg_data; }
};

//void octrl_set_debug_filename(const char *file_name);
//void octrl_debug_msg(const char *format, ...);


typedef std::vector<LONG> FontSizeList;
typedef FontSizeList::iterator FontSizeListIter;
BOOL EnumFontSize(HDC hdc, TCHAR *font_name, FontSizeList *font_size_list);
int GetSmallFontSize(HDC hdc, const TCHAR *font_name, int cur_size);
int GetLargeFontSize(HDC hdc, const TCHAR *font_name, int cur_size);
BOOL IsFontExist(const TCHAR *font_name);

#endif __OCTRL_UTIL_H_INCLUDED__
