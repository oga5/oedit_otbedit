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
#include "QueryCloseDlg.h"
#include "FileFormatDlg.h"
#include "scm_macro.h"

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
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE, OnUpdateFileClose)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_CLOSE_ALL, OnFileCloseAll)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE_ALL, OnUpdateFileCloseAll)
	ON_COMMAND(ID_FILE_LOAD_UNICODE, OnFileLoadUnicode)
	ON_COMMAND(ID_FILE_LOAD_UTF8, OnFileLoadUtf8)
	ON_COMMAND(ID_FILE_LOAD_AUTO, OnFileLoadAuto)
	ON_COMMAND(ID_FILE_RELOAD, OnFileReload)
	ON_COMMAND(ID_SET_FILE_FORMAT, OnSetFileFormat)
	ON_COMMAND(ID_FILE_INSERT, OnFileInsert)
	ON_COMMAND(ID_FILE_SAVE_ALL, OnFileSaveAll)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_ALL, OnUpdateFileSaveAll)
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
}

COeditDoc::~COeditDoc()
{
}

BOOL COeditDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	int idx = AddDocData(_T("無題"));
	m_doc_data_arr.GetDocData(idx)->is_new_document = TRUE;
	ChangeDocData(m_doc_data_arr.GetCurrentDocDataId());
	SetModifiedFlag(FALSE);

	if(g_option.default_edit_mode == "NONE") {
		((COeditApp *)AfxGetApp)->SetEditMode(g_option.edit_mode);
	} else {
		((COeditApp *)AfxGetApp)->SetEditMode(g_lang_setting_list.GetSettingIdx(g_option.default_edit_mode));
	}
	UpdateAllViews(NULL, UPD_EDIT_MODE, 0);

	// TODO: この位置に再初期化処理を追加してください。
	// (SDI ドキュメントはこのドキュメントを再利用します。)
	DispFileType();

	return TRUE;
}

int COeditDoc::AddDocData(CString file_name)
{
	int idx = m_doc_data_arr.AddDocData(file_name);
	m_doc_data_arr.ChangeCurrentIdx(idx);

	UpdateAllViews(NULL, UPD_NEW_DOCUMENT, (CObject *)((INT_PTR)idx));

	GetEditData()->set_str_token(&g_str_token);
	GetEditData()->set_undo_cnt(INT_MAX);
	GetEditData()->get_disp_data()->SetDispPoolSize(128);

	SetKanjiCode(g_option.default_kanji_code);
	SetLineType(g_option.default_line_type);

	return idx;
}

/////////////////////////////////////////////////////////////////////////////
// COeditDoc シリアライゼーション

void COeditDoc::DispFileType()
{
	g_file_type_name = CUnicodeArchive::MakeFileType(GetKanjiCode(), GetLineType());
}

int COeditDoc::LoadFile(CArchive &ar, int kanji_code)
{
	int	line_type;

	// 文字コードが判定できなかったとき、オプションの新規ファイル作成時の
	// 文字コードを使用する
	if(kanji_code == UnknownKanjiCode) {
		kanji_code = CUnicodeArchive::CheckKanjiCode(&ar, g_option.default_kanji_code);
	}
	GetEditData()->load_file(ar, kanji_code, line_type);
	SetKanjiCode(kanji_code);
	SetLineType(line_type);

	DispFileType();

	return 0;
}

int COeditDoc::SaveFile(CArchive &ar)
{
	GetEditData()->save_file(ar, GetKanjiCode(), GetLineType());

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
	if(g_option.save_modified && GetEditData()->is_edit_data()) {
		SetModifiedFlag(TRUE);
	} else {
		SetModifiedFlag(FALSE);
	}
	UpdateAllViews(NULL, UPD_CHANGE_MODIFIED, 0);	

//	return CDocument::SaveModified();
	return TRUE;
}

void COeditDoc::CheckModified()
{
	if(IsModified()) {
		if(GetEditData()->is_edit_data() == FALSE) {
			SetModifiedFlag(FALSE);
			SetTitle(GetTitle().Left(GetTitle().GetLength() - 2));
			UpdateAllViews(NULL, UPD_CHANGE_MODIFIED, 0);
		}
	} else {
		if(GetEditData()->is_edit_data()) {
			SetModifiedFlag(TRUE);
			SetTitle(GetTitle() + " *");
			UpdateAllViews(NULL, UPD_CHANGE_MODIFIED, 0);	
		}
	}
}

BOOL COeditDoc::OnOpenDocumentMain(LPCTSTR lpszPathName)
{
	if (IsModified())
		TRACE0("Warning: OnOpenDocument replaces an unsaved document.\n");

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

	// 最初に無題のドキュメントのみで起動して、次に別のファイルを開いたとき、無題ドキュメントが未編集の場合は無題タブを削除する
	if(m_doc_data_arr.GetCurrentDocDataId() == 1 && GetPathName() == _T("無題") &&
		GetEditData()->is_edit_data() == FALSE) {
		DeleteDocData();
	}

	{
		int idx = m_doc_data_arr.FindData(long_name);
		if(idx != -1) {
			ChangeDocData(idx, TRUE, FALSE);
			return TRUE;
		}
	}

//	if (!CDocument::OnOpenDocument(long_name))
//		return FALSE;
	AddDocData(long_name);

	if (!OnOpenDocumentMain(long_name))
		return FALSE;

	GetEditData()->clear_cur_operation();

	int idx = g_lang_setting_list.GetSettingIdxFromFileName(long_name);
	if(idx >= 0) ((COeditApp *)AfxGetApp)->SetEditMode(idx);

	SetEditMode(g_option.edit_mode);

	ChangeDocData(m_doc_data_arr.GetCurrentDocDataId(), FALSE, FALSE);

	CWnd *pwnd = AfxGetMainWnd();
	if(pwnd->IsIconic()) {
		pwnd->ShowWindow(SW_RESTORE);
	}

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

			if(iBad == 2 && newName.GetAt(1) == ':') {
				TCHAR	fileName[_MAX_PATH];
				::GetFileTitle(m_strTitle, fileName, _MAX_PATH);
				newName = fileName;
			}
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

		int kanji_code = GetKanjiCode();
		int line_type = GetLineType();
		if (!((COeditApp *)AfxGetApp())->DoPromptFileName(newName,
		  bReplace ? AFX_IDS_SAVEFILE : AFX_IDS_SAVEFILECOPY,
		  OFN_NOREADONLYRETURN | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, FALSE, pTemplate,
		  &kanji_code, &line_type))
			return FALSE;       // don't even attempt to save

		SetKanjiCode(kanji_code);
		SetLineType(line_type);

		// 勝手に拡張子をつけない
		//AddExt(newName);
	}

	CWaitCursor wait;

	if (!OnSaveDocument(newName)) {
		if (lpszPathName == NULL) {
			if(_wunlink(newName.GetBuffer(0)) != 0) {
				TRACE0("Warning: failed to delete file after failed SaveAs.\n");
			}
		}
		return FALSE;
	}

	// reset the title and change the document name
	if (bReplace)
		SetPathName(newName);

	UpdateAllViews(NULL, UPD_CHANGE_MODIFIED, 0);

	DispFileType();
	m_doc_data_arr.GetCurrentDocData()->is_new_document = FALSE;

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

	scm_call_event_handler(g_sc, SCM_EVENT_ON_CLOSE_FILE);

	POINT pt = GetEditData()->get_cur_pt();
	POINT scr_pt;
	scr_pt.y = GetEditCtrl()->GetScrollPos(SB_VERT);
	scr_pt.x = GetEditCtrl()->GetScrollPos(SB_HORZ);

	GetEditCtrl()->ClearAll();

	CFile file(file_name, CFile::modeRead|CFile::shareDenyNone);
	CArchive ar(&file, CArchive::load);

	LoadFile(ar, kanji_code);

	ar.Close();
	file.Close();

	CheckModified();

	if(pt.y >= GetEditData()->get_row_cnt()) pt.y = GetEditData()->get_row_cnt() - 1;
	if(pt.x > GetEditData()->get_row_len(pt.y)) pt.x = GetEditData()->get_row_len(pt.y);
	GetEditData()->set_cur(pt.y, pt.x);

//	UpdateAllViews(NULL, UPD_REDRAW);
	{	// 読み直す前のカーソル位置，スクロール位置にする
		GetEditCtrl()->Redraw(TRUE);

		if(scr_pt.y > GetEditCtrl()->GetScrollLimit(SB_VERT)) {
			scr_pt.y = GetEditCtrl()->GetScrollLimit(SB_VERT);
		}
		if(scr_pt.x > GetEditCtrl()->GetScrollLimit(SB_HORZ)) {
			scr_pt.x = GetEditCtrl()->GetScrollLimit(SB_HORZ);
		}
		GetEditCtrl()->SetScrollPos(SB_VERT, scr_pt.y, TRUE);
		GetEditCtrl()->SetScrollPos(SB_HORZ, scr_pt.x, TRUE);

		GetEditCtrl()->Redraw(FALSE);
	}

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

void COeditDoc::OnFileReload() 
{
	ReloadFile(GetKanjiCode());
}

void COeditDoc::OnFileLoadUnicodeNoSignature() 
{
	ReloadFile(KANJI_CODE_UTF16LE_NO_SIGNATURE);
}

void COeditDoc::OnFileLoadUtf8NoSignature() 
{
	ReloadFile(KANJI_CODE_UTF8_NO_SIGNATURE);
}

void COeditDoc::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU) 
{
	// ロングファイル名にする
	TCHAR	long_name[_MAX_PATH];
	if(GetLongPath(lpszPathName, long_name) == FALSE) {
		_tcscpy(long_name, lpszPathName);
	}

	CDocument::SetPathName(long_name, bAddToMRU);
	
	m_doc_data_arr.GetCurrentDocData()->file_name = long_name;

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
	CString title;
	if(pathName.IsEmpty()) return;

	if(g_option.window_title_is_path_name) {
		title = pathName;
	} else {
		TCHAR	fileName[_MAX_PATH];
		::GetFileTitle(pathName, fileName, _MAX_PATH);
		title = fileName;
	}

	if(GetEditData()->is_edit_data()) {
		title += _T(" *");
		SetModifiedFlag(TRUE);
	} else {
		SetModifiedFlag(FALSE);
	}
	SetTitle(title);
	UpdateAllViews(NULL, UPD_CHANGE_MODIFIED, 0);
}

void COeditDoc::DeleteContents() 
{
	CDocument::DeleteContents();
}

BOOL COeditDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	BOOL ret_v = CDocument::OnSaveDocument(lpszPathName);

	if(ret_v == TRUE) {
		GetEditData()->clear_cur_operation();
		UpdateAllViews(NULL, UPD_INVALIDATE);
	}

	return ret_v;
}

BOOL COeditDoc::ChangeDocData(CString file_name)
{
	int idx = m_doc_data_arr.FindData(file_name);
	if(idx == -1) return FALSE;
	
	return ChangeDocData(idx);
}

BOOL COeditDoc::ChangeDocData(int idx, BOOL save_data, BOOL call_scm_tab_changed_handler)
{
	int old_idx = m_doc_data_arr.GetCurrentDocDataId();

	UpdateAllViews(NULL, UPD_PRE_CHANGE_EDIT_DATA, (CObject *)((INT_PTR)idx));

	if(save_data) {
		// 編集モードを保存
		SetEditMode(g_option.edit_mode);
	}

	// アクティブデータを切り替え
	m_doc_data_arr.ChangeCurrentIdx(idx);

	// 編集モードを設定
	((COeditApp *)AfxGetApp)->SetEditMode(GetEditMode());

	SetPathName(GetPathName(), FALSE);
	CheckModified();
	DispFileType();

	if(idx > 0) {
		UpdateAllViews(NULL, UPD_POST_CHANGE_EDIT_DATA, (CObject *)((INT_PTR)idx));

		// File Openのときは、on-open-fileハンドラを先に実行するので、ここでは実行しない
		if(call_scm_tab_changed_handler && old_idx != idx) {
			scm_call_event_handler(g_sc, SCM_EVENT_ON_CHANGE_TAB);
		}
		return TRUE;
	}

	return FALSE;
}

void COeditDoc::OnFileClose() 
{
	DeleteDocData();
}

BOOL COeditDoc::DeleteDocData()
{
	if(CDocument::SaveModified() == 0) return FALSE;

	// マクロを実行
	scm_call_event_handler(g_sc, SCM_EVENT_ON_CLOSE_FILE);

	m_doc_data_arr.DeleteDocData();
	UpdateAllViews(NULL, UPD_CLOSE_DOCUMENT, (CObject *)0);
	ChangeDocData(m_doc_data_arr.GetCurrentDocDataId(), FALSE);

	return TRUE;
}

#define ALL_SAVE	1
#define ALL_NOSAVE	2

BOOL COeditDoc::CloseArr(std::vector<int> *arr)
{
	int mode = 0;

	if(arr->empty()) return FALSE;

	int cur_idx = m_doc_data_arr.GetCurrentDocDataId();

	for(std::vector<int>::iterator it = arr->begin(); it != arr->end(); it++) {
		ChangeDocData(*it);

		if(mode == 0 && IsModified()) {
			MessageBeep(MB_ICONEXCLAMATION);
			CQueryCloseDlg	dlg;
			dlg.m_msg.Format(_T("%sへの変更を保存しますか？"), GetPathName());
			if(dlg.DoModal() == IDCANCEL) goto EXIT;

			if(dlg.m_result == IDALLYES) mode = ALL_SAVE;
			if(dlg.m_result == IDALLNO) mode = ALL_NOSAVE;
			if(dlg.m_result == IDYES) {
				if(!DoFileSave()) goto EXIT;
			}
			if(dlg.m_result == IDNO) SetModifiedFlag(FALSE);
		}
		if(mode == ALL_NOSAVE) SetModifiedFlag(FALSE);
		if(mode == ALL_SAVE) {
			if(!DoFileSave()) goto EXIT;
		}

		if(DeleteDocData() == FALSE) goto EXIT;
	}

	ChangeDocData(cur_idx);
	return TRUE;

EXIT:
	ChangeDocData(cur_idx);
	return FALSE;
}

BOOL COeditDoc::CloseAll()
{
	int mode = 0;

	for(; m_doc_data_arr.IsEmpty() == FALSE;) {
		if(mode == 0 && IsModified()) {
			MessageBeep(MB_ICONEXCLAMATION);
			CQueryCloseDlg	dlg;
			dlg.m_msg.Format(_T("%sへの変更を保存しますか？"), GetPathName());
			if(dlg.DoModal() == IDCANCEL) return FALSE;

			if(dlg.m_result == IDALLYES) mode = ALL_SAVE;
			if(dlg.m_result == IDALLNO) mode = ALL_NOSAVE;
			if(dlg.m_result == IDYES) {
				if(!DoFileSave()) return FALSE;
			}
			if(dlg.m_result == IDNO) SetModifiedFlag(FALSE);
		}
		if(mode == ALL_NOSAVE) SetModifiedFlag(FALSE);
		if(mode == ALL_SAVE) {
			if(!DoFileSave()) return FALSE;
		}

		if(DeleteDocData() == FALSE) return FALSE;
	}
	return TRUE;
}

BOOL COeditDoc::CanCloseFrame(CFrameWnd* pFrame) 
{
	if(CloseAll() == FALSE) return FALSE;

	return CDocument::CanCloseFrame(pFrame);
}

void COeditDoc::OnUpdateFileClose(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!m_doc_data_arr.IsEmpty());
}

void COeditDoc::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!m_doc_data_arr.IsEmpty());
}

void COeditDoc::OnUpdateFileSaveAs(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!m_doc_data_arr.IsEmpty());
}

BOOL COeditDoc::FileIsOpen(CString file_name)
{
	if(m_doc_data_arr.FindData(file_name) != -1) return TRUE;
	return FALSE;
}

BOOL COeditDoc::IsSaveAsNewFile()
{
	DWORD dwAttrib = GetFileAttributes(m_strPathName);

	if(m_strPathName == "" ||
		(is_file_exist(m_strPathName.GetBuffer(0)) && (dwAttrib & FILE_ATTRIBUTE_READONLY))) {
		return TRUE;
	}

	if(m_doc_data_arr.GetCurrentDocData()->is_new_document) return TRUE;
	
	return FALSE;
}

void COeditDoc::OnFileSave() 
{
	if(IsSaveAsNewFile()) {
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

void COeditDoc::OnFileCloseAll() 
{
	CloseAll();
}

void COeditDoc::OnUpdateFileCloseAll(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!m_doc_data_arr.IsEmpty());
}

void COeditDoc::OnSetFileFormat() 
{
	CFileFormatDlg	dlg;
	
	// 現在の設定を，ダイアログのコンボボックスに反映
	dlg.InitData(GetKanjiCode(), GetLineType());

	if(dlg.DoModal() != IDOK) return;

	int kanji_code, line_type;
	dlg.GetData(kanji_code, line_type);
	SetKanjiCode(kanji_code);
	SetLineType(line_type);

	DispFileType();	
}

CFile* COeditDoc::GetFile(LPCTSTR lpszFileName, UINT nOpenFlags,
	CFileException* pError)
{
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
	GetEditData()->append_file(ar, kanji_code, line_type);

	UpdateAllViews(NULL, UPD_REDRAW);
}

void COeditDoc::OnFileSaveAll() 
{
	UpdateAllViews(NULL, UPD_SAVE_ALL);
}

void COeditDoc::OnUpdateFileSaveAll(CCmdUI* pCmdUI) 
{
	if(m_doc_data_arr.IsEmpty()) {
		pCmdUI->Enable(FALSE);
	} else {
		pCmdUI->Enable(TRUE);
	}
}

void COeditDoc::SaveEditMode()
{
	SetEditMode(g_option.edit_mode);
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
	SetEditDataOption(GetEditData());
}

