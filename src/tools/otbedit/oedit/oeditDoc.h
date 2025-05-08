/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // oeditDoc.h : COeditDoc クラスの宣言およびインターフェイスの定義をします。
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_OEDITDOC_H__52A23F4B_4E07_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_OEDITDOC_H__52A23F4B_4E07_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include "EditData.h"

#define UPD_NEW_DOCUMENT	100
#define UPD_CLOSE_DOCUMENT	101
#define UPD_REDRAW			110
#define UPD_INVALIDATE		111
#define UPD_EDITOR_OPTION	112
#define UPD_EDIT_MODE		114
#define UPD_WINDOW_MOVED	115
#define UPD_FONT			116

#define UPD_PRE_CHANGE_EDIT_DATA	120
#define UPD_POST_CHANGE_EDIT_DATA	121
#define UPD_TAB_MODE				125

#define UPD_CHANGE_MODIFIED			130

#define	UPD_SAVE_ALL				200

#include "editdocdata.h"

class COeditDoc : public CDocument
{
protected: // シリアライズ機能のみから作成します。
	COeditDoc();
	DECLARE_DYNCREATE(COeditDoc)

// アトリビュート
public:

// オペレーション
public:
	CEditData *GetEditData() { return &(m_doc_data_arr.GetCurrentDocData()->edit_data); }
	CCodeAssistEditCtrl *GetEditCtrl() { return &(m_doc_data_arr.GetCurrentDocData()->edit_ctrl); }

	void CheckModified();
	void SetDocTitle();

	virtual BOOL DoSave(LPCTSTR lpszPathName, BOOL bReplace = TRUE);

	virtual CFile* GetFile(LPCTSTR lpszFileName, UINT nOpenFlags,
		CFileException* pError);

	BOOL IsSaveAsNewFile();

	BOOL CloseArr(std::vector<int> *arr);

	void SetEditDataOption();
	void SetEditDataOption(CEditData *edit_data);

//オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(COeditDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);
	virtual void DeleteContents();
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
	protected:
	virtual BOOL SaveModified();
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~COeditDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	BOOL IsEmpty() { return m_doc_data_arr.IsEmpty(); }
	CString GetPathName() { return m_doc_data_arr.GetCurrentDocData()->file_name; }
	BOOL ChangeDocData(int idx, BOOL save_data = TRUE, BOOL call_scm_tab_changed_handler = TRUE);
	BOOL ChangeDocData(CString file_name);
	BOOL DeleteDocData();

	BOOL FileIsOpen(CString file_name);

	CString GetPathName_Idx(int idx) { return m_doc_data_arr.GetDocData(idx)->file_name; }
	BOOL IsModified_Idx(int idx) { return m_doc_data_arr.GetDocData(idx)->edit_data.is_edit_data(); }

	void ReloadFile(int kanji_code);
	void SaveEditMode();

protected:

// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(COeditDoc)
	afx_msg void OnFileLoadEuc();
	afx_msg void OnFileLoadSjis();
	afx_msg void OnFileLoadJis();
	afx_msg void OnFileClose();
	afx_msg void OnUpdateFileClose(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnFileSave();
	afx_msg void OnFileCloseAll();
	afx_msg void OnUpdateFileCloseAll(CCmdUI* pCmdUI);
	afx_msg void OnFileLoadUnicode();
	afx_msg void OnFileLoadUtf8();
	afx_msg void OnFileLoadAuto();
	afx_msg void OnFileReload();
	afx_msg void OnSetFileFormat();
	afx_msg void OnFileInsert();
	afx_msg void OnFileSaveAll();
	afx_msg void OnUpdateFileSaveAll(CCmdUI* pCmdUI);
	afx_msg void OnFileLoadUnicodeNoSignature();
	afx_msg void OnFileLoadUtf8NoSignature();
	afx_msg void OnFileLoadUtf16be();
	afx_msg void OnFileLoadUtf16beNoSignature();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CDocDataArray	m_doc_data_arr;

	int AddDocData(CString file_name);

	int GetKanjiCode() { return m_doc_data_arr.GetCurrentDocData()->kanji_code; }
	void SetKanjiCode(int kanji_code) { m_doc_data_arr.GetCurrentDocData()->kanji_code = kanji_code; }

	int GetLineType() { return m_doc_data_arr.GetCurrentDocData()->line_type; }
	void SetLineType(int line_type) { m_doc_data_arr.GetCurrentDocData()->line_type = line_type; }

	int GetEditMode() { return m_doc_data_arr.GetCurrentDocData()->edit_mode; }
	void SetEditMode(int edit_mode) { m_doc_data_arr.GetCurrentDocData()->edit_mode = edit_mode; }

	void DispFileType();
	int LoadFile(CArchive &ar, int kanji_code = UnknownKanjiCode);
	int SaveFile(CArchive &ar);

	BOOL OnOpenDocumentMain(LPCTSTR lpszPathName);

	BOOL CloseAll();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_OEDITDOC_H__52A23F4B_4E07_11D5_8505_00E018A83B1B__INCLUDED_)
