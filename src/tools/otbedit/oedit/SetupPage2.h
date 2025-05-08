/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_SETUPPAGE2_H__DCCFC89C_5E97_4BF3_ABA7_51AF5DEEFC91__INCLUDED_)
#define AFX_SETUPPAGE2_H__DCCFC89C_5E97_4BF3_ABA7_51AF5DEEFC91__INCLUDED_

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

	COLORREF m_color[FILETAB_CTRL_COLOR_CNT];

// ダイアログ データ
	//{{AFX_DATA(CSetupPage2)
	enum { IDD = IDD_OPT_SETUP2 };
	BOOL	m_disable_double_boot;
	BOOL	m_enable_tag_jump;
	BOOL	m_save_modified;
	BOOL	m_search_loop_msg;
	BOOL	m_tab_multi_row;
	BOOL	m_window_title_is_path_name;
	BOOL	m_tab_title_is_path_name;
	UINT	m_max_mru;
	BOOL	m_boot_on_ime;
	BOOL	m_tab_close_at_mclick;
	BOOL	m_hide_tab_bar_at_one;
	BOOL	m_search_dlg_close_at_found;
	BOOL	m_tab_drag_move;
	BOOL	m_close_btn_on_tab;
	BOOL	m_tab_create_at_dblclick;
	BOOL	m_show_tab_tooltip;
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
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnBtnBkColor();
	afx_msg void OnBtnBkColor2();
	afx_msg void OnBtnTextColor();
	afx_msg void OnBtnDefaultColor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void ChooseColor(int color_id);
	void DrawColorBtn(LPDRAWITEMSTRUCT lpDrawItemStruct, COLORREF clr);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_SETUPPAGE2_H__DCCFC89C_5E97_4BF3_ABA7_51AF5DEEFC91__INCLUDED_)
