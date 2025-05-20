/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

// FileTabCtrl.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "FileTabCtrl.h"
#include "ColorUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ID_CLOSE_BTN	294
#define IdToolTipTimer		102

/////////////////////////////////////////////////////////////////////////////
// CFileTabCtrl

CFileTabCtrl::CFileTabCtrl()
{
	ClearMClickPt();

	SetDefaultColor();

	m_cf_tab_idx = RegisterClipboardFormat(_T("OGAWA_EditCtrl_CF_TAB_IDX"));

	m_drop_target_idx = -1;
	m_ex_style = 0;
	m_last_tool_tip_col = -1;
	
	m_close_btn_top_offset = 2;

	b_min_erase_bkgnd = FALSE;
}

CFileTabCtrl::~CFileTabCtrl()
{
}

void CFileTabCtrl::SetDefaultColor()
{
	m_color[FILETAB_TEXT_COLOR] = ::GetSysColor(COLOR_WINDOWTEXT);
	m_color[FILETAB_ACTIVE_BK_COLOR] = ::GetSysColor(COLOR_BTNFACE);
	m_color[FILETAB_NO_ACTIVE_BK_COLOR] = make_dark_color(::GetSysColor(COLOR_BTNFACE), 0.85);
	m_color[FILETAB_BTN_COLOR] = ::GetSysColor(COLOR_BTNFACE);
}

BEGIN_MESSAGE_MAP(CFileTabCtrl, CTabCtrl)
	//{{AFX_MSG_MAP(CFileTabCtrl)
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_HSCROLL()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileTabCtrl メッセージ ハンドラ

int CFileTabCtrl::HitTabTest(CPoint point)
{
	TCHITTESTINFO	hit_test;
	hit_test.pt = point;
	hit_test.flags = TCHT_ONITEM;

	return HitTest(&hit_test);
}

void CFileTabCtrl::OnMButtonDown(UINT nFlags, CPoint point) 
{
	if(HitTabTest(point) >= 0) {
		m_click_pt = point;
		SetCapture();
	}
	
	CTabCtrl::OnMButtonDown(nFlags, point);
}

void CFileTabCtrl::OnMButtonUp(UINT nFlags, CPoint point) 
{
	if(GetCapture() == this) {
		ReleaseCapture();

		CRect	rect;
		rect.left = m_click_pt.x - 5;
		rect.right = m_click_pt.x + 5;
		rect.top = m_click_pt.y - 5;
		rect.bottom = m_click_pt.y + 5;
		if(rect.PtInRect(point)) {
			int hit_idx_up, hit_idx_down;
			hit_idx_up = HitTabTest(point);
			hit_idx_down = HitTabTest(m_click_pt);

			if(hit_idx_up >= 0 && hit_idx_up == hit_idx_down) {
				NMHDR		nmhdr;
				nmhdr.code = TC_MCLICK;
				nmhdr.hwndFrom = GetSafeHwnd();
				nmhdr.idFrom = hit_idx_up;

				GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&nmhdr);
			}
		}

		ClearMClickPt();
	}
	
	CTabCtrl::OnMButtonUp(nFlags, point);
}

void CFileTabCtrl::ClearMClickPt()
{
	m_click_pt.x = -1;
	m_click_pt.y = -1;
}

void CFileTabCtrl::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	CDC		dc;
	dc.Attach(lpDrawItemStruct->hDC);

	COLORREF bk_color;
	int		text_left;
	int		text_top;

	if(lpDrawItemStruct->itemState == 1) {
		text_top = lpDrawItemStruct->rcItem.top + 3;
		text_left = lpDrawItemStruct->rcItem.left + 8;
		bk_color = m_color[FILETAB_ACTIVE_BK_COLOR];
	} else {
		text_top = lpDrawItemStruct->rcItem.top + 3;
		text_left = lpDrawItemStruct->rcItem.left + 4;
		bk_color = m_color[FILETAB_NO_ACTIVE_BK_COLOR];
	}

	COLORREF old_text_color = dc.SetTextColor(m_color[FILETAB_TEXT_COLOR]);

	TCITEM	tcitem;
	TCHAR	buf[256];
	tcitem.mask = TCIF_TEXT;
	tcitem.pszText = buf;
	tcitem.cchTextMax = sizeof(buf);
	this->GetItem(lpDrawItemStruct->itemID, &tcitem);

	TRACE(_T("DrawItem %d:%s\n"), lpDrawItemStruct->itemID, buf);

	COLORREF old_bk_color = dc.SetBkColor(bk_color);

	dc.ExtTextOut(text_left, text_top, ETO_OPAQUE, 
		&(lpDrawItemStruct->rcItem), buf, (int)_tcslen(buf), NULL);

	dc.SetBkColor(old_bk_color);
	dc.SetTextColor(old_text_color);

	if(m_drop_target_idx == (int)lpDrawItemStruct->itemID) {
		CRect rect;
		rect.left = lpDrawItemStruct->rcItem.left;
		rect.right = rect.left + 2;
		rect.top = lpDrawItemStruct->rcItem.top + 1;
		rect.bottom = lpDrawItemStruct->rcItem.bottom - 1;

		dc.FillSolidRect(&rect, m_color[FILETAB_TEXT_COLOR]);
	} else if(m_drop_target_idx == (int)lpDrawItemStruct->itemID + 1) {
		CRect rect;
		rect.left = lpDrawItemStruct->rcItem.right - 2;
		rect.right = rect.left + 2;
		rect.top = lpDrawItemStruct->rcItem.top + 1;
		rect.bottom = lpDrawItemStruct->rcItem.bottom - 1;
		
		dc.FillSolidRect(&rect, m_color[FILETAB_TEXT_COLOR]);
	}

	dc.Detach();
}

int CFileTabCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTabCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if(m_droptarget.Register(this) == -1) return -1;

	m_close_btn.Create(this, ID_CLOSE_BTN);

	SetMinTabWidth(48);

	m_tool_tip.Create();

	return 0;
}

void CFileTabCtrl::SetExStyle(DWORD ex_style)
{
	if(m_ex_style == ex_style) return;

	m_ex_style = ex_style;

	int padding_x = 3;

	if(m_ex_style & FTC_CLOSE_BTN_ON_TAB) {
		padding_x = 16;
		m_close_btn_top_offset = 1;
	} else if(m_ex_style & FTC_CLOSE_BTN_ON_ALL_TAB) {
		padding_x = 16;
		m_close_btn_top_offset = 1;
	}
	SetPadding(CSize(padding_x, 3));

	{
		// タブの幅を再調節する
		InsertItem(0, _T(""), 0);
		DeleteItem(0);
	}

	SetCloseBtnPos();
}

BOOL CFileTabCtrl::TabDrag(int tab_idx)
{
	ASSERT(m_ex_style & FTC_TAB_DRAG_MOVE);

	HGLOBAL hData = GlobalAlloc(GHND, sizeof(int));
	int *idx = (int *)GlobalLock(hData);
	if(idx != NULL) {
		*idx = tab_idx;
	}
	GlobalUnlock(hData);

	COleDataSource dataSource;
	dataSource.CacheGlobalData(m_cf_tab_idx, hData);

	DROPEFFECT result = dataSource.DoDragDrop(DROPEFFECT_MOVE, NULL, NULL);
	m_drop_target_idx = -1;

	if(result == DROPEFFECT_MOVE) return TRUE;

	return FALSE;
}

void CFileTabCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	int tab_idx = HitTabTest(point);

	// アクティブタブを変更するとき、閉じるボタンの位置に残像みたいに残るので、
	// 一旦非表示にする
	if((m_ex_style & (FTC_CLOSE_BTN_ON_TAB | FTC_CLOSE_BTN_ON_ALL_TAB | FTC_TAB_DRAG_MOVE)) && 
		tab_idx >= 0 && tab_idx != GetCurSel()) {
		m_close_btn.ShowWindow(SW_HIDE);
	}

	CTabCtrl::OnLButtonDown(nFlags, point);

	if(m_ex_style & FTC_TAB_DRAG_MOVE) {
		if(tab_idx >= 0) TabDrag(tab_idx);
	}
}

DROPEFFECT CFileTabCtrl::DragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	return DROPEFFECT_NONE;
}

DROPEFFECT CFileTabCtrl::DragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	if(!pDataObject->IsDataAvailable(m_cf_tab_idx)) return DROPEFFECT_NONE;

	DROPEFFECT result = DROPEFFECT_NONE;

	int target_idx = HitTabTest(point);
	if(target_idx >= 0) {
		HGLOBAL hData = pDataObject->GetGlobalData(m_cf_tab_idx);
		int *pidx = (int *)GlobalLock(hData);
		int drag_idx = 0;
		if(pidx != NULL) {
			drag_idx = *pidx;
		}
		GlobalUnlock(hData);

		if(target_idx != drag_idx) {
			CRect rect;
			GetItemRect(target_idx, &rect);
			if(rect.left + (rect.Width() / 2) < point.x) target_idx++;

			result = DROPEFFECT_MOVE;
		}
	}

	if(m_drop_target_idx != target_idx) {
		m_drop_target_idx = target_idx;
		Invalidate();
	}

	return result;
}

BOOL CFileTabCtrl::Drop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	if(!pDataObject->IsDataAvailable(m_cf_tab_idx)) return FALSE;
	if(m_drop_target_idx < 0) return FALSE;

	BOOL result = FALSE;

	HGLOBAL hData = pDataObject->GetGlobalData(m_cf_tab_idx);
	int *pidx = (int *)GlobalLock(hData);
	int drag_idx = 0;
	if(pidx != NULL) {
		drag_idx = *pidx;
	}
	GlobalUnlock(hData);

	if(drag_idx < m_drop_target_idx) m_drop_target_idx--;
	if(drag_idx != m_drop_target_idx) {
		// 親ウィンドウに通知する
		NMHDR		nmhdr;
		nmhdr.code = TC_MOVE_TAB;
		nmhdr.hwndFrom = GetSafeHwnd();
		nmhdr.idFrom = m_drop_target_idx;

		GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&nmhdr);

		result = TRUE;
	}

	m_drop_target_idx = -1;
	Invalidate();

	return result;
}

void CFileTabCtrl::DragLeave()
{
	m_drop_target_idx = -1;
	Invalidate();
}

void CFileTabCtrl::SetFont(CFont *pFont)
{
	CTabCtrl::SetFont(pFont);
	m_tool_tip.SetFont(pFont);
	SetCloseBtnPos();
}

void CFileTabCtrl::SetCloseBtnPos(BOOL b_invalidate)
{
	int idx = GetCurSel();
	if(idx == -1 || !(m_ex_style & FTC_CLOSE_BTN_ON_TAB)) {
		m_close_btn.ShowWindow(SW_HIDE);
	} else {
		SetCloseBtnPos(idx, &m_close_btn, b_invalidate);
	}

	if(m_ex_style & FTC_CLOSE_BTN_ON_ALL_TAB) {
		int	i;
		for(i = 0; i < CTabCtrl::GetItemCount(); i++) {
			if((int)m_close_btn_vec.size() <= i) {
				CCloseBtn *btn = new CCloseBtn();
				btn->Create(this, ID_CLOSE_BTN);
				m_close_btn_vec.push_back(btn);
			}
			SetCloseBtnPos(i, m_close_btn_vec[i], b_invalidate);
		}
		for(; i < (int)m_close_btn_vec.size(); i++) {
			m_close_btn_vec[i]->ShowWindow(SW_HIDE);
		}
	} else {
		if(m_close_btn_vec.size() > 0) {
			ClearCloseBtnVec();
		}
	}
}

void CFileTabCtrl::SetCloseBtnPos(int idx, CCloseBtn *btn, BOOL b_invalidate)
{
	CRect btn_rect;
	btn->GetClientRect(btn_rect);

	CRect rect;
	GetItemRect(idx, &rect);

	rect.top = rect.top + m_close_btn_top_offset;
	if(idx != GetCurSel()) {
		rect.top += 2;
		btn->SetColor(m_color[FILETAB_NO_ACTIVE_BK_COLOR]);
	} else {
		btn->SetColor(m_color[FILETAB_BTN_COLOR]);
	}
	btn->SetParam(idx);

	rect.left = rect.right - btn_rect.Width() - 3;

	btn->SetWindowPos(NULL, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	btn->ShowWindow(SW_SHOW);

	if(b_invalidate) {
		InvalidateRect(rect);
	}
	btn->Invalidate();
}

int CFileTabCtrl::SetCurSel( int nItem )
{
	int ret_v = CTabCtrl::SetCurSel(nItem);
	SetCloseBtnPos();
	return ret_v;
}

void CFileTabCtrl::SetItemText(int nItem, const TCHAR *text)
{
	TC_ITEM	set_item;
	set_item.mask = TCIF_TEXT;
	set_item.pszText = (TCHAR *)text;
	set_item.cchTextMax = 0;
	SetItem(nItem, &set_item);
	
	SetCloseBtnPos();
}

BOOL CFileTabCtrl::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if(wParam == ID_CLOSE_BTN) {
		int idx = GetCurSel();

		if(m_ex_style & FTC_CLOSE_BTN_ON_ALL_TAB) {
			std::vector<CCloseBtn *>::iterator it;
			for(it = m_close_btn_vec.begin(); it != m_close_btn_vec.end(); it++) {
				if((*it)->GetSafeHwnd() == (HWND)lParam) {
					idx = (*it)->GetParam();
				}
			}
		}

		// 親ウィンドウに通知する
		NMHDR		nmhdr;
		nmhdr.code = TC_CLOSE_BTN;
		nmhdr.hwndFrom = GetSafeHwnd();
		nmhdr.idFrom = idx;
		GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&nmhdr);
	}
	
	return CTabCtrl::OnCommand(wParam, lParam);
}

BOOL CFileTabCtrl::DeleteItem(int nItem)
{
	BOOL ret_v = CTabCtrl::DeleteItem(nItem);
	SetCloseBtnPos();
	return ret_v;
}

int CFileTabCtrl::InsertItem(int nItem, TC_ITEM* pTabCtrlItem)
{
	int tab_idx = CTabCtrl::InsertItem(nItem, pTabCtrlItem);
	SetCloseBtnPos();
	return tab_idx;
}

int CFileTabCtrl::InsertItem(int nItem, const TCHAR *text, LPARAM lparam)
{
	int tab_idx = CTabCtrl::InsertItem(TCIF_TEXT | TCIF_PARAM, nItem, text, 0, lparam);
	SetCloseBtnPos();
	return tab_idx;
}

void CFileTabCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CTabCtrl::OnHScroll(nSBCode, nPos, pScrollBar);

	SetCloseBtnPos(TRUE);
}

void CFileTabCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	if(m_ex_style & FTC_SHOW_TAB_TOOLTIP) {
		int tab_idx = HitTabTest(point);
		if(tab_idx >= 0) {
			if(tab_idx != m_last_tool_tip_col) {
				KillTimer(IdToolTipTimer);
				m_tool_tip_pt = point;
				SetTimer(IdToolTipTimer, 200, NULL);
				m_last_tool_tip_col = -1;
			} else {
				m_last_tool_tip_col = -1;
			}
		}
	}
	
	CTabCtrl::OnMouseMove(nFlags, point);
}

void CFileTabCtrl::OnTimer(UINT_PTR nIDEvent) 
{
	if(nIDEvent == IdToolTipTimer) {
		KillTimer(IdToolTipTimer);
		CPoint pt;
		::GetCursorPos(&pt);
		ScreenToClient(&pt);
		if(m_tool_tip_pt == pt) ShowToolTip();
	}
	
	CTabCtrl::OnTimer(nIDEvent);
}

void CFileTabCtrl::ShowToolTip()
{
	int tab_idx = HitTabTest(m_tool_tip_pt);
	if(tab_idx < 0) return;

	m_last_tool_tip_col = tab_idx;

	struct file_tab_ctrl_nmhdr h;
	h.nmhdr.code = TC_TOOL_TIP_NAME;
	h.nmhdr.hwndFrom = GetSafeHwnd();
	h.nmhdr.idFrom = tab_idx;
	_tcscpy(h.str_buf, _T(""));

	GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&h);

	if(_tcslen(h.str_buf) > 0) {
		m_tool_tip.SetMessage(h.str_buf);
		CRect rect;
		rect.left = m_tool_tip_pt.x - 8;
		rect.right = rect.left + 20;
		rect.top = m_tool_tip_pt.y - 6;
		rect.bottom = rect.top;
		ClientToScreen(&rect);

		m_tool_tip.Show(GetSafeHwnd(), &rect);
	}
}

void CFileTabCtrl::ClearCloseBtnVec()
{
	if(!m_close_btn_vec.empty()) {
		std::vector<CCloseBtn *>::iterator it;
		for(it = m_close_btn_vec.begin(); it != m_close_btn_vec.end(); it++) {
			delete *it;
		}
		m_close_btn_vec.clear();
	}
}

void CFileTabCtrl::OnDestroy() 
{
	CTabCtrl::OnDestroy();

	ClearCloseBtnVec();
	m_tool_tip.Destroy();
}

void CFileTabCtrl::Redraw()
{
	if(!::IsWindow(GetSafeHwnd())) return;
	Invalidate();
}



BOOL CFileTabCtrl::OnEraseBkgnd(CDC* pDC)
{
	// TODO: ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。
	int  item_cnt;

	item_cnt = GetItemCount();

	if(b_min_erase_bkgnd && item_cnt > 0) {
		// ダイアログのサイズ変更時にちらつかないようにする
		// タブの外側のみbackgroundを塗りつぶす
		RECT client_rect;
		RECT item_rect;
		RECT last_item_rect;
		RECT fill_rect;
		DWORD bk_color = GetSysColor(COLOR_3DFACE);

		GetClientRect(&client_rect);
		GetItemRect(0, &item_rect);
		GetItemRect(item_cnt - 1, &last_item_rect);

		fill_rect = client_rect;
		fill_rect.bottom = fill_rect.top + 3;
		pDC->FillSolidRect(&fill_rect, bk_color);

		fill_rect = client_rect;
		fill_rect.top = item_rect.bottom - 2;
		pDC->FillSolidRect(&fill_rect, bk_color);

		fill_rect = client_rect;
		fill_rect.left = last_item_rect.right;
		pDC->FillSolidRect(&fill_rect, bk_color);

		return TRUE;
	} else {
		return CTabCtrl::OnEraseBkgnd(pDC);
	}
}

