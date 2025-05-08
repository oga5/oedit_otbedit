/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_SETUPPAGE_H__2F9C2722_AFF8_11D4_B06E_00E018A83B1B__INCLUDED_)
#define AFX_SETUPPAGE_H__2F9C2722_AFF8_11D4_B06E_00E018A83B1B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SetupPage.h : �w�b�_�[ �t�@�C��
//

/////////////////////////////////////////////////////////////////////////////
// CSetupPage �_�C�A���O

class CSetupPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CSetupPage)

// �R���X�g���N�V����
public:
	CSetupPage();
	~CSetupPage();

// �_�C�A���O �f�[�^
	//{{AFX_DATA(CSetupPage)
	enum { IDD = IDD_OPT_SETUP };
	CComboBox	m_combo_file_list_filter;
	CString	m_initial_dir;
	CString	m_browser_01;
	CString	m_browser_02;
	CString	m_browser_03;
	CString	m_file_list_filter;
	BOOL	m_ogrep_search_dir_as_current_dir;
	//}}AFX_DATA


// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B

	//{{AFX_VIRTUAL(CSetupPage)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
protected:
	// �������ꂽ���b�Z�[�W �}�b�v�֐�
	//{{AFX_MSG(CSetupPage)
	afx_msg void OnBtnInitialDir();
	afx_msg void OnBtnBrowser01();
	afx_msg void OnBtnBrowser02();
	afx_msg void OnBtnBrowser03();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void SetFileListFilter();

public:
	int		m_kanji_code;
	int		m_line_type;
	CString	m_edit_mode;

	int m_file_open_kanji_code;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_SETUPPAGE_H__2F9C2722_AFF8_11D4_B06E_00E018A83B1B__INCLUDED_)
