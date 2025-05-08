/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // ExtFileDialog.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "ExtFileDialog.h"

#include "UnicodeArchive.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExtFileDialog

IMPLEMENT_DYNAMIC(CExtFileDialog, CPlacesBarFileDlg)

CExtFileDialog::CExtFileDialog(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CPlacesBarFileDlg(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
	m_kanji_code = UnknownKanjiCode;
	m_line_type = LINE_TYPE_CR_LF;
}


BEGIN_MESSAGE_MAP(CExtFileDialog, CPlacesBarFileDlg)
	//{{AFX_MSG_MAP(CExtFileDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CExtFileDialog::OnInitDialog() 
{
	CPlacesBarFileDlg::OnInitDialog();

	if(m_b_open_mode == FALSE && m_kanji_code == UnknownKanjiCode) {
		m_kanji_code = KANJI_CODE_SJIS;
		m_line_type = LINE_TYPE_CR_LF;
	}

	CUnicodeArchive::SetKanjiCodeCombo((CComboBox *)GetDlgItem(IDC_COMBO_KANJI_CODE),
		m_kanji_code, m_b_open_mode);
	CUnicodeArchive::SetLineTypeCombo((CComboBox *)GetDlgItem(IDC_COMBO_LINE_TYPE),
		m_line_type);

	SetControlPos();

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

BOOL CExtFileDialog::OnFileNameOK()
{
	CComboBox *p_combo_kanji = (CComboBox *)GetDlgItem(IDC_COMBO_KANJI_CODE);
	if(p_combo_kanji != NULL) {
		int idx = p_combo_kanji->GetCurSel();
		if(idx != CB_ERR) {
			m_kanji_code = (int)p_combo_kanji->GetItemData(idx);
		}
	}
	CComboBox *p_combo_line = (CComboBox *)GetDlgItem(IDC_COMBO_LINE_TYPE);
	if(p_combo_line != NULL) {
		int idx = p_combo_line->GetCurSel();
		if(idx != CB_ERR) {
			m_line_type = (int)p_combo_line->GetItemData(idx);
		}
	}

	return CPlacesBarFileDlg::OnFileNameOK();
}

#include <DLGS.H>
void CExtFileDialog::SetControlPos()
{
	// ダイアログの幅を調節する
	CRect cur_rect;
	CRect parent_rect;
	GetClientRect(cur_rect);
	GetParent()->GetClientRect(parent_rect);
	SetWindowPos(NULL, 0, 0, parent_rect.Width(), cur_rect.Height(), SWP_NOMOVE | SWP_NOZORDER);

	// テキストの位置を調節
	CWnd *p_stc1 = GetParent()->GetDlgItem(stc2);
	if(p_stc1 != NULL) {
		CRect rect1;
		p_stc1->GetWindowRect(rect1);
		p_stc1->GetParent()->ScreenToClient(rect1);

		CWnd *p_static_kanji = GetDlgItem(IDC_STATIC_KANJI_CODE);
		if(p_static_kanji != NULL) {
			CRect rect2;
			p_static_kanji->GetWindowRect(rect2);
			p_static_kanji->GetParent()->ScreenToClient(rect2);
			p_static_kanji->SetWindowPos(NULL, rect1.left, rect2.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}
		CWnd *p_static_line = GetDlgItem(IDC_STATIC_LINE_TYPE);
		if(p_static_line != NULL) {
			CRect rect2;
			p_static_line->GetWindowRect(rect2);
			p_static_line->GetParent()->ScreenToClient(rect2);
			p_static_line->SetWindowPos(NULL, rect1.left, rect2.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}
	}

	// コンボボックスの位置を調節
	CWnd *p_cmb1 = GetParent()->GetDlgItem(cmb1);
	if(p_cmb1 != NULL) {
		CRect rect1;
		p_cmb1->GetWindowRect(rect1);
		p_cmb1->GetParent()->ScreenToClient(rect1);

		CWnd *p_combo_kanji = GetDlgItem(IDC_COMBO_KANJI_CODE);
		if(p_combo_kanji != NULL) {
			CRect rect2;
			p_combo_kanji->GetWindowRect(rect2);
			p_combo_kanji->GetParent()->ScreenToClient(rect2);
			p_combo_kanji->SetWindowPos(NULL, rect1.left, rect2.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}
		CWnd *p_combo_line = GetDlgItem(IDC_COMBO_LINE_TYPE);
		if(p_combo_line != NULL) {
			CRect rect2;
			p_combo_line->GetWindowRect(rect2);
			p_combo_line->GetParent()->ScreenToClient(rect2);
			p_combo_line->SetWindowPos(NULL, rect1.left, rect2.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}
	}
}

