/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_SEARCHDLG_H__8F6F9980_626D_11D4_B06E_00E018A83B1B__INCLUDED_)
#define AFX_SEARCHDLG_H__8F6F9980_626D_11D4_B06E_00E018A83B1B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SearchDlg.h : �w�b�_�[ �t�@�C��
//

#define WM_SEARCH_TEXT	WM_USER + 100
#define WM_REPLACE_TEXT	WM_USER + 101
#define WM_FILTER_TEXT	WM_USER + 102

#include "resource.h"
#include "SearchDlgData.h"
#include "DlgSingletonTmpl.h"

/////////////////////////////////////////////////////////////////////////////
// CSearchDlg �_�C�A���O

class CSearchDlg : public CDialog
{
// �R���X�g���N�V����
public:
	CSearchDlg(CWnd* pParent = NULL);   // �W���̃R���X�g���N�^
	~CSearchDlg();

	void ShowDialog(CWnd *wnd, int search_message, CSearchDlgData *search_data, BOOL b_close_at_found);
	INT_PTR DoModal(CWnd *wnd, int search_message, CSearchDlgData *search_data, BOOL b_close_at_found);

	CWnd *GetWnd() { return m_wnd; }
	void SetWnd(CWnd *wnd) { m_wnd = wnd; }

	void SetSearchText(CString search_text);

	CString		m_title;

// �_�C�A���O �f�[�^
	//{{AFX_DATA(CSearchDlg)
	enum { IDD = IDD_SEARCH_DLG };
	CButton	m_btn_check_regexp;
	CButton	m_btn_cancel;
	CComboBox	m_combo_search_text;
	CButton	m_btn_prev;
	CButton	m_btn_next;
	BOOL	m_distinct_lwr_upr;
	BOOL	m_regexp;
	//}}AFX_DATA


// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B

	//{{AFX_VIRTUAL(CSearchDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
protected:

	// �������ꂽ���b�Z�[�W �}�b�v�֐�
	//{{AFX_MSG(CSearchDlg)
	afx_msg void OnBtnNext();
	afx_msg void OnBtnPrev();
	virtual BOOL OnInitDialog();
	afx_msg void OnEditchangeComboSearchText();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual void OnOK();
	afx_msg void OnSelendokComboSearchText();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnCancel();
	//}}AFX_MSG
	afx_msg void OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI );
	DECLARE_MESSAGE_MAP()
private:
	BOOL		m_initialized;
	BOOL		m_close_at_found;
	CWnd		*m_wnd;
	int			m_search_message;
	CSearchDlgData	*m_data;
	int			*m_search_status;
	
	CString		m_search_text;

	int m_dlg_height;

	void InitDialog();
	void CheckBtn();
	void SaveSearchDlgData();

	void SaveSearchTextList();
	void LoadSearchTextList();

	void LoadDlgSize();
	void SaveDlgSize();
	void RelayoutControls();

	void DoSearch(int dir);

	BOOL UpdateData(BOOL bSaveAndValidate = TRUE);
public:
	BOOL m_distinct_width_ascii;
	CButton m_btn_bottom;
};

typedef DlgSingletonTmpl<CSearchDlg, IDD_SEARCH_DLG> CSearchDlgSingleton;


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_SEARCHDLG_H__8F6F9980_626D_11D4_B06E_00E018A83B1B__INCLUDED_)
