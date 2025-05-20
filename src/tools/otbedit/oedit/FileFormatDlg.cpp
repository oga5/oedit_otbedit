/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

// FileFormatDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "FileFormatDlg.h"
#include "UnicodeArchive.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static const char *THIS_FILE = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileFormatDlg ダイアログ


CFileFormatDlg::CFileFormatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFileFormatDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFileFormatDlg)
	//}}AFX_DATA_INIT
}


void CFileFormatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileFormatDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFileFormatDlg, CDialog)
	//{{AFX_MSG_MAP(CFileFormatDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileFormatDlg メッセージ ハンドラ


void CFileFormatDlg::InitData(int kanji_code, int line_type)
{
	m_kanji_code = kanji_code;
	m_line_type = line_type;
}

void CFileFormatDlg::GetData(int &kanji_code, int &line_type)
{
	kanji_code = m_kanji_code;
	line_type = m_line_type;
}

BOOL CFileFormatDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CUnicodeArchive::SetKanjiCodeCombo((CComboBox *)GetDlgItem(IDC_COMBO_KANJI_CODE),
		m_kanji_code, FALSE);
	CUnicodeArchive::SetLineTypeCombo((CComboBox *)GetDlgItem(IDC_COMBO_LINE_TYPE),
		m_line_type);

	return TRUE;
}

void CFileFormatDlg::OnOK() 
{
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
	
	CDialog::OnOK();
}
