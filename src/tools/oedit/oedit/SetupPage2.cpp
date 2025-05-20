/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // SetupPage2.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "oedit.h"
#include "SetupPage2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetupPage2 プロパティ ページ

IMPLEMENT_DYNCREATE(CSetupPage2, CPropertyPage)

CSetupPage2::CSetupPage2() : CPropertyPage(CSetupPage2::IDD)
{
	//{{AFX_DATA_INIT(CSetupPage2)
	m_enable_tag_jump = FALSE;
	m_save_modified = FALSE;
	m_search_loop_msg = FALSE;
	m_window_title_is_path_name = FALSE;
	m_max_mru = 0;
	m_boot_on_ime = FALSE;
	m_search_dlg_close_at_found = FALSE;
	//}}AFX_DATA_INIT
}

CSetupPage2::~CSetupPage2()
{
}

void CSetupPage2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetupPage2)
	DDX_Check(pDX, IDC_CHECK_ENABLE_TAG_JUMP, m_enable_tag_jump);
	DDX_Check(pDX, IDC_CHECK_SAVE_MODIFIED, m_save_modified);
	DDX_Check(pDX, IDC_CHECK_SEARCH_LOOP_MSG, m_search_loop_msg);
	DDX_Check(pDX, IDC_CHECK_WINDOW_TITLE_IS_PATH_NAME, m_window_title_is_path_name);
	DDX_Text(pDX, IDC_EDIT_MAX_MRU, m_max_mru);
	DDX_Check(pDX, IDC_CHECK_BOOT_ON_IME, m_boot_on_ime);
	DDX_Check(pDX, IDC_CHECK_SEARCH_DLG_CLOSE_AT_FOUND, m_search_dlg_close_at_found);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetupPage2, CPropertyPage)
	//{{AFX_MSG_MAP(CSetupPage2)
		// メモ: ClassWizard はこの位置に DDX および DDV の呼び出しコードを追加します。
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetupPage2 メッセージ ハンドラ

BOOL CSetupPage2::OnApply() 
{
	UpdateData(TRUE);
	if(m_max_mru > 16) m_max_mru = 16;
	if(m_max_mru < 0) m_max_mru = 0;
	
	return CPropertyPage::OnApply();
}
