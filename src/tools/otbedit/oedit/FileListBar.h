/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_FILELISTBAR_H__B30A5029_1FA5_49A1_B9B2_3E3DE72511C5__INCLUDED_)
#define AFX_FILELISTBAR_H__B30A5029_1FA5_49A1_B9B2_3E3DE72511C5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileListBar.h : ヘッダー ファイル
//

#include "sizecbar.h"
#include "scbarg.h"
#include "filetreectrl.h"

/////////////////////////////////////////////////////////////////////////////
// CFileListBar ウィンドウ

class CFileListBar : public CSizingControlBarG
{
// コンストラクション
public:
	CFileListBar();

// アトリビュート
private:
	CFileTreeCtrl	m_tree_ctrl;
	volatile BOOL	m_b_init;

public:
	void SetFilter(LPCTSTR filter);
	void Init();
	void WaitForInitializeThread();
	BOOL IsFileTreeActive();

// オペレーション
private:
	void OnTreeDblClick();

public:

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CFileListBar)
	protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CFileListBar();

	// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CFileListBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

int file_list_bar_init(CFileListBar *file_list_bar, LPCTSTR filter);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_FILELISTBAR_H__B30A5029_1FA5_49A1_B9B2_3E3DE72511C5__INCLUDED_)
