/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // MainFrm.h : CMainFrame �N���X�̐錾����уC���^�[�t�F�C�X�̒�`�����܂��B
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__52A23F49_4E07_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_MAINFRM_H__52A23F49_4E07_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "accellist.h"
#include "filelistbar.h"

class CMainFrame : public CFrameWnd
{
	
protected: // �V���A���C�Y�@�\�݂̂���쐬���܂��B
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// �A�g���r���[�g
public:

// �I�y���[�V����
public:
	void InitAccelerator();

// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL DestroyWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // �R���g���[�� �o�[�p�����o
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	CToolBar    m_wndToolBar2;
	
// �������ꂽ���b�Z�[�W �}�b�v�֐�
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
	afx_msg void OnUpdateViewFileBar(CCmdUI* pCmdUI);
	afx_msg void OnViewFileBar();
	afx_msg void OnMove(int x, int y);
	afx_msg void OnActivateApp(BOOL bActive, DWORD hTask);
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

	void CreateAccelerator();
	void DestroyAccelerator();
	void SetAcceleratorToMenu(CMenu *pMenu);

	void SetLangToolBar();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_MAINFRM_H__52A23F49_4E07_11D5_8505_00E018A83B1B__INCLUDED_)
