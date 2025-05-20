// ogrep.cpp : アプリケーション用クラスの定義を行います。
//

#include "stdafx.h"
#include "ogrep.h"
#include "ogrepDlg.h"
#include "fileutil.h"
#include "file_winutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static const char *THIS_FILE = __FILE__;
#endif


#if defined(UNICODE) || defined(_UNICODE)
#pragma comment(linker, "/entry:\"wWinMainCRTStartup\"")
#endif

/////////////////////////////////////////////////////////////////////////////
// COgrepApp

BEGIN_MESSAGE_MAP(COgrepApp, CWinApp)
	//{{AFX_MSG_MAP(COgrepApp)
		// メモ - ClassWizard はこの位置にマッピング用のマクロを追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COgrepApp クラスの構築

COgrepApp::COgrepApp()
{
	// TODO: この位置に構築用のコードを追加してください。
	// ここに InitInstance 中の重要な初期化処理をすべて記述してください。
	m_hMutex = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// 唯一の COgrepApp オブジェクト

COgrepApp theApp;

/////////////////////////////////////////////////////////////////////////////
// COgrepApp クラスの初期化

BOOL COgrepApp::InitInstance()
{
	// 標準的な初期化処理
	// もしこれらの機能を使用せず、実行ファイルのサイズを小さくしたけ
	//  れば以下の特定の初期化ルーチンの中から不必要なものを削除して
	//  ください。
	if(CheckExecute() == FALSE) return FALSE;

#ifdef _AFXDLL
	Enable3dControls();			// 共有 DLL 内で MFC を使う場合はここをコールしてください。
#else
// 古いMFCで必要だった処理
//	Enable3dControlsStatic();	// MFC と静的にリンクする場合はここをコールしてください。
#endif

	// INIファイルに保存する
	TCHAR ini_file_name[MAX_PATH];
	_stprintf(ini_file_name, _T("%sogrep.ini"), GetAppPath().GetBuffer(0));
	SetLocalIniFileName(ini_file_name);

	COgrepDlg dlg;
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

BOOL COgrepApp::CheckExecute()
{
	// 二重起動のチェック
	static const TCHAR* STR_MUTEX_NAME = _T("UNIQUE_STRING_OGAWA_OGREP");
	m_hMutex = ::CreateMutex( NULL, FALSE, STR_MUTEX_NAME );
	if ( ::GetLastError() == ERROR_ALREADY_EXISTS ) {
		if (m_hMutex != NULL) {
			::CloseHandle(m_hMutex);
			m_hMutex = NULL;
		}

		// アプリケーションをフォアグラウンドにする
		HWND	wnd = FindWindowLoop(OGREP_WINDOW_NAME);
		if(wnd != NULL) {
			if(GetParent(wnd) != NULL) {
				SetForegroundWindow(GetParent(wnd));
			}
		}

		return FALSE;
	}

	return TRUE;
}

int COgrepApp::ExitInstance() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
	if(m_hMutex != NULL) {
		::CloseHandle(m_hMutex);
		m_hMutex = NULL;
	}
	
	return CWinApp::ExitInstance();
}
