/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#if !defined(AFX_AcceleratorDLG_H__A8555F84_C7AB_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_AcceleratorDLG_H__A8555F84_C7AB_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AcceleratorDlg.h : �w�b�_�[ �t�@�C��
//
#include "AccelList.h"

/////////////////////////////////////////////////////////////////////////////
// CAcceleratorDlg �_�C�A���O

class CAcceleratorDlg : public CDialog
{
// �R���X�g���N�V����
public:
	CAcceleratorDlg(CWnd* pParent = NULL);   // �W���̃R���X�g���N�^
	~CAcceleratorDlg();

	CAccelList		m_accel_list;

// �_�C�A���O �f�[�^
	//{{AFX_DATA(CAcceleratorDlg)
	enum { IDD = IDD_ACCELERATOR_DLG };
	CButton	m_btn_delete;
	CButton	m_btn_add;
	CEdit	m_edit_new_key;
	CListCtrl	m_list_command;
	CListCtrl	m_list_category;
	CListCtrl	m_list_accel;
	CString	m_new_key;
	CString	m_current_menu;
	//}}AFX_DATA


// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CAcceleratorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
protected:

	// �������ꂽ���b�Z�[�W �}�b�v�֐�
	//{{AFX_MSG(CAcceleratorDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnItemchangedListCategory(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedListCommand(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBtnDefault();
	afx_msg void OnBtnAdd();
	afx_msg void OnBtnDelete();
	afx_msg void OnItemchangedListAccel(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeEditNewKey();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	void SetCategoryList();
	void SetCommandList(int category_id);
	void SetCommandList_Key();
	void SetAccelList(int menu_id);
	void ResetAccelList();

	void CheckBtn();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_AcceleratorDLG_H__A8555F84_C7AB_11D5_8505_00E018A83B1B__INCLUDED_)
