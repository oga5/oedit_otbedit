/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */


#ifndef __SEARCH_DLG_DATA_H_INCLUDED__
#define __SEARCH_DLG_DATA_H_INCLUDED__

class CSearchDlgData
{
public:
	CSearchDlgData();

	CString	m_search_text;
	CString	m_replace_text;
	int		m_dir;
	BOOL	m_distinct_lwr_upr;
	BOOL	m_distinct_width_ascii;
	BOOL	m_regexp;
	BOOL	m_all;
	BOOL	m_replace_selected_area;
};

void LoadSearchTextList(CComboBox *combo, CString section, int max_cnt, int *status_arr);
void SaveSearchTextList(CString section, int max_cnt, CString cur_text,
	BOOL b_save_status, int cur_status);
//int make_search_status(BOOL b_regexp, BOOL b_distinct_lwr_upr);
int make_search_status2(BOOL b_regexp, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii);
BOOL is_regexp(int status);
BOOL is_distinct_lwr_upr(int status);
BOOL is_distinct_width_ascii(int status);

CString MakeSearchMsg(int ret_v, int dir, BOOL b_looped);

void UpdateDataCombo(CComboBox &combo, CString &text, BOOL bSaveAndValidate);

#endif __SEARCH_DLG_DATA_H_INCLUDED__
