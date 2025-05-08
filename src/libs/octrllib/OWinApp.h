/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#if !defined(_OWINAPP_INCLUDED_)
#define _OWINAPP_INCLUDED_

#include "FastIniFile.h"

class COWinApp : public CWinApp
{
	DECLARE_DYNAMIC(COWinApp)

public:
	// 既存のAPIは実装しない(リンクエラーにする)
//	CString GetProfileString( LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault = NULL);
//	UINT GetProfileInt( LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault);
//	BOOL WriteProfileInt( LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue);
//	BOOL WriteProfileString( LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);

	CString GetIniFileString( LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault = NULL);
	UINT GetIniFileInt( LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault);
	BOOL WriteIniFileInt( LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue);
	BOOL WriteIniFileString( LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);

	void SetIniFileName(const TCHAR *ini_file_name) { m_fast_ini_file.SetIniFileName(ini_file_name); }
	bool LoadIniFile() { return m_fast_ini_file.LoadIniFile(); }
	bool SaveIniFile() { return m_fast_ini_file.SaveIniFile(); }

	void SetLocalIniFileName(const TCHAR *local_ini_file_name);

	static bool RegistryToIniFile(const TCHAR *reg_key, const TCHAR *prof_key, 
		const TCHAR *ini_file_name, const TCHAR *local_ini_file_name) {
		return CFastIniFile::RegistryToIniFile(reg_key, prof_key, ini_file_name, local_ini_file_name);
	}

	virtual CString GetCommandString(UINT nID);

	void UpdateAllDocViews(CView* pSender, LPARAM lHint, CObject* pHint);

private:
	CFastIniFile	m_fast_ini_file;
};

// AfxGetAppをGetOWinAppで置き換える
COWinApp *GetOWinApp();

#endif _OWINAPP_INCLUDED_
