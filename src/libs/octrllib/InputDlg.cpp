/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "InputDlg.h"

#define INPUT_DLG_EDIT_ID		1010
#define INPUT_DLG_OK_ID			1011
#define INPUT_DLG_CANCEL_ID		1012

CInputDlg::CInputDlg()
{
	memset(&m_dtp, 0, sizeof(m_dtp));
}

CInputDlg::~CInputDlg()
{
}

BOOL CInputDlg::CreateDlg(CWnd *parent, const TCHAR *title, const TCHAR *value)
{
	m_title = title;
	m_value = value;

	memset(&m_dtp, 0, sizeof(m_dtp));

	m_dtp.dtp.cdit = 0;
	m_dtp.dtp.cx = 250;
	m_dtp.dtp.cy = 70;
	m_dtp.dtp.x = 0;
	m_dtp.dtp.y = 0;
	m_dtp.dtp.style = WS_VISIBLE | DS_MODALFRAME | DS_3DLOOK;
	m_dtp.dtp.dwExtendedStyle = 0;

	InitModalIndirect( &m_dtp, parent );

	return TRUE;
}

BOOL CInputDlg::OnInitDialog()
{
	SetWindowText(m_title);

	m_font.CreatePointFont(90, _T("‚l‚r ƒSƒVƒbƒN"));
	SetFont(&m_font);

	CRect dlg_rect;
	GetClientRect(dlg_rect);

	LOGFONT	lf;
	GetFont()->GetLogFont(&lf);

	int font_height = 20;
	int edit_x = 10;
	int edit_y = 10;

	CRect rect(edit_x, edit_y, dlg_rect.Width() - 10, edit_y + font_height);
	m_edit.Create(WS_CHILD | WS_BORDER | WS_VISIBLE | ES_AUTOHSCROLL, 
		rect, this, INPUT_DLG_EDIT_ID);
	m_edit.SetFont(&m_font);
	m_edit.SetWindowText(m_value);

	int center = dlg_rect.Width() / 2;
	int btn_y = edit_y + font_height + 10;
	int btn_h = font_height + 4;
	int btn_w = 80;
	CRect ok_rect(center - 10 - btn_w, btn_y, center - 10, btn_y + btn_h);
	m_ok.Create(_T("OK"), WS_CHILD | WS_VISIBLE, ok_rect, this, IDOK);
	m_ok.SetFont(&m_font);

	CRect cancel_rect(center + 10, btn_y, center + 10 + btn_w, btn_y + btn_h);
	m_cancel.Create(_T("CANCEL"), WS_CHILD | WS_VISIBLE, cancel_rect, this, IDCANCEL);
	m_cancel.SetFont(&m_font);

	int win_width = dlg_rect.Width() + ::GetSystemMetrics(SM_CXFIXEDFRAME) * 2 + 15;
	int win_height = btn_y + btn_h + 25 + ::GetSystemMetrics(SM_CYCAPTION) + ::GetSystemMetrics(SM_CYFIXEDFRAME) * 2;
	SetWindowPos(NULL, 0, 0, win_width, win_height, SWP_NOMOVE | SWP_NOZORDER);

	return TRUE;
}

void CInputDlg::OnOK()
{
	m_edit.GetWindowText(m_value);
	CDialog::OnOK();
}