/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // SearchDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "SearchDlg.h"
#include "OWinApp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SAVE_SEARCH_TEXT_CNT	20
#define SEARCH_DLG_MIN_WIDTH	450

#define SEARCH_STATUS_REGEXP			(0x01 << 0)
#define SEARCH_STATUS_DISTINCT_LWR_UPR	(0x01 << 1)
#define SEARCH_STATUS_DISTINCT_WIDTH_ASCII	(0x01 << 2)

CSearchDlgData::CSearchDlgData()
{
	m_search_text = _T("");
	m_replace_text = _T("");
	m_dir = 1;
	m_distinct_lwr_upr = FALSE;
	m_distinct_width_ascii = FALSE;
	m_regexp = FALSE;
	m_all = FALSE;
	m_replace_selected_area = FALSE;
};

static int FindStringCombo(CComboBox &combo, CString &text)
{
	int		i;
	CString data;
	
	for(i = 0;; i++) {
		if(combo.GetLBTextLen(i) == CB_ERR) return CB_ERR;
		combo.GetLBText(i, data);
		if(data == text) return i;
	}
}

void UpdateDataCombo(CComboBox &combo, CString &text, BOOL bSaveAndValidate)
{
	if(bSaveAndValidate) {
		int idx = combo.GetCurSel();
		if(idx != CB_ERR) {
			combo.GetLBText(idx, text);
		} else {
			combo.GetWindowText(text);
		}
	} else {
		int idx = FindStringCombo(combo, text);
		if(idx != CB_ERR) {
			combo.SetCurSel(idx);
		} else {
			if(text != _T("")) {
				combo.InsertString(0, text);
				combo.SetCurSel(0);
			}
		}
	}
}

void LoadSearchTextList(CComboBox *combo, CString section, int max_cnt, int *status_arr)
{
	int			i, j;
	CString		entry;
	CString		text;
	COWinApp	*pApp = GetOWinApp();

	// コンボボックスをクリア
	for(; combo->GetCount() != 0;) {
		combo->DeleteString(0);
	}

	// レジストリのテキストを取得
	for(i = 0, j = 0; i < max_cnt; i++) {
		// テキストを取得
		entry.Format(_T("%d"), i);
		text = pApp->GetIniFileString(section, entry, _T("\f_NO_DATA_\n"));
		if(text == _T("\f_NO_DATA_\n")) break;

		if(status_arr != NULL) {
			entry.Format(_T("status_%d"), i);
			status_arr[j] = pApp->GetIniFileInt(section, entry, 0);
		}

		combo->InsertString(j, text);
		j++;
	}
}

void SaveSearchTextList(CString section, int max_cnt, CString cur_text,
	BOOL b_save_status, int cur_status)
{
	CString		entry;

	COWinApp	*pApp = GetOWinApp();
	int			i, j;
	CString		*search_text = NULL;
	int			*status = NULL;

	search_text = new CString[max_cnt];
	status = new int[max_cnt];
	if(search_text == NULL || status == NULL) {
		goto EXIT;
	}

	// 現在のレジストリのテキストを取得
	for(i = 0 ; i < max_cnt; i++) {
		// テキストを取得
		entry.Format(_T("%d"), i);
		search_text[i] = pApp->GetIniFileString(section, entry, _T(""));

		if(b_save_status) {
			entry.Format(_T("status_%d"), i);
			status[i] = pApp->GetIniFileInt(section, entry, 0);
		}

		// 変更なし
		if(i == 0 && search_text[0] == cur_text && status[0] == cur_status) goto EXIT;
	}

	// 現在のレジストリのテキストを後ろにずらす
	for(i = 0, j = 1; i < max_cnt && j < max_cnt; i++) {
		if(search_text[i] == _T("") || search_text[i] == cur_text) continue;

		// テキストを保存
		entry.Format(_T("%d"), j);
		pApp->WriteIniFileString(section, entry, search_text[i]);

		if(b_save_status) {
			entry.Format(_T("status_%d"), j);
			pApp->WriteIniFileInt(section, entry, status[i]);
		}

		j++;
	}

	// 今回使用したテキストを先頭に保存
	entry.Format(_T("%d"), 0);
	pApp->WriteIniFileString(section, entry, cur_text);

	if(b_save_status) {
		entry.Format(_T("status_%d"), 0);
		pApp->WriteIniFileInt(section, entry, cur_status);
	}

	// 残りを空白にする
	for(; j < SAVE_SEARCH_TEXT_CNT; j++) {
		// キーを作成
		entry.Format(_T("%d"), j);
		// テキストを保存
		pApp->WriteIniFileString(section, entry, NULL);

		if(b_save_status) {
			entry.Format(_T("status_%d"), j);
			pApp->WriteIniFileInt(section, entry, 0);
		}
	}

	pApp->SaveIniFile();

EXIT:
 	if(search_text != NULL) delete[] search_text;
	if(status != NULL) delete[] status;
}

int make_search_status2(BOOL b_regexp, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii)
{
	int		status = 0;
	if(b_regexp) status |= SEARCH_STATUS_REGEXP;
	if(b_distinct_lwr_upr) status |= SEARCH_STATUS_DISTINCT_LWR_UPR;
	if(b_distinct_width_ascii) status |= SEARCH_STATUS_DISTINCT_WIDTH_ASCII;

	return status;
}
/*
int make_search_status(BOOL b_regexp, BOOL b_distinct_lwr_upr)
{
int		status = 0;
if(b_regexp) status |= SEARCH_STATUS_REGEXP;
if(b_distinct_lwr_upr) status |= SEARCH_STATUS_DISTINCT_LWR_UPR;

return status;
}
*/

BOOL is_regexp(int status)
{
	return (status & SEARCH_STATUS_REGEXP) ? TRUE : FALSE;
}

BOOL is_distinct_lwr_upr(int status)
{
	return (status & SEARCH_STATUS_DISTINCT_LWR_UPR) ? TRUE : FALSE;
}

BOOL is_distinct_width_ascii(int status)
{
	return (status & SEARCH_STATUS_DISTINCT_WIDTH_ASCII) ? TRUE : FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CSearchDlg ダイアログ


CSearchDlg::CSearchDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSearchDlg::IDD, pParent)
	, m_distinct_width_ascii(FALSE)
{
	//{{AFX_DATA_INIT(CSearchDlg)
	m_distinct_lwr_upr = FALSE;
	m_distinct_width_ascii = FALSE;
	m_regexp = FALSE;
	//}}AFX_DATA_INIT

	m_initialized = FALSE;
	m_close_at_found = FALSE;

	m_search_status = new int[SAVE_SEARCH_TEXT_CNT];
	if(m_search_status != NULL) {
		int		i;
		for(i = 0; i < SAVE_SEARCH_TEXT_CNT; i++) {
			m_search_status[i] = 0;
		}
	}

	m_title.Empty();
}

CSearchDlg::~CSearchDlg()
{
	if(m_search_status != NULL) {
		delete[] m_search_status;
		m_search_status = NULL;
	}
}

void CSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSearchDlg)
	DDX_Control(pDX, IDC_CHECK_REGEXP, m_btn_check_regexp);
	DDX_Control(pDX, IDCANCEL, m_btn_cancel);
	DDX_Control(pDX, IDC_COMBO_SEARCH_TEXT, m_combo_search_text);
	DDX_Control(pDX, ID_BTN_PREV, m_btn_prev);
	DDX_Control(pDX, ID_BTN_NEXT, m_btn_next);
	DDX_Check(pDX, IDC_CHECK_DISTINCT_LWR_UPR, m_distinct_lwr_upr);
	DDX_Check(pDX, IDC_CHECK_REGEXP, m_regexp);
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_CHECK_DISTINCT_WIDTH_ASCII, m_distinct_width_ascii);
	DDX_Control(pDX, IDC_BTN_BOTTOM, m_btn_bottom);
}


BEGIN_MESSAGE_MAP(CSearchDlg, CDialog)
	//{{AFX_MSG_MAP(CSearchDlg)
	ON_BN_CLICKED(ID_BTN_NEXT, OnBtnNext)
	ON_BN_CLICKED(ID_BTN_PREV, OnBtnPrev)
	ON_CBN_EDITCHANGE(IDC_COMBO_SEARCH_TEXT, OnEditchangeComboSearchText)
	ON_WM_SHOWWINDOW()
	ON_CBN_SELENDOK(IDC_COMBO_SEARCH_TEXT, OnSelendokComboSearchText)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSearchDlg メッセージ ハンドラ

void CSearchDlg::SaveSearchDlgData()
{
	UpdateData(TRUE);

	m_data->m_search_text = m_search_text;
	m_data->m_distinct_lwr_upr = m_distinct_lwr_upr;
	m_data->m_distinct_width_ascii = m_distinct_width_ascii;
	m_data->m_regexp = m_regexp;
}

void CSearchDlg::DoSearch(int dir)
{
	if(!::IsWindow(m_wnd->GetSafeHwnd())) {
		ShowWindow(SW_HIDE);
		return;
	}

	CWaitCursor wait_cursor;
	UpdateData(TRUE);

	if(m_search_text == _T("")) return;

	SaveSearchDlgData();
	m_data->m_dir = dir;

	LRESULT ret_v = m_wnd->SendMessage(m_search_message, 0, 0);

	SaveSearchTextList();

	m_combo_search_text.SetFocus();

	if(m_close_at_found && ret_v == 0) OnCancel();
}

void CSearchDlg::OnBtnNext() 
{
	DoSearch(1);
}

void CSearchDlg::OnBtnPrev() 
{
	DoSearch(-1);
}

void CSearchDlg::CheckBtn()
{
	UpdateData(TRUE);

	if(m_search_text == _T("")) {
		m_btn_next.EnableWindow(FALSE);
		m_btn_prev.EnableWindow(FALSE);
	} else {
		m_btn_next.EnableWindow(TRUE);
		m_btn_prev.EnableWindow(TRUE);
	}
}

void CSearchDlg::ShowDialog(CWnd *wnd, int search_message, CSearchDlgData *search_data, BOOL b_close_at_found)
{
	m_wnd = wnd;
	m_search_message = search_message;
	m_data = search_data;
	m_initialized = TRUE;
	m_close_at_found = b_close_at_found;

	InitDialog();

	ShowWindow(SW_SHOWNORMAL);
}

void CSearchDlg::InitDialog()
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
	m_regexp = m_data->m_regexp;
	m_distinct_lwr_upr = m_data->m_distinct_lwr_upr;
	m_distinct_width_ascii = m_data->m_distinct_width_ascii;

	LoadSearchTextList();
	if(m_search_text == _T("") && m_combo_search_text.GetCount() != 0) {
		m_combo_search_text.GetLBText(0, m_search_text);
	}
	m_regexp = is_regexp(m_search_status[0]);
	m_distinct_lwr_upr = is_distinct_lwr_upr(m_search_status[0]);
	m_distinct_width_ascii = is_distinct_width_ascii(m_search_status[0]);

	m_combo_search_text.SetFocus();

	UpdateData(FALSE);
	CheckBtn();

	LoadDlgSize();
}

BOOL CSearchDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	InitDialog();

	if(!m_title.IsEmpty()) {
		SetWindowText(m_title);
	}

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CSearchDlg::LoadSearchTextList()
{
	::LoadSearchTextList(&m_combo_search_text, _T("SEARCH_TEXT"), SAVE_SEARCH_TEXT_CNT, m_search_status);
}

void CSearchDlg::SaveSearchTextList()
{
	int cur_status = make_search_status2(m_regexp, m_distinct_lwr_upr, m_distinct_width_ascii);

	::SaveSearchTextList(_T("SEARCH_TEXT"), SAVE_SEARCH_TEXT_CNT, m_search_text,
		TRUE, cur_status);

	LoadSearchTextList();
	UpdateData(FALSE);
	CheckBtn();
}

void CSearchDlg::OnEditchangeComboSearchText() 
{
	CheckBtn();
}

void CSearchDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
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

void CSearchDlg::OnOK() 
{
	OnBtnNext();
}

void CSearchDlg::OnSelendokComboSearchText() 
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

INT_PTR CSearchDlg::DoModal(CWnd *wnd, int search_message, CSearchDlgData *search_data, BOOL b_close_at_found)
{
	m_wnd = wnd;
	m_search_message = search_message;
	m_data = search_data;
	m_initialized = TRUE;
	m_close_at_found = b_close_at_found;

	return CDialog::DoModal();
}

CString MakeSearchMsg(int ret_v, int dir, BOOL b_looped)
{
	CString msg;
	if(ret_v != 0) {
		MessageBeep(MB_ICONEXCLAMATION);
		msg = _T("見つかりません");
	} else if(b_looped) {
		if(dir == 1) {
			msg = _T("ファイルの末尾まで検索しました");
		} else {
			msg = _T("ファイルの先頭まで検索しました");
		}
	} else {
		msg = _T("");
	}
	return msg;
}

BOOL CSearchDlg::UpdateData(BOOL bSaveAndValidate)
{
	UpdateDataCombo(m_combo_search_text, m_search_text, bSaveAndValidate);

	return CDialog::UpdateData(bSaveAndValidate);
}

void CSearchDlg::SetSearchText(CString search_text)
{
	m_search_text = search_text;
	UpdateData(FALSE);
}

void CSearchDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	RelayoutControls();
}

void CSearchDlg::OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI )
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

void CSearchDlg::RelayoutControls()
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

	m_btn_next.GetWindowRect(btn_rect);
	AdjustComboWidth(&m_combo_search_text, &win_rect, &btn_rect);
}

void CSearchDlg::OnCancel() 
{
	SaveDlgSize();
	
	CDialog::OnCancel();
}

void CSearchDlg::LoadDlgSize()
{
	COWinApp	*pApp = GetOWinApp();

	int w = pApp->GetIniFileInt(_T("SEARCH_DLG"), _T("DLG_WIDTH"), 0);
	if(w < SEARCH_DLG_MIN_WIDTH) w = SEARCH_DLG_MIN_WIDTH;

	SetWindowPos(NULL, 0, 0, w, m_dlg_height, SWP_NOMOVE | SWP_NOZORDER);
}

void CSearchDlg::SaveDlgSize()
{
	COWinApp	*pApp = GetOWinApp();
	int w = pApp->GetIniFileInt(_T("SEARCH_DLG"), _T("DLG_WIDTH"), 0);

	CRect	win_rect;
	GetWindowRect(win_rect);

	if(w == win_rect.Width()) return;

	pApp->WriteIniFileInt(_T("SEARCH_DLG"), _T("DLG_WIDTH"), win_rect.Width());
	pApp->SaveIniFile();
}

