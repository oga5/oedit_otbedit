/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#include "stdafx.h"
#include <stdio.h>
#include <string.h>

#include "CodeAssistData.h"
#include "colorutil.h"

CCodeAssistData::CCodeAssistData()
{
	m_max_comment_disp_width = 240;
	m_gray_candidate = FALSE;

	m_bk_color = m_keyword2_color = m_keyword_color = m_text_color = RGB(0, 0, 0);
}

void CCodeAssistData::InitData(BOOL b_disp_comment)
{
	Init(_CODE_ASSIST_COL_CNT_, 64);
	SetDispFlg(CODE_ASSIST_DATA_HINT, FALSE);
	SetDispFlg(CODE_ASSIST_DATA_CURSOR_SQL, FALSE);
	SetDispFlg(CODE_ASSIST_DATA_BK_COLOR, FALSE);
	SetDispFlg(CODE_ASSIST_DATA_COMMENT, b_disp_comment);
}

void CCodeAssistData::SetMaxCommentDispWidth(int w)
{
	if(w < 200) w = 200;
	if(w > 1000) w = 1000;
	m_max_comment_disp_width = w;
}

void CCodeAssistData::SetDispColWidth(int col, int width)
{
	if(col == CODE_ASSIST_DATA_COMMENT) {
		if(width > m_max_comment_disp_width) width = m_max_comment_disp_width;
	}

	CGridData::SetDispColWidth(col, width);
}

int CCodeAssistData::getKeywordType(int row, int col)
{
	if(_tcscmp(Get_ColData(row, CODE_ASSIST_DATA_TYPE), _T("KEYWORD")) == 0) return 1;
	if(_tcscmp(Get_ColData(row, CODE_ASSIST_DATA_TYPE), _T("KEYWORD2")) == 0) return 2;
	return 0;
}

BOOL CCodeAssistData::IsEnableRow(int row)
{
	if(_tcscmp(Get_ColData(row, CODE_ASSIST_DATA_BK_COLOR), _T("DISABLE")) == 0) return FALSE;
	return TRUE;
}

COLORREF CCodeAssistData::GetCellTextColor(int row, int col)
{
	COLORREF color = m_text_color;

	int type = getKeywordType(row, col);
	if(type == 1) color = m_keyword_color;
	if(type == 2) color = m_keyword2_color;

	if(m_gray_candidate && !IsEnableRow(row)) {
		// 背景色とテキストをブレンドする
		color = mix_color(m_bk_color, color, 0.5);
	}

	return color;
}

void CCodeAssistData::EnableRow(int row, BOOL enable)
{
	static const TCHAR *enable_str = _T("");
	static const TCHAR *disable_str = _T("DISABLE");

	if(enable) {
		SetData(row, CODE_ASSIST_DATA_BK_COLOR, enable_str);
	} else {
		SetData(row, CODE_ASSIST_DATA_BK_COLOR, disable_str);
	}
}

void CCodeAssistData::EnableAllRow()
{
	for(int	row = 0; row < Get_RowCnt(); row++) {
		EnableRow(row, TRUE);
	}
}

void CCodeAssistData::CopySetting(CCodeAssistData *data)
{
	data->SetGrayCandidate(m_gray_candidate);
	data->SetKeywordColor(m_keyword_color);
	data->SetKeyword2Color(m_keyword2_color);
	data->SetTextColor(m_text_color);
	data->SetBkColor(m_bk_color);
	data->SetDispData(GetDispData());
}

