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

#include "EditData.h"
#include "AfxPriv.h"

#define UPD_NEW_DOCUMENT		100
#define UPD_INVALIDATE			102
#define UPD_EDITOR_OPTION		103
#define UPD_FONT				104
#define UPD_EDIT_MODE			105
#define UPD_DELETE_PANE			110
#define UPD_WINDOW_MOVED		111
#define UPD_SET_FOCUS			112
#define UPD_PRE_RELOAD_FILE		120
#define UPD_POST_RELOAD_FILE	121

class COeditDoc : public CDocument
{
protected: // シリアライズ機能のみから作成します。
	COeditDoc();
	DECLARE_DYNCREATE(COeditDoc)

// アトリビュート
public:

// オペレーション
public:
	CEditData *GetEditData() { return &m_edit_data; }
	void CheckModified();
	void SetDocTitle();

	void ReloadFile(int kanji_code);
	virtual BOOL DoSave(LPCTSTR lpszPathName, BOOL bReplace = TRUE);

	virtual CFile* GetFile(LPCTSTR lpszFileName, UINT nOpenFlags,
		CFileException* pError);

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
	virtual void OnCloseDocument();
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

protected:

// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(COeditDoc)
	afx_msg void OnFileLoadEuc();
	afx_msg void OnFileLoadSjis();
	afx_msg void OnFileLoadJis();
	afx_msg void OnFileSave();
	afx_msg void OnFileLoadUnicode();
	afx_msg void OnFileLoadUtf8();
	afx_msg void OnFileLoadAuto();
	afx_msg void OnFileReload();
	afx_msg void OnRedraw();
	afx_msg void OnFileInsert();
	afx_msg void OnFileLoadUnicodeNoSignature();
	afx_msg void OnFileLoadUtf8NoSignature();
	afx_msg void OnFileLoadUtf16be();
	afx_msg void OnFileLoadUtf16beNoSignature();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void RedrawEditCtrl();

private:
	CEditData	m_edit_data;
	int			m_kanji_code;
	int			m_line_type;

	void DispFileType();
	int LoadFile(CArchive &ar, int kanji_code = UnknownKanjiCode);
	int SaveFile(CArchive &ar);

	BOOL OnOpenDocumentMain(LPCTSTR lpszPathName);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_OEDITDOC_H__52A23F4B_4E07_11D5_8505_00E018A83B1B__INCLUDED_)
