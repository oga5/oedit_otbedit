/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "OWinApp.h"
#include "fileutil.h"
#include "UnicodeArchive.h"
#include "EditData.h"

IMPLEMENT_DYNAMIC(COWinApp, CWinApp)

COWinApp *GetOWinApp()
{
	CWinApp *pApp = AfxGetApp();
	ASSERT(pApp->IsKindOf(RUNTIME_CLASS(COWinApp)));
	return (COWinApp *)pApp;
}

CString COWinApp::GetIniFileString( LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault)
{
	return m_fast_ini_file.GetIniFileString(lpszSection, lpszEntry, lpszDefault);
/*
	CString result = CWinApp::GetProfileString(lpszSection, lpszEntry, lpszDefault);
	CString result2 = m_fast_ini_file.GetIniFileString(lpszSection, lpszEntry, lpszDefault);
	if(result.Compare(result2) != 0) {
		TRACE("%s\\%s %s:%s\n", lpszSection, lpszEntry, result, result2);
	}
	return result2;
*/
}

UINT COWinApp::GetIniFileInt( LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault )
{
	return m_fast_ini_file.GetIniFileInt(lpszSection, lpszEntry, nDefault);
/*
	UINT result = CWinApp::GetProfileInt(lpszSection, lpszEntry, nDefault);
	UINT result2 = m_fast_ini_file.GetIniFileInt(lpszSection, lpszEntry, nDefault);
	if(result != result2) {
		TRACE("%s\\%s %d:%d\n", lpszSection, lpszEntry, result, result2);
	}
	return result2;
*/
}

BOOL COWinApp::WriteIniFileInt( LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue )
{
	return m_fast_ini_file.WriteIniFileInt(lpszSection, lpszEntry, nValue);
}

BOOL COWinApp::WriteIniFileString( LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue )
{
	return m_fast_ini_file.WriteIniFileString(lpszSection, lpszEntry, lpszValue);
}

CString COWinApp::GetCommandString(UINT nID)
{
	CString msg;
	msg.LoadString(nID);
	return msg;
}

static BOOL FileConvertToUnicode(const TCHAR *file_name)
{
	CEditData	edit_data;

	// load
	{
		int			kanji_code = UnknownKanjiCode;
		int			line_type = LINE_TYPE_CR_LF;

		CFile		file(file_name, CFile::modeRead|CFile::shareDenyNone);
		CArchive	ar(&file, CArchive::load);
		edit_data.load_file(ar, kanji_code, line_type);
		ar.Close();
		file.Close();
	}

	// store
	{
		CFile		file2(file_name, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone);
		CArchive	ar2(&file2, CArchive::store);
		edit_data.save_file(ar2, KANJI_CODE_UTF16LE, LINE_TYPE_LF);
		ar2.Close();
		file2.Close();
	}

	return TRUE;
}

void COWinApp::SetLocalIniFileName(const TCHAR *local_ini_file_name)
{
	if(m_pszProfileName != NULL) free((void*)m_pszProfileName);
	m_pszProfileName = _tcsdup(local_ini_file_name);

	// local iniファイルが存在しない場合、空のファイルをUNICODEで生成する
	// UNICODEで生成しないと、MRUリストにUNICODEファイル名が正しく保存されない
	if(!is_file_exist(local_ini_file_name)) {
		FILE *local_fp = _tfopen(local_ini_file_name, _T("wb"));
		if(local_fp != NULL) {
			write_utf16le_sig(local_fp);
			_ftprintf(local_fp, _T("\n"));
			fclose(local_fp);
		}
	} else {
		// iniファイルがSJISの場合は、UNICODEに変換する
		FILE *local_fp = _tfopen(local_ini_file_name, _T("rb"));
		if(local_fp != NULL) {
			if(!check_utf16le_signature_fp(local_fp)) {
				fclose(local_fp);
				FileConvertToUnicode(local_ini_file_name);
			}
			fclose(local_fp);
		}
	}
}

void COWinApp::UpdateAllDocViews(CView* pSender, LPARAM lHint, CObject* pHint)
{
	POSITION	doc_tmpl_pos = GetFirstDocTemplatePosition();
	for(; doc_tmpl_pos != NULL;) {
		CDocTemplate	*doc_tmpl = GetNextDocTemplate(doc_tmpl_pos);
		POSITION doc_pos = doc_tmpl->GetFirstDocPosition();
		for(; doc_pos != NULL; ) {
			doc_tmpl->GetNextDoc(doc_pos)->UpdateAllViews(pSender, lHint, pHint);
		}
	}
}
