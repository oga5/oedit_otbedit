/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "StdAfx.h"
#include "file_winutil.h"
#include "fileutil.h"

#include "ddeml.h"

//
// プログラム(Exe,Dll)のバージョンを取得
//
BOOL GetFileVersion2(TCHAR *filename, int *v1, int *v2, int *v3, int *v4)
{
	DWORD	dwVerInfoSize;
	DWORD	dwHnd;
	void	*pBuffer;
	VS_FIXEDFILEINFO	*pFixedInfo;
	UINT	uVersionLen;

	// 0で初期化
	*v1 = *v2 = *v3 = *v4 = 0;

	dwVerInfoSize = GetFileVersionInfoSize(filename, &dwHnd);
	if(dwVerInfoSize == 0) return FALSE;

	pBuffer = malloc(dwVerInfoSize);
	if(pBuffer == NULL) return FALSE;

	GetFileVersionInfo(filename, dwHnd, dwVerInfoSize, pBuffer);

	VerQueryValue(pBuffer, _T("\\"), (void **)&pFixedInfo, (UINT *)&uVersionLen);

	*v1 = HIWORD(pFixedInfo->dwFileVersionMS);
	*v2 = LOWORD(pFixedInfo->dwFileVersionMS);
	*v3 = HIWORD(pFixedInfo->dwFileVersionLS);
	*v4 = LOWORD(pFixedInfo->dwFileVersionLS);

	free(pBuffer);

	return TRUE;
}

//
// プログラム(Exe,Dll)のバージョンを取得
//
BOOL GetFileVersion(TCHAR *filename, CString *str, int format)
{
	int		v1, v2, v3, v4;

	if(!GetFileVersion2(filename, &v1, &v2, &v3, &v4)) return FALSE;

	// format == 0 なら X,X,X,X
	// format == 1 なら X.X
	switch(format) {
	case 0:
		str->Format(_T("%u.%u.%u.%u"), v1, v2, v3, v4);
		break;
	case 1:
		str->Format(_T("%u.%u"), v1, v2);
		break;
	default:
		break;
	}
	return TRUE;
}

CString GetAppVersion()
{
	TCHAR	filename[MAX_PATH];
	CString	file_version;

	GetModuleFileName(AfxGetInstanceHandle(), filename, sizeof(filename)/sizeof(filename[0]));
	GetFileVersion(filename, &file_version);

	return file_version;
}

//
// ブラウザを起動する
//
static BOOL ViewBrowserExecute(TCHAR *app_path, TCHAR *file)
{
	CString cmd;
	cmd.Format(_T("%s \"%s\""), app_path, file);

	PROCESS_INFORMATION		pi;
	STARTUPINFO				si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	if(CreateProcess(NULL, cmd.GetBuffer(0), NULL, NULL, FALSE,
		0, NULL, NULL, &si, &pi) == FALSE) {
		AfxMessageBox(_T("アプリケーションの起動に失敗しました。\n")
			_T("ツール→オプション→設定タブで，ブラウザの設定を確認してください。"),
			MB_OK);
		return FALSE;
	}

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	return TRUE;
}

static HDDEDATA CALLBACK DdeCallback(
  UINT uType,     // transaction type
  UINT uFmt,      // clipboard data format
  HCONV hconv,    // handle to the conversation
  HSZ hsz1,       // handle to a string
  HSZ hsz2,       // handle to a string
  HDDEDATA hdata, // handle to a global memory object
  ULONG_PTR dwData1,  // transaction-specific data
  ULONG_PTR dwData2   // transaction-specific data
)
{
	return 0;
}
 
//
// DDE通信でファイルを表示する
//
#define NS_OPENURL	_T("WWW_OpenURL")
#define NS_ACTIVATE	_T("WWW_Activate")
BOOL ViewBrowser(TCHAR *app, TCHAR *app_path, TCHAR *file, BOOL new_window)
{
	if(new_window) {
		return ViewBrowserExecute(app_path, file);
	}

	DWORD m_dwInstID = 0;
    if (DdeInitialize(&m_dwInstID, DdeCallback, APPCLASS_STANDARD | APPCMD_CLIENTONLY, 0L)) {
		AfxMessageBox(_T("DDEの初期化に失敗しました。"), MB_OK);
        return FALSE;
	}

	{
		HSZ hszService = DdeCreateStringHandle(m_dwInstID, app, CP_WINANSI);
		HSZ hszTopic   = DdeCreateStringHandle(m_dwInstID, NS_ACTIVATE, CP_WINANSI);
    
		HCONV hConv = DdeConnect(m_dwInstID, hszService, hszTopic, NULL);

		DdeFreeStringHandle(m_dwInstID, hszService);
		DdeFreeStringHandle(m_dwInstID, hszTopic);
		
		if(hConv == NULL) {
			// DDE通信に失敗したときは，アプリケーションを起動する
			DdeUninitialize(m_dwInstID);
			return ViewBrowserExecute(app_path, file);
		}

		// FIXME: Win98だとアクティブにならない
		HSZ hszActive = DdeCreateStringHandle(m_dwInstID, _T("0xFFFFFFFF,0x0"), CP_WINANSI);
		DdeClientTransaction(NULL, 0, hConv, hszActive, CF_TEXT, XTYP_REQUEST, 1000, NULL);
		DdeFreeStringHandle(m_dwInstID, hszActive);
		DdeDisconnect(hConv);
	}

	{
		HSZ hszService = DdeCreateStringHandle(m_dwInstID, app, CP_WINANSI);
		HSZ hszTopic   = DdeCreateStringHandle(m_dwInstID, NS_OPENURL, CP_WINANSI);
    
		HCONV hConv = DdeConnect(m_dwInstID, hszService, hszTopic, NULL);

		DdeFreeStringHandle(m_dwInstID, hszService);
		DdeFreeStringHandle(m_dwInstID, hszTopic);
		
		if(hConv == NULL) {
			// DDE通信に失敗したときは，アプリケーションを起動する
			DdeUninitialize(m_dwInstID);
			return ViewBrowserExecute(app_path, file);
		}

		CString	cmd;
		if(_tcsstr(file, _T("://")) == NULL) {
			cmd.Format(_T("\"file://%s\",,0xFFFFFFFF,0x03"), file);
		} else {
			cmd.Format(_T("\"%s\",,0xFFFFFFFF,0x03"), file);
		}
		HSZ hszOpen = DdeCreateStringHandle(m_dwInstID, cmd.GetBuffer(0), CP_WINANSI);
		//DdeClientTransaction(NULL, 0, hConv, hszOpen, CF_TEXT, XTYP_REQUEST, TIMEOUT_ASYNC, NULL);
		DdeClientTransaction(NULL, 0, hConv, hszOpen, CF_TEXT, XTYP_REQUEST, 1000, NULL);
		DdeFreeStringHandle(m_dwInstID, hszOpen);

		DdeDisconnect(hConv);
	}

	DdeUninitialize(m_dwInstID);

	return TRUE;
}

HWND FindWindowLoop(TCHAR *window_name)
{
	// アプリケーションをフォアグラウンドにする
	HWND	wnd = GetTopWindow(NULL);
	HWND	find = NULL;
	for(; wnd != NULL; ) {
		find = FindWindowEx(wnd, NULL, NULL, window_name);
		if(find != NULL) return find;
		wnd = GetNextWindow(wnd, GW_HWNDNEXT);
	}
	return NULL;
}

CDocument *GetActiveDocument()
{
	CWnd *pwnd = AfxGetMainWnd();
	if(pwnd->IsKindOf(RUNTIME_CLASS(CMDIFrameWnd)) == FALSE) return NULL;
	
	CMDIFrameWnd *pmdi_frame = (CMDIFrameWnd *)pwnd;
	CFrameWnd *pframe = pmdi_frame->GetActiveFrame();
	return pframe->GetActiveDocument();
}

CString GetFullPath(const TCHAR *file_name)
{
	// 相対パスをフルパスにする
	// FIXME: ファイルをオープンしないで，変換できるようにする
	CFile fff;
	if(!fff.Open(file_name, CFile::modeRead|CFile::shareDenyNone, NULL)) {
		// ファイルが開けない
		return file_name;
	}
	CString full_path_name = fff.GetFilePath();
	fff.Close();

	return full_path_name;
}

void OpenHelpFile(const TCHAR *file_name)
{
	CString file_path;
	file_path.Format(_T("%sdoc\\%s"), GetAppPath().GetBuffer(0), file_name);

	if(!is_file_exist(file_path)) {
		MessageBox(NULL, _T("ファイルが見つかりません"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	ShellExecute(NULL, _T("open"), file_path, NULL, NULL, SW_SHOW);
}


//
// DLLをロードするPATHを指定する (WindowsXP SP1以降)
//
typedef BOOL (WINAPI *FP_SetDllDirectory)(LPCTSTR lpPathName);

BOOL SetDllDir(const TCHAR *dir_name)
{
	HINSTANCE	dll = NULL;
	FP_SetDllDirectory		func = NULL;
	BOOL		b;

	if(!is_directory_exist(dir_name)) return FALSE;

	dll = LoadLibrary(_T("KERNEL32.DLL"));
	if(dll == NULL) return FALSE;

	func = (FP_SetDllDirectory)GetProcAddress(dll, "SetDllDirectoryW");
	if(func == NULL) goto ERR1;

	b = func(dir_name);

	FreeLibrary(dll);

	return b;

ERR1:
	if(dll) FreeLibrary(dll);
	return FALSE;
}

