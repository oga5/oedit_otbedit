/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_LINEJUMPDLG_H__753F4441_A2F7_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_LINEJUMPDLG_H__753F4441_A2F7_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LineJumpDlg.h : �w�b�_�[ �t�@�C��
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CLineJumpDlg �_�C�A���O

class CLineJumpDlg : public CDialog
{
// �R���X�g���N�V����
public:
	CLineJumpDlg(CWnd* pParent = NULL);   // �W���̃R���X�g���N�^

// �_�C�A���O �f�[�^
	//{{AFX_DATA(CLineJumpDlg)
	enum { IDD = IDD_LINE_JUMP_DLG };
	CButton	m_ok;
	int		m_line_no;
	//}}AFX_DATA


// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CLineJumpDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
protected:

	// �������ꂽ���b�Z�[�W �}�b�v�֐�
	//{{AFX_MSG(CLineJumpDlg)
	afx_msg void OnChangeEditLineNo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_LINEJUMPDLG_H__753F4441_A2F7_11D5_8505_00E018A83B1B__INCLUDED_)
