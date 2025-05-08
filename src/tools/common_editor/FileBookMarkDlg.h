/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#pragma once


#include "OWinApp.h"


// CFileBookMark ダイアログ

class CFileBookMarkDlg : public CDialog
{
	DECLARE_DYNAMIC(CFileBookMarkDlg)

public:
	CFileBookMarkDlg(CWnd* pParent = nullptr);   // 標準コンストラクター
	virtual ~CFileBookMarkDlg();

	COWinApp*	m_app;
	CString			m_open_file_path;
	CStringArray	m_open_file_path_arr;
	BOOL		m_edit_flg;
	BOOL		m_b_multi_sel;

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BOOKMARK_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CListCtrl m_list_bookmark;

private:
	void LoadBookmark();
	void SaveBookmark();
	virtual void OnOK();
	virtual void OnCancel();

	BOOL IsExists(CString file_name, CString folder_name);
	int AddList(CString file_name, CString folder_name);
	void CheckBtn();
	void SwapRow(int i1, int i2);


public:
	afx_msg void OnClickedButtonAdd();
	afx_msg void OnClickedButtonDel();
	afx_msg void OnClickedButtonUp();
	afx_msg void OnClickedButtonDown();
	CButton m_btn_del;
	CButton m_btn_down;
	CButton m_btn_up;
	CButton m_ok;
	afx_msg void OnItemchangedListBookmark(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkListBookmark(NMHDR* pNMHDR, LRESULT* pResult);
};
