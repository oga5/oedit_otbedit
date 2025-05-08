/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // oedit.h : OEDIT アプリケーションのメイン ヘッダー ファイル
//

#if !defined(AFX_OEDIT_H__52A23F45_4E07_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_OEDIT_H__52A23F45_4E07_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // メイン シンボル
#include "filenames.h"

/////////////////////////////////////////////////////////////////////////////
// COeditApp:
// このクラスの動作の定義に関しては oedit.cpp ファイルを参照してください。
//

#include "global.h"
#include "OWinApp.h"

class COeditApp : public COWinApp
{
public:
	COeditApp();
	~COeditApp();
	void SetEditMode(int edit_mode);

	virtual BOOL DoPromptFileName(CString& fileName, UINT nIDSTitle,
			DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate,
			int *kanji_code, int *line_type);

	CString GetCommandString(UINT nID);
	void RedrawEditCtrl();


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(COeditApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
	//}}AFX_VIRTUAL

// インプリメンテーション
	//{{AFX_MSG(COeditApp)
	afx_msg void OnAppAbout();
	afx_msg void OnOption();
	afx_msg void OnSetFont();
	afx_msg void OnFileOpen();
	afx_msg void OnNewWindow();
	afx_msg void OnExtReg();
	afx_msg void OnPlayKbmacro();
	afx_msg void OnRecKbmacro();
	afx_msg void OnUpdateRecKbmacro(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePlayKbmacro(CCmdUI* pCmdUI);
	afx_msg void OnKbmacroSave();
	afx_msg void OnUpdateKbmacroSave(CCmdUI* pCmdUI);
	afx_msg void OnKbmacroLoad();
	afx_msg void OnKbmacroPlay();
	afx_msg void OnToggleShowRowNum();
	afx_msg void OnToggleShowColNum();
	afx_msg void OnShowRegexpHelp();
	afx_msg void OnScmLoadFile();
	afx_msg void OnShowSchemeHelp();
	afx_msg void OnShowOeditmacroHelp();
	afx_msg void OnScmEvalClipboard();
	afx_msg void OnFontSizeLarge();
	afx_msg void OnFontSizeSmall();
	//}}AFX_MSG
	afx_msg BOOL OnOpenRecentFile(UINT nID);
	afx_msg void OnUpdateEditMode(CCmdUI* pCmdUI);
	afx_msg void OnEditMode(UINT nID);
	DECLARE_MESSAGE_MAP()

private:
	void LoadOption();
	void LoadEditMode();
	void LoadFontOption();
	void SaveOption();
	void SaveEditMode();
	void SaveOtherSettings();
	void SaveFontOption();
	void OptionChanged();

	void ProcessBootOption(CBootParameter &file_names);

	BOOL CreateFont();
	
	BOOL OpenFiles(CBootParameter &file_names, CCommandLineInfo &cmdInfo, TCHAR *cmd_line);
	void SetFileBootOption(CBootParameter &file_names, int idx);

	void ChangeFontSize(BOOL b_large);
public:
	afx_msg void OnFileBookmark();
	afx_msg void OnShowCredits();
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_OEDIT_H__52A23F45_4E07_11D5_8505_00E018A83B1B__INCLUDED_)
