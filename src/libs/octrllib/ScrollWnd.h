/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#if !defined(AFX_SCROLLWND_H__3C5B7342_1244_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_SCROLLWND_H__3C5B7342_1244_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ScrollWnd.h : ヘッダー ファイル
//

// FIXME: 説明を書く
#define NO_HSCROLL_BAR		(1 << 0)
#define NO_WS_VSCROLL		(1 << 1)
#define NO_WS_HSCROLL		(1 << 2)
#define NO_VSCROLL_BAR		(1 << 3)
#define VSCROLL_ALWAYS_ON	(1 << 4)
#define HSCROLL_ALWAYS_ON	(1 << 5)
#define LOCK_WINDOW			(1 << 6)
#define KEEP_WS_VH_SCROLL_STYLE		(1 << 7)

/////////////////////////////////////////////////////////////////////////////
// CScrollWnd ウィンドウ

class CScrollWnd : public CWnd
{
	DECLARE_DYNAMIC(CScrollWnd)

// コンストラクション
public:
	CScrollWnd();

// アトリビュート
public:

// オペレーション
public:
	// overwrite scroll function
	int GetScrollPos( int nBar ) const;
	int SetScrollPos( int nBar, int nPos, BOOL bRedraw = TRUE );
	virtual CScrollBar* GetScrollBarCtrl(int nBar) const;

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CScrollWnd)
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CScrollWnd();

	virtual int GetShowCol() = 0;
	virtual int GetShowRow() = 0;

	virtual int GetScrollLeftMargin() = 0;
	virtual int GetScrollTopMargin() = 0;
	virtual int GetScrollRightMargin() { return 0; }
	virtual int GetScrollBottomMargin() { return 0; }

	int GetScrollLimit( int nBar );

	BOOL IsSplitterMode() const { return m_splitter_mode; }
	void SetSplitterMode(BOOL splitter_mode);

	void SetWheelScrollRate(double wheel_scroll_rate) { m_wheel_scroll_rate = wheel_scroll_rate; }
	void SetHWheelScrollRate(double hwheel_scroll_rate) { m_hwheel_scroll_rate = hwheel_scroll_rate; }
	void SetHWheelScrollEnable(BOOL b_enable) { m_b_hwheel_scroll_enable = b_enable; }

	void VScroll(UINT nSBCode);
	void HScroll(UINT nSBCode);

	int GetScrollStyle() { return m_scroll_style; }
	void SetScrollStyle(int scroll_style) { m_scroll_style = scroll_style; }

	BOOL IsThumbTracking() { return m_b_thumb_tracking; }

// FIXME: 要テスト
//	BOOL IsShowVScrollBar();
//	BOOL IsShowHScrollBar();

	// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CScrollWnd)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	int			m_scroll_style;
	BOOL		m_v_bar_enable;
	BOOL		m_h_bar_enable;
	double		m_wheel_scroll_rate;
	double		m_hwheel_scroll_rate;
	BOOL		m_b_hwheel_scroll_enable;

	BOOL		m_splitter_mode;
	BOOL		m_b_thumb_tracking;

	BOOL		m_on_mdi_wnd;

	BOOL IsNeedVScrollBar(int height);
	BOOL IsNeedHScrollBar(int width);

	void CheckOnMDIWnd();

protected:
/*
	void CheckVScrollBar();
	void CheckHScrollBar();
*/
	void CheckScrollBar();

	BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE, BOOL bThumbTrack = FALSE);

	virtual int GetHScrollSizeLimit() = 0;
	virtual int GetVScrollSizeLimit() = 0;
	virtual int GetHScrollSize(int xOrig, int x) = 0;
	virtual int GetVScrollSize(int yOrig, int y) = 0;

	virtual int GetVScrollMin() { return 0; }
	virtual int GetVScrollMax() { return 0; }
	virtual int GetVScrollPage() { return 0; }
	virtual int GetHScrollMin() { return 0; }
	virtual int GetHScrollMax() { return 0; }
	virtual int GetHScrollPage() { return 0; }

	virtual int GetVLineSize() { return 1; }
	virtual int GetHLineSize() { return 1; }

	virtual void Scrolled(CSize sizeScroll, BOOL bThumbTrack) {}
	virtual void EndScroll() {}

	// overwrite scroll function
	BOOL GetScrollInfo( int nBar, LPSCROLLINFO lpScrollInfo, UINT nMask = SIF_ALL );
	BOOL SetScrollInfo( int nBar, LPSCROLLINFO lpScrollInfo, BOOL bRedraw = TRUE );

	// splitter
	CSplitterWnd *GetParentSplitter();

	void GetDispRect( LPRECT lpRect ) const;
public:
	afx_msg void OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_SCROLLWND_H__3C5B7342_1244_11D5_8505_00E018A83B1B__INCLUDED_)
