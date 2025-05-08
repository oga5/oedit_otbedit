/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef __FILENAMES_H_INCLUDE__
#define __FILENAMES_H_INCLUDE__

class CFileNameOption
{
public:
	CFileNameOption(const TCHAR *file_name);

	CString GetFileName() { return m_file_name; }

	void SetOption(const TCHAR *key, const TCHAR *data) { m_option.SetAt(key, data); }
	CString GetOption(const TCHAR *key) {
		CString rstr;
		if(!m_option.Lookup(key, rstr)) return _T("");
		return rstr;
	}

private:
	CString		m_file_name;
	CMapStringToString	m_option;
	int			m_row;
};

class CBootParameter
{
private:
	CPtrArray			m_file_name_opt_arr;
	CMapStringToString	m_option;

	void AddFileName(const TCHAR *file_name);
	BOOL ParseOption(const TCHAR *&p1, TCHAR *msg_buf);
	CString ParseFileName(const TCHAR *&p1);
	CString ParseOptionData(const TCHAR *&p1);

public:
	CBootParameter();
	~CBootParameter();

	BOOL InitParameter(const TCHAR *cmd_line, TCHAR *msg_buf, BOOL file_name_flg = TRUE);

	INT_PTR GetFileCnt() { return m_file_name_opt_arr.GetSize(); }
	CFileNameOption *GetFileNameOption(int idx) { return ((CFileNameOption *)m_file_name_opt_arr.GetAt(idx)); }
	CString GetFileName(int idx) { return GetFileNameOption(idx)->GetFileName(); }

	CString GetOption(const TCHAR *key);

	CString GetBootDir() { return GetOption(_T("d")); }
};

#endif __FILENAMES_H_INCLUDE__
