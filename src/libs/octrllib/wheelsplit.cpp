/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

///////////////////////////////////////////////////////////////////////////// 
// WheelSplitterWnd.cpp
#include "stdafx.h"

#include "wheelsplit.h"
#include "octrl_msg.h"

IMPLEMENT_DYNCREATE(CWheelSplitterWnd, CSplitterWnd)

BEGIN_MESSAGE_MAP(CWheelSplitterWnd, CSplitterWnd)
	//{{AFX_MSG_MAP(CWheelSplitterWnd)
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONUP()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CWheelSplitterWnd::CWheelSplitterWnd()
{
	m_delete_pane_msg = 0;
	m_wheel_mode = WHEEL_MODE_VSCROLL_MSG | WHEEL_MODE_ACTIVE_VIEW;

	// splitter barの幅
	// 標準は7, 細くする場合は5 or 6にする (4以下だと表示がおかしくなったり、マウスでDragできなくなったりする)
	// FIXME: 設定可能にする
//	m_cxSplitter = m_cySplitter = 6;		// Drag中に表示する線の幅
//	m_cxBorderShare = m_cyBorderShare = 0;
//	m_cxSplitterGap = m_cySplitterGap = 6;	// barの幅
//	m_cxBorder = m_cyBorder = 2;
}

CWheelSplitterWnd::~CWheelSplitterWnd()
{
}

BOOL CWheelSplitterWnd::OnMouseWheel(UINT fFlags, short zDelta, CPoint point)
{
	CWnd		*pPane = NULL;

	if(m_wheel_mode & WHEEL_MODE_ACTIVE_VIEW) {
		pPane = GetActivePane();
	} else {
		int		c, r;
		for(r = 0; r < GetRowCount(); r++) {
			for(c = 0; c < GetColumnCount(); c++) {
				CWnd *pWnd = GetPane(r, c);
				if(pWnd == NULL) continue;

				CRect rect;
				pWnd->GetWindowRect(rect);
				if(rect.PtInRect(point)) {
					pPane = pWnd;
					break;
				}
			}
		}
	}
	if(pPane == NULL) return TRUE;

	if(m_wheel_mode & WHEEL_MODE_VSCROLL_MSG) {
		if(zDelta > 0) {
			pPane->SendMessage(WM_VSCROLL, MAKELPARAM(SB_LINEUP, 0), 0);
		} else {
			pPane->SendMessage(WM_VSCROLL, MAKELPARAM(SB_LINEDOWN, 0), 0);
		}
	} else {
		pPane->SendMessage(WSW_WM_WHEEL_TRANSFER_MSG, MAKELPARAM(fFlags, zDelta), MAKEWPARAM(point.x, point.y));
	}

	return TRUE;
}

BOOL CWheelSplitterWnd::SplitRow( int cyBefore )
{
	if(GetActivePane() == NULL) {
		SetActivePane(0, 0);
	}

	int		r, c;
	int		scr_y = GetPane(0, 0)->GetScrollPos(SB_VERT);
	int		scr_x[10] = {};
	for(c = 0; c < GetColumnCount(); c++) {
		scr_x[c] = GetPane(0, c)->GetScrollPos(SB_HORZ);
	}

	BOOL result = CSplitterWnd::SplitRow(cyBefore);

	for(r = 0; r < GetRowCount(); r++) {
		for(c = 0; c < GetColumnCount(); c++) {
			GetPane(r, c)->SetScrollPos(SB_HORZ, scr_x[c]);
			GetPane(r, c)->SetScrollPos(SB_VERT, scr_y);
			GetPane(r, c)->Invalidate();
		}
	}

	return result;

//	return CSplitterWnd::SplitRow(cyBefore);
}

BOOL CWheelSplitterWnd::SplitColumn( int cxBefore )
{
	if(GetActivePane() == NULL) {
		SetActivePane(0, 0);
	}

	int		r, c;
	int		scr_x = GetPane(0, 0)->GetScrollPos(SB_HORZ);
	int		scr_y[10] = {};
	for(r = 0; r < GetRowCount(); r++) {
		scr_y[r] = GetPane(r, 0)->GetScrollPos(SB_VERT);
	}

	BOOL result = CSplitterWnd::SplitColumn(cxBefore);

	for(r = 0; r < GetRowCount(); r++) {
		for(c = 0; c < GetColumnCount(); c++) {
			GetPane(r, c)->SetScrollPos(SB_VERT, scr_y[r]);
			GetPane(r, c)->SetScrollPos(SB_HORZ, scr_x);
			GetPane(r, c)->Invalidate();
		}
	}

	return result;

//	return CSplitterWnd::SplitColumn(cxBefore);
}

void CWheelSplitterWnd::DeleteRow( int rowDelete )
{
	CSplitterWnd::DeleteRow(rowDelete);

	if(m_delete_pane_msg != 0 && GetPane(0, 0) != NULL && 
			GetPane(0, 0)->IsKindOf(RUNTIME_CLASS(CView))) {
		CView *pview = (CView *)GetPane(0, 0);
		pview->GetDocument()->UpdateAllViews(NULL, m_delete_pane_msg, 0);
	}
}

void CWheelSplitterWnd::DeleteColumn( int colDelete )
{
	CSplitterWnd::DeleteColumn(colDelete);

	if(m_delete_pane_msg != 0 && GetPane(0, 0) != NULL && 
			GetPane(0, 0)->IsKindOf(RUNTIME_CLASS(CView))) {
		CView *pview = (CView *)GetPane(0, 0);
		pview->GetDocument()->UpdateAllViews(NULL, m_delete_pane_msg, 0);
	}
}

void CWheelSplitterWnd::SetMaxCols(int col)
{
	for(; GetColumnCount() > col; ) DeleteColumn(1);
	m_nMaxCols = col;
	RecalcLayout();
}

void CWheelSplitterWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// CSplitterWndの中にCSplitterWndがある場合、クリックしただけで境界位置がずれる問題を回避
	if(m_bTracking && GetPane(0, 0)->IsKindOf(RUNTIME_CLASS(CSplitterWnd))) {
		m_rectTracker.top -= 2;
		CRect rect = m_rectTracker;
		CSplitterWnd::OnLButtonUp(nFlags, point);
		InvalidateRect(rect);
		return;
	}
	
	CSplitterWnd::OnLButtonUp(nFlags, point);
}

#define _AfxGetDlgCtrlID(hWnd)          ((UINT)(WORD)::GetDlgCtrlID(hWnd))

void CWheelSplitterWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if(pScrollBar == NULL) return;
	int col = _AfxGetDlgCtrlID(pScrollBar->m_hWnd) - AFX_IDW_HSCROLL_FIRST;
	if(col < 0 && col >= m_nMaxCols) return;

	CSplitterWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CWheelSplitterWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if(pScrollBar == NULL) return;
	int row = _AfxGetDlgCtrlID(pScrollBar->m_hWnd) - AFX_IDW_VSCROLL_FIRST;
	if(row < 0 && row >= m_nMaxRows) return;

	CSplitterWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}
