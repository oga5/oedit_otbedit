/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef _CODE_ASSIST_DATA_H_INCLUDE
#define _CODE_ASSIST_DATA_H_INCLUDE

#include "PtrGridData.h"

#define ASSIST_FORWARD_MATCH 0
#define ASSIST_PARTIAL_MATCH 1

#define CODE_ASSIST_DATA_NAME		0
#define CODE_ASSIST_DATA_TYPE		1
#define CODE_ASSIST_DATA_HINT		2
#define CODE_ASSIST_DATA_CURSOR_SQL	3
#define CODE_ASSIST_DATA_BK_COLOR	4
#define CODE_ASSIST_DATA_COMMENT	5
#define _CODE_ASSIST_COL_CNT_		6

class CCodeAssistData : public CPtrGridData
{
private:
	COLORREF	m_keyword_color;
	COLORREF	m_keyword2_color;
	COLORREF	m_text_color;
	COLORREF	m_bk_color;
	BOOL		m_gray_candidate;
	int			m_max_comment_disp_width;

	int getKeywordType(int row, int col);
	BOOL IsEnableRow(int row);

public:
	CCodeAssistData();

	void InitData(BOOL b_disp_comment);

	void SetGrayCandidate(BOOL enable) { m_gray_candidate = enable; }
	void SetKeywordColor(COLORREF col) { m_keyword_color = col; }
	void SetKeyword2Color(COLORREF col) { m_keyword2_color = col; }
	void SetTextColor(COLORREF col) { m_text_color = col; }
	void SetBkColor(COLORREF col) { m_bk_color = col; }

	void EnableRow(int row, BOOL enable);
	void EnableAllRow();

	virtual void SetDispColWidth(int col, int width);

	virtual COLORREF GetCellTextColor(int row, int col);

	void CopySetting(CCodeAssistData *data);

	void SetMaxCommentDispWidth(int w);
};

#endif _CODE_ASSIST_DATA_H_INCLUDE
