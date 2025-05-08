/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_EDITOROPTIONFULLSCREEN_H__11478262_11F3_45B2_B34F_0C0440B6CE73__INCLUDED_)
#define AFX_EDITOROPTIONFULLSCREEN_H__11478262_11F3_45B2_B34F_0C0440B6CE73__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditorOptionFullScreen.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CEditorOptionFullScreen ダイアログ

class CEditorOptionFullScreen : public CPropertyPage
{
	DECLARE_DYNCREATE(CEditorOptionFullScreen)

// コンストラクション
public:
	CEditorOptionFullScreen();
	~CEditorOptionFullScreen();

// ダイアログ データ
	//{{AFX_DATA(CEditorOptionFullScreen)
	enum { IDD = IDD_OPT_FULL_SCREEN };
	UINT	m_height;
	UINT	m_left_space;
	UINT	m_top_space;
	UINT	m_width;
	CString	m_bg_image_file;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CEditorOptionFullScreen)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CEditorOptionFullScreen)
	afx_msg void OnBtnBgImage();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_EDITOROPTIONFULLSCREEN_H__11478262_11F3_45B2_B34F_0C0440B6CE73__INCLUDED_)
