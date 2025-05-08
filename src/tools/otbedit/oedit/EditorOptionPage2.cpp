/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // EditorOptionPage2.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "oedit.h"
#include "EditorOptionPage2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditorOptionPage2 プロパティ ページ

IMPLEMENT_DYNCREATE(CEditorOptionPage2, CPropertyPage)

CEditorOptionPage2::CEditorOptionPage2() : CPropertyPage(CEditorOptionPage2::IDD)
{
	//{{AFX_DATA_INIT(CEditorOptionPage2)
	m_drag_drop_edit = FALSE;
	m_tab_as_space = FALSE;
	m_indent_mode = -1;
	m_mouse_select_copy = FALSE;
	m_check_row_copy_at_no_sel = FALSE;
	m_use_keyword_window = FALSE;
	m_enable_confinement = FALSE;
	m_enable_word_wrap = FALSE;
	m_first_confinement_str = _T("");
	m_last_confinement_str = _T("");
	m_text_drop_edit = FALSE;
	m_brackets_blink_time = 0;
	m_clear_after_copy = FALSE;
	//}}AFX_DATA_INIT
}

CEditorOptionPage2::~CEditorOptionPage2()
{
}

void CEditorOptionPage2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditorOptionPage2)
	DDX_Control(pDX, IDC_CHECK_TEXT_DROP_EDIT, m_check_text_drop_edit);
	DDX_Control(pDX, IDC_EDIT_LAST_CONFINEMENT_STR, m_edit_last_confinement_str);
	DDX_Control(pDX, IDC_EDIT_FIRST_CONFINEMENT_STR, m_edit_first_confinement_str);
	DDX_Check(pDX, IDC_CHECK_DRAG_DROP_EDIT, m_drag_drop_edit);
	DDX_Check(pDX, IDC_CHECK_TAB_AS_SPACE, m_tab_as_space);
	DDX_CBIndex(pDX, IDC_COMBO_INDENT, m_indent_mode);
	DDX_Check(pDX, IDC_CHECK_MOUSE_SELECT_COPY, m_mouse_select_copy);
	DDX_Check(pDX, IDC_CHECK_ROW_COPY_AT_NO_SEL, m_check_row_copy_at_no_sel);
	DDX_Check(pDX, IDC_CHECK_KEYWORD_WINDOW, m_use_keyword_window);
	DDX_Check(pDX, IDC_CHECK_ENABLE_CONFINEMENT, m_enable_confinement);
	DDX_Check(pDX, IDC_CHECK_ENABLE_WORD_WRAP, m_enable_word_wrap);
	DDX_Text(pDX, IDC_EDIT_FIRST_CONFINEMENT_STR, m_first_confinement_str);
	DDX_Text(pDX, IDC_EDIT_LAST_CONFINEMENT_STR, m_last_confinement_str);
	DDX_Check(pDX, IDC_CHECK_TEXT_DROP_EDIT, m_text_drop_edit);
	DDX_Text(pDX, IDC_EDIT_BRACKETS_BLINK_TIME, m_brackets_blink_time);
	DDV_MinMaxInt(pDX, m_brackets_blink_time, 0, 5000);
	DDX_Check(pDX, IDC_CHECK_CLEAR_AFTER_COPY, m_clear_after_copy);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditorOptionPage2, CPropertyPage)
	//{{AFX_MSG_MAP(CEditorOptionPage2)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_CONFINEMENT, OnCheckEnableConfinement)
	ON_BN_CLICKED(IDC_CHECK_DRAG_DROP_EDIT, OnCheckDragDropEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditorOptionPage2 メッセージ ハンドラ

void CEditorOptionPage2::OnOK() 
{
	// TODO: この位置にその他の検証用のコードを追加してください
	UpdateData(TRUE);
	
	CPropertyPage::OnOK();
}

void CEditorOptionPage2::OnCheckEnableConfinement() 
{
	CheckConfinementCtrl();
}

BOOL CEditorOptionPage2::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	UpdateData(FALSE);

	CheckConfinementCtrl();
	CheckTextDrop();

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CEditorOptionPage2::CheckConfinementCtrl()
{
	UpdateData(TRUE);
	m_edit_first_confinement_str.EnableWindow(m_enable_confinement);
	m_edit_last_confinement_str.EnableWindow(m_enable_confinement);
}

void CEditorOptionPage2::OnCheckDragDropEdit() 
{
	CheckTextDrop();
}

void CEditorOptionPage2::CheckTextDrop()
{
	UpdateData(TRUE);
	m_check_text_drop_edit.EnableWindow(!m_drag_drop_edit);
}

