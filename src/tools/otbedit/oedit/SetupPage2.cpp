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
#include "EditorOptionPage.h"
#include "ColorUtil.h"

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
	m_disable_double_boot = FALSE;
	m_enable_tag_jump = FALSE;
	m_save_modified = FALSE;
	m_search_loop_msg = FALSE;
	m_tab_multi_row = FALSE;
	m_window_title_is_path_name = FALSE;
	m_tab_title_is_path_name = FALSE;
	m_max_mru = 0;
	m_boot_on_ime = FALSE;
	m_tab_close_at_mclick = FALSE;
	m_hide_tab_bar_at_one = FALSE;
	m_search_dlg_close_at_found = FALSE;
	m_tab_drag_move = FALSE;
	m_close_btn_on_tab = FALSE;
	m_tab_create_at_dblclick = FALSE;
	m_show_tab_tooltip = FALSE;
	//}}AFX_DATA_INIT
}

CSetupPage2::~CSetupPage2()
{
}

void CSetupPage2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetupPage2)
	DDX_Check(pDX, IDC_CHECK_DISABLE_DOUBLE_BOOT, m_disable_double_boot);
	DDX_Check(pDX, IDC_CHECK_ENABLE_TAG_JUMP, m_enable_tag_jump);
	DDX_Check(pDX, IDC_CHECK_SAVE_MODIFIED, m_save_modified);
	DDX_Check(pDX, IDC_CHECK_SEARCH_LOOP_MSG, m_search_loop_msg);
	DDX_Check(pDX, IDC_CHECK_TAB_MULTI_ROW, m_tab_multi_row);
	DDX_Check(pDX, IDC_CHECK_WINDOW_TITLE_IS_PATH_NAME, m_window_title_is_path_name);
	DDX_Check(pDX, IDC_CHECK_TAB_TITLE_IS_PATH_NAME, m_tab_title_is_path_name);
	DDX_Text(pDX, IDC_EDIT_MAX_MRU, m_max_mru);
	DDX_Check(pDX, IDC_CHECK_BOOT_ON_IME, m_boot_on_ime);
	DDX_Check(pDX, IDC_CHECK_TAB_CLOSE_AT_MCLICK, m_tab_close_at_mclick);
	DDX_Check(pDX, IDC_CHECK_HIDE_TAB_BAR_AT_ONE, m_hide_tab_bar_at_one);
	DDX_Check(pDX, IDC_CHECK_SEARCH_DLG_CLOSE_AT_FOUND, m_search_dlg_close_at_found);
	DDX_Check(pDX, IDC_CHECK_TAB_DRAG_MOVE, m_tab_drag_move);
	DDX_Check(pDX, IDC_CHECK_CLOSE_BTN_ON_TAB, m_close_btn_on_tab);
	DDX_Check(pDX, IDC_CHECK_TAB_CREATE_AT_DBLCLICK, m_tab_create_at_dblclick);
	DDX_Check(pDX, IDC_CHECK_SHOW_TAB_TOOLTIP, m_show_tab_tooltip);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetupPage2, CPropertyPage)
	//{{AFX_MSG_MAP(CSetupPage2)
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_BTN_BK_COLOR, OnBtnBkColor)
	ON_BN_CLICKED(IDC_BTN_BK_COLOR2, OnBtnBkColor2)
	ON_BN_CLICKED(IDC_BTN_TEXT_COLOR, OnBtnTextColor)
	ON_BN_CLICKED(IDC_BTN_DEFAULT_COLOR, OnBtnDefaultColor)
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

void CSetupPage2::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	switch(nIDCtl) {
	case IDC_BTN_TEXT_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[FILETAB_TEXT_COLOR]);
		break;
	case IDC_BTN_BK_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[FILETAB_ACTIVE_BK_COLOR]);
		break;
	case IDC_BTN_BK_COLOR2:
		DrawColorBtn(lpDrawItemStruct, m_color[FILETAB_NO_ACTIVE_BK_COLOR]);
		break;
	}
	
	CPropertyPage::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CSetupPage2::OnBtnBkColor() 
{
	ChooseColor(FILETAB_ACTIVE_BK_COLOR);
}

void CSetupPage2::OnBtnBkColor2() 
{
	ChooseColor(FILETAB_NO_ACTIVE_BK_COLOR);
}

void CSetupPage2::OnBtnTextColor() 
{
	ChooseColor(FILETAB_TEXT_COLOR);
}

void CSetupPage2::OnBtnDefaultColor() 
{
	m_color[FILETAB_TEXT_COLOR] = ::GetSysColor(COLOR_WINDOWTEXT);
	m_color[FILETAB_ACTIVE_BK_COLOR] = ::GetSysColor(COLOR_BTNFACE);
	m_color[FILETAB_NO_ACTIVE_BK_COLOR] = make_dark_color(m_color[FILETAB_ACTIVE_BK_COLOR], 0.85);
	Invalidate();
}

void CSetupPage2::DrawColorBtn(LPDRAWITEMSTRUCT lpDrawItemStruct, COLORREF clr)
{
	CRect rect(lpDrawItemStruct->rcItem);

	::DrawEdge(lpDrawItemStruct->hDC, rect, EDGE_BUMP, BF_RECT);

	rect.DeflateRect(2, 2);
	::SetBkColor(lpDrawItemStruct->hDC, clr);
	::ExtTextOut(lpDrawItemStruct->hDC, 0, 0, ETO_OPAQUE, 
		rect, NULL, 0, NULL);
}

void CSetupPage2::ChooseColor(int color_id)
{
	CColorDialog	dlg(m_color[color_id]);

	if(dlg.DoModal() != IDOK) return;

	m_color[color_id] = dlg.GetColor();
	Invalidate();
}

