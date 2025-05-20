// InputDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "resource.h"
#include "NameInputDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNameInputDlg ダイアログ


CNameInputDlg::CNameInputDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNameInputDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNameInputDlg)
	m_data = _T("");
	//}}AFX_DATA_INIT
}


void CNameInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNameInputDlg)
	DDX_Control(pDX, IDOK, m_ok);
	DDX_Text(pDX, IDC_EDIT_DATA, m_data);
	DDV_MaxChars(pDX, m_data, 100);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNameInputDlg, CDialog)
	//{{AFX_MSG_MAP(CNameInputDlg)
	ON_EN_CHANGE(IDC_EDIT_DATA, OnChangeEditData)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNameInputDlg メッセージ ハンドラ

void CNameInputDlg::OnChangeEditData() 
{
	CheckBtn();
}

void CNameInputDlg::CheckBtn()
{
	UpdateData(TRUE);

	m_ok.EnableWindow(!m_data.IsEmpty());
}

BOOL CNameInputDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	SetWindowText(m_title);
	
	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}
