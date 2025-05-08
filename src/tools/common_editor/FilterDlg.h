/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_FILTERDLG_H__16EF2846_FC8B_4795_81C3_AE378C601AB0__INCLUDED_)
#define AFX_FILTERDLG_H__16EF2846_FC8B_4795_81C3_AE378C601AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FilterDlg.h : �w�b�_�[ �t�@�C��
//

#include "resource.h"
#include "SearchDlgData.h"
#include "DlgSingletonTmpl.h"

/////////////////////////////////////////////////////////////////////////////
// CFilterDlg �_�C�A���O

#define FILTER_GREP_MODE		0
#define FILTER_DELETE_MODE		1

class CFilterDlg : public CDialog
{
// �R���X�g���N�V����
public:
	CFilterDlg(CWnd* pParent = NULL);   // �W���̃R���X�g���N�^
	~CFilterDlg();

	void ShowDialog(CWnd *wnd, int search_message, int filter_messeage, 
		CSearchDlgData *search_data);
	INT_PTR DoModal(CWnd *wnd, int search_message, int filter_messeage, 
		CSearchDlgData *search_data);

	CWnd *GetWnd() { return m_wnd; }
	void SetWnd(CWnd *wnd) { m_wnd = wnd; }

// �_�C�A���O �f�[�^
	//{{AFX_DATA(CFilterDlg)
	enum { IDD = IDD_FILTER_DLG };
	CButton	m_btn_filter;
	CButton	m_btn_next;
	CComboBox	m_combo_search_text;
	BOOL	m_distinct_lwr_upr;
	BOOL	m_regexp;
	int		m_combo_action;
	//}}AFX_DATA


// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CFilterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
protected:

	// �������ꂽ���b�Z�[�W �}�b�v�֐�
	//{{AFX_MSG(CFilterDlg)
	afx_msg void OnBtnNext();
	afx_msg void OnBtnFilter();
	virtual BOOL OnInitDialog();
	afx_msg void OnEditchangeComboSearchText();
	afx_msg void OnSelendokComboSearchText();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	BOOL		m_initialized;
	CWnd		*m_wnd;
	int			m_search_message;
	int			m_filter_message;
	CSearchDlgData	*m_data;
	int			*m_search_status;
	
	CString		m_search_text;

	void InitDialog();
	void CheckBtn();
	void SaveSearchDlgData();

	void SaveSearchTextList();
	void LoadSearchTextList();

	BOOL UpdateData(BOOL bSaveAndValidate = TRUE);
public:
	BOOL m_distinct_width_ascii;
	CButton m_btn_bottom;
};

typedef DlgSingletonTmpl<CFilterDlg, IDD_FILTER_DLG> CFilterDlgSingleton;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_FILTERDLG_H__16EF2846_FC8B_4795_81C3_AE378C601AB0__INCLUDED_)
