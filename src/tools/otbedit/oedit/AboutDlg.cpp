#include "stdafx.h"

#include "resource.h"
#include "file_winutil.h"
#include "AboutDlg.h"
#include "EditCtrl.h"

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	m_static0 = _T("");
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Control(pDX, IDC_STATIC_URL, m_static_url);
	DDX_Text(pDX, IDC_STATIC_VERSION, m_static0);
	//}}AFX_DATA_MAP
}

/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	// TODO: この位置に初期化の補足処理を追加してください
	// exeのバージョンを表示
	TCHAR	filename[MAX_PATH];
	CString	file_version;

	GetModuleFileName(AfxGetInstanceHandle(), filename, sizeof(filename));
	GetFileVersion(filename, &file_version);
	
	m_static0.Format(_T("otbedit Version %s"), file_version);

	UpdateData(FALSE);

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

BOOL CAboutDlg::LinkHitTest(CPoint point)
{
	if(m_static_url.IsWindowVisible() == FALSE) return FALSE;

	RECT	rect;
	m_static_url.GetWindowRect(&rect);
	ScreenToClient(&rect);

	if((rect.top <= point.y && rect.bottom >= point.y) &&
		(rect.left <= point.x && rect.right >= point.x)) {
		return TRUE;
	}

	return FALSE;
}

void CAboutDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: この位置にメッセージ ハンドラ用のコードを追加するかまたはデフォルトの処理を呼び出してください
	if(LinkHitTest(point) == TRUE) {
		if(CEditCtrl::m_link_cursor != NULL) {
			SetCursor(CEditCtrl::m_link_cursor);
		}
	}

	CDialog::OnMouseMove(nFlags, point);
}

void CAboutDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if(LinkHitTest(point) == TRUE) {
		CString url;
		m_static_url.GetWindowText(url);
		ShellExecute(NULL, NULL, url, NULL, NULL, SW_SHOWNORMAL);
	}
	
	CDialog::OnLButtonDown(nFlags, point);
}

HBRUSH CAboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: この位置で DC のアトリビュートを変更してください
	int id = pWnd->GetDlgCtrlID();
	if(id == IDC_STATIC_URL) {
		pDC->SetTextColor(RGB(0, 0, 255));
	}

	// TODO: デフォルトのブラシが望みのものでない場合には、違うブラシを返してください
	return hbr;
}

