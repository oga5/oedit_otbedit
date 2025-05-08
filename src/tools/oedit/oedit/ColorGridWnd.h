/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_COLORGRIDWND_H__D5DD1882_0E69_11D6_8508_00E018A83B1B__INCLUDED_)
#define AFX_COLORGRIDWND_H__D5DD1882_0E69_11D6_8508_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorGridWnd.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CColorGridWnd ウィンドウ

#define CGC_WM_SELECTED_COLOR		WM_USER + 300
#define CGC_WM_DBL_CLICK			WM_USER + 301
#define MAX_COLOR_CNT	64
#define MAX_UNDO_CNT	20

class CColorGridWnd : public CWnd
{
// コンストラクション
public:
	CColorGridWnd();

// アトリビュート
public:

// オペレーション
public:
	void SetColor(int idx, COLORREF col);
	void SetSelectedColor(COLORREF col);
	COLORREF GetColor(int idx) { return m_color_info[idx]; }
	COLORREF GetSelectedColor() { return m_color_info[GetSelectedIdx()]; }

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CColorGridWnd)
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CColorGridWnd();

private:
	POINT		m_selected;
	int			m_x;
	int			m_y;
	int			m_col_width;
	int			m_col_height;
	COLORREF	m_color_info[MAX_COLOR_CNT];

	struct {
		int			idx;
		COLORREF	old_color_info;
		COLORREF	new_color_info;
	} m_undo_data[MAX_UNDO_CNT];
	int			m_undo_cnt;

	void InvalidateCell(int x, int y);
	void SelectCell(int x, int y);
	void Undo();
	void Redo();

	int GetIdx(int x, int y) { return y * m_x + x; }
	int GetSelectedIdx() { return GetIdx(m_selected.x, m_selected.y); }

	// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CColorGridWnd)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_COLORGRIDWND_H__D5DD1882_0E69_11D6_8508_00E018A83B1B__INCLUDED_)
