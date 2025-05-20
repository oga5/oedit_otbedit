/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // FilterDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "FilterDlg.h"
#include "OWinApp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SAVE_SEARCH_TEXT_CNT	20

/////////////////////////////////////////////////////////////////////////////
// CFilterDlg ダイアログ


CFilterDlg::CFilterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFilterDlg::IDD, pParent)
	, m_distinct_width_ascii(FALSE)
{
	//{{AFX_DATA_INIT(CFilterDlg)
	m_distinct_lwr_upr = FALSE;
	m_distinct_width_ascii = FALSE;
	m_regexp = FALSE;
	m_combo_action = -1;
	//}}AFX_DATA_INIT

	m_initialized = FALSE;

	m_combo_action = FILTER_GREP_MODE;
	m_data = NULL;
	m_filter_message = 0;
	m_search_message = 0;
	m_wnd = NULL;

	m_search_status = new int[SAVE_SEARCH_TEXT_CNT];
	if(m_search_status != NULL) {
		int		i;
		for(i = 0; i < SAVE_SEARCH_TEXT_CNT; i++) {
			m_search_status[i] = 0;
		}
	}
}

CFilterDlg::~CFilterDlg()
{
	if(m_search_status != NULL) {
		delete[] m_search_status;
		m_search_status = NULL;
	}
}

void CFilterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFilterDlg)
	DDX_Control(pDX, ID_BTN_FILTER, m_btn_filter);
	DDX_Control(pDX, ID_BTN_NEXT, m_btn_next);
	DDX_Control(pDX, IDC_COMBO_SEARCH_TEXT, m_combo_search_text);
	DDX_Check(pDX, IDC_CHECK_DISTINCT_LWR_UPR, m_distinct_lwr_upr);
	DDX_Check(pDX, IDC_CHECK_REGEXP, m_regexp);
	DDX_CBIndex(pDX, IDC_COMBO_ACTION, m_combo_action);
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_CHECK_DISTINCT_WIDTH_ASCII, m_distinct_width_ascii);
	DDX_Control(pDX, IDC_BTN_BOTTOM, m_btn_bottom);
}


BEGIN_MESSAGE_MAP(CFilterDlg, CDialog)
	//{{AFX_MSG_MAP(CFilterDlg)
	ON_BN_CLICKED(ID_BTN_NEXT, OnBtnNext)
	ON_BN_CLICKED(ID_BTN_FILTER, OnBtnFilter)
	ON_CBN_EDITCHANGE(IDC_COMBO_SEARCH_TEXT, OnEditchangeComboSearchText)
	ON_CBN_SELENDOK(IDC_COMBO_SEARCH_TEXT, OnSelendokComboSearchText)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilterDlg メッセージ ハンドラ


void CFilterDlg::SaveSearchDlgData()
{
	UpdateData(TRUE);

	m_data->m_search_text = m_search_text;
	m_data->m_distinct_lwr_upr = m_distinct_lwr_upr;
	m_data->m_distinct_width_ascii = m_distinct_width_ascii;
	m_data->m_regexp = m_regexp;
}

void CFilterDlg::OnBtnNext() 
{
	if(!::IsWindow(m_wnd->GetSafeHwnd())) {
		ShowWindow(SW_HIDE);
		return;
	}

	CWaitCursor wait_cursor;
	UpdateData(TRUE);

	if(m_search_text == _T("")) return;

	SaveSearchDlgData();
	m_data->m_dir = 1;

	m_wnd->SendMessage(m_search_message, 0, 0);

	SaveSearchTextList();

	m_combo_search_text.SetFocus();
}

void CFilterDlg::OnBtnFilter() 
{
	if(!::IsWindow(m_wnd->GetSafeHwnd())) {
		ShowWindow(SW_HIDE);
		return;
	}

	CWaitCursor wait_cursor;
	UpdateData(TRUE);

	if(m_search_text == "") return;

	SaveSearchDlgData();

	m_data->m_dir = (m_combo_action == FILTER_DELETE_MODE);

	m_wnd->SendMessage(m_filter_message, 0, 0);

	SaveSearchTextList();
	COWinApp *pApp = GetOWinApp();
	pApp->WriteIniFileInt(_T("FILTER_DLG"), _T("ACTION"), m_combo_action);
	pApp->SaveIniFile();

	m_combo_search_text.SetFocus();
}

void CFilterDlg::CheckBtn()
{
	UpdateData(TRUE);

	if(m_search_text == "") {
		m_btn_next.EnableWindow(FALSE);
	} else {
		m_btn_next.EnableWindow(TRUE);
	}
}

void CFilterDlg::ShowDialog(CWnd *wnd, int search_message, int filter_message,
	CSearchDlgData *search_data)
{
	m_wnd = wnd;
	m_search_message = search_message;
	m_filter_message = filter_message;
	m_data = search_data;
	m_initialized = TRUE;

	InitDialog();

	ShowWindow(SW_SHOWNORMAL);
}

void CFilterDlg::InitDialog()
{
	if(m_initialized == FALSE) return;

	m_search_text = m_data->m_search_text;
	m_regexp = m_data->m_regexp;
	m_distinct_lwr_upr = m_data->m_distinct_lwr_upr;
	m_distinct_width_ascii = m_data->m_distinct_width_ascii;

	LoadSearchTextList();
	if(m_search_text == "" && m_combo_search_text.GetCount() != 0) {
		m_combo_search_text.GetLBText(0, m_search_text);
		m_regexp = is_regexp(m_search_status[0]);
		m_distinct_lwr_upr = is_distinct_lwr_upr(m_search_status[0]);
		m_distinct_width_ascii = is_distinct_width_ascii(m_search_status[0]);
	}

	m_combo_action = GetOWinApp()->GetIniFileInt(_T("FILTER_DLG"), _T("ACTION"), FILTER_GREP_MODE);

	m_combo_search_text.SetFocus();

	UpdateData(FALSE);
	CheckBtn();
}

BOOL CFilterDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	InitDialog();

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CFilterDlg::LoadSearchTextList()
{
	::LoadSearchTextList(&m_combo_search_text, _T("SEARCH_TEXT"), SAVE_SEARCH_TEXT_CNT, m_search_status);
}

void CFilterDlg::SaveSearchTextList()
{
	int cur_status = make_search_status2(m_regexp, m_distinct_lwr_upr, m_distinct_width_ascii);

	::SaveSearchTextList(_T("SEARCH_TEXT"), SAVE_SEARCH_TEXT_CNT, m_search_text,
		TRUE, cur_status);

	LoadSearchTextList();
	UpdateData(FALSE);
	CheckBtn();
}

void CFilterDlg::OnEditchangeComboSearchText() 
{
	CheckBtn();
}

void CFilterDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
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

		m_combo_search_text.SetFocus();
	}
}

void CFilterDlg::OnOK() 
{
	OnBtnNext();
}

void CFilterDlg::OnSelendokComboSearchText() 
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

INT_PTR CFilterDlg::DoModal(CWnd *wnd, int search_message, int filter_message,
	CSearchDlgData *search_data)
{
	m_wnd = wnd;
	m_search_message = search_message;
	m_filter_message = filter_message;
	m_data = search_data;
	m_initialized = TRUE;

	return CDialog::DoModal();
}

BOOL CFilterDlg::UpdateData(BOOL bSaveAndValidate)
{
	UpdateDataCombo(m_combo_search_text, m_search_text, bSaveAndValidate);

	return CDialog::UpdateData(bSaveAndValidate);
}

