/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // MainFrm.h : CMainFrame クラスの宣言およびインターフェイスの定義をします。
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__52A23F49_4E07_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_MAINFRM_H__52A23F49_4E07_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "accellist.h"
#include "wheelsplit.h"

class CMainFrame : public CFrameWnd
{
	
protected: // シリアライズ機能のみから作成します。
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)


// アトリビュート
public:

// オペレーション
public:
	void InitAccelerator();

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL DestroyWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // コントロール バー用メンバ
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	CToolBar    m_wndToolBar2;

// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnViewToolbarEditMode();
	afx_msg void OnUpdateViewToolbarEditMode(CCmdUI* pCmdUI);
	afx_msg void OnAccelerator();
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnUpdateShowTopmost(CCmdUI* pCmdUI);
	afx_msg void OnShowTopmost();
	afx_msg void OnWindowActive1();
	afx_msg void OnWindowActive2();
	afx_msg void OnWindowActive3();
	afx_msg void OnWindowActive4();
	afx_msg void OnWindowActive5();
	afx_msg void OnWindowActive6();
	afx_msg void OnWindowActive7();
	afx_msg void OnWindowActive8();
	afx_msg void OnNextWindow();
	afx_msg void OnPrevWindow();
	afx_msg void OnUpdateNextWindow(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePrevWindow(CCmdUI* pCmdUI);
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	afx_msg void OnSplitWindow();
	afx_msg void OnUpdateSplitWindow(CCmdUI* pCmdUI);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnActivateApp(BOOL bActive, DWORD hTask);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnViewFullscreen();
	afx_msg void OnUpdateViewFullscreen(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnUpdateCursorPos(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFileType(CCmdUI *pCmdUI);
	afx_msg void OnUpdateBoxSelect(CCmdUI *pCmdUI);
	DECLARE_MESSAGE_MAP()

public:
	void SetNextWindowPos();
	void GetMessageString( UINT nID, CString& rMessage ) const;

private:
	void GetNextWindowPos(int &left, int &top, int &width, int &height);

	CAccelList m_accel_list;
	HACCEL m_accel;
	CWheelSplitterWnd		m_wndSplitter;

	void CreateAccelerator();
	void DestroyAccelerator();
	void SetAcceleratorToMenu(CMenu *pMenu);

	void AddWindowList(CMenu* pPopupMenu);
	void SetLangToolBar();

	// for full screen mode
	CRect	m_FullScreenWindowRect;
	BOOL	m_bStatusBarVisible;
	BOOL	m_bToolBarVisible;
	BOOL	m_bToolBar2Visible;
	CToolBar *m_pwndFullScrnBar;
	WINDOWPLACEMENT m_wpPrev;

	BOOL IsFullScreen();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_MAINFRM_H__52A23F49_4E07_11D5_8505_00E018A83B1B__INCLUDED_)
