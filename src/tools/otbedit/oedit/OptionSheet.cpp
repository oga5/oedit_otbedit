/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // OptionSheet.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "oedit.h"
#include "OptionSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static const char *THIS_FILE = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionSheet

IMPLEMENT_DYNAMIC(COptionSheet, CPropertySheet)

COptionSheet::COptionSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

COptionSheet::COptionSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

COptionSheet::~COptionSheet()
{
}


BEGIN_MESSAGE_MAP(COptionSheet, CPropertySheet)
	//{{AFX_MSG_MAP(COptionSheet)
		// メモ - ClassWizard はこの位置にマッピング用のマクロを追加または削除します。
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionSheet メッセージ ハンドラ

INT_PTR COptionSheet::DoModal() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
	m_psh.dwFlags |= PSH_NOAPPLYNOW;	// 適用ボタンを外す

	AddPage(&m_editor_page);
	AddPage(&m_editor_page2);
	AddPage(&m_setup_page);
	AddPage(&m_setup_page2);
	
	return CPropertySheet::DoModal();
}
