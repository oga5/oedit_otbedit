#if !defined(AFX_ADDEXTDLG_H__9FFE1A84_AE73_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_ADDEXTDLG_H__9FFE1A84_AE73_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddExtDlg.h : �w�b�_�[ �t�@�C��
//

/////////////////////////////////////////////////////////////////////////////
// CAddExtDlg �_�C�A���O

class CAddExtDlg : public CDialog
{
// �R���X�g���N�V����
public:
	CAddExtDlg(CWnd* pParent = NULL);   // �W���̃R���X�g���N�^

	BOOL	m_update;

// �_�C�A���O �f�[�^
	//{{AFX_DATA(CAddExtDlg)
	enum { IDD = IDD_ADD_EXT };
	CEdit	m_edit_ext;
	CButton	m_ok;
	CString	m_file_type_name;
	CString	m_ext;
	BOOL	m_explorer_rclick;
	//}}AFX_DATA


// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CAddExtDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
protected:

	// �������ꂽ���b�Z�[�W �}�b�v�֐�
	//{{AFX_MSG(CAddExtDlg)
	afx_msg void OnChangeEditExt();
	afx_msg void OnChangeEditFileTypeName();
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckExplorerRclick();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void CheckBtn();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_ADDEXTDLG_H__9FFE1A84_AE73_11D5_8505_00E018A83B1B__INCLUDED_)
