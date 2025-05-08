/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#if !defined(AFX_PLACESBARFILEDLG_H__F899EBB2_FAB5_472C_B3D0_D9A9895236A0__INCLUDED_)
#define AFX_PLACESBARFILEDLG_H__F899EBB2_FAB5_472C_B3D0_D9A9895236A0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PlacesBarFileDlg.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CPlacesBarFileDlg ダイアログ

class CPlacesBarFileDlg : public CFileDialog
{
	DECLARE_DYNAMIC(CPlacesBarFileDlg)

public:
	CPlacesBarFileDlg(BOOL bOpenFileDialog, // TRUE ならば FileOpen、 FALSE ならば FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

protected:
	//{{AFX_MSG(CPlacesBarFileDlg)
		// メモ - ClassWizard はこの位置にメンバ関数を追加または削除します。
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	virtual INT_PTR DoModal();
};


BOOL DoFileDlg(LPCTSTR lpszTitle, BOOL bOpenFileDialog,
	LPCTSTR lpszDefExt, LPCTSTR lpszFileName, DWORD dwFlags,
	LPCTSTR lpszFilter, CWnd* pParentWnd, TCHAR *file_name);
BOOL DoFileDlg_SetDir(LPCTSTR lpszTitle, BOOL bOpenFileDialog,
	LPCTSTR lpszDefExt, LPCTSTR lpszFileName, DWORD dwFlags,
	LPCTSTR lpszFilter, CWnd* pParentWnd, TCHAR *file_name, CString &ini_dir);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_PLACESBARFILEDLG_H__F899EBB2_FAB5_472C_B3D0_D9A9895236A0__INCLUDED_)
