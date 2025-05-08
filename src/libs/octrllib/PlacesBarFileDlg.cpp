/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // PlacesBarFileDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "PlacesBarFileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlacesBarFileDlg

IMPLEMENT_DYNAMIC(CPlacesBarFileDlg, CFileDialog)

CPlacesBarFileDlg::CPlacesBarFileDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
}


BEGIN_MESSAGE_MAP(CPlacesBarFileDlg, CFileDialog)
	//{{AFX_MSG_MAP(CPlacesBarFileDlg)
		// メモ -  ClassWizard はこの位置にマッピング用のマクロを追加または削除します。
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



#include <AfxPriv.h>
INT_PTR CPlacesBarFileDlg::DoModal()
{
	ASSERT_VALID(this);
	ASSERT(m_ofn.Flags & OFN_ENABLEHOOK);
	ASSERT(m_ofn.lpfnHook != NULL); // can still be a user hook

	// zero out the file buffer for consistent parsing later
	ASSERT(AfxIsValidAddress(m_ofn.lpstrFile, m_ofn.nMaxFile));
	DWORD nOffset = lstrlen(m_ofn.lpstrFile)+1;
	ASSERT(nOffset <= m_ofn.nMaxFile);
	memset(m_ofn.lpstrFile+nOffset, 0, (m_ofn.nMaxFile-nOffset)*sizeof(TCHAR));

	// WINBUG: This is a special case for the file open/save dialog,
	//  which sometimes pumps while it is coming up but before it has
	//  disabled the main window.
	HWND hWndFocus = ::GetFocus();
	BOOL bEnableParent = FALSE;
	m_ofn.hwndOwner = PreModal();
	AfxUnhookWindowCreate();
	if (m_ofn.hwndOwner != NULL && ::IsWindowEnabled(m_ofn.hwndOwner))
	{
		bEnableParent = TRUE;
		::EnableWindow(m_ofn.hwndOwner, FALSE);
	}

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	ASSERT(pThreadState->m_pAlternateWndInit == NULL);

	if (m_ofn.Flags & OFN_EXPLORER)
		pThreadState->m_pAlternateWndInit = this;
	else
		AfxHookWindowCreate(this);

	int nResult;
	if (m_bOpenFileDialog)
		nResult = ::GetOpenFileName(&m_ofn);
	else
		nResult = ::GetSaveFileName(&m_ofn);

	if (nResult) ASSERT(pThreadState->m_pAlternateWndInit == NULL);
	pThreadState->m_pAlternateWndInit = NULL;

	// WINBUG: Second part of special case for file open/save dialog.
	if (bEnableParent) ::EnableWindow(m_ofn.hwndOwner, TRUE);
	if (::IsWindow(hWndFocus)) ::SetFocus(hWndFocus);

	PostModal();
	return nResult ? nResult : IDCANCEL;
}

////////////////////////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------------
 ファイルダイアログ
------------------------------------------------------------------------------*/
BOOL DoFileDlg(LPCTSTR lpszTitle, BOOL bOpenFileDialog,
	LPCTSTR lpszDefExt, LPCTSTR lpszFileName, DWORD dwFlags,
	LPCTSTR lpszFilter, CWnd* pParentWnd, TCHAR *file_name)
{
	CPlacesBarFileDlg	file_dlg(bOpenFileDialog, lpszDefExt, lpszFileName,
		dwFlags, lpszFilter, pParentWnd);
	file_dlg.m_ofn.lpstrTitle = lpszTitle;

	// デフォルトディレクトリを設定
	TCHAR	cur_dir[_MAX_PATH];
	GetCurrentDirectory(sizeof(cur_dir)/sizeof(cur_dir[0]), cur_dir);
	file_dlg.m_ofn.lpstrInitialDir = cur_dir;

	if(file_dlg.DoModal() != IDOK) {
		return FALSE;
	}

	_tcscpy(file_name, file_dlg.GetPathName().GetBuffer(0));

	return TRUE;
}

/*------------------------------------------------------------------------------
 ファイルダイアログ(ディレクトリ指定)
------------------------------------------------------------------------------*/
BOOL DoFileDlg_SetDir(LPCTSTR lpszTitle, BOOL bOpenFileDialog,
	LPCTSTR lpszDefExt, LPCTSTR lpszFileName, DWORD dwFlags,
	LPCTSTR lpszFilter, CWnd* pParentWnd, TCHAR *file_name, CString &ini_dir)
{
	TCHAR	cur_dir[_MAX_PATH];
	GetCurrentDirectory(sizeof(cur_dir)/sizeof(cur_dir[0]), cur_dir);
	
	SetCurrentDirectory(ini_dir.GetBuffer(0));

	if(DoFileDlg(lpszTitle, bOpenFileDialog,
		lpszDefExt, lpszFileName, dwFlags,
		lpszFilter, pParentWnd, file_name) == FALSE) {
		SetCurrentDirectory(cur_dir);
		return FALSE;
	}

	GetCurrentDirectory(_MAX_PATH, ini_dir.GetBuffer(_MAX_PATH));
	ini_dir.ReleaseBuffer();

	SetCurrentDirectory(cur_dir);

	return TRUE;
}
