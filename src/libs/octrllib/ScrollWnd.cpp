/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // ScrollWnd.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "ScrollWnd.h"

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CScrollWnd, CWnd)
/////////////////////////////////////////////////////////////////////////////
// CScrollWnd

CScrollWnd::CScrollWnd()
{
	m_scroll_style = 0;
	m_v_bar_enable = FALSE;
	m_h_bar_enable = FALSE;
	m_wheel_scroll_rate = 40.0;
	m_hwheel_scroll_rate = 30.0;
	m_splitter_mode = 0;
	m_b_thumb_tracking = FALSE;
	m_on_mdi_wnd = FALSE;
	m_b_hwheel_scroll_enable = FALSE;
}

CScrollWnd::~CScrollWnd()
{
}

void CScrollWnd::CheckOnMDIWnd()
{
	CWnd *pwnd = GetParent();
	for(; pwnd != NULL; pwnd = pwnd->GetParent()) {
		if(pwnd->IsKindOf(RUNTIME_CLASS(CDialog))) break;
		if(pwnd->IsKindOf(RUNTIME_CLASS(CMDIFrameWnd))) m_on_mdi_wnd = TRUE;
	}
}

BEGIN_MESSAGE_MAP(CScrollWnd, CWnd)
	//{{AFX_MSG_MAP(CScrollWnd)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_RBUTTONDOWN()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_WM_MOUSEHWHEEL()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CScrollWnd メッセージ ハンドラ

void CScrollWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (pScrollBar != NULL && pScrollBar->SendChildNotifyLastMsg()) return; // eat it

	// ignore scroll bar msgs from other controls
	if (pScrollBar != GetScrollBarCtrl(SB_HORZ)) return;

	SCROLLINFO scr_info;
	scr_info.cbSize = sizeof(scr_info);
	GetScrollInfo(SB_HORZ, &scr_info, SIF_POS | SIF_TRACKPOS);
	
	if(nSBCode == SB_THUMBTRACK) {
		OnScroll(MAKEWORD(nSBCode, -1), scr_info.nTrackPos);
	} else {
		OnScroll(MAKEWORD(nSBCode, -1), scr_info.nPos);
	}
}

void CScrollWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (pScrollBar != NULL && pScrollBar->SendChildNotifyLastMsg()) return; // eat it

	// ignore scroll bar msgs from other controls
	if (pScrollBar != GetScrollBarCtrl(SB_VERT)) return;

	SCROLLINFO scr_info;
	scr_info.cbSize = sizeof(scr_info);
	GetScrollInfo(SB_VERT, &scr_info, SIF_POS | SIF_TRACKPOS);
	
	if(nSBCode == SB_THUMBTRACK) {
		OnScroll(MAKEWORD(-1, nSBCode), scr_info.nTrackPos);
	} else {
		OnScroll(MAKEWORD(-1, nSBCode), scr_info.nPos);
	}
}

BOOL CScrollWnd::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll) 
{
	// calc new x position
	int x = GetScrollPos(SB_HORZ);
	int xOrig = x;
	CRect winrect;
	GetDispRect(winrect);

	BOOL bThumbTrack = FALSE;

	switch (LOBYTE(nScrollCode))
	{
	case SB_TOP:
		x = 0;
		break;
	case SB_BOTTOM:
		x = INT_MAX;
		break;
	case SB_LINEUP:
		x -= GetHLineSize();
		break;
	case SB_LINEDOWN:
		x += GetHLineSize();
		break;
	case SB_PAGEUP:
		x -= GetShowCol() - 1;
		if(x >= xOrig) x = xOrig - 1;
		break;
	case SB_PAGEDOWN:
		x += GetShowCol() - 1;
		if(x <= xOrig) x = xOrig + 1;
		break;
	case SB_THUMBTRACK:
		x = nPos;
		bThumbTrack = TRUE;
		m_b_thumb_tracking = TRUE;
		break;
	case SB_ENDSCROLL:
		m_b_thumb_tracking = FALSE;
		EndScroll();
		break;
	}

	// calc new y position
	int y = GetScrollPos(SB_VERT);
	int yOrig = y;
	switch (HIBYTE(nScrollCode))
	{
	case SB_TOP:
		y = 0;
		break;
	case SB_BOTTOM:
		y = INT_MAX;
		break;
	case SB_LINEUP:
		y -= GetVLineSize();
		break;
	case SB_LINEDOWN:
		y += GetVLineSize();
		break;
	case SB_PAGEUP:
		y -= GetShowRow() - 1;
		if(y >= yOrig) y = yOrig - 1;
		break;
	case SB_PAGEDOWN:
		y += GetShowRow() - 1;
		if(y <= yOrig) y = yOrig + 1;
		break;
	case SB_THUMBTRACK:
		y = nPos;
		bThumbTrack = TRUE;
		m_b_thumb_tracking = TRUE;
		break;
	case SB_ENDSCROLL:
		m_b_thumb_tracking = FALSE;
		EndScroll();
		break;
	}

	return OnScrollBy(CSize(x - xOrig, y - yOrig), bDoScroll, bThumbTrack);
}

BOOL CScrollWnd::OnScrollBy(CSize sizeScroll, BOOL bDoScroll, BOOL bThumbTrack) 
{
	int xOrig, x;
	int yOrig, y;

	// don't scroll if there is no valid scroll range (ie. no scroll bar)
	CScrollBar* pBar;
	DWORD dwStyle = GetStyle();
	pBar = GetScrollBarCtrl(SB_VERT);
	if ((pBar != NULL && !pBar->IsWindowEnabled()) ||
		(pBar == NULL && !(dwStyle & WS_VSCROLL)) ||
		m_v_bar_enable == FALSE)
	{
		// vertical scroll bar not enabled
		if(!(m_scroll_style & NO_VSCROLL_BAR)) sizeScroll.cy = 0;
	}
	pBar = GetScrollBarCtrl(SB_HORZ);
	if ((pBar != NULL && !pBar->IsWindowEnabled()) ||
		(pBar == NULL && !(dwStyle & WS_HSCROLL)) ||
		m_h_bar_enable == FALSE)
	{
		// horizontal scroll bar not enabled
		if(!(m_scroll_style & NO_HSCROLL_BAR)) sizeScroll.cx = 0;
	}

	// adjust current x position
	xOrig = x = GetScrollPos(SB_HORZ);
	int xMax = GetScrollLimit(SB_HORZ);
	if(xMax < 0) xMax = 0;

	x += sizeScroll.cx;
	if (x < GetHScrollMin())
		x = GetHScrollMin();
	else if (x > xMax)
		x = xMax;

	// adjust current y position
	yOrig = y = GetScrollPos(SB_VERT);
	int yMax = GetScrollLimit(SB_VERT);
	if(yMax < 0) yMax = 0;

	y += sizeScroll.cy;
	if (y < GetVScrollMin())
		y = GetVScrollMin();
	else if (y > yMax)
		y = yMax;

	// did anything change?
	if (x == xOrig && y == yOrig) return FALSE;

	if (bDoScroll)
	{
		CRect winrect, headrect;
		GetDispRect(winrect);

		if(m_scroll_style & LOCK_WINDOW) LockWindowUpdate();

		// do scroll and update scroll positions
		// 本体をスクロール
		CRect rect1(GetScrollLeftMargin(), GetScrollTopMargin(), 
			winrect.Width() - GetScrollRightMargin(), 
			winrect.Height() - GetScrollBottomMargin());
		ScrollWindow(GetHScrollSize(xOrig, x), GetVScrollSize(yOrig, y), rect1, rect1);
		if (x != xOrig) {
			SetScrollPos(SB_HORZ, x);
		}
		if (y != yOrig) {
			SetScrollPos(SB_VERT, y);
		}

		// ヘッダ部分をスクロール
		if(x != xOrig && GetScrollTopMargin() != 0) {
			rect1.left = GetScrollLeftMargin();
			rect1.right = winrect.Width() - GetScrollRightMargin();
			rect1.top = 0;
			rect1.bottom = GetScrollTopMargin();
			ScrollWindow(GetHScrollSize(xOrig, x), 0, rect1, rect1);
		}
		if(y != yOrig && GetScrollLeftMargin() != 0) {
			rect1.left = 0;
			rect1.right = GetScrollLeftMargin();
			rect1.top = GetScrollTopMargin();
			rect1.bottom = winrect.Height() - GetScrollBottomMargin();
			ScrollWindow(0, GetVScrollSize(yOrig, y), rect1, rect1);
		}

		Scrolled(sizeScroll, bThumbTrack);

		if(m_scroll_style & LOCK_WINDOW) UnlockWindowUpdate();
	}

	return TRUE;
}

BOOL CScrollWnd::IsNeedVScrollBar(int height)
{
	int		v_limit = GetVScrollSizeLimit();

	if(m_scroll_style & VSCROLL_ALWAYS_ON) return TRUE;

	if(IsSplitterMode()) {
		// 分割されているときは，常に表示
		CSplitterWnd *p = GetParentSplitter();
		if(p != NULL && p->GetRowCount() > 1)  return TRUE;
		if(m_scroll_style & NO_VSCROLL_BAR) return FALSE;
		return (v_limit > height);
	} else if(m_scroll_style & NO_WS_VSCROLL) {
		if(m_scroll_style & NO_VSCROLL_BAR) return FALSE;
		return (v_limit > height);
	}

	return TRUE;
}

BOOL CScrollWnd::IsNeedHScrollBar(int width)
{
	int		h_limit = GetHScrollSizeLimit();

	if(m_scroll_style & HSCROLL_ALWAYS_ON) return TRUE;

	if(IsSplitterMode()) {
		// 分割されているときは，常に表示
		CSplitterWnd *p = GetParentSplitter();
		if(p != NULL && p->GetColumnCount() > 1) return TRUE;
		if(m_scroll_style & NO_HSCROLL_BAR) return FALSE;
		return (h_limit > width);
	} else if(m_scroll_style & NO_WS_HSCROLL) {
		if(m_scroll_style & NO_HSCROLL_BAR) return FALSE;
		return (h_limit > width);
	}

	return TRUE;
}

void CScrollWnd::CheckScrollBar()
{
	DWORD		scr_style = GetStyle();
	CRect	winrect;
	GetDispRect(winrect);

	if(IsSplitterMode()) {
		CSplitterWnd *p = GetParentSplitter();
		if(p != NULL) scr_style = p->GetScrollStyle();
	}

	DWORD		new_scr_style = scr_style;

	if(scr_style & WS_VSCROLL) winrect.right += ::GetSystemMetrics(SM_CXVSCROLL);
	if(scr_style & WS_HSCROLL) winrect.bottom += ::GetSystemMetrics(SM_CYHSCROLL);

	// タブの切替時に、スクロールバーの位置が0になってしまうことがある問題を回避
	if(m_on_mdi_wnd) {
		if(winrect.Height() <= ::GetSystemMetrics(SM_CYHSCROLL)) return;
		if(winrect.Width() <= ::GetSystemMetrics(SM_CXVSCROLL)) return;
	}

	BOOL v_scr = IsNeedVScrollBar(winrect.Height());
	BOOL h_scr = IsNeedHScrollBar(winrect.Width());
	if(v_scr && !h_scr) h_scr = IsNeedHScrollBar(winrect.Width() - ::GetSystemMetrics(SM_CXVSCROLL));
	if(h_scr && !v_scr) v_scr = IsNeedVScrollBar(winrect.Height() - ::GetSystemMetrics(SM_CYHSCROLL));

	if(v_scr) {
		new_scr_style |= WS_VSCROLL;
	} else {
		new_scr_style &= (~WS_VSCROLL);
	}

	if(h_scr) {
		new_scr_style |= WS_HSCROLL;
	} else {
		new_scr_style &= (~WS_HSCROLL);
	}

	if(scr_style != new_scr_style) {
		if(IsSplitterMode()) {
			CSplitterWnd *p = GetParentSplitter();
			if(p != NULL) {
				p->SetScrollStyle(new_scr_style);
				if(m_scroll_style  & KEEP_WS_VH_SCROLL_STYLE) {
					if(!(GetStyle() & WS_VSCROLL) || !(GetStyle() & WS_HSCROLL)) {
						ModifyStyle(0, WS_VSCROLL | WS_HSCROLL);
					}
				}
				p->RecalcLayout();
			}
		} else {
			if((scr_style & WS_VSCROLL) != (new_scr_style & WS_VSCROLL)) {
				ShowScrollBar(SB_VERT, (new_scr_style & WS_VSCROLL));
			}
			if((scr_style & WS_HSCROLL) != (new_scr_style & WS_HSCROLL)) {
				ShowScrollBar(SB_HORZ, (new_scr_style & WS_HSCROLL));
			}
		}
	}

	CRect	newrect;
	GetDispRect(newrect);

	SCROLLINFO scr_info;
	scr_info.cbSize = sizeof(SCROLLINFO);
	scr_info.fMask = SIF_RANGE | SIF_PAGE;

	if(newrect.Height() > GetVScrollSizeLimit()) {
		EnableScrollBar(SB_VERT, ESB_DISABLE_BOTH);
		m_v_bar_enable = FALSE;
	} else {
		EnableScrollBar(SB_VERT, ESB_ENABLE_BOTH);
		m_v_bar_enable = TRUE;
	}
	scr_info.nMin = GetVScrollMin();
	scr_info.nMax = GetVScrollMax();
	scr_info.nPage = GetVScrollPage();
	scr_info.nPos = 0;
	SetScrollInfo(SB_VERT, &scr_info, TRUE);
	if(v_scr) {
		SetScrollInfo(SB_VERT, &scr_info, TRUE);
	} else {
		SetScrollInfo(SB_VERT, &scr_info, FALSE);
		ShowScrollBar(SB_VERT, FALSE);
	}

	if(newrect.Width() > GetHScrollSizeLimit()) {
		EnableScrollBar(SB_HORZ, ESB_DISABLE_BOTH);
		m_h_bar_enable = FALSE;
	} else {
		EnableScrollBar(SB_HORZ, ESB_ENABLE_BOTH);
		m_h_bar_enable = TRUE;
	}
	scr_info.nMin = GetHScrollMin();
	scr_info.nMax = GetHScrollMax();
	scr_info.nPage = GetHScrollPage();
	scr_info.nPos = 0;
	if(h_scr) {
		SetScrollInfo(SB_HORZ, &scr_info, TRUE);
	} else {
		// Windows7対応
		// SetScrollInfoの実行後にスクロールバーのサイズの範囲で、WM_LBUTTONDOWNイベントが通知されなくなる
		// ShowScrollBar(FALSE)で回避できる
		SetScrollInfo(SB_HORZ, &scr_info, FALSE);
		ShowScrollBar(SB_HORZ, FALSE);
	}
}

BOOL CScrollWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	int scr_cnt = -(int)(ceil(zDelta / m_wheel_scroll_rate));
	OnScrollBy(CSize(0, scr_cnt * GetVLineSize()));
	return TRUE;
}

void CScrollWnd::OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// この機能には Windows Vista 以降のバージョンが必要です。
	// シンボル _WIN32_WINNT は >= 0x0600 にする必要があります。
	// TODO: ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。

	if(m_b_hwheel_scroll_enable) {
		int scr_cnt = (int)(ceil(zDelta / m_hwheel_scroll_rate));
		OnScrollBy(CSize(scr_cnt * GetHLineSize(), 0));
	}

	CWnd::OnMouseHWheel(nFlags, zDelta, pt);
}

void CScrollWnd::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// 親ウィンドウの座標に変換する
	POINT	pt;
	pt.x = point.x;
	pt.y = point.y;
	ClientToScreen(&pt);
	GetParent()->ScreenToClient(&pt);

	// 親ウィンドウに転送
	GetParent()->SendMessage(WM_RBUTTONDOWN, nFlags, MAKELPARAM(pt.x, pt.y));
	
//	CWnd::OnRButtonDown(nFlags, point);
}

void CScrollWnd::VScroll(UINT nSBCode)
{
	OnVScroll(nSBCode, 0, GetScrollBarCtrl(SB_VERT));
}

void CScrollWnd::HScroll(UINT nSBCode)
{
	OnHScroll(nSBCode, 0, GetScrollBarCtrl(SB_HORZ));
}

int CScrollWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: この位置に固有の作成用コードを追加してください
	if(!(lpCreateStruct->style & WS_VSCROLL)) {
		m_scroll_style |= NO_WS_VSCROLL;
	}
	if(!(lpCreateStruct->style & WS_HSCROLL)) {
		m_scroll_style |= NO_WS_HSCROLL;
	}

	CheckOnMDIWnd();
	
	return 0;
}

void CScrollWnd::SetSplitterMode(BOOL splitter_mode)
{
	//if(splitter_mode) m_scroll_style |= NO_HSCROLL_BAR | NO_VSCROLL_BAR;
	m_splitter_mode = splitter_mode;
}

CScrollBar* CScrollWnd::GetScrollBarCtrl(int nBar) const
{
	if(IsSplitterMode()) {
		// for TrackPoint
		if((nBar == SB_VERT && GetStyle() & WS_VSCROLL) ||
			(nBar == SB_HORZ && GetStyle() & WS_HSCROLL)) {
			return CWnd::GetScrollBarCtrl(nBar);
		}

		if(GetParent() != NULL) {
			return GetParent()->GetScrollBarCtrl(nBar);
		}
	}
	return CWnd::GetScrollBarCtrl(nBar);
}

BOOL CScrollWnd::GetScrollInfo( int nBar, LPSCROLLINFO lpScrollInfo, UINT nMask )
{
	if(lpScrollInfo == NULL) return FALSE;

	if(IsSplitterMode()) {
		if(GetParent() != NULL) {
			return GetParent()->GetScrollInfo(nBar, lpScrollInfo, nMask);
		}
	}
	return CWnd::GetScrollInfo(nBar, lpScrollInfo, nMask);
}

int CScrollWnd::GetScrollPos( int nBar ) const
{
	if(IsSplitterMode()) {
		if(GetParent() != NULL) {
			return GetParent()->GetScrollPos(nBar);
		}
	}
	return CWnd::GetScrollPos(nBar);
}

int CScrollWnd::GetScrollLimit( int nBar )
{
	if(IsSplitterMode()) {
		if(GetParent() != NULL && GetParent()->GetParent() != NULL) {
			return GetParent()->GetScrollLimit(nBar);
		}
	}
	return CWnd::GetScrollLimit(nBar);
}

BOOL CScrollWnd::SetScrollInfo( int nBar, LPSCROLLINFO lpScrollInfo, BOOL bRedraw )
{
	if(IsSplitterMode()) {
		// for TrackPoint
		if((nBar == SB_VERT && GetStyle() & WS_VSCROLL) ||
			(nBar == SB_HORZ && GetStyle() & WS_HSCROLL)) {
			// スクロールバーが消えないようにする
			if(lpScrollInfo->nMax > 0 && lpScrollInfo->nMax > (int)lpScrollInfo->nPage) {
				CWnd::SetScrollInfo(nBar, lpScrollInfo, bRedraw);
			}
		}

		if(GetParent() != NULL) {
			return GetParent()->SetScrollInfo(nBar, lpScrollInfo, bRedraw);
		}
	}
	return CWnd::SetScrollInfo(nBar, lpScrollInfo, bRedraw);
}

int CScrollWnd::SetScrollPos( int nBar, int nPos, BOOL bRedraw )
{
	if(IsSplitterMode()) {
		// for TrackPoint
		if((nBar == SB_VERT && GetStyle() & WS_VSCROLL) ||
			(nBar == SB_HORZ && GetStyle() & WS_HSCROLL)) {
			CWnd::SetScrollPos(nBar, nPos, bRedraw);
		}

		if(GetParent() != NULL) {
			return GetParent()->SetScrollPos(nBar, nPos, bRedraw);
		}
	}
	return CWnd::SetScrollPos(nBar, nPos, bRedraw);
}

CSplitterWnd *CScrollWnd::GetParentSplitter()
{
	if(GetParent() == NULL) return NULL;
	CWnd *pwnd = GetParent()->GetParent();
	if(pwnd == NULL) return NULL;

	if(pwnd->IsKindOf(RUNTIME_CLASS(CSplitterWnd))) {
		return (CSplitterWnd *)pwnd;
	}
	return NULL;
}

void CScrollWnd::GetDispRect( LPRECT lpRect ) const
{
	if(IsSplitterMode()) {
		GetParent()->GetClientRect(lpRect);
	} else {
		GetClientRect(lpRect);
	}
}

// FIXME: 要テスト
/*
BOOL CScrollWnd::IsShowVScrollBar()
{
	if(IsSplitterMode()) {
		return (GetParent()->GetStyle() & WS_VSCROLL);
	}
	return (GetStyle() & WS_VSCROLL);
}

BOOL CScrollWnd::IsShowHScrollBar()
{
	if(IsSplitterMode()) {
		return (GetParent()->GetStyle() & WS_HSCROLL);
	}
	return (GetStyle() & WS_HSCROLL);
}
*/


