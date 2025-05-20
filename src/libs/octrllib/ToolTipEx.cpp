/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include <windows.h>
#include <time.h>

#include "ToolTipEx.h"

//ウィンドウの左上を原点として，文字を書き出す場所の設定
#define TIP_TEXT_LEFT_MARGINE	3
#define TIP_TEXT_TOP_MARGINE	2

CToolTipEx::CToolTipEx()
{
	m_msg.Empty();
	m_hWnd = NULL;
	m_bk_cr = RGB(255, 255, 224);
	m_text_cr = RGB(0, 0, 0);
	m_font = NULL;
	m_pt = CPoint(0, 0);
	m_size = CSize(0, 0);

	m_b_show = FALSE;
	m_parent_wnd = NULL;
}

CToolTipEx::~CToolTipEx()
{
}

BOOL CToolTipEx::Create()
{
	return Create(AfxGetApp()->m_hInstance);
}

BOOL CToolTipEx::Create(HINSTANCE hInst)
{
	WNDCLASS wndcls;

    if (!(GetClassInfo(hInst, TIP_CTRL_CLASSNAME, &wndcls)))
	{
        // otherwise we need to register a new class
        wndcls.style            = NULL;//CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wndcls.lpfnWndProc      = CToolTipEx::ToolTipExWndProc;
        wndcls.cbClsExtra       = 0;
		wndcls.cbWndExtra		= sizeof(CToolTipEx *);
        wndcls.hInstance        = hInst;
        wndcls.hIcon            = NULL;
        wndcls.hCursor          = NULL;
        wndcls.hbrBackground    = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wndcls.lpszMenuName     = NULL;
        wndcls.lpszClassName    = TIP_CTRL_CLASSNAME;

        if (!RegisterClass(&wndcls)) {
			return FALSE;
		}
    }

	m_hWnd = CreateWindowEx(
		WS_EX_TOOLWINDOW, // タスクバーにアイコンが表示されない
		TIP_CTRL_CLASSNAME, _T(""), 
		WS_POPUP, //WS_CHILD | WS_BORDER | WS_VISIBLE | WS_TABSTOP, 
		0, 0, 10,10, ::GetDesktopWindow(), 
		(HMENU) NULL, hInst, (LPVOID) NULL); 
	if(m_hWnd == NULL) return FALSE;

	SetWindowLongPtr(m_hWnd, 0, (LONG_PTR)this);

	return TRUE;
}

// ウィンドウ・プロシージャ
LRESULT CALLBACK CToolTipEx::ToolTipExWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CToolTipEx	*tipctrl;

	tipctrl = (CToolTipEx *)GetWindowLongPtr(hWnd, 0);

	switch(message) {
	case WM_PAINT:
		tipctrl->OnPaint();
		break;

	case WM_TIMER:
		tipctrl->OnTimer(wParam);
		break;
	}

	if(message >= WM_MOUSEFIRST && message <= WM_MOUSELAST && message != WM_MOUSEMOVE) {
		tipctrl->Hide();

		switch(message) {
		case WM_LBUTTONDOWN:
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			break;
		case WM_RBUTTONDOWN:
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
			break;
		case WM_MBUTTONDOWN:
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
			break;
		}
	}

	return(::DefWindowProc(hWnd, message, wParam, lParam));
}

void CToolTipEx::OnTimer(UINT_PTR nIDEvent)
{
	CPoint pt;
	switch(nIDEvent){
	case TIMER_ID_MOUSE_POS_CHECK:
		{
			::GetCursorPos(&pt);
			CRect rc = m_disp_rect;
			rc.DeflateRect(TIP_TEXT_LEFT_MARGINE, 0);
			if ( !rc.PtInRect(pt) ) Hide();
		}
		break;
	}

	return;
}

// ウィンドウ描画
void CToolTipEx::OnPaint()
{
	HDC		hdc;
	RECT	rect;
	HGDIOBJ font_old, brush_old;
	HBRUSH	brush;

	hdc = ::GetDC(m_hWnd);
	::SetBkMode(hdc, TRANSPARENT);

	if(m_font != NULL) font_old = ::SelectObject(hdc, m_font);

	brush = ::CreateSolidBrush(m_bk_cr);
	brush_old = ::SelectObject(hdc, brush);
	rect = CRect(CPoint(0,0), m_size);
	rect.bottom = m_disp_rect.Height();
	rect.right = m_size.cx + TIP_TEXT_LEFT_MARGINE - 1;
	::Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

	COLORREF old_text_color = ::SetTextColor(hdc, m_text_cr);
	::TextOut(hdc, TIP_TEXT_LEFT_MARGINE, TIP_TEXT_TOP_MARGINE, m_msg.GetBuffer(0), m_msg.GetLength());
	::SetTextColor(hdc, old_text_color);

	::SelectObject(hdc, brush_old);
	::DeleteObject(brush);

	if(m_font != NULL) ::SelectObject(hdc, font_old);
	
	::ReleaseDC(m_hWnd, hdc);
}

void CToolTipEx::SetMessage(CString msg)
{
	m_msg = msg;
	CalcSize();
}

void CToolTipEx::CalcSize()
{
	HDC		hdc;
	DWORD	dw;
	HGDIOBJ pfnt_bk;

	hdc = ::GetDC(m_hWnd);
	if(m_font != NULL) pfnt_bk = ::SelectObject(hdc, m_font);
	dw = ::GetTabbedTextExtent(hdc, m_msg, m_msg.GetLength(), 1, NULL);
	m_size = CSize(LOWORD(dw) + TIP_TEXT_LEFT_MARGINE, HIWORD(dw) + TIP_TEXT_TOP_MARGINE);
	if(m_font != NULL) ::SelectObject(hdc, pfnt_bk);
	::ReleaseDC(NULL, hdc);
}

void CToolTipEx::SetFont(CFont *font)
{
	m_font = *font;
	CalcSize();
};


BOOL CToolTipEx::Show(HWND parent_wnd, RECT *rect)
{
	Hide();

	m_disp_rect = *rect;
	if(m_disp_rect.Height() == 0) {
		m_disp_rect.bottom = m_disp_rect.top + m_size.cy + TIP_TEXT_TOP_MARGINE;
	}

	CPoint pt;
	::GetCursorPos(&pt);
	if ( !m_disp_rect.PtInRect(pt) ) return FALSE;

	if ( m_disp_rect.Width() < m_size.cx ) {
		::SetTimer(m_hWnd, TIMER_ID_MOUSE_POS_CHECK, 200, NULL);
		::SetWindowPos(m_hWnd, HWND_TOPMOST, -1, -1, -1, -1, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE); // 手前に表示

		::MoveWindow(m_hWnd, m_disp_rect.left + m_pt.x, m_disp_rect.top + m_pt.y, 
			m_size.cx + TIP_TEXT_LEFT_MARGINE - 1, m_disp_rect.Height(), TRUE);

		m_parent_wnd = parent_wnd;
		m_b_show = TRUE;
		return ::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
	}

	return FALSE;
}

BOOL CToolTipEx::Hide()
{
	if(!m_b_show) return TRUE;

	m_b_show = FALSE;
	::KillTimer(m_hWnd, TIMER_ID_MOUSE_POS_CHECK);
	return ::ShowWindow(m_hWnd, SW_HIDE);
}

// ウィンドウ破棄時の処理
BOOL CToolTipEx::Destroy()
{
	Hide();
	return ::DestroyWindow(m_hWnd);
}
