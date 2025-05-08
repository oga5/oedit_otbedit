#if !defined(AFX_INPUTDLG_H__6FA0A366_9DFB_43ED_AB18_3C47E2A19842__INCLUDED_)
#define AFX_INPUTDLG_H__6FA0A366_9DFB_43ED_AB18_3C47E2A19842__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InputDlg.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CNameInputDlg ダイアログ

class CNameInputDlg : public CDialog
{
// コンストラクション
public:
	CNameInputDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ

	CString		m_title;

// ダイアログ データ
	//{{AFX_DATA(CNameInputDlg)
	enum { IDD = IDD_NAME_INPUT_DLG };
	CButton	m_ok;
	CString	m_data;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CNameInputDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CNameInputDlg)
	afx_msg void OnChangeEditData();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void CheckBtn();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_INPUTDLG_H__6FA0A366_9DFB_43ED_AB18_3C47E2A19842__INCLUDED_)
