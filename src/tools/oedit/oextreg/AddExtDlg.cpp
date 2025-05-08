// AddExtDlg.cpp : �C���v�������e�[�V���� �t�@�C��
//

#include "stdafx.h"
#include "oextreg.h"
#include "AddExtDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddExtDlg �_�C�A���O


CAddExtDlg::CAddExtDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAddExtDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAddExtDlg)
	m_file_type_name = _T("");
	m_ext = _T("");
	m_explorer_rclick = FALSE;
	//}}AFX_DATA_INIT

	m_update = FALSE;
}


void CAddExtDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddExtDlg)
	DDX_Control(pDX, IDC_EDIT_EXT, m_edit_ext);
	DDX_Control(pDX, IDOK, m_ok);
	DDX_Text(pDX, IDC_EDIT_FILE_TYPE_NAME, m_file_type_name);
	DDX_Text(pDX, IDC_EDIT_EXT, m_ext);
	DDX_Check(pDX, IDC_CHECK_EXPLORER_RCLICK, m_explorer_rclick);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddExtDlg, CDialog)
	//{{AFX_MSG_MAP(CAddExtDlg)
	ON_EN_CHANGE(IDC_EDIT_EXT, OnChangeEditExt)
	ON_EN_CHANGE(IDC_EDIT_FILE_TYPE_NAME, OnChangeEditFileTypeName)
	ON_BN_CLICKED(IDC_CHECK_EXPLORER_RCLICK, OnCheckExplorerRclick)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddExtDlg ���b�Z�[�W �n���h��

void CAddExtDlg::OnChangeEditExt() 
{
	CheckBtn();
}

void CAddExtDlg::OnChangeEditFileTypeName() 
{
	CheckBtn();
}

BOOL CAddExtDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if(m_update) {
		m_edit_ext.SetReadOnly(TRUE);
		SetWindowText(_T("�֘A�t���̕ύX"));
	} else {
		SetWindowText(_T("�֘A�t���̒ǉ�"));
	}

	UpdateData(FALSE);
	
	return TRUE;  // �R���g���[���Ƀt�H�[�J�X��ݒ肵�Ȃ��Ƃ��A�߂�l�� TRUE �ƂȂ�܂�
	              // ��O: OCX �v���p�e�B �y�[�W�̖߂�l�� FALSE �ƂȂ�܂�
}

void CAddExtDlg::CheckBtn()
{
	UpdateData(TRUE);

	if(m_ext == _T("") || m_file_type_name == _T("")) {
		m_ok.EnableWindow(FALSE);
	} else {
		m_ok.EnableWindow(TRUE);
	}
}

void CAddExtDlg::OnCheckExplorerRclick() 
{
	CheckBtn();
}
