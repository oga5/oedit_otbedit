#if !defined(AFX_ADDEXTDLG_H__9FFE1A84_AE73_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_ADDEXTDLG_H__9FFE1A84_AE73_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddExtDlg.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CAddExtDlg ダイアログ

class CAddExtDlg : public CDialog
{
// コンストラクション
public:
	CAddExtDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ

	BOOL	m_update;

// ダイアログ データ
	//{{AFX_DATA(CAddExtDlg)
	enum { IDD = IDD_ADD_EXT };
	CEdit	m_edit_ext;
	CButton	m_ok;
	CString	m_file_type_name;
	CString	m_ext;
	BOOL	m_explorer_rclick;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CAddExtDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
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
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_ADDEXTDLG_H__9FFE1A84_AE73_11D5_8505_00E018A83B1B__INCLUDED_)
