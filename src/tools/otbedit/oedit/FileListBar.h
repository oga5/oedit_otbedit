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
// FileListBar.h : �w�b�_�[ �t�@�C��
//

#include "sizecbar.h"
#include "scbarg.h"
#include "filetreectrl.h"

/////////////////////////////////////////////////////////////////////////////
// CFileListBar �E�B���h�E

class CFileListBar : public CSizingControlBarG
{
// �R���X�g���N�V����
public:
	CFileListBar();

// �A�g���r���[�g
private:
	CFileTreeCtrl	m_tree_ctrl;
	volatile BOOL	m_b_init;

public:
	void SetFilter(LPCTSTR filter);
	void Init();
	void WaitForInitializeThread();
	BOOL IsFileTreeActive();

// �I�y���[�V����
private:
	void OnTreeDblClick();

public:

// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CFileListBar)
	protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
public:
	virtual ~CFileListBar();

	// �������ꂽ���b�Z�[�W �}�b�v�֐�
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
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_FILELISTBAR_H__B30A5029_1FA5_49A1_B9B2_3E3DE72511C5__INCLUDED_)
