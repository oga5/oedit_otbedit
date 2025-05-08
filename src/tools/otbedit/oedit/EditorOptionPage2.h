/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_EDITOROPTIONPAGE2_H__FDD86AE1_DACC_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_EDITOROPTIONPAGE2_H__FDD86AE1_DACC_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditorOptionPage2.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CEditorOptionPage2 ダイアログ

class CEditorOptionPage2 : public CPropertyPage
{
	DECLARE_DYNCREATE(CEditorOptionPage2)

// コンストラクション
public:
	CEditorOptionPage2();
	~CEditorOptionPage2();

// ダイアログ データ
	//{{AFX_DATA(CEditorOptionPage2)
	enum { IDD = IDD_OPT_EDITOR2 };
	CButton	m_check_text_drop_edit;
	CEdit	m_edit_last_confinement_str;
	CEdit	m_edit_first_confinement_str;
	BOOL	m_drag_drop_edit;
	BOOL	m_tab_as_space;
	int		m_indent_mode;
	BOOL	m_mouse_select_copy;
	BOOL	m_check_row_copy_at_no_sel;
	BOOL	m_use_keyword_window;
	BOOL	m_enable_confinement;
	BOOL	m_enable_word_wrap;
	CString	m_first_confinement_str;
	CString	m_last_confinement_str;
	BOOL	m_text_drop_edit;
	int		m_brackets_blink_time;
	BOOL	m_clear_after_copy;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CEditorOptionPage2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CEditorOptionPage2)
	virtual void OnOK();
	afx_msg void OnCheckEnableConfinement();
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckDragDropEdit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void CheckConfinementCtrl();

	void CheckTextDrop();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_EDITOROPTIONPAGE2_H__FDD86AE1_DACC_11D5_8505_00E018A83B1B__INCLUDED_)
