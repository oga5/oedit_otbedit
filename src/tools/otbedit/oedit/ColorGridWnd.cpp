/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // ColorGridWnd.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "oedit.h"
#include "ColorGridWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorGridWnd

#define COLORGRIDCTRL_CLASSNAME	_T("OGAWA_COLORGRIDCtrl")

CColorGridWnd::CColorGridWnd()
{
	WNDCLASS wndcls;
	HINSTANCE hInst = AfxGetInstanceHandle();

	if (!(::GetClassInfo(hInst, COLORGRIDCTRL_CLASSNAME, &wndcls)))
	{
		// otherwise we need to register a new class
		wndcls.style            = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndcls.lpfnWndProc      = ::DefWindowProc;
		wndcls.cbClsExtra       = wndcls.cbWndExtra = 0;
		wndcls.hInstance        = hInst;
		wndcls.hIcon            = NULL;
		wndcls.hCursor          = AfxGetApp()->LoadStandardCursor(IDC_IBEAM);
		wndcls.hbrBackground    = (HBRUSH) COLOR_WINDOW;
		wndcls.lpszMenuName     = NULL;
		wndcls.lpszClassName    = COLORGRIDCTRL_CLASSNAME;

		if (!AfxRegisterClass(&wndcls)) {
			AfxThrowResourceException();
		}
	}

	m_x = 8;
	m_y = 8;

	m_selected.x = 0;
	m_selected.y = 0;


	int		i;

	m_undo_cnt = 0;
	for(i = 0; i < MAX_UNDO_CNT; i++) {
		m_undo_data[i].idx = -1;
		m_undo_data[i].old_color_info = RGB(0xff, 0xff, 0xff);
		m_undo_data[i].new_color_info = RGB(0xff, 0xff, 0xff);
	}

	for(i = 0; i < MAX_COLOR_CNT; i++) {
		m_color_info[i] = RGB(0xff, 0xff, 0xff);
	}
}

CColorGridWnd::~CColorGridWnd()
{
}


BEGIN_MESSAGE_MAP(CColorGridWnd, CWnd)
	//{{AFX_MSG_MAP(CColorGridWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_GETDLGCODE()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CColorGridWnd メッセージ ハンドラ

void CColorGridWnd::OnPaint() 
{
	CPaintDC dc(this); // 描画用のデバイス コンテキスト
	
	// TODO: この位置にメッセージ ハンドラ用のコードを追加してください
	CRect	edge_rect;
	edge_rect.top = 0;
	edge_rect.bottom = m_y * m_col_height + 5;
	edge_rect.left = 0;
	edge_rect.right = m_x * m_col_width + 5;
	if(dc.RectVisible(edge_rect)) {
		dc.DrawEdge(edge_rect, EDGE_SUNKEN, BF_RECT);
	}

	int		x, y;

	CRect	paint_rect;
	CPen	pen1, pen2;
	pen1.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
	pen2.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOWTEXT));
	CPen	*old_pen;

	for(y = 0; y < m_y; y++) {
		for(x = 0; x < m_x; x++) {
			paint_rect.top = y * m_col_height + 2;
			paint_rect.bottom = paint_rect.top + m_col_height;
			paint_rect.left = x * m_col_width + 2;
			paint_rect.right = paint_rect.left + m_col_width;

			if(dc.RectVisible(paint_rect)) {
				dc.FillSolidRect(&paint_rect, GetSysColor(COLOR_BTNFACE));

				paint_rect.DeflateRect(1, 1);
				paint_rect.left++;
				paint_rect.top++;
				if(m_selected.y == y && m_selected.x == x) {
					dc.FillSolidRect(&paint_rect, GetSysColor(COLOR_WINDOW));
					
					old_pen = dc.SelectObject(&pen2);
					dc.MoveTo(paint_rect.left, paint_rect.top);
					dc.LineTo(paint_rect.left, paint_rect.bottom);
					dc.LineTo(paint_rect.right, paint_rect.bottom);
					dc.LineTo(paint_rect.right, paint_rect.top);
					dc.LineTo(paint_rect.left, paint_rect.top);
					dc.SelectObject(old_pen);
				}

				paint_rect.DeflateRect(3, 3);
				dc.FillSolidRect(&paint_rect, m_color_info[GetIdx(x, y)]);
				old_pen = dc.SelectObject(&pen1);
				dc.MoveTo(paint_rect.left, paint_rect.top);
				dc.LineTo(paint_rect.left, paint_rect.bottom);
				dc.LineTo(paint_rect.right, paint_rect.bottom);
				dc.LineTo(paint_rect.right, paint_rect.top);
				dc.LineTo(paint_rect.left, paint_rect.top);
				dc.SelectObject(old_pen);
			}
		}
	}

	// 描画用メッセージとして CWnd::OnPaint() を呼び出してはいけません
}

void CColorGridWnd::SetColor(int idx, COLORREF col)
{
	m_color_info[idx] = col;
}

void CColorGridWnd::SetSelectedColor(COLORREF col)
{
	m_undo_cnt++;
	if(m_undo_cnt >= MAX_UNDO_CNT) m_undo_cnt = 0;
	m_undo_data[m_undo_cnt].idx = GetSelectedIdx();
	m_undo_data[m_undo_cnt].old_color_info = m_color_info[m_undo_data[m_undo_cnt].idx];
	m_undo_data[m_undo_cnt].new_color_info = col;

	int undo_cnt = m_undo_cnt + 1;
	if(undo_cnt >= MAX_UNDO_CNT) undo_cnt = 0;
	m_undo_data[undo_cnt].idx = -1;

	m_color_info[GetSelectedIdx()] = col;
	InvalidateCell(m_selected.x, m_selected.y);
	GetParent()->SendMessage(CGC_WM_SELECTED_COLOR, (WPARAM)GetSelectedColor());
}

void CColorGridWnd::InvalidateCell(int x, int y)
{
	CRect paint_rect;
	paint_rect.top = y * m_col_height + 2;
	paint_rect.bottom = paint_rect.top + m_col_height;
	paint_rect.left = x * m_col_width + 2;
	paint_rect.right = paint_rect.left + m_col_width;

	InvalidateRect(paint_rect);
}

void CColorGridWnd::SelectCell(int x, int y)
{
	InvalidateCell(m_selected.x, m_selected.y);
	m_selected.x = x;
	m_selected.y = y;
	InvalidateCell(m_selected.x, m_selected.y);

	GetParent()->SendMessage(CGC_WM_SELECTED_COLOR, (WPARAM)GetSelectedColor());
}

void CColorGridWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetFocus();

	// TODO: この位置にメッセージ ハンドラ用のコードを追加するかまたはデフォルトの処理を呼び出してください
	int x = point.x / m_col_width;
	int	y = point.y / m_col_height;

	SelectCell(x, y);
	
	CWnd::OnLButtonDown(nFlags, point);
}

int CColorGridWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: この位置に固有の作成用コードを追加してください
	CRect	win_rect;
	GetClientRect(win_rect);

	m_col_width = (win_rect.Width() - 5) / m_x;
	m_col_height = (win_rect.Height() - 5) / m_y;
	
	return 0;
}

BOOL CColorGridWnd::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;	
}

void CColorGridWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: この位置にメッセージ ハンドラ用のコードを追加するかまたはデフォルトの処理を呼び出してください
	if(GetAsyncKeyState(VK_CONTROL) < 0) {
		switch(nChar) {
		case 'Z':
			Undo();
			break;
		case 'Y':
			Redo();
			break;
		}
	} else {
		int		x = m_selected.x;
		int		y = m_selected.y;
		switch(nChar) {
		case VK_LEFT:
			x--;
			break;
		case VK_RIGHT:
			x++;
			break;
		case VK_UP:
			y--;
			break;
		case VK_DOWN:
			y++;
			break;
		}

		if(x < 0) x = 0;
		if(x >= m_x) x = m_x - 1;
		if(y < 0) y = 0;
		if(y >= m_y) y = m_y - 1;
		
		if(x != m_selected.x || y != m_selected.y) SelectCell(x, y);
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

UINT CColorGridWnd::OnGetDlgCode() 
{
	// TODO: この位置にメッセージ ハンドラ用のコードを追加するかまたはデフォルトの処理を呼び出してください
	return DLGC_WANTARROWS | DLGC_WANTCHARS;
//	return CWnd::OnGetDlgCode();
}

void CColorGridWnd::Undo()
{
	if(m_undo_data[m_undo_cnt].idx == -1) return;

	m_color_info[m_undo_data[m_undo_cnt].idx] = m_undo_data[m_undo_cnt].old_color_info;
	SelectCell(m_undo_data[m_undo_cnt].idx % m_x, m_undo_data[m_undo_cnt].idx / m_x);

	m_undo_cnt--;
	if(m_undo_cnt < 0) m_undo_cnt = MAX_UNDO_CNT - 1;
}

void CColorGridWnd::Redo()
{
	int undo_cnt = m_undo_cnt + 1;
	if(undo_cnt >= MAX_UNDO_CNT) undo_cnt = 0;
	if(m_undo_data[undo_cnt].idx == -1) return;

	m_undo_cnt = undo_cnt;
	m_color_info[m_undo_data[m_undo_cnt].idx] = m_undo_data[m_undo_cnt].new_color_info;
	SelectCell(m_undo_data[m_undo_cnt].idx % m_x, m_undo_data[m_undo_cnt].idx / m_x);
}


void CColorGridWnd::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	GetParent()->SendMessage(CGC_WM_DBL_CLICK, (WPARAM)GetSelectedColor());
	
	CWnd::OnLButtonDblClk(nFlags, point);
}
