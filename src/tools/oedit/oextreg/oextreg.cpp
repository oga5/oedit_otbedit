// oextreg.cpp : アプリケーション用クラスの定義を行います。
//

#include "stdafx.h"
#include "oextreg.h"
#include "oextregDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if defined(UNICODE) || defined(_UNICODE)
#pragma comment(linker, "/entry:\"wWinMainCRTStartup\"")
#endif

/////////////////////////////////////////////////////////////////////////////
// COextregApp

BEGIN_MESSAGE_MAP(COextregApp, CWinApp)
	//{{AFX_MSG_MAP(COextregApp)
		// メモ - ClassWizard はこの位置にマッピング用のマクロを追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COextregApp クラスの構築

COextregApp::COextregApp()
{
	// TODO: この位置に構築用のコードを追加してください。
	// ここに InitInstance 中の重要な初期化処理をすべて記述してください。
	m_hMutex = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// 唯一の COextregApp オブジェクト

COextregApp theApp;

/////////////////////////////////////////////////////////////////////////////
// COextregApp クラスの初期化

BOOL COextregApp::InitInstance()
{
	// 標準的な初期化処理
	// もしこれらの機能を使用せず、実行ファイルのサイズを小さくしたけ
	//  れば以下の特定の初期化ルーチンの中から不必要なものを削除して
	//  ください。

	// ２重起動の防止
	if(CheckExecute() == FALSE) return FALSE;

#ifdef _AFXDLL
	Enable3dControls();			// 共有 DLL 内で MFC を使う場合はここをコールしてください。
#else
	Enable3dControlsStatic();	// MFC と静的にリンクする場合はここをコールしてください。
#endif

	COextregDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: ダイアログが <OK> で消された時のコードを
		//       記述してください。
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: ダイアログが <キャンセル> で消された時のコードを
		//       記述してください。
	}

	// ダイアログが閉じられてからアプリケーションのメッセージ ポンプを開始するよりは、
	// アプリケーションを終了するために FALSE を返してください。
	return FALSE;
}

BOOL COextregApp::CheckExecute()
{
	// 二重起動のチェック
	static const TCHAR* STR_MUTEX_NAME = _T("UNIQUE_STRING_OGAWA_OEXTREG");
	m_hMutex = ::CreateMutex( NULL, FALSE, STR_MUTEX_NAME );
	if ( ::GetLastError() == ERROR_ALREADY_EXISTS ) {
		::CloseHandle(m_hMutex);
		m_hMutex = NULL;

		// アプリケーションをフォアグラウンドにする
		HWND	wnd = GetTopWindow(NULL);
		HWND	find = NULL;
		for(; wnd != NULL; ) {
			find = FindWindowEx(wnd, NULL, NULL, OEXTREG_WINDOW_NAME);
			if(find != NULL) {
				if(GetParent(find) != NULL) {
					SetForegroundWindow(GetParent(find));
				}
				break;
			}
			wnd = GetNextWindow(wnd, GW_HWNDNEXT);
		}

		return FALSE;
	}

	return TRUE;
}

int COextregApp::ExitInstance() 
{
	if(m_hMutex != NULL) {
		::CloseHandle(m_hMutex);
		m_hMutex = NULL;
	}
	
	return CWinApp::ExitInstance();
}
