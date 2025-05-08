/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_PRINTDLG_H__32156BE7_4EA6_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_PRINTDLG_H__32156BE7_4EA6_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PrintDlg.h : ヘッダー ファイル
//

#include "resource.h"
#include "editdata.h"
#include "editctrl.h"

#include <winspool.h>

/////////////////////////////////////////////////////////////////////////////
// CPrintDlg ダイアログ

class CPrintDlg : public CDialog
{
// コンストラクション
public:
	CPrintDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ
	~CPrintDlg();

	static void DeleteBackupData();

	HDC		printer_dc;

	int		m_line_mode;
	int		m_line_len;
	int		m_font_point;
	int		m_row_num_digit;
	CString	m_printer_name;

// ダイアログ データ
	//{{AFX_DATA(CPrintDlg)
	enum { IDD = IDD_PRINT_DLG };
	CButton	m_btn_printer_setup;
	CButton	m_ok;
	CComboBox	m_combo_printer;
	int		m_space_bottom;
	int		m_space_left;
	int		m_space_right;
	int		m_space_top;
	BOOL	m_print_page;
	BOOL	m_print_title;
	CString	m_page_info;
	BOOL	m_print_row_num;
	CString	m_font_size;
	CString	m_font_face_name;
	BOOL	m_print_date;
	BOOL	m_print_selected_area;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CPrintDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CPrintDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnChangeEditSpaceBottom();
	afx_msg void OnChangeEditSpaceLeft();
	afx_msg void OnChangeEditSpaceRight();
	afx_msg void OnChangeEditSpaceTop();
	afx_msg void OnSelendokComboPrinter();
	afx_msg void OnCheckPrintRowNum();
	afx_msg void OnPrinterSetup();
	afx_msg void OnSetFont();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void SetPageInfo();
	void VerifySpace();

	BOOL CreatePrnInfo();
	void DeletePrnInfo();

	BOOL CreatePrinterDC();
	void DeletePrinterDC();

	void LoadPrintOption();
	void SavePrintOption();

	void CheckBtn();

	BOOL UpdateData(BOOL bSaveAndValidate = TRUE);


	PRINTER_INFO_2 *DuplicatePrnInfo(TCHAR *printer_name, PRINTER_INFO_2 *prninfo, DWORD prninfo_size);

	PRINTER_INFO_2	*m_prninfo;
	DWORD	m_prninfo_size;

	BOOL CreateBackupData();
	static struct backup_data_st {
		PRINTER_INFO_2	*prninfo;
		DWORD			prninfo_size;
		TCHAR			*printer_name;
	} m_backup_data;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_PRINTDLG_H__32156BE7_4EA6_11D5_8505_00E018A83B1B__INCLUDED_)
