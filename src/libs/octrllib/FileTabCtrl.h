/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#if !defined(AFX_FILETABCTRL_H__7E865C8B_53D7_430F_9A9B_5BFA91CFB422__INCLUDED_)
#define AFX_FILETABCTRL_H__7E865C8B_53D7_430F_9A9B_5BFA91CFB422__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileTabCtrl.h : ヘッダー ファイル
//
#include <vector>

#include "octrl_msg.h"
#include "DropTargetTmpl.h"
#include "CloseBtn.h"
#include "ToolTipEx.h"

/////////////////////////////////////////////////////////////////////////////
// CFileTabCtrl ウィンドウ

enum {
	FILETAB_TEXT_COLOR,
	FILETAB_ACTIVE_BK_COLOR,
	FILETAB_NO_ACTIVE_BK_COLOR,
	FILETAB_BTN_COLOR,
	FILETAB_CTRL_COLOR_CNT,
};

#define FTC_TAB_DRAG_MOVE			(0x0001 << 0)
#define FTC_SHOW_TAB_TOOLTIP		(0x0001 << 1)
#define FTC_CLOSE_BTN_ON_TAB		(0x0001 << 2)
#define FTC_CLOSE_BTN_ON_ALL_TAB	(0x0001 << 4)

struct file_tab_ctrl_nmhdr {
	NMHDR		nmhdr;
	TCHAR		str_buf[1024];
};

class CFileTabCtrl : public CTabCtrl
{
// コンストラクション
public:
	CFileTabCtrl();

// アトリビュート
public:
	BOOL b_min_erase_bkgnd;

// オペレーション
public:

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CFileTabCtrl)
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL


// インプリメンテーション
public:
	virtual ~CFileTabCtrl();

	// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CFileTabCtrl)
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

public:
	void SetColor(int idx, COLORREF cr) { m_color[idx] = cr; }
	void SetDefaultColor();
	COLORREF GetColor(int idx) { return m_color[idx]; }

	void SetFont(CFont *pFont);

	int InsertItem(int nItem, TC_ITEM* pTabCtrlItem);
	int InsertItem(int nItem, const TCHAR *text, LPARAM lparam);
	BOOL DeleteItem(int nItem);
	void SetItemText(int nItem, const TCHAR *text);
	int SetCurSel(int nItem);

	DROPEFFECT DragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	DROPEFFECT DragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	BOOL Drop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	void DragLeave();

	void SetExStyle(DWORD ex_style);

	void SetCloseBtnTopOffset(int offset) { m_close_btn_top_offset = offset; }
	void Redraw();

protected:
	void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );

private:
	DropTargetTmpl<CFileTabCtrl>	m_droptarget;
	void ClearMClickPt();
	CPoint	m_click_pt;

	COLORREF	m_color[FILETAB_CTRL_COLOR_CNT];

	int HitTabTest(CPoint point);
	void SetCloseBtnPos(int idx, CCloseBtn *btn, BOOL b_invalidate);
	void SetCloseBtnPos(BOOL b_invalidate = FALSE);

	DWORD		m_ex_style;

	int			m_drop_target_idx;
	UINT		m_cf_tab_idx;
	BOOL TabDrag(int tab_idx);

	CCloseBtn	m_close_btn;

	void ShowToolTip();
	CPoint		m_tool_tip_pt;
	int			m_last_tool_tip_col;
	CToolTipEx	m_tool_tip;

	//
	void ClearCloseBtnVec();
	int m_close_btn_top_offset;
	std::vector<CCloseBtn *>	m_close_btn_vec;
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_FILETABCTRL_H__7E865C8B_53D7_430F_9A9B_5BFA91CFB422__INCLUDED_)
