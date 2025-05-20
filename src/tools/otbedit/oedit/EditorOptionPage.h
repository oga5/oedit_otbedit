/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_EDITOROPTIONPAGE_H__71AC5182_A085_11D4_B06E_00E018A83B1B__INCLUDED_)
#define AFX_EDITOROPTIONPAGE_H__71AC5182_A085_11D4_B06E_00E018A83B1B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// EditorOptionPage.h : ヘッダー ファイル
//

#include "EditCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CEditorOptionPage ダイアログ

class CEditorOptionPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CEditorOptionPage)

// コンストラクション
public:
	CEditorOptionPage();
	~CEditorOptionPage();

	CStrToken m_str_token;

// ダイアログ データ
	//{{AFX_DATA(CEditorOptionPage)
	enum { IDD = IDD_OPT_EDITOR };
	BOOL	m_show_line_feed;
	BOOL	m_show_tab;
	int		m_tabstop;
	BOOL	m_show_row_num;
	BOOL	m_show_col_num;
	int		m_row_space;
	int		m_char_space;
	int		m_left_space;
	int		m_top_space;
	int		m_line_len;
	BOOL	m_show_2byte_space;
	BOOL	m_clickable_url;
	BOOL	m_show_row_line;
	BOOL	m_show_edit_row;
	int		m_wheel_scroll_row;
	BOOL	m_show_space;
	BOOL	m_show_line_end;
	BOOL	m_show_brackets_bold;
	BOOL	m_show_ruled_line;
	//}}AFX_DATA

	COLORREF m_color[EDIT_CTRL_COLOR_CNT];

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CEditorOptionPage)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CEditorOptionPage)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckShowLineFeed();
	afx_msg void OnCheckShowTab();
	afx_msg void OnChangeEditTabstop();
	afx_msg void OnBtnTextColor();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnBtnPenColor();
	afx_msg void OnBtnDefaultColor();
	afx_msg void OnBtnBgColor();
	afx_msg void OnBtnCommentColor();
	afx_msg void OnBtnKeywordColor();
	afx_msg void OnBtnQuoteColor();
	afx_msg void OnCheckShowColNum();
	afx_msg void OnCheckShowRowNum();
	afx_msg void OnChangeEditRowSpace();
	afx_msg void OnChangeEditCharSpace();
	afx_msg void OnChangeEditLeftSpace();
	afx_msg void OnChangeEditTopSpace();
	afx_msg void OnChangeEditLineLen();
	afx_msg void OnCheckShow2byteSpace();
	afx_msg void OnBtnSearchColor();
	afx_msg void OnBtnSelectedColor();
	afx_msg void OnCheckClickableUrl();
	afx_msg void OnBtnRulerColor();
	afx_msg void OnCheckShowRowLine();
	afx_msg void OnBtnOperatorColor();
	afx_msg void OnCheckShowEditRow();
	afx_msg void OnBtnKeyword2Color();
	afx_msg void OnChangeEditWheelScrollRow();
	afx_msg void OnCheckShowSpace();
	afx_msg void OnCheckShowLineEnd();
	afx_msg void OnCheckShowBracketsBold();
	afx_msg void OnBtnImeCaretColor();
	afx_msg void OnBtnRuledLineColor();
	afx_msg void OnCheckShowRuledLine();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void CreateEditCtrl();
	void SetDefaultEditColor();
	void SetEditColor();
	void ChooseColor(int color_id);
	void DrawColorBtn(LPDRAWITEMSTRUCT lpDrawItemStruct, COLORREF clr);
	void SetEditorOption();
	CEditData	m_edit_data;
	CEditCtrl	m_edit_ctrl;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_EDITOROPTIONPAGE_H__71AC5182_A085_11D4_B06E_00E018A83B1B__INCLUDED_)
