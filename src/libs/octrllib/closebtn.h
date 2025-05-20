/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#if !defined(AFX_CLOSEBTN_H__CC4A629A_AC66_409F_83CD_07F133CD683D__INCLUDED_)
#define AFX_CLOSEBTN_H__CC4A629A_AC66_409F_83CD_07F133CD683D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CloseBtn.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CCloseBtn ウィンドウ

class CCloseBtn : public CButton
{
// コンストラクション
public:
	CCloseBtn();

	BOOL Create(CWnd *pParentWnd, UINT nID, BOOL border = TRUE, int btn_size = 16, int bmp_offset = 4);
	void SetColor(COLORREF color) { m_color = color; }
	void SetParam(int param) { m_param = param; }
	int GetParam() { return m_param; }

// アトリビュート
public:

// オペレーション
public:

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CCloseBtn)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CCloseBtn();

	// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CCloseBtn)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	CBitmap m_x_bmp;

	void CreateXBitmap();
	BOOL		m_border;
	int			m_btn_size;
	int			m_bmp_offset;
	
	COLORREF	m_color;
	int			m_param;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_CLOSEBTN_H__CC4A629A_AC66_409F_83CD_07F133CD683D__INCLUDED_)
