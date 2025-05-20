/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 ///////////////////////////////////////////////////////////////////////////// 
// ExpSplitterWnd.h

#if !defined(ExpSplitterWnd_H_INCLUDED)
#define ExpSplitterWnd_H_INCLUDED

#include "wheelsplit.h"

// CSplitterWndの拡張
//
// マウス操作では各ペインの高さの変更をできなくする
//
class CExpSplitterWnd : public CWheelSplitterWnd
{
protected:
	DECLARE_DYNCREATE(CExpSplitterWnd)

public:
	CExpSplitterWnd();
	virtual ~CExpSplitterWnd();

	void SetHorizonalMode(int splitter_cx);
	void SetFixLastColWidth(int cx);

protected:
	int HitTest(CPoint pt) const;

	int HitTestV(CPoint pt) const;
	int HitTestH(CPoint pt) const;

	//{{AFX_MSG(CExpSplitterWnd)
	afx_msg void OnMouseMove(UINT nFlags, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint pt);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	BOOL	m_horizonal_mode;
	int		m_org_splitter_cy;
	int		m_fixed_last_col_width;
};


/////////////////////////////////////////////////////////////////////////////

#endif // !defined(ExpSplitterWnd_H_INCLUDED)
