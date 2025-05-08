/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_SETUPPAGE2_H__B2CCCF64_491B_45C8_80FB_77E45441D60B__INCLUDED_)
#define AFX_SETUPPAGE2_H__B2CCCF64_491B_45C8_80FB_77E45441D60B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetupPage2.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CSetupPage2 ダイアログ

class CSetupPage2 : public CPropertyPage
{
	DECLARE_DYNCREATE(CSetupPage2)

// コンストラクション
public:
	CSetupPage2();
	~CSetupPage2();

// ダイアログ データ
	//{{AFX_DATA(CSetupPage2)
	enum { IDD = IDD_OPT_SETUP2 };
	BOOL	m_enable_tag_jump;
	BOOL	m_save_modified;
	BOOL	m_search_loop_msg;
	BOOL	m_window_title_is_path_name;
	UINT	m_max_mru;
	BOOL	m_boot_on_ime;
	BOOL	m_search_dlg_close_at_found;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CSetupPage2)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CSetupPage2)
		// メモ: ClassWizard はこの位置にメンバ関数を追加します。
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_SETUPPAGE2_H__B2CCCF64_491B_45C8_80FB_77E45441D60B__INCLUDED_)
