// oextregDlg.h : �w�b�_�[ �t�@�C��
//

#if !defined(AFX_OEXTREGDLG_H__850FECA7_ADEF_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_OEXTREGDLG_H__850FECA7_ADEF_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// COextregDlg �_�C�A���O

class COextregDlg : public CDialog
{
// �\�z
public:
	COextregDlg(CWnd* pParent = NULL);	// �W���̃R���X�g���N�^

// �_�C�A���O �f�[�^
	//{{AFX_DATA(COextregDlg)
	enum { IDD = IDD_OEXTREG_DIALOG };
	CButton	m_btn_update;
	CButton	m_btn_delete;
	CListCtrl	m_ext_list;
	BOOL	m_check_java;
	BOOL	m_check_txt;
	BOOL	m_ie_source_viewer;
	//}}AFX_DATA

	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(COextregDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �̃T�|�[�g
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
protected:
	HICON m_hIcon;
	HICON m_hSmallIcon;

	// �������ꂽ���b�Z�[�W �}�b�v�֐�
	//{{AFX_MSG(COextregDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	afx_msg void OnItemchangedExtList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBtnAdd();
	afx_msg void OnBtnDelete();
	afx_msg void OnBtnUpdate();
	afx_msg void OnCheckJava();
	afx_msg void OnCheckTxt();
	afx_msg void OnIeSourceViewer();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void ClearExtList();
	void InitExtList();
	void InitIESourceViewer();
	void SelectList(TCHAR *ext);
	void AddExtList(TCHAR *ext);

	void UnRegisterShellFileTypes(TCHAR *ext);
	void RegisterShellFileTypes(TCHAR *ext, TCHAR *fileTypeName, BOOL b_explorer_rclick);
	BOOL CheckRegisterShellFileTypes(TCHAR *ext);

	void CheckBtn();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_OEXTREGDLG_H__850FECA7_ADEF_11D5_8505_00E018A83B1B__INCLUDED_)
