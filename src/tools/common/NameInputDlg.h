#if !defined(AFX_INPUTDLG_H__6FA0A366_9DFB_43ED_AB18_3C47E2A19842__INCLUDED_)
#define AFX_INPUTDLG_H__6FA0A366_9DFB_43ED_AB18_3C47E2A19842__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InputDlg.h : �w�b�_�[ �t�@�C��
//

/////////////////////////////////////////////////////////////////////////////
// CNameInputDlg �_�C�A���O

class CNameInputDlg : public CDialog
{
// �R���X�g���N�V����
public:
	CNameInputDlg(CWnd* pParent = NULL);   // �W���̃R���X�g���N�^

	CString		m_title;

// �_�C�A���O �f�[�^
	//{{AFX_DATA(CNameInputDlg)
	enum { IDD = IDD_NAME_INPUT_DLG };
	CButton	m_ok;
	CString	m_data;
	//}}AFX_DATA


// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CNameInputDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
protected:

	// �������ꂽ���b�Z�[�W �}�b�v�֐�
	//{{AFX_MSG(CNameInputDlg)
	afx_msg void OnChangeEditData();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void CheckBtn();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_INPUTDLG_H__6FA0A366_9DFB_43ED_AB18_3C47E2A19842__INCLUDED_)
