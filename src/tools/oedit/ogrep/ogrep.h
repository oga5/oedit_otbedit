// ogrep.h : OGREP アプリケーションのメイン ヘッダー ファイルです。
//

#if !defined(AFX_OGREP_H__57F719A5_806D_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_OGREP_H__57F719A5_806D_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// メイン シンボル

#define OGREP_WINDOW_NAME	_T("OGREP_WINDOW_NAME")

/////////////////////////////////////////////////////////////////////////////
// COgrepApp:
// このクラスの動作の定義に関しては ogrep.cpp ファイルを参照してください。
//
#include "OWinApp.h"

class COgrepApp : public COWinApp
{
public:
	COgrepApp();

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(COgrepApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// インプリメンテーション

	//{{AFX_MSG(COgrepApp)
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

#endif // !defined(AFX_OGREP_H__57F719A5_806D_11D5_8505_00E018A83B1B__INCLUDED_)
