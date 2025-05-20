/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

// SelectColorDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "oedit.h"
#include "SelectColorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectColorDlg ダイアログ

CSelectColorDlg::CSelectColorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectColorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectColorDlg)
	m_rgb_value = _T("");
	m_format = -1;
	//}}AFX_DATA_INIT

	m_initialized = FALSE;
	m_spoit_cursor = AfxGetApp()->LoadCursor(IDC_SPOIT_CURSOR);
}


void CSelectColorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectColorDlg)
	DDX_Text(pDX, IDC_EDIT_RGB, m_rgb_value);
	DDX_CBIndex(pDX, IDC_COMBO_FORMAT, m_format);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectColorDlg, CDialog)
	//{{AFX_MSG_MAP(CSelectColorDlg)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_SEL_COLOR_BTN, OnSelColorBtn)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SHOWWINDOW()
	ON_CBN_SELCHANGE(IDC_COMBO_FORMAT, OnSelchangeComboFormat)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectColorDlg メッセージ ハンドラ
void CSelectColorDlg::InitDialog()
{
	CreateColorWnd();

	m_bCapturing = FALSE;
	LoadColorInfo();

	m_format = GetOWinApp()->GetIniFileInt(_T("COLORDIALOG"), _T("FORMAT"), 0);
	m_selected_color = m_color_ctrl.GetSelectedColor();
	m_rgb_value = GetRGBValue(m_selected_color);
	UpdateData(FALSE);
}

BOOL CSelectColorDlg::OnInitDialog() 
{
	if(m_initialized == FALSE) return FALSE;

	CDialog::OnInitDialog();
	
	// TODO: この位置に初期化の補足処理を追加してください
	InitDialog();

	return FALSE;
}

void CSelectColorDlg::CreateColorWnd()
{
	if(::IsWindow(m_color_ctrl.GetSafeHwnd())) return;

	RECT rect;
	GetClientRect(&rect);

	m_color_ctrl.Create(NULL, NULL, WS_VISIBLE | WS_CHILD, 
		CRect(10, 10, rect.right - 140, rect.bottom - 50), 
		this, NULL);
	m_color_ctrl.SetFocus();
}

COLORREF default_color[] = {
	RGB(0x00, 0x00, 0x00),
	RGB(0xff, 0x00, 0x00),
	RGB(0x00, 0xff, 0x00),
	RGB(0xff, 0xff, 0x00),
	RGB(0x00, 0x00, 0xff),
	RGB(0xff, 0x00, 0xff),
	RGB(0x00, 0xff, 0xff),
	RGB(0xff, 0xff, 0xff),
	RGB(0x80, 0x00, 0x00),
	RGB(0x00, 0x80, 0x00),
	RGB(0x80, 0x80, 0x00),
	RGB(0x00, 0x00, 0x80),
	RGB(0x80, 0x00, 0x80),
	RGB(0x00, 0x80, 0x80),
	RGB(0x80, 0x80, 0x80),
	RGB(0xC0, 0xC0, 0xC0),
};

void CSelectColorDlg::LoadColorInfo()
{
	CString entry;
	int		i;
	COWinApp *pApp = GetOWinApp();
	for(i = 0; i < sizeof(default_color) / sizeof(default_color[0]); i++) {
		entry.Format(_T("CUSTOMER_COLOR_%d"), i);
		m_color_ctrl.SetColor(i, pApp->GetIniFileInt(_T("COLORDIALOG"), entry, default_color[i]));
	}
	for(; i < MAX_COLOR_CNT; i++) {
		entry.Format(_T("CUSTOMER_COLOR_%d"), i);
		m_color_ctrl.SetColor(i, pApp->GetIniFileInt(_T("COLORDIALOG"), entry, RGB(0xff, 0xff, 0xff)));
	}
}

void CSelectColorDlg::SaveColorInfo()
{
	CString entry;
	int		i;
	COWinApp *pApp = GetOWinApp();
	for(i = 0; i < MAX_COLOR_CNT; i++) {
		entry.Format(_T("CUSTOMER_COLOR_%d"), i);
		pApp->WriteIniFileInt(_T("COLORDIALOG"), entry, m_color_ctrl.GetColor(i));
	}
	pApp->WriteIniFileInt(_T("COLORDIALOG"), _T("FORMAT"), m_format);
	pApp->SaveIniFile();
}

void CSelectColorDlg::GetColorBtnRect(RECT *rect)
{
	CRect btn_rect;
	GetDlgItem(IDC_SEL_COLOR_BTN)->GetWindowRect(&btn_rect);
	ScreenToClient(&btn_rect);

	*rect = btn_rect;
	rect->top = btn_rect.bottom + 10;
	rect->bottom = rect->top + btn_rect.Height();
}

void CSelectColorDlg::GetSelectedColorRect(RECT *rect)
{
	RECT win_rect;
	GetClientRect(&win_rect);

	CRect btn_rect;
	GetDlgItem(IDC_STATIC_RESULT)->GetWindowRect(&btn_rect);
	ScreenToClient(&btn_rect);

	*rect = btn_rect;
	rect->top = btn_rect.bottom;
	rect->bottom = win_rect.bottom - 50;
}

void CSelectColorDlg::OnPaint() 
{
	CPaintDC dc(this); // 描画用のデバイス コンテキスト
	
	// TODO: この位置にメッセージ ハンドラ用のコードを追加してください
	CRect draw_rect;
	GetSelectedColorRect(&draw_rect);
	if(dc.RectVisible(&draw_rect)) {
		dc.DrawEdge(&draw_rect, EDGE_SUNKEN, BF_RECT);
		draw_rect.DeflateRect(4, 4);
		dc.FillSolidRect(&draw_rect, m_selected_color);
	}

	RECT	btn_rect;
	GetColorBtnRect(&btn_rect);
	if(dc.RectVisible(&btn_rect)) {
		dc.Draw3dRect(&btn_rect, GetSysColor(COLOR_BTNHILIGHT), GetSysColor(COLOR_BTNSHADOW));
		CFont *old_font = dc.SelectObject(GetFont());
		dc.SetBkColor(GetSysColor(COLOR_BTNFACE));
		dc.DrawText(_T("スポイト"), -1, &btn_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		dc.SelectObject(old_font);
	}
}

void CSelectColorDlg::InvalidateSelectedColor()
{
	RECT draw_rect;
	GetSelectedColorRect(&draw_rect);
	InvalidateRect(&draw_rect);

	m_rgb_value = GetRGBValue(m_selected_color);
	UpdateData(FALSE);
}

LRESULT CSelectColorDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message) {
	case CGC_WM_SELECTED_COLOR:
		m_selected_color = (COLORREF)wParam;
		InvalidateSelectedColor();
		break;
	case CGC_WM_DBL_CLICK:
		OnOK();
		break;
	}

	return CDialog::WindowProc(message, wParam, lParam);
}

void CSelectColorDlg::OnSelColorBtn() 
{
	CColorDialog	dlg(m_color_ctrl.GetSelectedColor(),
		CC_FULLOPEN | CC_RGBINIT);

	if(dlg.DoModal() != IDOK) return;

	m_color_ctrl.SetSelectedColor(dlg.GetColor());
}

void CSelectColorDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bCapturing)
	{
		ClientToScreen(&point);
		HDC hScreenDC = ::GetDC(NULL);
		COLORREF crColor = ::GetPixel(hScreenDC,point.x,point.y);
		m_selected_color = (COLORREF)crColor;
		InvalidateSelectedColor();
	}
	
	CDialog::OnMouseMove(nFlags, point);
}

void CSelectColorDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect	btn_rect;
	GetColorBtnRect(btn_rect);
	if(btn_rect.PtInRect(point)) {
		StartCapture();
	}

	CDialog::OnLButtonDown(nFlags, point);
}

BOOL CSelectColorDlg::PreTranslateMessage(MSG* pMsg) 
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE && m_bCapturing) {
		StopCapture();
		m_selected_color = m_saved_selected_color;
		InvalidateSelectedColor();
		return TRUE;
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CSelectColorDlg::StartCapture()
{
	SetCapture();
	SetCursor(m_spoit_cursor);
	m_saved_selected_color = m_selected_color;
	m_bCapturing = TRUE;
}

void CSelectColorDlg::StopCapture()
{
	ReleaseCapture();
	m_bCapturing = FALSE;
	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
}

void CSelectColorDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if(m_bCapturing) {
		ClientToScreen(&point);
		HDC hScreenDC = ::GetDC(NULL);			
		COLORREF crColor = ::GetPixel(hScreenDC,point.x,point.y);
		m_color_ctrl.SetSelectedColor(crColor);		

		StopCapture();
	}
	
	CDialog::OnLButtonUp(nFlags, point);
}

void CSelectColorDlg::OnOK() 
{
	m_wnd->SendMessage(m_paste_color_message, (WPARAM)m_rgb_value.GetBuffer(0), 0);
}

#define BGR(rgb) (COLORREF)( \
	(((COLORREF)rgb & 0x00FF0000) >> 16) | \
	(((COLORREF)rgb & 0x0000FF00)) | \
	(((COLORREF)rgb & 0x000000FF) << 16) \
	)

CString CSelectColorDlg::GetRGBValue(int selected_color)
{
	CString ret;

	if(m_format == 1) {
		ret.Format(_T("RGB(0x%.2x, 0x%.2x, 0x%.2x)"), 
			(selected_color & 0x000000FF),
			(selected_color & 0x0000FF00) >> 8,
			(selected_color & 0x00FF0000) >> 16);
		return ret;
	}

	ret.Format(_T("#%.6x"), BGR(selected_color));
	return ret;
}

void CSelectColorDlg::ShowDialog(CWnd *wnd, int paste_color_message)
{
	m_wnd = wnd;
	m_paste_color_message = paste_color_message;

	if(!m_initialized) 	CDialog::OnInitDialog();
	m_initialized = TRUE;

	InitDialog();

	ShowWindow(SW_SHOWNORMAL);
}

INT_PTR CSelectColorDlg::DoModal(CWnd *wnd, int paste_color_message)
{
	m_wnd = wnd;
	m_paste_color_message = paste_color_message;
	m_initialized = TRUE;

	return CDialog::DoModal();
}

void CSelectColorDlg::OnCancel() 
{
	SaveColorInfo();

	CDialog::OnCancel();
}

void CSelectColorDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);

	// 親ウィンドウの中央に表示する
	if(bShow) {
		CRect	win_rect, parent_rect;
		GetWindowRect(win_rect);
		GetParent()->GetWindowRect(parent_rect);

		int x, y;
		x = parent_rect.left + parent_rect.Width() / 2 - win_rect.Width() / 2;
		y = parent_rect.top + parent_rect.Height() / 2 - win_rect.Height() / 2;

		SetWindowPos(NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
}

void CSelectColorDlg::OnSelchangeComboFormat() 
{
	UpdateData();
	InvalidateSelectedColor();
}
