/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // LineJumpDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "LineJumpDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLineJumpDlg ダイアログ


CLineJumpDlg::CLineJumpDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLineJumpDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLineJumpDlg)
	m_line_no = 0;
	//}}AFX_DATA_INIT
}


void CLineJumpDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLineJumpDlg)
	DDX_Control(pDX, IDOK, m_ok);
	DDX_Text(pDX, IDC_EDIT_LINE_NO, m_line_no);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLineJumpDlg, CDialog)
	//{{AFX_MSG_MAP(CLineJumpDlg)
	ON_EN_CHANGE(IDC_EDIT_LINE_NO, OnChangeEditLineNo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLineJumpDlg メッセージ ハンドラ

void CLineJumpDlg::OnChangeEditLineNo() 
{
	UpdateData(TRUE);

	if(m_line_no > 0) {
		m_ok.EnableWindow(TRUE);
	} else {
		m_ok.EnableWindow(FALSE);
	}
}
