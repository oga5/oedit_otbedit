/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_EXTFILEDIALOG_H__832406C7_254C_4397_9783_90435DBFEC90__INCLUDED_)
#define AFX_EXTFILEDIALOG_H__832406C7_254C_4397_9783_90435DBFEC90__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExtFileDialog.h : ヘッダー ファイル
//

#include "PlacesBarFileDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CExtFileDialog ダイアログ

class CExtFileDialog : public CPlacesBarFileDlg
{
	DECLARE_DYNAMIC(CExtFileDialog)

public:
	CExtFileDialog(BOOL bOpenFileDialog, // TRUE ならば FileOpen、 FALSE ならば FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

	int m_kanji_code;
	int m_line_type;
	BOOL m_b_open_mode;

protected:
	//{{AFX_MSG(CExtFileDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	BOOL OnFileNameOK();

	void SetControlPos();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_EXTFILEDIALOG_H__832406C7_254C_4397_9783_90435DBFEC90__INCLUDED_)
