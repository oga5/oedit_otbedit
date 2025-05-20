/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // oeditDoc.cpp : COeditDoc クラスの動作の定義を行います。
//

#include "stdafx.h"
#include "oedit.h"

#include "oeditDoc.h"

#include "fileutil.h"
#include "scm_macro.h"

//#include "AfxPriv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COeditDoc

IMPLEMENT_DYNCREATE(COeditDoc, CDocument)

BEGIN_MESSAGE_MAP(COeditDoc, CDocument)
	//{{AFX_MSG_MAP(COeditDoc)
	ON_COMMAND(ID_FILE_LOAD_EUC, OnFileLoadEuc)
	ON_COMMAND(ID_FILE_LOAD_SJIS, OnFileLoadSjis)
	ON_COMMAND(ID_FILE_LOAD_JIS, OnFileLoadJis)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_LOAD_UNICODE, OnFileLoadUnicode)
	ON_COMMAND(ID_FILE_LOAD_UTF8, OnFileLoadUtf8)
	ON_COMMAND(ID_FILE_LOAD_AUTO, OnFileLoadAuto)
	ON_COMMAND(ID_FILE_RELOAD, OnFileReload)
	ON_COMMAND(ID_REDRAW, OnRedraw)
	ON_COMMAND(ID_FILE_INSERT, OnFileInsert)
	ON_COMMAND(ID_FILE_LOAD_UNICODE_NO_SIGNATURE, OnFileLoadUnicodeNoSignature)
	ON_COMMAND(ID_FILE_LOAD_UTF8_NO_SIGNATURE, OnFileLoadUtf8NoSignature)
	ON_COMMAND(ID_FILE_LOAD_UTF16BE, OnFileLoadUtf16be)
	ON_COMMAND(ID_FILE_LOAD_UTF16BE_NO_SIGNATURE, OnFileLoadUtf16beNoSignature)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COeditDoc クラスの構築/消滅

COeditDoc::COeditDoc()
{
	m_edit_data.set_str_token(&g_str_token);
	m_edit_data.set_undo_cnt(INT_MAX);
	m_edit_data.get_disp_data()->SetDispPoolSize(128);

	m_kanji_code = g_option.default_kanji_code;
	m_line_type = g_option.default_line_type;
}

COeditDoc::~COeditDoc()
{
}

BOOL COeditDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: この位置に再初期化処理を追加してください。
	// (SDI ドキュメントはこのドキュメントを再利用します。)
	UpdateAllViews(NULL, UPD_NEW_DOCUMENT);

	if(g_option.default_edit_mode == "NONE") {
		((COeditApp *)AfxGetApp)->SetEditMode(g_option.edit_mode);
	} else {
		((COeditApp *)AfxGetApp)->SetEditMode(g_lang_setting_list.GetSettingIdx(g_option.default_edit_mode));
	}
	UpdateAllViews(NULL, UPD_EDIT_MODE, 0);

	DispFileType();

	// 共有メモリにファイル名を登録
	g_share_data.UpdateOpenFileName(AfxGetMainWnd()->GetSafeHwnd(), GetTitle());

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// COeditDoc シリアライゼーション

void COeditDoc::DispFileType()
{
	g_file_type_name = CUnicodeArchive::MakeFileType(m_kanji_code, m_line_type);
	scm_set_doc_line_type(m_line_type);
}

int COeditDoc::LoadFile(CArchive &ar, int kanji_code)
{
	m_kanji_code = kanji_code;

	// 文字コードが判定できなかったとき、オプションの新規ファイル作成時の
	// 文字コードを使用する
	if(m_kanji_code == UnknownKanjiCode) {
		m_kanji_code = CUnicodeArchive::CheckKanjiCode(&ar, g_option.default_kanji_code);
	}
	m_edit_data.load_file(ar, m_kanji_code, m_line_type);

	DispFileType();

	return 0;
}

int COeditDoc::SaveFile(CArchive &ar)
{
	m_edit_data.save_file(ar, m_kanji_code, m_line_type);

	return 0;
}

void COeditDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		SaveFile(ar);
	}
	else
	{
		LoadFile(ar, g_open_kanji_code);
	}
}

/////////////////////////////////////////////////////////////////////////////
// COeditDoc クラスの診断

#ifdef _DEBUG
void COeditDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void COeditDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// COeditDoc コマンド

BOOL COeditDoc::SaveModified() 
{
	if(g_option.save_modified && m_edit_data.is_edit_data()) {
		SetModifiedFlag(TRUE);
	} else {
		SetModifiedFlag(FALSE);
	}

	return CDocument::SaveModified();
}

void COeditDoc::CheckModified()
{
	if(IsModified()) {
		if(m_edit_data.is_edit_data() == FALSE) {
			SetModifiedFlag(FALSE);
			SetTitle(GetTitle().Left(GetTitle().GetLength() - 2));
		}
	} else {
		if(m_edit_data.is_edit_data()) {
			SetModifiedFlag(TRUE);
			SetTitle(GetTitle() + " *");
		}
	}
}

BOOL COeditDoc::OnOpenDocumentMain(LPCTSTR lpszPathName)
{
	if (IsModified())
		TRACE(_T("Warning: OnOpenDocument replaces an unsaved document.\n"));

	CFileException fe;
//	CFile* pFile = GetFile(lpszPathName,
//		CFile::modeRead|CFile::shareDenyWrite, &fe);
	// 他のプロセスがファイルを使っているときでも，ファイルを読めるようにする。
	CFile* pFile = GetFile(lpszPathName, CFile::modeRead|CFile::shareDenyNone, &fe);

	// 存在しないファイルを指定されたとき，空データで起動する
	if(pFile == NULL && fe.m_cause == CFileException::fileNotFound) {
		DeleteContents();
		SetModifiedFlag(FALSE);     // start off with unmodified
		return TRUE;
	}

	if (pFile == NULL)
	{
		ReportSaveLoadException(lpszPathName, &fe,
			FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		return FALSE;
	}

	// マクロを実行
	scm_call_event_handler(g_sc, SCM_EVENT_ON_CLOSE_FILE);

	DeleteContents();
	SetModifiedFlag();  // dirty during de-serialize

	CArchive loadArchive(pFile, CArchive::load | CArchive::bNoFlushOnDelete);
	loadArchive.m_pDocument = this;
	loadArchive.m_bForceFlat = FALSE;

	CWaitCursor wait;
	if (pFile->GetLength() != 0) Serialize(loadArchive);     // load me
	loadArchive.Close();
	ReleaseFile(pFile, FALSE);

/*
	TRY
	{
		CWaitCursor wait;
		if (pFile->GetLength() != 0)
			Serialize(loadArchive);     // load me
		loadArchive.Close();
		ReleaseFile(pFile, FALSE);
	}
	CATCH_ALL(e)
	{
		ReleaseFile(pFile, TRUE);
		DeleteContents();   // remove failed contents

		TRY
		{
			ReportSaveLoadException(lpszPathName, e,
				FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		}
		END_TRY
		e->Delete();
//		DELETE_EXCEPTION(e);
		return FALSE;
	}
	END_CATCH_ALL
*/

	SetModifiedFlag(FALSE);     // start off with unmodified

	return TRUE;
}

BOOL COeditDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	// ロングファイル名にする
	TCHAR	long_name[_MAX_PATH];
	if(GetLongPath(lpszPathName, long_name) == FALSE) {
		_tcscpy(long_name, lpszPathName);
	}

//	if (!CDocument::OnOpenDocument(long_name))
//		return FALSE;
	if (!OnOpenDocumentMain(long_name))
		return FALSE;

	m_edit_data.clear_cur_operation();

	TCHAR	ext[_MAX_PATH];
	_tsplitpath(long_name, NULL, NULL, NULL, ext);
	_tcslwr(ext);

	int idx = g_lang_setting_list.GetSettingIdxFromFileName(long_name);
	if(idx >= 0) ((COeditApp *)AfxGetApp)->SetEditMode(idx);
/*
	if(strcmp(ext, ".h") == 0 || strcmp(ext, ".c") == 0 || strcmp(ext, ".cpp") == 0) {
		((COeditApp *)AfxGetApp)->SetEditMode(EDIT_MODE_CPP);
	} else if(strcmp(ext, ".java") == 0 || strcmp(ext, ".js") == 0) {
		((COeditApp *)AfxGetApp)->SetEditMode(EDIT_MODE_JAVA);
	} else if(strcmp(ext, ".htm") == 0 || strcmp(ext, ".html") == 0 || strcmp(ext, ".shtml") == 0) {
		((COeditApp *)AfxGetApp)->SetEditMode(EDIT_MODE_HTML);
	} else if(strcmp(ext, ".sql") == 0) {
		((COeditApp *)AfxGetApp)->SetEditMode(EDIT_MODE_SQL);
	} else if(strcmp(ext, ".pl") == 0 || strcmp(ext, ".cgi") == 0) {
		((COeditApp *)AfxGetApp)->SetEditMode(EDIT_MODE_PERL);
	} else if(strcmp(ext, ".txt") == 0 || strcmp(ext, ".csv") == 0) {
		((COeditApp *)AfxGetApp)->SetEditMode(EDIT_MODE_TEXT);
	} else if(strstr(long_name, "Temporary Internet Files") != NULL) {	// for IE
		((COeditApp *)AfxGetApp)->SetEditMode(EDIT_MODE_HTML);
	}
*/
	UpdateAllViews(NULL, UPD_EDIT_MODE, 0);
	RedrawEditCtrl();

	// 共有メモリにファイル名を登録
	g_share_data.UpdateOpenFileName(AfxGetMainWnd()->GetSafeHwnd(), long_name);

	return TRUE;
}

static void AddExt(CString &newName)
{
	if(newName.FindOneOf(_T(".")) != -1) return;
	newName += g_lang_setting_list.GetSetting(g_option.edit_mode)->GetDefaultExt();
}

BOOL COeditDoc::DoSave(LPCTSTR lpszPathName, BOOL bReplace)
{
	CString newName = lpszPathName;

	if (newName.IsEmpty()) {
		CDocTemplate* pTemplate = GetDocTemplate();
		ASSERT(pTemplate != NULL);

		newName = m_strPathName;
		if (bReplace && newName.IsEmpty()) {
			newName = m_strTitle;
			// check for dubious filename
			int iBad = newName.FindOneOf(_T(" #%;/\\"));
			if (iBad != -1)
				newName.ReleaseBuffer(iBad);
/*
			// append the default suffix if there is one
			CString strExt;
			if (pTemplate->GetDocString(strExt, CDocTemplate::filterExt) &&
			  !strExt.IsEmpty()) {
				ASSERT(strExt[0] == '.');
				newName += strExt;
			}
*/
			AddExt(newName);
		}

		if (!((COeditApp *)AfxGetApp())->DoPromptFileName(newName,
		  bReplace ? AFX_IDS_SAVEFILE : AFX_IDS_SAVEFILECOPY,
		  OFN_NOREADONLYRETURN | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, FALSE, pTemplate,
		  &m_kanji_code, &m_line_type))
			return FALSE;       // don't even attempt to save

		// 勝手に拡張子をつけない
		//AddExt(newName);
	}

	CWaitCursor wait;

	if (!OnSaveDocument(newName)) {
		if (lpszPathName == NULL) {
			if(_wunlink(newName.GetBuffer(0)) != 0) {
				TRACE(_T("Warning: failed to delete file after failed SaveAs.\n"));
			}
		}
		return FALSE;
	}

	// reset the title and change the document name
	if (bReplace)
		SetPathName(newName);

	DispFileType();

	scm_call_event_handler(g_sc, SCM_EVENT_ON_FILE_SAVED);

	return TRUE;        // success
}

void COeditDoc::ReloadFile(int kanji_code)
{
	if(this->IsModified()) {
		if(AfxGetMainWnd()->MessageBox(_T("変更を破棄して読み直しますか？"), 
			_T("確認"), MB_YESNO | MB_ICONQUESTION) == IDNO) return;
	}

	CString		file_name = this->GetPathName();
	if(file_name == _T("")) {
		AfxMessageBox(_T("ファイルが存在しません。"), MB_OK);
		return;
	}

	UpdateAllViews(NULL, UPD_PRE_RELOAD_FILE);
	scm_call_event_handler(g_sc, SCM_EVENT_ON_CLOSE_FILE);

	POINT pt = m_edit_data.get_cur_pt();
	m_edit_data.del_all();

	CFile file(file_name, CFile::modeRead|CFile::shareDenyNone);
	CArchive ar(&file, CArchive::load);
	LoadFile(ar, kanji_code);

	ar.Close();
	file.Close();

	CheckModified();

	if(pt.y >= m_edit_data.get_row_cnt()) pt.y = m_edit_data.get_row_cnt() - 1;
	if(pt.x > m_edit_data.get_row_len(pt.y)) pt.x = m_edit_data.get_row_len(pt.y);
	m_edit_data.set_cur(pt.y, pt.x);

	UpdateAllViews(NULL, UPD_POST_RELOAD_FILE);
	scm_call_event_handler(g_sc, SCM_EVENT_ON_OPEN_FILE);
}

void COeditDoc::OnFileLoadEuc() 
{
	ReloadFile(KANJI_CODE_EUC);
}

void COeditDoc::OnFileLoadSjis() 
{
	ReloadFile(KANJI_CODE_SJIS);
}

void COeditDoc::OnFileLoadJis() 
{
	ReloadFile(KANJI_CODE_JIS);
}

void COeditDoc::OnFileLoadUnicode() 
{
	ReloadFile(KANJI_CODE_UTF16LE);
}

void COeditDoc::OnFileLoadUtf16be() 
{
	ReloadFile(KANJI_CODE_UTF16BE);
}

void COeditDoc::OnFileLoadUtf16beNoSignature() 
{
	ReloadFile(KANJI_CODE_UTF16BE_NO_SIGNATURE);
}

void COeditDoc::OnFileLoadUtf8() 
{
	ReloadFile(KANJI_CODE_UTF8);
}

void COeditDoc::OnFileLoadAuto() 
{
	ReloadFile(UnknownKanjiCode);
}


void COeditDoc::OnFileLoadUnicodeNoSignature() 
{
	ReloadFile(KANJI_CODE_UTF16LE_NO_SIGNATURE);
}

void COeditDoc::OnFileLoadUtf8NoSignature() 
{
	ReloadFile(KANJI_CODE_UTF8_NO_SIGNATURE);
}

void COeditDoc::OnFileReload() 
{
	ReloadFile(m_kanji_code);
}

void COeditDoc::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU) 
{
	// ロングファイル名にする
	TCHAR	long_name[_MAX_PATH];
	if(GetLongPath(lpszPathName, long_name) == FALSE) {
		_tcscpy(long_name, lpszPathName);
	}

	CDocument::SetPathName(long_name, bAddToMRU);

	SetDocTitle();

	// ファイルのあるディレクトリを、カレントディレクトリにする
	{
		TCHAR	drive[_MAX_PATH];
		TCHAR	dir[_MAX_PATH];

		_tsplitpath(long_name, drive, dir, NULL, NULL);

		CString path;
		path.Format(_T("%s%s"), drive, dir);
		SetCurrentDirectory(path.GetBuffer(0));
	}
}

void COeditDoc::SetDocTitle()
{
	CString pathName = GetPathName();
	if(pathName.IsEmpty()) return;

	if(g_option.window_title_is_path_name) {
		SetTitle(pathName);
	} else {
		TCHAR	fileName[_MAX_PATH];
		::GetFileTitle(pathName, fileName, _MAX_PATH);
		SetTitle(fileName);
	}
}

void COeditDoc::DeleteContents() 
{
	UpdateAllViews(NULL, UPD_NEW_DOCUMENT);
	
	CDocument::DeleteContents();
}

BOOL COeditDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
	
	BOOL ret_v = CDocument::OnSaveDocument(lpszPathName);

	if(ret_v == TRUE) {
		m_edit_data.clear_cur_operation();
		UpdateAllViews(NULL, UPD_INVALIDATE);
	}

	// 共有メモリにファイル名を登録
	g_share_data.UpdateOpenFileName(AfxGetMainWnd()->GetSafeHwnd(), lpszPathName);

	return ret_v;
}

void COeditDoc::OnFileSave() 
{
	DWORD dwAttrib = GetFileAttributes(m_strPathName);
	if(m_strPathName == "" ||
		(is_file_exist(m_strPathName.GetBuffer(0)) && (dwAttrib & FILE_ATTRIBUTE_READONLY)) ||
		!is_file_exist(m_strPathName.GetBuffer(0))) {
		// we do not have read-write access or the file does not (now) exist
		if (!DoSave(NULL)) {
			TRACE0("Warning: File save with new name failed.\n");
		}
	} else {
		if (!DoSave(m_strPathName)) {
			TRACE0("Warning: File save failed.\n");
		}
	}
}

void COeditDoc::RedrawEditCtrl()
{
	m_edit_data.get_disp_data()->SendNotifyMessage_OneWnd(EC_WM_REDRAW_ALLWND, FALSE, 0);
}

void COeditDoc::OnRedraw() 
{
	RedrawEditCtrl();
}

CFile* COeditDoc::GetFile(LPCTSTR lpszFileName, UINT nOpenFlags,
	CFileException* pError)
{
/*
	class CMyMirrorFile : public CMirrorFile
	{
	public:
		virtual void Close()
		{
			CString m_strName = m_strFileName; //file close empties string
			CFile::Close();

			if (!m_strMirrorName.IsEmpty()) {
				CFile::Remove(m_strName);
				CFile::Rename(m_strMirrorName, m_strName);
			}
		}
	};
*/
	CFile* pFile = new CFile;
	ASSERT(pFile != NULL);
	if (!pFile->Open(lpszFileName, nOpenFlags, pError))
	{
		delete pFile;
		pFile = NULL;
	}
	return pFile;
}

void COeditDoc::OnFileInsert() 
{
	CString newName;
	int kanji_code = g_open_kanji_code;
	if (!((COeditApp *)AfxGetApp())->DoPromptFileName(newName, 
		AFX_IDS_OPENFILE, OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, TRUE, NULL,
		&kanji_code, NULL)) {
		return; // open cancelled
	}

	CFile file(newName, CFile::modeRead|CFile::shareDenyNone);
	CArchive ar(&file, CArchive::load);

	int line_type;
	m_edit_data.append_file(ar, kanji_code, line_type);

	RedrawEditCtrl();
}

void COeditDoc::OnCloseDocument() 
{
	// マクロを実行
	scm_call_event_handler(g_sc, SCM_EVENT_ON_CLOSE_FILE);

	CDocument::OnCloseDocument();
}

void COeditDoc::SetEditDataOption(CEditData *edit_data)
{
	edit_data->set_word_wrap(g_option.text_editor.enable_word_wrap);
	if(g_option.text_editor.enable_confinement) {
		edit_data->set_first_confinement(g_option.first_confinement_str);
		edit_data->set_last_confinement(g_option.last_confinement_str);
	} else {
		edit_data->set_first_confinement(_T(""));
		edit_data->set_last_confinement(_T(""));
	}

	edit_data->set_indent_mode((INDENT_MODE)g_option.text_editor.indent_mode);
	if(g_lang_setting_list.GetSetting(g_option.edit_mode)->IsEnableCStyleIndent() == FALSE) {
		if(g_option.text_editor.indent_mode == INDENT_MODE_SMART) {
			edit_data->set_indent_mode(INDENT_MODE_AUTO);
		}
	}

	edit_data->set_tab_as_space(g_option.text_editor.tab_as_space);

	if(edit_data->get_tabstop() != g_option.text_editor.tabstop) {
		edit_data->set_tabstop(g_option.text_editor.tabstop);
	}
}

void COeditDoc::SetEditDataOption()
{
	SetEditDataOption(&m_edit_data);
}
