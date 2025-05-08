/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_SETUPPAGE_H__2F9C2722_AFF8_11D4_B06E_00E018A83B1B__INCLUDED_)
#define AFX_SETUPPAGE_H__2F9C2722_AFF8_11D4_B06E_00E018A83B1B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SetupPage.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CSetupPage ダイアログ

class CSetupPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CSetupPage)

// コンストラクション
public:
	CSetupPage();
	~CSetupPage();

// ダイアログ データ
	//{{AFX_DATA(CSetupPage)
	enum { IDD = IDD_OPT_SETUP };
	CComboBox	m_combo_file_list_filter;
	CString	m_initial_dir;
	CString	m_browser_01;
	CString	m_browser_02;
	CString	m_browser_03;
	CString	m_file_list_filter;
	BOOL	m_ogrep_search_dir_as_current_dir;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CSetupPage)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CSetupPage)
	afx_msg void OnBtnInitialDir();
	afx_msg void OnBtnBrowser01();
	afx_msg void OnBtnBrowser02();
	afx_msg void OnBtnBrowser03();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void SetFileListFilter();

public:
	int		m_kanji_code;
	int		m_line_type;
	CString	m_edit_mode;

	int m_file_open_kanji_code;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_SETUPPAGE_H__2F9C2722_AFF8_11D4_B06E_00E018A83B1B__INCLUDED_)
