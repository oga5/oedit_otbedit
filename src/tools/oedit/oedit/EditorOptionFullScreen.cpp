/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // EditorOptionFullScreen.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "oedit.h"
#include "EditorOptionFullScreen.h"
#include "PlacesBarFileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditorOptionFullScreen プロパティ ページ

IMPLEMENT_DYNCREATE(CEditorOptionFullScreen, CPropertyPage)

CEditorOptionFullScreen::CEditorOptionFullScreen() : CPropertyPage(CEditorOptionFullScreen::IDD)
{
	//{{AFX_DATA_INIT(CEditorOptionFullScreen)
	m_height = 0;
	m_left_space = 0;
	m_top_space = 0;
	m_width = 0;
	m_bg_image_file = _T("");
	//}}AFX_DATA_INIT
}

CEditorOptionFullScreen::~CEditorOptionFullScreen()
{
}

void CEditorOptionFullScreen::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditorOptionFullScreen)
	DDX_Text(pDX, IDC_EDIT_HEIGHT, m_height);
	DDX_Text(pDX, IDC_EDIT_LEFT_SPACE, m_left_space);
	DDX_Text(pDX, IDC_EDIT_TOP_SPACE, m_top_space);
	DDX_Text(pDX, IDC_EDIT_WIDTH, m_width);
	DDX_Text(pDX, IDC_EDIT_BG_IMAGE, m_bg_image_file);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditorOptionFullScreen, CPropertyPage)
	//{{AFX_MSG_MAP(CEditorOptionFullScreen)
	ON_BN_CLICKED(IDC_BTN_BG_IMAGE, OnBtnBgImage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditorOptionFullScreen メッセージ ハンドラ

void CEditorOptionFullScreen::OnBtnBgImage() 
{
	CPlacesBarFileDlg		dlg(TRUE, NULL, NULL,
		OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
		_T("Image Files (*.jpg;*.jpeg)|*.jpg; *.jpeg|All Files (*.*)|*.*||"), this);

	if(dlg.DoModal() == IDOK) {
		UpdateData(TRUE);
		m_bg_image_file = dlg.GetPathName();
		UpdateData(FALSE);
	}	
}
