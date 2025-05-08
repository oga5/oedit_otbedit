// oextreg.h : OEXTREG アプリケーションのメイン ヘッダー ファイルです。
//

#if !defined(AFX_OEXTREG_H__850FECA5_ADEF_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_OEXTREG_H__850FECA5_ADEF_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// メイン シンボル

#define OEXTREG_WINDOW_NAME	_T("OEXTREG_WINDOW_NAME")

#define APP_NAME	_T("oedit")


/////////////////////////////////////////////////////////////////////////////
// COextregApp:
// このクラスの動作の定義に関しては oextreg.cpp ファイルを参照してください。
//

class COextregApp : public CWinApp
{
public:
	COextregApp();

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(COextregApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// インプリメンテーション

	//{{AFX_MSG(COextregApp)
		// メモ - ClassWizard はこの位置にメンバ関数を追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	HANDLE	m_hMutex;
	BOOL CheckExecute();
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_OEXTREG_H__850FECA5_ADEF_11D5_8505_00E018A83B1B__INCLUDED_)
