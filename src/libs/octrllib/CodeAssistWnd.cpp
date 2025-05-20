/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
// CodeAssistWnd.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "CodeAssistWnd.h"

#include "ostrutil.h"
#include "get_char.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CODE_ASSIST_SCROLL_ROW_CNT		10

IMPLEMENT_DYNAMIC(CCodeAssistWnd, CGridCtrl)
/////////////////////////////////////////////////////////////////////////////
// CCodeAssistWnd

CCodeAssistWnd::CCodeAssistWnd()
{
	m_focus_timer = 0;
	m_mode = AssistInitial;

	m_cur_griddata = NULL;
	m_orig_griddata = NULL;
	m_parent_wnd = NULL;
	m_row_height = 0;
	m_window_pt.x = m_window_pt.y = 0;

	// FIXME: optionで設定可能にする
	m_incremental_search = FALSE;
	m_disp_cnt = CODE_ASSIST_SCROLL_ROW_CNT;
	m_max_comment_disp_width = 240;

	m_assist_match_type = ASSIST_FORWARD_MATCH;
}

CCodeAssistWnd::~CCodeAssistWnd()
{
}

BEGIN_MESSAGE_MAP(CCodeAssistWnd, CGridCtrl)
	//{{AFX_MSG_MAP(CCodeAssistWnd)
	ON_WM_SETFOCUS()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_TIMER()
	ON_WM_ACTIVATEAPP()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCodeAssistWnd メッセージ ハンドラ

BOOL CCodeAssistWnd::Create(CWnd *pParentWnd)
{
	m_parent_wnd = pParentWnd;

	if(CGridCtrl::CreateEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
		GRIDCTRL_CLASSNAME, _T(""),
		WS_POPUP | WS_DLGFRAME, CRect(0, 0, 100, 100),
		pParentWnd, 0) == FALSE) {
		return FALSE;
	}

	return TRUE;
}

void CCodeAssistWnd::CopyAllDispData()
{
	m_disp_griddata.DeleteAllRow();

	for(int r = 0; r < m_orig_griddata->Get_RowCnt(); r++) {
		m_disp_griddata.AddRow();
		for(int c = 0; c < m_orig_griddata->Get_ColCnt(); c++) {
			m_disp_griddata.SetData(r, c, m_orig_griddata->Get_ColData(r, c));
		}
	}
}

void CCodeAssistWnd::SetGridData(CCodeAssistData *griddata)
{
	m_orig_griddata = griddata;

	if(m_incremental_search) {
		m_disp_griddata.InitData(FALSE);
		CopyAllDispData();
		for(int c = 0; c < m_orig_griddata->Get_ColCnt(); c++) {
			m_disp_griddata.SetDispColWidth(c, griddata->GetDispColWidth(c));
		}
		m_orig_griddata->CopySetting(&m_disp_griddata);

		CGridCtrl::SetGridData(&m_disp_griddata);
		m_cur_griddata = &m_disp_griddata;
	} else {
		CGridCtrl::SetGridData(m_orig_griddata);
		m_cur_griddata = m_orig_griddata;
	}

	m_cur_griddata->SetMaxCommentDispWidth(m_max_comment_disp_width);

	AdjustAllColWidth(FALSE, TRUE);
	SetAssistMode(AssistTemp);
}

void CCodeAssistWnd::OnSetFocus(CWnd* pOldWnd) 
{
	CGridCtrl::OnSetFocus(pOldWnd);

	m_focus_timer = SetTimer(1000, 100, NULL);
}

void CCodeAssistWnd::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if(m_grid_data->get_cur_row() >= 0) {
		m_parent_wnd->PostMessage(CAW_WM_DBLCLICK, 0, 0);
		return;
	}
	
	CGridCtrl::OnLButtonDblClk(nFlags, point);
}

BOOL CCodeAssistWnd::GetMatchDataForwardMatch(const TCHAR* word, int* match_row, BOOL no_break_cur_grid_data)
{
	int		word_len = static_cast<int>(_tcslen(word));
	const TCHAR* grid_data;
	int		row;

	int		cmp_len;
	int		max_cmp_len = 0;
	int		result_row = -1;

	unsigned int first_ch = get_char_nocase(word);
	unsigned int grid_data_ch;

	int grid_row = m_orig_griddata->Get_RowCnt();

	for(row = 0; row < grid_row; row++) {
		grid_data = m_orig_griddata->Get_ColData(row, CODE_ASSIST_DATA_NAME);

		// 高速化
		grid_data_ch = get_char_nocase(grid_data);
		if(first_ch != grid_data_ch) {
			if(!m_incremental_search) {
				m_orig_griddata->EnableRow(row, FALSE);
			}
			continue;
		}

		cmp_len = ostr_get_cmplen_nocase(word, grid_data, word_len);
		if(cmp_len > max_cmp_len) {
			max_cmp_len = cmp_len;
			result_row = row;
		}
		
		if(m_incremental_search && !no_break_cur_grid_data) {
			if(cmp_len == word_len) {
				m_disp_griddata.AddRow();
				int row_idx = m_disp_griddata.Get_RowCnt() - 1;
				for(int c = 0; c < m_orig_griddata->Get_ColCnt(); c++) {
					m_disp_griddata.SetData(row_idx, c, m_orig_griddata->Get_ColData(row, c));
				}
			}
		}

		if(!m_incremental_search) {
			m_orig_griddata->EnableRow(row, (cmp_len == word_len));
		}
	}

	if(match_row != NULL) *match_row = result_row;

	if(max_cmp_len == word_len) return TRUE;

	return FALSE;
}


BOOL CCodeAssistWnd::GetMatchDataPartialMatch(const TCHAR* word, int* match_row, BOOL no_break_cur_grid_data)
{
	int		word_len = static_cast<int>(_tcslen(word));
	const TCHAR* grid_data;
	int		row;

	int		result_row = -1;
	BOOL	func_result = FALSE;

	int grid_row = m_orig_griddata->Get_RowCnt();

	for(row = 0; row < grid_row; row++) {
		grid_data = m_orig_griddata->Get_ColData(row, CODE_ASSIST_DATA_NAME);

		BOOL comp_result = FALSE;
		if(ostr_strstr_nocase(grid_data, word, word_len) != NULL) {
			comp_result = TRUE;
			func_result = TRUE;
		}

		if(m_incremental_search && !no_break_cur_grid_data) {
			if(comp_result) {
				m_disp_griddata.AddRow();
				int row_idx = m_disp_griddata.Get_RowCnt() - 1;
				for(int c = 0; c < m_orig_griddata->Get_ColCnt(); c++) {
					m_disp_griddata.SetData(row_idx, c, m_orig_griddata->Get_ColData(row, c));
				}
				if(result_row == -1) {
					result_row = row_idx;
				}
			}
		}

		if(!m_incremental_search) {
			m_orig_griddata->EnableRow(row, comp_result);
			if(comp_result && result_row == -1) {
				result_row = row;
			}
		}
	}

	if(match_row != NULL) *match_row = result_row;

	return func_result;
}

BOOL CCodeAssistWnd::GetMatchData(const TCHAR *word, int *match_row, BOOL no_break_cur_grid_data)
{
	int		word_len = static_cast<int>(_tcslen(word));

	if(word_len == 0) {
		if(match_row != NULL) *match_row = -1;

		if(m_incremental_search) {
			CopyAllDispData();
			RecalcWindowSize();
		} else {
			m_orig_griddata->EnableAllRow();
			Invalidate();
		}

		return FALSE;
	}

	if(m_incremental_search && !no_break_cur_grid_data) {
		m_disp_griddata.DeleteAllRow();
	}

	BOOL result = FALSE;
	
	if(m_assist_match_type == ASSIST_FORWARD_MATCH) {
		result = GetMatchDataForwardMatch(word, match_row, no_break_cur_grid_data);
	} else if(m_assist_match_type == ASSIST_PARTIAL_MATCH) {
		result = GetMatchDataPartialMatch(word, match_row, no_break_cur_grid_data);
	}

	if(m_incremental_search && !no_break_cur_grid_data) {
//		SetCurGridData(&m_disp_griddata);
		RecalcWindowSize();
	} else {
		Invalidate();
	}

	return result;
}

void CCodeAssistWnd::SelectData(const TCHAR *word)
{
	int match_row;
	if(GetMatchData(word, &match_row, FALSE)) {
		SetAssistMode(AssistCommit);
	} else {
		SetAssistMode(AssistTemp);
	}

	if(match_row >= 0) {
		if(m_incremental_search) {
			SetCell(0, 0);
		} else {
			SetCell(0, match_row);
		}
	} else {
		UnSelect();
	}
}

void CCodeAssistWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CGridCtrl::OnLButtonDown(nFlags, point);

	if(m_grid_data->get_cur_row() >= 0) {
		SetAssistMode(AssistCommit);
	}
}

void CCodeAssistWnd::SetAssistMode(CodeAssistMode mode)
{
	if(m_mode == mode) return;

	m_mode = mode;

	if(mode == AssistCommit) {
		SetGridStyle(GetGridStyle() | GRS_LINE_SELECT);
	} else {
		SetGridStyle(GetGridStyle() & (~GRS_LINE_SELECT));
	}
	if(m_grid_data->get_cur_row() >= 0) {
		InvalidateRow(m_grid_data->get_cur_row());
	}
}

void CCodeAssistWnd::UnSelect()
{
	CGridCtrl::UnSelect();
	SetAssistMode(AssistTemp);
}

int CCodeAssistWnd::CalcWindowWidth()
{
	int	width = 10;

	for(int c = 0; c < m_grid_data->Get_ColCnt(); c++) {
		width += GetDispColWidth(c);
	}

	if(m_grid_data->Get_RowCnt() >= GetDispCnt()) {
		width += GetSystemMetrics(SM_CXVSCROLL);
	}

	return width;
}

int CCodeAssistWnd::CalcWindowHeight()
{
	int	height = 10;

	if(m_grid_data->Get_RowCnt() >= GetDispCnt()) {
		height += GetRowHeight() * GetDispCnt();
	} else {
		height += GetRowHeight() * m_grid_data->Get_RowCnt();
	}

	return height;
}

void CCodeAssistWnd::SetAssistWindowPos(POINT pt, int row_height)
{
	m_window_pt = pt;
	m_row_height = row_height;
	RecalcWindowSize();
}

void CCodeAssistWnd::RecalcWindowSize()
{
	POINT point = m_window_pt;

	int height = CalcWindowHeight();

	RECT work_rect;
	if(SystemParametersInfo(SPI_GETWORKAREA, 0, &work_rect, 0)) {
		if(point.y + height > work_rect.bottom) {
			point.y -= (m_row_height + height);
		}
	}

	SetWindowPos(NULL,
		point.x, point.y,
		CalcWindowWidth(), height,
		SWP_NOZORDER | SWP_NOACTIVATE);

	Invalidate();
}

void CCodeAssistWnd::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == m_focus_timer) {
		m_parent_wnd->SetFocus();

		KillTimer(m_focus_timer);
		m_focus_timer = 0;
	}
	
	CGridCtrl::OnTimer(nIDEvent);
}

void CCodeAssistWnd::OnActivateApp(BOOL bActive, DWORD hTask) 
{
	CGridCtrl::OnActivateApp(bActive, hTask);

	m_parent_wnd->PostMessage(CAW_WM_ACTIVATEAPP, (WPARAM)bActive);
}

void CCodeAssistWnd::OnDestroy() 
{
	if(m_focus_timer != 0){
		KillTimer(m_focus_timer);
		m_focus_timer = 0;
	}	

	CGridCtrl::OnDestroy();
}

void CCodeAssistWnd::LineUp(BOOL b_loop, CString word)
{
	SetAssistMode(AssistCommit);

	if(word.IsEmpty()) {
		if(b_loop && m_grid_data->get_cur_row() <= 0) {
			SetCell(0, m_grid_data->Get_RowCnt() - 1);
			return;
		}

		CGridCtrl::LineUp();
		return;
	}

	// wordに一致する候補に移動する
	int word_len = word.GetLength();
	unsigned int first_ch = get_char_nocase(word.GetBuffer(0));

	for(int row = m_grid_data->get_cur_row() - 1; row >= 0; row--) {
		const TCHAR *grid_data = m_grid_data->Get_ColData(row, CODE_ASSIST_DATA_NAME);

		// 高速化
		if(get_char_nocase(grid_data) != first_ch) continue;

		if(ostr_get_cmplen_nocase(word, grid_data, word_len) == word_len) {
			CGridCtrl::SetCell(0, row);
			break;
		}
	}
}

void CCodeAssistWnd::LineDown(BOOL b_loop, CString word)
{
	SetAssistMode(AssistCommit);

	if(word.IsEmpty()) {
		if(b_loop && m_grid_data->get_cur_row() == m_grid_data->Get_RowCnt() - 1) {
			SetCell(0, 0);
			return;
		}

		CGridCtrl::LineDown();
		return;
	}

	// wordに一致する候補に移動する
	int word_len = word.GetLength();
	unsigned int first_ch = get_char_nocase(word.GetBuffer(0));

	for(int row = m_grid_data->get_cur_row() + 1; row < m_grid_data->Get_RowCnt(); row++) {
		const TCHAR *grid_data = m_grid_data->Get_ColData(row, CODE_ASSIST_DATA_NAME);

		// 高速化
		if(get_char_nocase(grid_data) != first_ch) continue;

		if(ostr_get_cmplen_nocase(word, grid_data, word_len) == word_len) {
			CGridCtrl::SetCell(0, row);
			break;
		}
	}
}
