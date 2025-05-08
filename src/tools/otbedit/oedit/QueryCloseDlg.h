/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_QUERYCLOSEDLG_H__C6512B01_D9D4_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_QUERYCLOSEDLG_H__C6512B01_D9D4_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// QueryCloseDlg.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CQueryCloseDlg ダイアログ

class CQueryCloseDlg : public CDialog
{
// コンストラクション
public:
	CQueryCloseDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ

	BOOL	m_result;

// ダイアログ データ
	//{{AFX_DATA(CQueryCloseDlg)
	enum { IDD = IDD_QUERY_CLOSE_DLG };
	CStatic	m_icon;
	CString	m_msg;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CQueryCloseDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CQueryCloseDlg)
	virtual void OnOK();
	afx_msg void OnNo();
	virtual void OnCancel();
	afx_msg void OnAllyes();
	afx_msg void OnAllno();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_QUERYCLOSEDLG_H__C6512B01_D9D4_11D5_8505_00E018A83B1B__INCLUDED_)
