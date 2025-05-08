// oextregDlg.h : ヘッダー ファイル
//

#if !defined(AFX_OEXTREGDLG_H__850FECA7_ADEF_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_OEXTREGDLG_H__850FECA7_ADEF_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// COextregDlg ダイアログ

class COextregDlg : public CDialog
{
// 構築
public:
	COextregDlg(CWnd* pParent = NULL);	// 標準のコンストラクタ

// ダイアログ データ
	//{{AFX_DATA(COextregDlg)
	enum { IDD = IDD_OEXTREG_DIALOG };
	CButton	m_btn_update;
	CButton	m_btn_delete;
	CListCtrl	m_ext_list;
	BOOL	m_check_java;
	BOOL	m_check_txt;
	BOOL	m_ie_source_viewer;
	//}}AFX_DATA

	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(COextregDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV のサポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	HICON m_hIcon;
	HICON m_hSmallIcon;

	// 生成されたメッセージ マップ関数
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
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_OEXTREGDLG_H__850FECA7_ADEF_11D5_8505_00E018A83B1B__INCLUDED_)
