/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // ReplaceDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "ReplaceDlg.h"
#include "OWinApp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SAVE_SEARCH_TEXT_CNT	20

#define SEARCH_DLG_MIN_WIDTH	550

/////////////////////////////////////////////////////////////////////////////
// CReplaceDlg ダイアログ


CReplaceDlg::CReplaceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CReplaceDlg::IDD, pParent)
	, m_distinct_width_ascii(FALSE)
{
	//{{AFX_DATA_INIT(CReplaceDlg)
	m_distinct_lwr_upr = FALSE;
	m_regexp = FALSE;
	m_replace_area = -1;
	//}}AFX_DATA_INIT
	m_disable_selected_area = FALSE;

	m_initialized = FALSE;

	m_search_status = new int[SAVE_SEARCH_TEXT_CNT];
	if(m_search_status != NULL) {
		int		i;
		for(i = 0; i < SAVE_SEARCH_TEXT_CNT; i++) {
			m_search_status[i] = 0;
		}
	}
}

CReplaceDlg::~CReplaceDlg()
{
	if(m_search_status != NULL) {
		delete[] m_search_status;
		m_search_status = NULL;
	}
}

void CReplaceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReplaceDlg)
	DDX_Control(pDX, IDCANCEL, m_btn_cancel);
	DDX_Control(pDX, IDC_COMBO_SEARCH_TEXT, m_combo_search_text);
	DDX_Control(pDX, IDC_COMBO_REPLACE_TEXT, m_combo_replace_text);
	DDX_Control(pDX, IDC_RADIO_SELECTED_AREA, m_radio_selected_area);
	DDX_Control(pDX, ID_BTN_REPLACE_ALL, m_btn_replace_all);
	DDX_Control(pDX, ID_BTN_REPLACE, m_btn_replace);
	DDX_Control(pDX, ID_BTN_PREV, m_btn_prev);
	DDX_Control(pDX, ID_BTN_NEXT, m_btn_next);
	DDX_Check(pDX, IDC_CHECK_DISTINCT_LWR_UPR, m_distinct_lwr_upr);
	DDX_Check(pDX, IDC_CHECK_REGEXP, m_regexp);
	DDX_Radio(pDX, IDC_RADIO_SELECTED_AREA, m_replace_area);
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_CHECK_DISTINCT_WIDTH_ASCII, m_distinct_width_ascii);
	DDX_Control(pDX, IDC_BTN_BOTTOM, m_btn_bottom);
}


BEGIN_MESSAGE_MAP(CReplaceDlg, CDialog)
	//{{AFX_MSG_MAP(CReplaceDlg)
	ON_BN_CLICKED(ID_BTN_NEXT, OnBtnNext)
	ON_BN_CLICKED(ID_BTN_PREV, OnBtnPrev)
	ON_BN_CLICKED(ID_BTN_REPLACE, OnBtnReplace)
	ON_BN_CLICKED(ID_BTN_REPLACE_ALL, OnBtnReplaceAll)
	ON_BN_CLICKED(IDC_RADIO_ALL, OnRadioAll)
	ON_BN_CLICKED(IDC_RADIO_SELECTED_AREA, OnRadioSelectedArea)
	ON_CBN_EDITCHANGE(IDC_COMBO_SEARCH_TEXT, OnEditchangeComboSearchText)
	ON_CBN_EDITCHANGE(IDC_COMBO_REPLACE_TEXT, OnEditchangeComboReplaceText)
	ON_WM_SHOWWINDOW()
	ON_CBN_SELENDOK(IDC_COMBO_SEARCH_TEXT, OnSelendokComboSearchText)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReplaceDlg メッセージ ハンドラ

void CReplaceDlg::SaveSearchDlgData()
{
	UpdateData(TRUE);

	m_data->m_search_text = m_search_text;
	m_data->m_replace_text = m_replace_text;
	m_data->m_distinct_lwr_upr = m_distinct_lwr_upr;
	m_data->m_distinct_width_ascii = m_distinct_width_ascii;
	m_data->m_regexp = m_regexp;
	if(m_replace_area == REPLACE_SELECTED_AREA) {
		m_data->m_replace_selected_area = TRUE;
	} else {
		m_data->m_replace_selected_area = FALSE;
	}
}

void CReplaceDlg::OnBtnNext() 
{
	CWaitCursor wait_cursor;
	UpdateData(TRUE);

	if(m_search_text == "") return;

	SaveSearchDlgData();
	m_data->m_dir = 1;

	m_wnd->SendMessage(m_search_message, 0, 0);

	SaveSearchTextList();
}

void CReplaceDlg::OnBtnPrev() 
{
	CWaitCursor wait_cursor;
	UpdateData(TRUE);

	if(m_search_text == "") return;

	SaveSearchDlgData();
	m_data->m_dir = -1;

	m_wnd->SendMessage(m_search_message, 0, 0);

	SaveSearchTextList();
}

void CReplaceDlg::OnBtnReplace() 
{
	if(m_btn_replace.IsWindowEnabled() == FALSE) return;

	CWaitCursor wait_cursor;
	UpdateData(TRUE);

	if(m_search_text == "") return;

	SaveSearchDlgData();
	m_data->m_dir = 1;
	m_data->m_all = FALSE;

	m_wnd->SendMessage(m_replace_message, 0, 0);
	SaveSearchTextList();

	m_combo_search_text.SetFocus();
}

void CReplaceDlg::OnBtnReplaceAll() 
{
	CWaitCursor	wait_cursor;
	UpdateData(TRUE);

	if(m_search_text == "") return;

	SaveSearchDlgData();
	m_data->m_dir = 1;
	m_data->m_all = TRUE;

	m_wnd->SendMessage(m_replace_message, 0, 0);

	SaveSearchTextList();
}

void CReplaceDlg::ShowDialog(CWnd *wnd, int search_message, int replace_message,
	CSearchDlgData *search_data)
{
	m_wnd = wnd;
	m_search_message = search_message;
	m_replace_message = replace_message;
	m_data = search_data;
	m_initialized = TRUE;
	m_disable_selected_area = FALSE;

	InitDialog();

	ShowWindow(SW_SHOWNORMAL);
}

void CReplaceDlg::InitDialog()
{
	// ダイアログの高さを決める
	// ディスプレイの設定が変更される可能性があるので、表示する度に計算する
	CRect	btn_rect;
	POINT pt = { 0, 0 };

	m_btn_bottom.GetWindowRect(btn_rect);
	pt.x = btn_rect.left;
	pt.y = btn_rect.bottom;
	::ScreenToClient(GetSafeHwnd(), &pt);
	m_dlg_height = pt.y + 10 + ::GetSystemMetrics(SM_CYCAPTION) + ::GetSystemMetrics(SM_CYSIZEFRAME) * 2;

	if(m_initialized == FALSE) return;

	m_search_text = m_data->m_search_text;
	m_replace_text = m_data->m_replace_text;
	m_distinct_lwr_upr = m_data->m_distinct_lwr_upr;
	m_distinct_width_ascii = m_data->m_distinct_width_ascii;
	m_regexp = m_data->m_regexp;
	if(m_data->m_replace_selected_area == TRUE) {
		m_replace_area = REPLACE_SELECTED_AREA;
	} else {
		m_replace_area = REPLACE_ALL;
	}

	if(m_disable_selected_area) {
		m_radio_selected_area.EnableWindow(FALSE);
	}

	LoadSearchTextList();

	if(m_search_text == "" && m_combo_search_text.GetCount() != 0) {
		m_combo_search_text.GetLBText(0, m_search_text);
		m_regexp = is_regexp(m_search_status[0]);
		m_distinct_lwr_upr = is_distinct_lwr_upr(m_search_status[0]);
		m_distinct_width_ascii = is_distinct_width_ascii(m_search_status[0]);
	}
	if(m_replace_text == "" && m_combo_replace_text.GetCount() != 0) {
		m_combo_replace_text.GetLBText(0, m_replace_text);
	}

	m_combo_search_text.SetFocus();

	UpdateData(FALSE);
	CheckBtn();

	LoadDlgSize();
}

BOOL CReplaceDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	InitDialog();
	
	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CReplaceDlg::CheckBtn()
{
	UpdateData(TRUE);

	if(m_search_text == "") {
		m_btn_next.EnableWindow(FALSE);
		m_btn_prev.EnableWindow(FALSE);
		m_btn_replace.EnableWindow(FALSE);
		m_btn_replace_all.EnableWindow(FALSE);
	} else {
		if(m_replace_area == REPLACE_ALL) {
			m_btn_next.EnableWindow(TRUE);
			m_btn_prev.EnableWindow(TRUE);
			m_btn_replace.EnableWindow(TRUE);
		} else {
			m_btn_next.EnableWindow(FALSE);
			m_btn_prev.EnableWindow(FALSE);
			m_btn_replace.EnableWindow(FALSE);
		}
		m_btn_replace_all.EnableWindow(TRUE);
	}
}

void CReplaceDlg::OnRadioAll() 
{
	CheckBtn();
}

void CReplaceDlg::OnRadioSelectedArea() 
{
	CheckBtn();
}

void CReplaceDlg::OnEditchangeComboSearchText() 
{
	CheckBtn();
}

void CReplaceDlg::OnEditchangeComboReplaceText() 
{
	CheckBtn();
}

void CReplaceDlg::LoadSearchTextList()
{
	::LoadSearchTextList(&m_combo_search_text, "SEARCH_TEXT", SAVE_SEARCH_TEXT_CNT, m_search_status);
	::LoadSearchTextList(&m_combo_replace_text, "REPLACE_TEXT", SAVE_SEARCH_TEXT_CNT, NULL);
}

void CReplaceDlg::SaveSearchTextList()
{
	int cur_status = make_search_status2(m_regexp, m_distinct_lwr_upr, m_distinct_width_ascii);
	::SaveSearchTextList("SEARCH_TEXT", SAVE_SEARCH_TEXT_CNT, m_search_text, TRUE, cur_status);
	::LoadSearchTextList(&m_combo_search_text, "SEARCH_TEXT", SAVE_SEARCH_TEXT_CNT, m_search_status);

	::SaveSearchTextList("REPLACE_TEXT", SAVE_SEARCH_TEXT_CNT, m_replace_text, FALSE, 0);
	::LoadSearchTextList(&m_combo_replace_text, "REPLACE_TEXT", SAVE_SEARCH_TEXT_CNT, NULL);

	UpdateData(FALSE);
	CheckBtn();
}

void CReplaceDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
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

void CReplaceDlg::OnOK()
{
	OnBtnReplace();
}

void CReplaceDlg::OnSelendokComboSearchText() 
{
	int idx = m_combo_search_text.GetCurSel();
	if(idx != CB_ERR) {
		m_combo_search_text.GetLBText(idx, m_search_text);
		m_regexp = is_regexp(m_search_status[idx]);
		m_distinct_lwr_upr = is_distinct_lwr_upr(m_search_status[idx]);
		m_distinct_width_ascii = is_distinct_width_ascii(m_search_status[idx]);

		UpdateData(FALSE);
	}

	CheckBtn();
}

INT_PTR CReplaceDlg::DoModal(CWnd *wnd, int search_message, int replace_message,
	BOOL disable_selected_area, CSearchDlgData *search_data)
{
	m_wnd = wnd;
	m_search_message = search_message;
	m_replace_message = replace_message;
	m_data = search_data;
	m_initialized = TRUE;
	m_disable_selected_area = disable_selected_area;

	return CDialog::DoModal();
}

BOOL CReplaceDlg::UpdateData(BOOL bSaveAndValidate)
{
	UpdateDataCombo(m_combo_search_text, m_search_text, bSaveAndValidate);
	UpdateDataCombo(m_combo_replace_text, m_replace_text, bSaveAndValidate);

	return CDialog::UpdateData(bSaveAndValidate);
}

void CReplaceDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	RelayoutControls();
}

void CReplaceDlg::OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI )
{
	// 横方向のみ可変にする
	lpMMI->ptMinTrackSize.x = SEARCH_DLG_MIN_WIDTH;
	lpMMI->ptMinTrackSize.y = m_dlg_height;
	lpMMI->ptMaxTrackSize.y = m_dlg_height;
}

static void AdjustBtnPos(CButton *btn, CRect *win_rect, POINT *pt)
{
	CRect	btn_rect;
	int		x, y;

	btn->GetWindowRect(btn_rect);
	x = win_rect->Width() - btn_rect.Width() - 18;
	y = btn_rect.top - pt->y;
	btn->SetWindowPos(NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

static void AdjustComboWidth(CComboBox *combo, CRect *win_rect, CRect *btn_rect)
{
	CRect	combo_rect;
	int		w, h;

	combo->GetWindowRect(combo_rect);
	w = btn_rect->left - combo_rect.left - 10;
	h = combo_rect.Height();
	combo->SetWindowPos(NULL, 0, 0, w, h, SWP_NOMOVE| SWP_NOZORDER);
}

void CReplaceDlg::RelayoutControls()
{
	if(!::IsWindow(GetSafeHwnd())) return;
	if(!::IsWindow(m_btn_next.GetSafeHwnd())) return;

	CRect	win_rect, btn_rect, combo_rect;
	POINT pt = { 0, 0 };

	GetWindowRect(win_rect);
	::ClientToScreen(GetSafeHwnd(), &pt);

	AdjustBtnPos(&m_btn_next, &win_rect, &pt);
	AdjustBtnPos(&m_btn_prev, &win_rect, &pt);
	AdjustBtnPos(&m_btn_cancel, &win_rect, &pt);
	AdjustBtnPos(&m_btn_replace, &win_rect, &pt);
	AdjustBtnPos(&m_btn_replace_all, &win_rect, &pt);

	m_btn_next.GetWindowRect(btn_rect);
	AdjustComboWidth(&m_combo_search_text, &win_rect, &btn_rect);
	AdjustComboWidth(&m_combo_replace_text, &win_rect, &btn_rect);
}

void CReplaceDlg::LoadDlgSize()
{
	COWinApp	*pApp = GetOWinApp();

	int w = pApp->GetIniFileInt(_T("REPLACE_DLG"), _T("DLG_WIDTH"), 0);
	if(w < SEARCH_DLG_MIN_WIDTH) w = SEARCH_DLG_MIN_WIDTH;

	SetWindowPos(NULL, 0, 0, w, m_dlg_height, SWP_NOMOVE | SWP_NOZORDER);
}

void CReplaceDlg::SaveDlgSize()
{
	COWinApp	*pApp = GetOWinApp();
	int w = pApp->GetIniFileInt(_T("REPLACE_DLG"), _T("DLG_WIDTH"), 0);

	CRect	win_rect;
	GetWindowRect(win_rect);

	if(w == win_rect.Width()) return;

	pApp->WriteIniFileInt(_T("REPLACE_DLG"), _T("DLG_WIDTH"), win_rect.Width());
	pApp->SaveIniFile();
}

void CReplaceDlg::OnCancel() 
{
	SaveDlgSize();
	
	CDialog::OnCancel();
}

