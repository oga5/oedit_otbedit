/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_REPLACEDLG_H__7C840F80_6C50_11D4_B06E_00E018A83B1B__INCLUDED_)
#define AFX_REPLACEDLG_H__7C840F80_6C50_11D4_B06E_00E018A83B1B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ReplaceDlg.h : �w�b�_�[ �t�@�C��
//

/////////////////////////////////////////////////////////////////////////////
// CReplaceDlg �_�C�A���O

#include "resource.h"
#include "SearchDlgData.h"
#include "DlgSingletonTmpl.h"

#define REPLACE_SELECTED_AREA	0
#define REPLACE_ALL				1


class CReplaceDlg : public CDialog
{
// �R���X�g���N�V����
public:
	CReplaceDlg(CWnd* pParent = NULL);   // �W���̃R���X�g���N�^
	~CReplaceDlg();

	void ShowDialog(CWnd *wnd, int search_message, int replace_message,
		CSearchDlgData *search_data);
	INT_PTR DoModal(CWnd *wnd, int search_message, int replace_message,
		BOOL disable_selected_area, CSearchDlgData *search_data);

	CWnd *GetWnd() { return m_wnd; }
	void SetWnd(CWnd *wnd) { m_wnd = wnd; }

// �_�C�A���O �f�[�^
	//{{AFX_DATA(CReplaceDlg)
	enum { IDD = IDD_REPLACE_DLG };
	CButton	m_btn_cancel;
	CComboBox	m_combo_search_text;
	CComboBox	m_combo_replace_text;
	CButton	m_radio_selected_area;
	CButton	m_btn_replace_all;
	CButton	m_btn_replace;
	CButton	m_btn_prev;
	CButton	m_btn_next;
	BOOL	m_distinct_lwr_upr;
	BOOL	m_regexp;
	int		m_replace_area;
	//}}AFX_DATA


// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B

	//{{AFX_VIRTUAL(CReplaceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
protected:
	// �������ꂽ���b�Z�[�W �}�b�v�֐�
	//{{AFX_MSG(CReplaceDlg)
	afx_msg void OnBtnNext();
	afx_msg void OnBtnPrev();
	afx_msg void OnBtnReplace();
	afx_msg void OnBtnReplaceAll();
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioAll();
	afx_msg void OnRadioSelectedArea();
	afx_msg void OnEditchangeComboSearchText();
	afx_msg void OnEditchangeComboReplaceText();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual void OnOK();
	afx_msg void OnSelendokComboSearchText();
	virtual void OnCancel();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg void OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI );
	DECLARE_MESSAGE_MAP()

private:
	BOOL		m_initialized;
	CWnd		*m_wnd;
	int			m_search_message;
	int			m_replace_message;
	BOOL		m_disable_selected_area;
	CSearchDlgData	*m_data;
	int			*m_search_status;

	CString		m_search_text;
	CString		m_replace_text;

	int			m_dlg_height;

	void InitDialog();
	void CheckBtn();
	void SaveSearchDlgData();

	void SaveSearchTextList();
	void LoadSearchTextList();

	void LoadDlgSize();
	void SaveDlgSize();
	void RelayoutControls();

	BOOL UpdateData(BOOL bSaveAndValidate = TRUE);
public:
	BOOL m_distinct_width_ascii;
	CButton m_btn_bottom;
};

typedef DlgSingletonTmpl<CReplaceDlg, IDD_REPLACE_DLG> CReplaceDlgSingleton;

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_REPLACEDLG_H__7C840F80_6C50_11D4_B06E_00E018A83B1B__INCLUDED_)
