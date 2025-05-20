// ShowCLobDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "global.h"
#include "ShowCLobDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CLOB_DLG_MIN_WIDTH		250
#define CLOB_DLG_MIN_HEIGHT		150

/////////////////////////////////////////////////////////////////////////////
// CShowCLobDlg ダイアログ


CShowCLobDlg::CShowCLobDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CShowCLobDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CShowCLobDlg)
	m_line_mode_right = FALSE;
	//}}AFX_DATA_INIT

	m_initialized = FALSE;
	m_b_modal = FALSE;
	m_edit_ctrl_created = FALSE;
	m_font = NULL;
	m_title = _T("");
}


void CShowCLobDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CShowCLobDlg)
	DDX_Control(pDX, IDC_CHECK_LINE_MODE_RIGHT, m_btn_line_mode_right);
	DDX_Control(pDX, IDOK2, m_ok);
	DDX_Check(pDX, IDC_CHECK_LINE_MODE_RIGHT, m_line_mode_right);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CShowCLobDlg, CDialog)
	//{{AFX_MSG_MAP(CShowCLobDlg)
	ON_BN_CLICKED(IDOK2, OnOk2)
	ON_BN_CLICKED(IDC_CHECK_LINE_MODE_RIGHT, OnCheckLineModeRight)
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_WM_GETMINMAXINFO()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

#define WMU_EDIT_ESCAPE		WM_APP + 0x01

static LRESULT CALLBACK Edit_SubclassWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
	case WM_KEYDOWN:
		if(wParam == VK_ESCAPE) {
			::PostMessage(::GetParent(hwnd), WMU_EDIT_ESCAPE, 0, 0);
			return 0;
		}
	default:
		break;
	}
	return (CallWindowProc((WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA),
		hwnd, message, wParam, lParam));
}

/////////////////////////////////////////////////////////////////////////////
// CShowCLobDlg メッセージ ハンドラ

void CShowCLobDlg::OnOK() 
{
//	CDialog::OnOK();
}

void CShowCLobDlg::OnOk2() 
{
	if(m_line_mode_right != (BOOL)AfxGetApp()->GetProfileInt(_T("SHOW_DATA_DLG"), _T("LINE_MODE_RIGHT"), FALSE)) {
		AfxGetApp()->WriteProfileInt(_T("SHOW_DATA_DLG"), _T("LINE_MODE_RIGHT"), m_line_mode_right);
	}

	if(m_b_modal) {
		CDialog::OnOK();
	} else {
		ShowWindow(SW_HIDE);
		DestroyWindow();
	}
}

void CShowCLobDlg::DoModal2()
{
	m_initialized = TRUE;
	m_b_modal = TRUE;
	CDialog::DoModal();
}

void CShowCLobDlg::ShowDialog()
{
	m_initialized = TRUE;
	m_b_modal = FALSE;
	InitDialog();
	ShowWindow(SW_SHOWNORMAL);
}

void CShowCLobDlg::InitDialog()
{
	if(!m_initialized) return;

	CreateEditCtrl();

	if(!m_title.IsEmpty()) {
		SetWindowText(m_title);
	}

	UpdateData(FALSE);
}

void CShowCLobDlg::SetColName(const TCHAR *col_name)
{
	m_title.Format(_T("データ参照 (%s)"), col_name);

	if(!m_initialized) return;

	SetWindowText(m_title);
}


BOOL CShowCLobDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	InitDialog();

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CShowCLobDlg::CreateEditCtrl()
{
	CRect rect;
	GetClientRect(&rect);

	CRect ok_rect;
	m_ok.GetWindowRect(&ok_rect);
	ScreenToClient(&ok_rect);

	m_edit_ctrl.SetEditData(&m_edit_data);
	m_edit_ctrl.SetReadOnly(TRUE);

	if(m_edit_ctrl.GetSafeHwnd() == NULL) {
		m_edit_ctrl.Create(NULL, NULL,
			WS_VISIBLE | WS_CHILD | WS_BORDER, 
			CRect(5, 5, rect.Width() - 5, ok_rect.top - 5), 
			this, NULL);

		// サブクラス化
		// 古いウィンドウプロシージャを保存する
		HWND hwnd = m_edit_ctrl.GetSafeHwnd();
		::SetWindowLongPtr (hwnd, GWLP_USERDATA, GetWindowLongPtr(hwnd, GWLP_WNDPROC));
		// ウィンドウプロシージャを切り替える
		::SetWindowLongPtr (hwnd, GWLP_WNDPROC, (LONG_PTR)Edit_SubclassWndProc);

		m_line_mode_right = AfxGetApp()->GetProfileInt(_T("SHOW_DATA_DLG"), _T("LINE_MODE_RIGHT"), FALSE);
		SetLineMode();
	}

	m_edit_ctrl.SetFont(m_font);

	QWORD style = ECS_ON_DIALOG;
	
	if(g_option.grid.show_space) style |= ECS_SHOW_SPACE;
	if(g_option.grid.show_2byte_space) style |= ECS_SHOW_2BYTE_SPACE;
	if(g_option.grid.show_tab) style |= ECS_SHOW_TAB;
	if(g_option.grid.show_line_feed) style |= ECS_SHOW_LINE_FEED;

	m_edit_ctrl.SetExStyle2(style);
}

LRESULT CShowCLobDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
	if(message == WMU_EDIT_ESCAPE) {
		OnCancel();
	}
	
	return CDialog::WindowProc(message, wParam, lParam);
}

void CShowCLobDlg::OnCheckLineModeRight() 
{
	UpdateData();
	SetLineMode();
}

void CShowCLobDlg::SetLineMode()
{
	if(m_line_mode_right) {
		m_edit_ctrl.SetLineMode(EC_LINE_MODE_RIGHT);
	} else {
		m_edit_ctrl.SetLineMode(EC_LINE_MODE_NORMAL);
	}
}

void CShowCLobDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
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

void CShowCLobDlg::RelayoutControls()
{
	if(!::IsWindow(GetSafeHwnd())) return;
	if(!::IsWindow(m_ok.GetSafeHwnd())) return;

	CRect	win_rect, client_rect, ok_btn_rect, line_mode_btn_rect, edit_rect;
	POINT pt = { 0, 0 };
	POINT edit_pt = { 0, 0 };
	POINT btn_pt = { 0, 0 };

	GetWindowRect(win_rect);
	GetClientRect(client_rect);
	::ClientToScreen(GetSafeHwnd(), &pt);

	int line_mode_x, line_mode_y;
	int ok_x, ok_y;
	int	w, h;

	m_ok.GetWindowRect(ok_btn_rect);
	m_btn_line_mode_right.GetWindowRect(line_mode_btn_rect);

	ok_x = win_rect.Width() / 2 - ok_btn_rect.Width() / 2;
	ok_y = win_rect.bottom - pt.y - ok_btn_rect.Height() - 10;
	line_mode_x = win_rect.Width() - line_mode_btn_rect.Width() - 10;
	line_mode_y = ok_y;

	if(ok_x + ok_btn_rect.Width() >= line_mode_x) {
		ok_x = line_mode_x - ok_btn_rect.Width() - 20;
	}

	m_ok.SetWindowPos(NULL, ok_x, ok_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	m_btn_line_mode_right.SetWindowPos(NULL, line_mode_x, line_mode_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	m_edit_ctrl.GetWindowRect(edit_rect);
	edit_pt.x = edit_rect.left;
	edit_pt.y = edit_rect.top;
	::ScreenToClient(GetSafeHwnd(), &edit_pt);
	w = client_rect.Width() - edit_pt.x * 2;
	h = ok_y - edit_pt.y - edit_pt.x;
	m_edit_ctrl.SetWindowPos(NULL, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);

	m_ok.RedrawWindow();
	m_btn_line_mode_right.RedrawWindow();
}

void CShowCLobDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	RelayoutControls();

	RedrawWindow();
}

void CShowCLobDlg::OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI )
{
	lpMMI->ptMinTrackSize.x = CLOB_DLG_MIN_WIDTH;
	lpMMI->ptMinTrackSize.y = CLOB_DLG_MIN_HEIGHT;
}


void CShowCLobDlg::OnClose()
{
	// TODO: ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。

//	CDialog::OnClose();
	OnOk2();
}

void CShowCLobDlg::OnCancel()
{
	// TODO: ここに特定なコードを追加するか、もしくは基底クラスを呼び出してください。

//	CDialog::OnCancel();
	OnOk2();
}

void CShowCLobDlg::RedrawEditCtrl()
{
	m_edit_ctrl.Redraw();
}
