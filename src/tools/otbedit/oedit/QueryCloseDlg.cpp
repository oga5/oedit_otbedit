/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // QueryCloseDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "oedit.h"
#include "QueryCloseDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQueryCloseDlg ダイアログ


CQueryCloseDlg::CQueryCloseDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CQueryCloseDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQueryCloseDlg)
	m_msg = _T("");
	//}}AFX_DATA_INIT
}


void CQueryCloseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQueryCloseDlg)
	DDX_Control(pDX, IDC_STATIC_ICON, m_icon);
	DDX_Text(pDX, IDC_STATIC_MSG, m_msg);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQueryCloseDlg, CDialog)
	//{{AFX_MSG_MAP(CQueryCloseDlg)
	ON_BN_CLICKED(IDNO, OnNo)
	ON_BN_CLICKED(IDALLYES, OnAllyes)
	ON_BN_CLICKED(IDALLNO, OnAllno)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQueryCloseDlg メッセージ ハンドラ

void CQueryCloseDlg::OnOK() 
{
	m_result = IDYES;

	CDialog::OnOK();
}

void CQueryCloseDlg::OnNo() 
{
	m_result = IDNO;

	CDialog::OnOK();
}

void CQueryCloseDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CQueryCloseDlg::OnAllyes() 
{
	m_result = IDALLYES;

	CDialog::OnOK();	
}

void CQueryCloseDlg::OnAllno() 
{
	m_result = IDALLNO;

	CDialog::OnOK();	
}

BOOL CQueryCloseDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: この位置に初期化の補足処理を追加してください
	m_icon.SetIcon(LoadIcon(NULL, IDI_EXCLAMATION));

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}
