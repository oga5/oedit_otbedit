/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_FILEFORMATDLG_H__649BBE58_716D_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_FILEFORMATDLG_H__649BBE58_716D_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileFormatDlg.h : ヘッダー ファイル
//
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CFileFormatDlg ダイアログ

class CFileFormatDlg : public CDialog
{
// コンストラクション
public:
	CFileFormatDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ

	void InitData(int kanji_code, int line_type);
	void GetData(int &kanji_code, int &line_type);

// ダイアログ データ
	//{{AFX_DATA(CFileFormatDlg)
	enum { IDD = IDD_FILE_FORMAT_DLG };
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CFileFormatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CFileFormatDlg)
		virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	int		m_kanji_code;
	int		m_line_type;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_FILEFORMATDLG_H__649BBE58_716D_11D5_8505_00E018A83B1B__INCLUDED_)
