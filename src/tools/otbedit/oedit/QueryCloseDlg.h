/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_QUERYCLOSEDLG_H__C6512B01_D9D4_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_QUERYCLOSEDLG_H__C6512B01_D9D4_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// QueryCloseDlg.h : �w�b�_�[ �t�@�C��
//

/////////////////////////////////////////////////////////////////////////////
// CQueryCloseDlg �_�C�A���O

class CQueryCloseDlg : public CDialog
{
// �R���X�g���N�V����
public:
	CQueryCloseDlg(CWnd* pParent = NULL);   // �W���̃R���X�g���N�^

	BOOL	m_result;

// �_�C�A���O �f�[�^
	//{{AFX_DATA(CQueryCloseDlg)
	enum { IDD = IDD_QUERY_CLOSE_DLG };
	CStatic	m_icon;
	CString	m_msg;
	//}}AFX_DATA


// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CQueryCloseDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
protected:

	// �������ꂽ���b�Z�[�W �}�b�v�֐�
	//{{AFX_MSG(CQueryCloseDlg)
	virtual void OnOK();
	afx_msg void OnNo();
	virtual void OnCancel();
	afx_msg void OnAllyes();
	afx_msg void OnAllno();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_QUERYCLOSEDLG_H__C6512B01_D9D4_11D5_8505_00E018A83B1B__INCLUDED_)
