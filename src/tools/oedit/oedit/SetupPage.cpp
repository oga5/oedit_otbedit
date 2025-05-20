/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // SetupPage.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "oedit.h"
#include "SetupPage.h"

#include "fileutil.h"
#include "PlacesBarFileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static const char *THIS_FILE = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetupPage プロパティ ページ

IMPLEMENT_DYNCREATE(CSetupPage, CPropertyPage)

CSetupPage::CSetupPage() : CPropertyPage(CSetupPage::IDD)
{
	//{{AFX_DATA_INIT(CSetupPage)
	m_initial_dir = _T("");
	m_browser_01 = _T("");
	m_browser_02 = _T("");
	m_browser_03 = _T("");
	m_ogrep_search_dir_as_current_dir = FALSE;
	//}}AFX_DATA_INIT

	m_file_open_kanji_code = 0;
	m_kanji_code = 0;
	m_line_type = 0;
}

CSetupPage::~CSetupPage()
{
}

void CSetupPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetupPage)
	DDX_Text(pDX, IDC_EDIT_INITIAL_DIR, m_initial_dir);
	DDX_Text(pDX, IDC_EDIT_BROWSER_01, m_browser_01);
	DDX_Text(pDX, IDC_EDIT_BROWSER_02, m_browser_02);
	DDX_Text(pDX, IDC_EDIT_BROWSER_03, m_browser_03);
	DDX_Check(pDX, IDC_CHECK_OGREP_DIR_AS_CURRENT_DIR, m_ogrep_search_dir_as_current_dir);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetupPage, CPropertyPage)
	//{{AFX_MSG_MAP(CSetupPage)
	ON_BN_CLICKED(IDC_BTN_INITIAL_DIR, OnBtnInitialDir)
	ON_BN_CLICKED(IDC_BTN_BROWSER_01, OnBtnBrowser01)
	ON_BN_CLICKED(IDC_BTN_BROWSER_02, OnBtnBrowser02)
	ON_BN_CLICKED(IDC_BTN_BROWSER_03, OnBtnBrowser03)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetupPage メッセージ ハンドラ

void CSetupPage::OnBtnInitialDir() 
{
	TCHAR	dir[MAX_PATH];
	TCHAR	*p;

	// ディレクトリ名の最後の \をはずす
	UpdateData(TRUE);
	p = m_initial_dir.GetBuffer(MAX_PATH);
	make_dirname2(p);
	m_initial_dir.ReleaseBuffer();

	if(SelectFolder(GetSafeHwnd(), dir, m_initial_dir.GetBuffer(0),
		_T("起動時のディレクトリの選択")) == TRUE) {
		m_initial_dir = dir;
		UpdateData(FALSE);
	}
}

BOOL CSetupPage::OnApply() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
	UpdateData(TRUE);

	CComboBox *p_combo_kanji = (CComboBox *)GetDlgItem(IDC_COMBO_KANJI_CODE);
	if(p_combo_kanji != NULL) {
		int idx = p_combo_kanji->GetCurSel();
		if(idx != CB_ERR) {
            m_kanji_code = static_cast<int>(p_combo_kanji->GetItemData(idx));
		}
	}

	CComboBox *p_combo_line = (CComboBox *)GetDlgItem(IDC_COMBO_LINE_TYPE);
	if(p_combo_line != NULL) {
		int idx = p_combo_line->GetCurSel();
		if(idx != CB_ERR) {
            m_line_type = static_cast<int>(p_combo_line->GetItemData(idx));
		}
	}

	CComboBox *p_fo_combo_kanji = (CComboBox *)GetDlgItem(IDC_COMBO_FILE_OPEN_KANJI_CODE);
	if(p_fo_combo_kanji != NULL) {
		int idx = p_fo_combo_kanji->GetCurSel();
		if(idx != CB_ERR) {
            m_file_open_kanji_code = static_cast<int>(p_fo_combo_kanji->GetItemData(idx));
		}
	}

	CComboBox *p_combo_edit_mode = (CComboBox *)GetDlgItem(IDC_COMBO_EDIT_MODE);
	if(p_combo_edit_mode != NULL) {
		m_edit_mode = "NONE";
		int idx = p_combo_edit_mode->GetCurSel();
		if(idx != CB_ERR && idx > 0) {
			p_combo_edit_mode->GetLBText(idx, m_edit_mode);
		}
	}

	return CPropertyPage::OnApply();
}

void CSetupPage::OnBtnBrowser01() 
{
	CPlacesBarFileDlg		dlg(TRUE, NULL, NULL,
		OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
		NULL, this);

	dlg.m_ofn.lpstrInitialDir = m_initial_dir.GetBuffer(0);

	if(dlg.DoModal() == IDOK) {
		UpdateData(TRUE);
		m_browser_01 = dlg.GetPathName();
		UpdateData(FALSE);
	}
}

void CSetupPage::OnBtnBrowser03() 
{
	CPlacesBarFileDlg		dlg(TRUE, NULL, NULL,
		OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
		NULL, this);

	dlg.m_ofn.lpstrInitialDir = m_initial_dir.GetBuffer(0);

	if(dlg.DoModal() == IDOK) {
		UpdateData(TRUE);
		m_browser_03 = dlg.GetPathName();
		UpdateData(FALSE);
	}
}

void CSetupPage::OnBtnBrowser02() 
{
	CPlacesBarFileDlg		dlg(TRUE, NULL, NULL,
		OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
		NULL, this);

	dlg.m_ofn.lpstrInitialDir = m_initial_dir.GetBuffer(0);

	if(dlg.DoModal() == IDOK) {
		UpdateData(TRUE);
		m_browser_02 = dlg.GetPathName();
		UpdateData(FALSE);
	}
}

BOOL CSetupPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	CUnicodeArchive::SetKanjiCodeCombo((CComboBox *)GetDlgItem(IDC_COMBO_KANJI_CODE),
		m_kanji_code, FALSE);
	CUnicodeArchive::SetLineTypeCombo((CComboBox *)GetDlgItem(IDC_COMBO_LINE_TYPE),
		m_line_type);
	CUnicodeArchive::SetKanjiCodeCombo((CComboBox *)GetDlgItem(IDC_COMBO_FILE_OPEN_KANJI_CODE),
		m_file_open_kanji_code, TRUE);

	CComboBox *p_combo_edit_mode = (CComboBox *)GetDlgItem(IDC_COMBO_EDIT_MODE);
	g_lang_setting_list.AddComboEditModeForSetup(p_combo_edit_mode, m_edit_mode);

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

