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
// EditorOptionFullScreen.h : �w�b�_�[ �t�@�C��
//

/////////////////////////////////////////////////////////////////////////////
// CEditorOptionFullScreen �_�C�A���O

class CEditorOptionFullScreen : public CPropertyPage
{
	DECLARE_DYNCREATE(CEditorOptionFullScreen)

// �R���X�g���N�V����
public:
	CEditorOptionFullScreen();
	~CEditorOptionFullScreen();

// �_�C�A���O �f�[�^
	//{{AFX_DATA(CEditorOptionFullScreen)
	enum { IDD = IDD_OPT_FULL_SCREEN };
	UINT	m_height;
	UINT	m_left_space;
	UINT	m_top_space;
	UINT	m_width;
	CString	m_bg_image_file;
	//}}AFX_DATA


// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B

	//{{AFX_VIRTUAL(CEditorOptionFullScreen)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
protected:
	// �������ꂽ���b�Z�[�W �}�b�v�֐�
	//{{AFX_MSG(CEditorOptionFullScreen)
	afx_msg void OnBtnBgImage();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_EDITOROPTIONFULLSCREEN_H__11478262_11F3_45B2_B34F_0C0440B6CE73__INCLUDED_)
