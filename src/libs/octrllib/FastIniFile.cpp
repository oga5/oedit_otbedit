/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "UnicodeArchive.h"
#include "FastIniFile.h"
#include "ostrutil.h"

#include <string>
#include <map>

#pragma warning( disable : 4786 )

class CFastIniFileImpl
{
public:
	CFastIniFileImpl();
	~CFastIniFileImpl();

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
	std::wstring m_ini_file_name;
	std::map<std::pair<std::wstring, std::wstring>, std::wstring>	m_ini_data;
	typedef std::map<std::pair<std::wstring, std::wstring>, std::wstring>::iterator ini_data_iter;

	HANDLE m_mutex;

	ini_data_iter SearchIniData(const TCHAR *section, const TCHAR *entry);
	void insert_data(const TCHAR *section, const TCHAR *entry, const TCHAR *data);

	static bool EnumRegistryValue(HKEY key, FILE *fp);
	static bool RegistryKeyToIniFile(HKEY parent_key, TCHAR *key_name, FILE *fp, FILE *local_fp);
};

CFastIniFileImpl::CFastIniFileImpl()
{
	m_mutex = NULL;
}

CFastIniFileImpl::~CFastIniFileImpl()
{
	if(m_mutex) CloseHandle(m_mutex);
	m_ini_data.clear();
}

void CFastIniFileImpl::SetIniFileName(const TCHAR *ini_file_name)
{
	m_ini_file_name = ini_file_name;

	CString mutex_name = ini_file_name;
	mutex_name.Replace('\\', '_');
	m_mutex = CreateMutex(NULL, FALSE, mutex_name);
}

void CFastIniFileImpl::insert_data(const TCHAR *section, const TCHAR *entry, const TCHAR *data)
{
	m_ini_data.insert(std::pair<std::pair<std::wstring, std::wstring>, std::wstring>
		(std::pair<std::wstring, std::wstring>(section, entry), data));
}

bool CFastIniFileImpl::LoadIniFile()
{
	DWORD dw = WaitForSingleObject(m_mutex, INFINITE);
	if(dw != WAIT_OBJECT_0) return false;

	int size_of_buf = 1024 * 8;

	TCHAR *buf = NULL;
	TCHAR *data_buf = NULL;
	TCHAR section[2048];
	TCHAR entry[2048];

	CUnicodeArchive uni_ar;
	if(!uni_ar.OpenFile(m_ini_file_name.c_str(), _T("r"))) goto ERR1;

	buf = (TCHAR *)malloc(size_of_buf * sizeof(TCHAR));
	if(buf == NULL) goto ERR1;
	data_buf = (TCHAR *)malloc(size_of_buf * sizeof(TCHAR));
	if(data_buf == NULL) goto ERR1;

	for(;;) {
		if(uni_ar.ReadLine(buf, size_of_buf) == NULL) break;
		ostr_chomp(buf, '\0');
		ostr_trimleft(buf);

		if(buf[0] == '\0') continue;		// 空行
		if(buf[0] == '#') continue;			// コメント行
		
		if(buf[0] == '[') {					// section
			TCHAR *close_p = _tcschr(buf, ']');
			if(close_p == NULL) {
				_tcscpy(section, buf + 1);
			} else {
				_sntprintf(section, sizeof(section)/sizeof(section[0]), _T("%.*s"), (int)(close_p - buf - 1), buf + 1);
			}
			continue;
		}

		TCHAR *p = _tcschr(buf, '=');
		if(p == NULL) continue;				// 無効な行

		_sntprintf(entry, sizeof(entry)/sizeof(entry[0]), _T("%.*s"), (int)(p - buf), buf);
		_tcscpy(data_buf, p + 1);

		insert_data(section, entry, data_buf);
	}

	if(buf != NULL) free(buf);
	if(data_buf != NULL) free(data_buf);
	ReleaseMutex(m_mutex);
	return true;

ERR1:
	if(buf != NULL) free(buf);
	if(data_buf != NULL) free(data_buf);
	ReleaseMutex(m_mutex);
	return false;
}

bool CFastIniFileImpl::SaveIniFile()
{
	DWORD dw = WaitForSingleObject(m_mutex, INFINITE);
	if(dw != WAIT_OBJECT_0) return false;

	FILE *fp = NULL;
	ini_data_iter p;
	TCHAR section[1024] = _T("");

	fp = _tfopen(m_ini_file_name.c_str(), _T("wb"));
	if(fp == NULL) goto ERR1;

	write_utf16le_sig(fp);

	for(p = m_ini_data.begin(); p != m_ini_data.end(); p++) {
		if(_tcscmp(section, p->first.first.c_str()) != 0) {
			// sectionの切り替え
			_tcscpy(section, p->first.first.c_str());
			_ftprintf(fp, _T("\n[%s]\n"), section);
		}
		const TCHAR *i = p->first.second.c_str();
		_ftprintf(fp, _T("%s=%s\n"), p->first.second.c_str(), p->second.c_str());
	}

	if(fp != NULL) fclose(fp);
	ReleaseMutex(m_mutex);
	return true;

ERR1:
	if(fp != NULL) fclose(fp);
	ReleaseMutex(m_mutex);
	return false;
}

CFastIniFileImpl::ini_data_iter CFastIniFileImpl::SearchIniData(const TCHAR *section, const TCHAR *entry)
{
	return m_ini_data.find(std::pair<std::wstring, std::wstring>(section, entry));
}

unsigned int CFastIniFileImpl::GetIniFileInt(const TCHAR *section, const TCHAR *entry, unsigned int ndefault)
{
	ini_data_iter p = SearchIniData(section, entry);
	if(p == m_ini_data.end()) return ndefault;
	return _ttoi(p->second.c_str());
}

CString CFastIniFileImpl::GetIniFileString(const TCHAR *section, const TCHAR *entry, const TCHAR *ndefault)
{
	ini_data_iter p = SearchIniData(section, entry);
	if(p == m_ini_data.end()) {
		if(ndefault == NULL) return _T("");
		return ndefault;
	}
	return p->second.c_str();
}

bool CFastIniFileImpl::DeleteIniFileEntry(const TCHAR *section, const TCHAR *entry)
{
	ini_data_iter p = SearchIniData(section, entry);
	if(p != m_ini_data.end()) m_ini_data.erase(p);
	return true;
}

bool CFastIniFileImpl::WriteIniFileInt(const TCHAR *section, const TCHAR *entry, unsigned int nValue)
{
	DeleteIniFileEntry(section, entry);

	TCHAR data_buf[1024];
	_sntprintf(data_buf, sizeof(data_buf)/sizeof(data_buf[0]), _T("%d"), nValue);
	insert_data(section, entry, data_buf);
	return true;
}

bool CFastIniFileImpl::WriteIniFileString(const TCHAR *section, const TCHAR *entry, const TCHAR *lpszValue)
{
	DeleteIniFileEntry(section, entry);

	// データがNULLだったら既存のエントリを削除するだけで終わり
	if(lpszValue == NULL) return true;

	insert_data(section, entry, lpszValue);
	return true;
}

bool CFastIniFileImpl::EnumRegistryValue(HKEY key, FILE *fp)
{
	int size_of_data = 1024 * 32;
	DWORD value_idx;
	TCHAR value_name[1024];
	DWORD value_name_size;
	BYTE *data = NULL;
	DWORD data_size;
	DWORD data_type;
	LONG result;

	data = (BYTE *)malloc(size_of_data);
	if(data == NULL) goto ERR1;

	for(value_idx = 0;; value_idx++) {
		value_name_size = sizeof(value_name)/sizeof(value_name[0]);
		data_size = size_of_data;
		result = RegEnumValue(key, value_idx, value_name, &value_name_size, NULL, 
			&data_type, data, &data_size);
		if(result == ERROR_NO_MORE_ITEMS) break;
		if(result != ERROR_SUCCESS) goto ERR1;

		switch(data_type) {
		case REG_DWORD:
			{
				DWORD *dp = reinterpret_cast<DWORD *>(data);
				DWORD d = *dp;
				_ftprintf(fp, _T("%s=%u\n"), value_name, d);
			}
			break;
		case REG_SZ:
			{
				TCHAR *str = reinterpret_cast<TCHAR *>(data);
				_ftprintf(fp, _T("%s=%s\n"), value_name, str);
			}
			break;
		default:
			goto ERR1;
		}
	}

	if(data != NULL) free(data);
	return true;

ERR1:
	if(data != NULL) free(data);
	return false;
}

bool CFastIniFileImpl::RegistryKeyToIniFile(HKEY parent_key, TCHAR *key_name, FILE *fp, FILE *local_fp)
{
	DWORD entry_idx;
	TCHAR entry_name[1024];
	DWORD entry_name_size;
	FILETIME entry_file_time;
	LONG result;
	HKEY hSubKey = NULL;
	TCHAR sub_key_name[1024] = _T("");

	// BarState-*とRecent File Listはlocal iniファイルへ移行する
	if(_tcsncmp(key_name, _T("BarState-"), 8) == 0 || _tcscmp(key_name, _T("Recent File List")) == 0) {
		fp = local_fp;
	}

	if(_tcslen(key_name) != 0) {
		_ftprintf(fp, _T("[%s]\n"), key_name);
	}

	if(!EnumRegistryValue(parent_key, fp)) goto ERR1;
	_ftprintf(fp, _T("\n"));

	for(entry_idx = 0;; entry_idx++) {
		entry_name_size = sizeof(entry_name)/sizeof(entry_name[0]);
		result = RegEnumKeyEx(parent_key, entry_idx, entry_name, &entry_name_size, NULL, 
			NULL, NULL, &entry_file_time);
		if(result == ERROR_NO_MORE_ITEMS) break;
		if(result != ERROR_SUCCESS) goto ERR1;

		if(RegOpenKeyEx(parent_key, entry_name, 0, KEY_READ, &hSubKey) != ERROR_SUCCESS) goto ERR1;
		if(_tcslen(key_name) != 0) {
			_sntprintf(sub_key_name, sizeof(sub_key_name)/sizeof(sub_key_name[0]), _T("%s\\%s"), key_name, entry_name);
		} else {
			_tcscpy(sub_key_name, entry_name);
		}
		if(!RegistryKeyToIniFile(hSubKey, sub_key_name, fp, local_fp)) goto ERR1;
	}

	if(hSubKey != NULL) RegCloseKey(hSubKey);
	return true;

ERR1:
	if(hSubKey != NULL) RegCloseKey(hSubKey);
	return false;
}

bool CFastIniFileImpl::RegistryToIniFile(const TCHAR *reg_key, const TCHAR *prof_key, 
	const TCHAR *ini_file_name, const TCHAR *local_ini_file_name)
{
	HKEY hSoftKey = NULL;
	HKEY hRegKey = NULL;
	HKEY hProfKey = NULL;
	FILE *fp = NULL;
	FILE *local_fp = NULL;

	fp = _tfopen(ini_file_name, _T("wb"));
	if(fp == NULL) goto ERR1;
	local_fp = _tfopen(local_ini_file_name, _T("wb"));
	if(local_fp == NULL) goto ERR1;

	write_utf16le_sig(fp);
	write_utf16le_sig(local_fp);

	if(RegOpenKeyEx(HKEY_CURRENT_USER, _T("software"), 0, KEY_READ, &hSoftKey) != ERROR_SUCCESS) goto ERR2;
	if(RegOpenKeyEx(hSoftKey, reg_key, 0, KEY_READ, &hRegKey) != ERROR_SUCCESS) goto ERR2;
	if(RegOpenKeyEx(hRegKey, prof_key, 0, KEY_READ, &hProfKey) != ERROR_SUCCESS) goto ERR2;

	if(!RegistryKeyToIniFile(hProfKey, _T(""), fp, local_fp)) goto ERR1;

	if(fp != NULL) fclose(fp);
	if(local_fp != NULL) fclose(local_fp);
	if(hProfKey != NULL) RegCloseKey(hProfKey);
	if(hRegKey != NULL) RegCloseKey(hRegKey);
	if(hSoftKey != NULL) RegCloseKey(hSoftKey);
	return true;

ERR2:
	if(fp != NULL) fclose(fp);
	if(local_fp != NULL) fclose(local_fp);
	if(hProfKey != NULL) RegCloseKey(hProfKey);
	if(hRegKey != NULL) RegCloseKey(hRegKey);
	if(hSoftKey != NULL) RegCloseKey(hSoftKey);
	return true;

ERR1:
	if(fp != NULL) fclose(fp);
	if(local_fp != NULL) fclose(local_fp);
	if(hProfKey != NULL) RegCloseKey(hProfKey);
	if(hRegKey != NULL) RegCloseKey(hRegKey);
	if(hSoftKey != NULL) RegCloseKey(hSoftKey);
	return false;
}


CFastIniFile::CFastIniFile()
{
	p_impl = new CFastIniFileImpl();
}

CFastIniFile::~CFastIniFile()
{
	delete p_impl;
}

void CFastIniFile::SetIniFileName(const TCHAR *ini_file_name)
{
	p_impl->SetIniFileName(ini_file_name);
}

bool CFastIniFile::LoadIniFile()
{
	return p_impl->LoadIniFile();
}

bool CFastIniFile::SaveIniFile()
{
	return p_impl->SaveIniFile();
}

unsigned int CFastIniFile::GetIniFileInt(const TCHAR *section, const TCHAR *entry, unsigned int ndefault)
{
	return p_impl->GetIniFileInt(section, entry, ndefault);
}

CString CFastIniFile::GetIniFileString(const TCHAR *section, const TCHAR *entry, const TCHAR *ndefault)
{
	return p_impl->GetIniFileString(section, entry, ndefault);
}

bool CFastIniFile::WriteIniFileInt(const TCHAR *section, const TCHAR *entry, unsigned int nValue)
{
	return p_impl->WriteIniFileInt(section, entry, nValue);
}

bool CFastIniFile::WriteIniFileString(const TCHAR *section, const TCHAR *entry, const TCHAR *lpszValue)
{
	return p_impl->WriteIniFileString(section, entry, lpszValue);
}

bool CFastIniFile::DeleteIniFileEntry(const TCHAR *section, const TCHAR *entry)
{
	return p_impl->DeleteIniFileEntry(section, entry);
}

bool CFastIniFile::RegistryToIniFile(const TCHAR *reg_key, const TCHAR *prof_key, 
	const TCHAR *ini_file_name, const TCHAR *local_file_name)
{
	return CFastIniFileImpl::RegistryToIniFile(reg_key, prof_key, ini_file_name, local_file_name);
}
