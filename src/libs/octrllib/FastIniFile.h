/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#if !defined(_FAST_INI_FILE_INCLUDED_)
#define _FAST_INI_FILE_INCLUDED_

class CFastIniFileImpl;

class CFastIniFile
{
public:
	CFastIniFile();
	~CFastIniFile();

	void SetIniFileName(const TCHAR *ini_file_name);

	bool LoadIniFile();
	bool SaveIniFile();

	unsigned int GetIniFileInt(const TCHAR *section, const TCHAR *entry, unsigned int ndefault);
	CString GetIniFileString(const TCHAR *section, const TCHAR *entry, const TCHAR *ndefault);
	bool WriteIniFileInt(const TCHAR *section, const TCHAR *entry, unsigned int nValue);
	bool WriteIniFileString(const TCHAR *section, const TCHAR *entry, const TCHAR *lpszValue);
	bool DeleteIniFileEntry(const TCHAR *section, const TCHAR *entry);

	static bool RegistryToIniFile(const TCHAR *reg_key, const TCHAR *prof_key, 
		const TCHAR *ini_file_name, const TCHAR *local_file_name);

private:
	class CFastIniFileImpl *p_impl;
};

#endif _FAST_INI_FILE_INCLUDED_
