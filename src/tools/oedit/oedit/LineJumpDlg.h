/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_LINEJUMPDLG_H__753F4441_A2F7_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_LINEJUMPDLG_H__753F4441_A2F7_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LineJumpDlg.h : ヘッダー ファイル
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CLineJumpDlg ダイアログ

class CLineJumpDlg : public CDialog
{
// コンストラクション
public:
	CLineJumpDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ

// ダイアログ データ
	//{{AFX_DATA(CLineJumpDlg)
	enum { IDD = IDD_LINE_JUMP_DLG };
	CButton	m_ok;
	int		m_line_no;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CLineJumpDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CLineJumpDlg)
	afx_msg void OnChangeEditLineNo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_LINEJUMPDLG_H__753F4441_A2F7_11D5_8505_00E018A83B1B__INCLUDED_)
