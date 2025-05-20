// ogrepDlg.h : ヘッダー ファイル
//

#if !defined(AFX_OGREPDLG_H__57F719A7_806D_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_OGREPDLG_H__57F719A7_806D_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EditCtrl.h"


#define SAVE_COMBO_CNT	10

/////////////////////////////////////////////////////////////////////////////
// COgrepDlg ダイアログ

class COgrepDlg : public CDialog
{
// 構築
public:
	COgrepDlg(CWnd* pParent = NULL);	// 標準のコンストラクタ

// ダイアログ データ
	//{{AFX_DATA(COgrepDlg)
	enum { IDD = IDD_OGREP_DIALOG };
	CComboBox	m_combo_kanji_code;
	CComboBox	m_combo_search_text;
	CComboBox	m_combo_search_folder;
	CComboBox	m_combo_file_type;
	CButton	m_btn_search;
	BOOL	m_check_distinct_lwr_upr;
	BOOL	m_check_regexp;
	BOOL	m_check_search_sub_folder;
	CString	m_file_type;
	CString	m_search_folder;
	CString	m_search_text;
	//}}AFX_DATA

	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(COgrepDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV のサポート
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	HICON m_hIcon;
	HICON m_hSmallIcon;

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(COgrepDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnEditchangeComboFileType();
	afx_msg void OnEditchangeComboSearchFolder();
	afx_msg void OnEditchangeComboSearchText();
	afx_msg void OnSelendokComboFileType();
	afx_msg void OnSelendokComboSearchFolder();
	afx_msg void OnSelendokComboSearchText();
	afx_msg void OnBtnSearch();
	afx_msg void OnBtnSelectFolder();
	virtual void OnCancel();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	//}}AFX_MSG
	afx_msg void OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI );
	DECLARE_MESSAGE_MAP()

private:
	CEditData		m_edit_data;
	CEditCtrl		m_edit_ctrl;
	CFont			m_font;
	BOOL			m_oedit_mode;
	int				m_kanji_code;

	struct _searched_data_st {
		CString			m_search_text;
		BOOL			m_check_distinct_lwr_upr;
		BOOL			m_check_distinct_width_ascii;
		BOOL			m_check_regexp;
	} m_searched_data;

	void CheckOeditMode();

	void CreateEditCtrl();
	void SetEditorOption();
	void CheckBtn(CString *search_text = NULL, CString *file_type = NULL, CString *search_folder = NULL);

	void LoadComboData(CComboBox *combo_box, CString section);
	void SetComboData(CComboBox *combo_box, CString data);
	void SaveComboData(CComboBox *combo_box, CString section);
	void SetFileType();
	BOOL GetSearchData(TCHAR *data, TCHAR *file_name, int *row);
	void OpenFile(TCHAR *file_name);

	void ShowSearchData(TCHAR *data);

	void SetSearchDataOedit(HWND hwnd, int row);
	void ShowSearchDataOedit(TCHAR *data);

	void SetSearchDataOtbedit(HWND hwnd, int row, TCHAR *file_name);
	void ShowSearchDataOtbedit(TCHAR *data);

	void LoadDialogPosition();
	void SaveDialogPosition();
public:
	BOOL m_check_distinct_width_ascii;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_OGREPDLG_H__57F719A7_806D_11D5_8505_00E018A83B1B__INCLUDED_)
