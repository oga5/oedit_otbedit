// DataEditDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
//#include "ogrid.h"
#include "Resource.h"
#include "DataEditDlg.h"
#include "UnicodeArchive.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DATAEDIT_DLG_MIN_WIDTH		400
#define DATAEDIT_DLG_MIN_HEIGHT		200


#define WMU_EDIT_ESCAPE		WM_APP + 0x01
#define WMU_EDIT_TAB		WM_APP + 0x02
#define WMU_EDIT_RETURN		WM_APP + 0x03

static LRESULT CALLBACK Edit_SubclassWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
	case WM_SYSKEYDOWN:
		// Alt + Enterで，OKボタンにする
		if(wParam == VK_RETURN && GetAsyncKeyState(VK_MENU) < 0) {
			::PostMessage(::GetParent(hwnd), WMU_EDIT_RETURN, 0, 0);
			return 0;
		}
		break;
	case WM_KEYDOWN:
		if(wParam == VK_ESCAPE) {
			::PostMessage(::GetParent(hwnd), WMU_EDIT_ESCAPE, 0, 0);
			return 0;
		} else if(wParam == VK_TAB && GetAsyncKeyState(VK_SHIFT) < 0) {
			//::PostMessage(::GetParent(hwnd), WMU_EDIT_TAB, 0, 0);
			//return 0;
		}
	default:
		break;
	}
	return (CallWindowProc((WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA),
		hwnd, message, wParam, lParam));
}

/////////////////////////////////////////////////////////////////////////////
// CDataEditDlg ダイアログ


CDataEditDlg::CDataEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDataEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDataEditDlg)
	m_line_mode_right = FALSE;
	//}}AFX_DATA_INIT
	m_line_type = LINE_TYPE_UNKNOWN;
	m_read_only = FALSE;
}


void CDataEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDataEditDlg)
	DDX_Control(pDX, IDC_STATIC_LINE_TYPE, m_static_line_type);
	DDX_Control(pDX, IDCANCEL, m_cancel);
	DDX_Control(pDX, IDC_COMBO_LINE_TYPE, m_combo_line_type);
	DDX_Control(pDX, IDOK2, m_ok);
	DDX_Check(pDX, IDC_CHECK_LINE_MODE_RIGHT, m_line_mode_right);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_CHECK_LINE_MODE_RIGHT, m_check_line_mode_right);
}


BEGIN_MESSAGE_MAP(CDataEditDlg, CDialog)
	//{{AFX_MSG_MAP(CDataEditDlg)
	ON_BN_CLICKED(IDOK2, OnOk2)
	ON_CBN_SELENDOK(IDC_COMBO_LINE_TYPE, OnSelendokComboLineType)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_CHECK_LINE_MODE_RIGHT, OnCheckLineModeRight)
	//}}AFX_MSG_MAP
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDataEditDlg メッセージ ハンドラ

void CDataEditDlg::OnOK() 
{
//	CDialog::OnOK();
}

void CDataEditDlg::SaveSetting()
{
	AfxGetApp()->WriteProfileInt(_T("DATA_EDIT"), _T("LINE_TYPE"), m_line_type);
	AfxGetApp()->WriteProfileInt(_T("SHOW_DATA_DLG"), _T("LINE_MODE_RIGHT"), m_line_mode_right);
}

void CDataEditDlg::OnOk2() 
{
	SaveSetting();
	CDialog::OnOK();
}

void CDataEditDlg::OnCancel() 
{
	SaveSetting();
	CDialog::OnCancel();
}

BOOL CDataEditDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if(m_line_type == LINE_TYPE_UNKNOWN) {
		m_line_type = AfxGetApp()->GetProfileInt(_T("DATA_EDIT"), _T("LINE_TYPE"), LINE_TYPE_CR_LF);
	}

	InitLineType();

	CreateEditCtrl();

	m_edit_ctrl.SetFocus();

	UpdateData(FALSE);

	if(!m_window_title.IsEmpty()) {
		SetWindowText(m_window_title);
	}
	
	return FALSE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CDataEditDlg::CreateEditCtrl()
{
	CRect rect;
	GetClientRect(&rect);

	CRect ok_rect;
	m_ok.GetWindowRect(&ok_rect);
	ScreenToClient(&ok_rect);

	m_edit_ctrl.SetEditData(m_edit_data);
	m_edit_ctrl.Create(NULL, NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER, 
		CRect(5, 5,
		rect.Width() - 10, ok_rect.top - 5), 
		this, NULL);
	m_edit_ctrl.SetFont(m_font);

	m_edit_ctrl.SetColor(BG_COLOR, m_grid_color[GRID_BG_COLOR]);
	m_edit_ctrl.SetColor(TEXT_COLOR, m_grid_color[GRID_TEXT_COLOR]);
	m_edit_ctrl.SetColor(PEN_COLOR, m_grid_color[GRID_MARK_COLOR]);
	m_edit_ctrl.SetColor(SELECTED_COLOR, m_grid_color[GRID_SELECT_COLOR]);

	m_edit_ctrl.SetExStyle2(m_edit_style);

	if(m_read_only) m_edit_ctrl.SetReadOnly(TRUE);

	// サブクラス化
	// 古いウィンドウプロシージャを保存する
	HWND hwnd = m_edit_ctrl.GetSafeHwnd();
	::SetWindowLongPtr (hwnd, GWLP_USERDATA, GetWindowLongPtr(hwnd, GWLP_WNDPROC));
	// ウィンドウプロシージャを切り替える
	::SetWindowLongPtr (hwnd, GWLP_WNDPROC, (LONG_PTR)Edit_SubclassWndProc);

	m_line_mode_right = AfxGetApp()->GetProfileInt(_T("SHOW_DATA_DLG"), _T("LINE_MODE_RIGHT"), FALSE);
	SetLineMode();
}

void CDataEditDlg::SetData(CEditData *edit_data, int line_type,
	CFont *font, QWORD edit_style, COLORREF *grid_color)
{
	m_edit_data = edit_data;
	m_line_type = line_type;
	m_font = font;
	m_edit_style = edit_style | ECS_ON_DIALOG | ECS_NO_COMMENT_CHECK;
	m_grid_color = grid_color;
	m_ini_line_type = line_type;
}

void CDataEditDlg::OnSelendokComboLineType() 
{
	SetLineType(GetLineType());
}

void CDataEditDlg::SetLineType(int line_type)
{
	m_edit_data->set_copy_line_type(line_type);
	m_line_type = line_type;
}

int CDataEditDlg::GetLineType()
{
	CComboBox *p_combo_line = &m_combo_line_type;

	int idx = p_combo_line->GetCurSel();
	if(idx == CB_ERR) LINE_TYPE_CR_LF;
	return (int)p_combo_line->GetItemData(idx);
}

void CDataEditDlg::InitLineType()
{
	CComboBox *p_combo_line = &m_combo_line_type;

	p_combo_line->InsertString(0, _T("CR+LF(\\r\\n)"));
	p_combo_line->SetItemData(0, LINE_TYPE_CR_LF);
	p_combo_line->InsertString(1, _T("LF(\\n)"));
	p_combo_line->SetItemData(1, LINE_TYPE_LF);
	p_combo_line->InsertString(2, _T("CR(\\r)"));
	p_combo_line->SetItemData(2, LINE_TYPE_CR);

	for(int i = 0; p_combo_line->GetItemData(i) != CB_ERR; i++) {
		if(p_combo_line->GetItemData(i) == (UINT)m_line_type) {
			p_combo_line->SetCurSel(i);
			break;
		}
	}
}

LRESULT CDataEditDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message) {
	case WMU_EDIT_ESCAPE:
		OnCancel();
		break;
	case WMU_EDIT_TAB:
		NextDlgCtrl();
		break;
	case WMU_EDIT_RETURN:
		OnOk2();
		break;
	}
	
	return CDialog::WindowProc(message, wParam, lParam);
}

BOOL CDataEditDlg::IsEditData()
{
	if(m_edit_data->is_edit_data()) return TRUE;
	if(m_ini_line_type != LINE_TYPE_UNKNOWN && m_ini_line_type != m_line_type) return TRUE;

	return FALSE;
}

void CDataEditDlg::OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI )
{
	lpMMI->ptMinTrackSize.x = DATAEDIT_DLG_MIN_WIDTH;
	lpMMI->ptMinTrackSize.y = DATAEDIT_DLG_MIN_HEIGHT;
}

void CDataEditDlg::RelayoutControls()
{
	if(!::IsWindow(GetSafeHwnd())) return;
	if(!::IsWindow(m_ok.GetSafeHwnd())) return;

	CRect	win_rect, client_rect, ok_btn_rect, line_type_rect, static_line_type_rect, edit_rect;
	POINT pt = { 0, 0 };
	POINT edit_pt = { 0, 0 };
	POINT btn_pt = { 0, 0 };

	GetWindowRect(win_rect);
	GetClientRect(client_rect);
	::ClientToScreen(GetSafeHwnd(), &pt);

	int line_type_x, line_type_y;
	int static_line_type_x, static_line_type_y;
	int line_mode_right_x, line_mode_right_y;
	int ok_x, ok_y;
	int cancel_x, cancel_y;
	int	w, h;

	m_ok.GetWindowRect(ok_btn_rect);
	m_combo_line_type.GetWindowRect(line_type_rect);
	m_static_line_type.GetWindowRect(static_line_type_rect);

	ok_x = win_rect.Width() / 2 - ok_btn_rect.Width() - 10;
	ok_y = win_rect.bottom - pt.y - ok_btn_rect.Height() - 15;

	cancel_x = win_rect.Width() / 2 + 10;
	cancel_y = ok_y;

	line_type_x = win_rect.Width() - line_type_rect.Width() - 20;
	line_type_y = ok_y;

	static_line_type_x = line_type_x - static_line_type_rect.Width() - 2;
	static_line_type_y = ok_y;

	line_mode_right_x = 10;
	line_mode_right_y = ok_y;

	if(ok_x + ok_btn_rect.Width() >= line_type_x) {
		ok_x = line_type_x - ok_btn_rect.Width() - 20;
	}

	m_ok.SetWindowPos(NULL, ok_x, ok_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	m_cancel.SetWindowPos(NULL, cancel_x, cancel_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	m_combo_line_type.SetWindowPos(NULL, line_type_x, line_type_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	m_static_line_type.SetWindowPos(&wndBottom, static_line_type_x, static_line_type_y, 0, 0, SWP_NOSIZE);
	m_check_line_mode_right.SetWindowPos(NULL, line_mode_right_x, line_mode_right_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	m_edit_ctrl.GetWindowRect(edit_rect);
	edit_pt.x = edit_rect.left;
	edit_pt.y = edit_rect.top;
	::ScreenToClient(GetSafeHwnd(), &edit_pt);
	w = client_rect.Width() - edit_pt.x * 2;
	h = ok_y - edit_pt.y - edit_pt.x;
	m_edit_ctrl.SetWindowPos(NULL, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);

	// Cancelボタンを改行コードのラベルより上に表示したいので、m_cancel.RedrawWindowを最後にする
	m_combo_line_type.RedrawWindow();
	m_static_line_type.RedrawWindow();
	m_ok.RedrawWindow();
	m_check_line_mode_right.RedrawWindow();
	m_cancel.RedrawWindow();
}

void CDataEditDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	RelayoutControls();	
}

void CDataEditDlg::OnCheckLineModeRight() 
{
	UpdateData();
	SetLineMode();
}

void CDataEditDlg::SetLineMode()
{
	if(m_line_mode_right) {
		m_edit_ctrl.SetLineMode(EC_LINE_MODE_RIGHT);
	} else {
		m_edit_ctrl.SetLineMode(EC_LINE_MODE_NORMAL);
	}
}

