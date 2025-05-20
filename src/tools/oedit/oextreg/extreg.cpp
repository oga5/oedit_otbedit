#include "StdAfx.h"
//#include "afxisapi.h"
#include "afxpriv.h"

static const TCHAR _afxShellOpenFmt[] = _T("%s\\shell\\open\\command");
static const TCHAR _afxShellPrintFmt[] = _T("%s\\shell\\print\\command");
static const TCHAR _afxShellPrintToFmt[] = _T("%s\\shell\\printto\\command");
static const TCHAR _afxDefaultIconFmt[] = _T("%s\\DefaultIcon");
static const TCHAR _afxDdeExecFmt[] = _T("%s\\shell\\open\\ddeexec");

static const TCHAR _afxShellNewFmt[] = _T("%s\\ShellNew");
static const TCHAR _afxShellNewValueName[] = _T("NullFile");
static const TCHAR _afxShellNewValue[] = _T("");
static const TCHAR _afxDdeExecValue[] = _T("[open(\"%1\")]");

static BOOL RegDeleteKeyRecursive(HKEY hk, LPCTSTR lpszKey)
{
	if(lpszKey == NULL || _tcslen(lpszKey) == 0) return FALSE;

	TCHAR			subkey[1024];
	DWORD			subkey_size;
	int				idx = 0;
	FILETIME		ft;
	HKEY			hk2;
	CStringArray	subkey_arr;

	if(RegOpenKeyEx(hk, lpszKey, 0, KEY_ENUMERATE_SUB_KEYS, &hk2) != ERROR_SUCCESS) {
		return FALSE;
	}

	for(idx = 0;; idx++) {
		_tcscpy(subkey, _T(""));
		subkey_size = sizeof(subkey);
		if(RegEnumKeyEx(hk2, idx, subkey, &subkey_size, NULL,
			NULL, NULL, &ft) != ERROR_SUCCESS) break;

		subkey_arr.Add(subkey);
	}

	for(idx = 0; idx < subkey_arr.GetSize(); idx++) {
		if(RegDeleteKeyRecursive(hk2, subkey_arr.GetAt(idx)) == FALSE) {
			return FALSE;
		}
	}
	RegCloseKey(hk2);

	if(RegDeleteKey(hk, lpszKey) != ERROR_SUCCESS) {
		return FALSE;
	}

	return TRUE;
}

BOOL DeleteRegKey(LPCTSTR lpszKey)
{
	// copy the string
	LPTSTR lpszKeyCopy = _tcsdup(lpszKey);
	LPTSTR lpszLast = lpszKeyCopy + lstrlen(lpszKeyCopy);

	// work until the end of the string
	while (lpszLast != NULL)
	{
		*lpszLast = '\0';
		lpszLast = _tcsdec(lpszKeyCopy, lpszLast);

		// try to open that key
		HKEY hKey;
		if (::RegOpenKey(HKEY_CLASSES_ROOT, lpszKeyCopy, &hKey) != ERROR_SUCCESS)
			break;

		// enumerate the keys underneath
		TCHAR szScrap[_MAX_PATH+1];
		DWORD dwLen = sizeof(szScrap)/sizeof(szScrap[0]);
		BOOL bItExists = FALSE;

		if (::RegEnumKey(hKey, 0, szScrap, dwLen) == ERROR_SUCCESS)
			bItExists = TRUE;
		::RegCloseKey(hKey);

		// found one?  quit looping
		if (bItExists)
			break;

		// otherwise, delete and find the previous backwhack
		::RegDeleteKey(HKEY_CLASSES_ROOT, lpszKeyCopy);
		lpszLast = _tcsrchr(lpszKeyCopy, '\\');
	}

	// release the string and return
	free(lpszKeyCopy);
	return TRUE;
}

static BOOL SetRegKey(HKEY hk, LPCTSTR lpszKey, LPCTSTR lpszValue, LPCTSTR lpszValueName = NULL)
{
	if (lpszValueName == NULL)
	{
		if (::RegSetValue(hk, lpszKey, REG_SZ,
			  lpszValue, lstrlen(lpszValue) * sizeof(TCHAR)) != ERROR_SUCCESS)
		{
			TRACE1("Warning: registration database update failed for key '%s'.\n",
				lpszKey);
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		HKEY hKey;

		if(::RegCreateKey(hk, lpszKey, &hKey) == ERROR_SUCCESS)
		{
			LONG lResult = ::RegSetValueEx(hKey, lpszValueName, 0, REG_SZ,
				(CONST BYTE*)lpszValue, (lstrlen(lpszValue) + 1) * sizeof(TCHAR));

			if(::RegCloseKey(hKey) == ERROR_SUCCESS && lResult == ERROR_SUCCESS)
				return TRUE;
		}
		TRACE1("Warning: registration database update failed for key '%s'.\n", lpszKey);
		return FALSE;
	}
}

CString MakeFilterExt(TCHAR *ext)
{
	CString	filterExt;
	if(ext[0] == '.') {
		filterExt = ext;
	} else {
		filterExt.Format(_T(".%s"), ext);
	}

	return filterExt;
}

CString GetFileTypeId(TCHAR *ext)
{
	HKEY			hk;
	CString			filterExt = MakeFilterExt(ext);
	DWORD			type;
	DWORD			dataSize;
	CString			fileTypeId = "";

	if(RegOpenKeyEx(HKEY_CLASSES_ROOT, filterExt, 0, KEY_QUERY_VALUE, &hk) != ERROR_SUCCESS) {
		return fileTypeId;
	}

	dataSize = _MAX_PATH * 2;
	LONG lResult = RegQueryValueEx(hk, _T(""), NULL, &type, (LPBYTE)fileTypeId.GetBuffer(dataSize), &dataSize);
	fileTypeId.ReleaseBuffer();
	RegCloseKey(hk);

	if(lResult != ERROR_SUCCESS || type != REG_SZ) {
		fileTypeId = _T("");
		return fileTypeId;
	}

	return fileTypeId;
}

CString GetFileTypeName(TCHAR *ext)
{
	HKEY			hk;
	CString			filterExt = MakeFilterExt(ext);
	CString			fileTypeId = GetFileTypeId(filterExt.GetBuffer(0));
	DWORD			type;
	DWORD			dataSize;
	CString			fileTypeName = _T("");

	if(RegOpenKeyEx(HKEY_CLASSES_ROOT, fileTypeId, 0, KEY_QUERY_VALUE, &hk) != ERROR_SUCCESS) {
		return fileTypeName;
	}

	dataSize = _MAX_PATH * 2;
	LONG lResult = RegQueryValueEx(hk, _T(""), NULL, &type, (LPBYTE)fileTypeName.GetBuffer(dataSize), &dataSize);
	RegCloseKey(hk);
	fileTypeName.ReleaseBuffer();

	if(lResult != ERROR_SUCCESS || type != REG_SZ) {
		fileTypeName = "";
		return fileTypeName;
	}

	return fileTypeName;
}

BOOL CheckExplorerRclick(TCHAR *ext)
{
	HKEY			hk, hk2;
	CString			filterExt = MakeFilterExt(ext);

	if(RegOpenKeyEx(HKEY_CLASSES_ROOT, filterExt, 0, KEY_QUERY_VALUE, &hk) != ERROR_SUCCESS) {
		return FALSE;
	}

	if(RegOpenKeyEx(hk, _T("ShellNew"), 0, KEY_QUERY_VALUE, &hk2) == ERROR_SUCCESS) {
		CString	nullFile;
		DWORD	dataSize = _MAX_PATH * 2;
		DWORD	type;

		LONG lResult = RegQueryValueEx(hk2, _T("NullFile"), NULL, &type, 
			(LPBYTE)nullFile.GetBuffer(dataSize), &dataSize);
		RegCloseKey(hk2);
		nullFile.ReleaseBuffer();
		
		if(lResult == ERROR_SUCCESS && type == REG_SZ && _tcscmp(nullFile, _T("")) == 0) {
			RegCloseKey(hk);
			return TRUE;
		}
	}

	RegCloseKey(hk);

	return FALSE;
}

void UnRegisterShellFileTypes(TCHAR *ext)
{
	CString	filterExt = MakeFilterExt(ext);
	CString fileTypeId = GetFileTypeId(filterExt.GetBuffer(0));

	CString strTemp;

	// コマンドを削除
	strTemp.Format(_afxShellOpenFmt, fileTypeId);
	DeleteRegKey(strTemp);
	strTemp.Format(_afxShellPrintFmt, fileTypeId);
	DeleteRegKey(strTemp);
	strTemp.Format(_afxShellPrintToFmt, fileTypeId);
	DeleteRegKey(strTemp);
	strTemp.Format(_afxDdeExecFmt, fileTypeId);
	DeleteRegKey(strTemp);

	// アイコンを削除
	strTemp.Format(_afxDefaultIconFmt, fileTypeId);
	DeleteRegKey(strTemp);

	// アイコンを削除
	strTemp.Format(_afxDefaultIconFmt, fileTypeId);
	RegDeleteKey(HKEY_CLASSES_ROOT, strTemp);

	// 拡張子を削除
	RegDeleteKeyRecursive(HKEY_CLASSES_ROOT, filterExt);
}

void RegisterShellFileTypes(TCHAR *ext, TCHAR *fileTypeId, TCHAR *fileTypeName, BOOL b_explorer_rclick,
	TCHAR *appFileName, TCHAR *iconFileName, int iconIdx, BOOL b_dde)
{
	UnRegisterShellFileTypes(ext);

	CString	filterExt = MakeFilterExt(ext);

	CString	strTemp;
	CString strPathName;

	// ドキュメントタイプを登録
	(void)SetRegKey(HKEY_CLASSES_ROOT, fileTypeId, fileTypeName);

	// ファイルオープンコマンドを登録
	strTemp.Format(_afxShellOpenFmt, fileTypeId);
	CString strOpenCommandLine;
	strOpenCommandLine.Format(_T("%s \"%%1\""), appFileName);
	(void)SetRegKey(HKEY_CLASSES_ROOT, strTemp, strOpenCommandLine);

	// アイコンを登録
	strTemp.Format(_afxDefaultIconFmt, fileTypeId);
	CString strDefaultIconCommandLine;
	strDefaultIconCommandLine.Format(_T("%s,%d"), iconFileName, iconIdx);
	(void)SetRegKey(HKEY_CLASSES_ROOT, strTemp, strDefaultIconCommandLine);

	// 拡張子を登録
	(void)SetRegKey(HKEY_CLASSES_ROOT, filterExt, fileTypeId);

	if(b_dde) {
		// DDEを登録
		strTemp.Format(_afxDdeExecFmt, fileTypeId);
		CString strDdeExecValue;
		strDdeExecValue.Format(_T("%s"), _afxDdeExecValue);
		(void)SetRegKey(HKEY_CLASSES_ROOT, strTemp, strDdeExecValue);
	}

	if(b_explorer_rclick) {
		// エクスプローラの右クリックメニューに登録
		strTemp.Format(_afxShellNewFmt, filterExt);
		(void)SetRegKey(HKEY_CLASSES_ROOT, strTemp, _afxShellNewValue, _afxShellNewValueName);
	}
}

BOOL CheckRegisterShellFileTypes(TCHAR *ext, TCHAR *fileTypeId)
{
	CString		filterExt = MakeFilterExt(ext);
	CString		strTemp = GetFileTypeId(filterExt.GetBuffer(0));

	if (strTemp.Compare(fileTypeId) == 0) {
		return TRUE;
	}

	return FALSE;
}


static const TCHAR _ieSourceViewer[] = _T("SOFTWARE\\Microsoft\\Internet Explorer\\View Source Editor");
static const TCHAR _ieSourceEditorName[] = _T("SOFTWARE\\Microsoft\\Internet Explorer\\View Source Editor\\Editor Name");

BOOL UnSetIESourceViewer()
{
	RegDeleteKeyRecursive(HKEY_LOCAL_MACHINE, _ieSourceViewer);

	return TRUE;
}

BOOL SetIESourceViewer(TCHAR *path)
{
//	RegDeleteKeyRecursive(HKEY_LOCAL_MACHINE, _ieSourceViewer);
	UnSetIESourceViewer();

	if(SetRegKey(HKEY_LOCAL_MACHINE, _ieSourceViewer, _T("")) == FALSE) return FALSE;

	if(SetRegKey(HKEY_LOCAL_MACHINE, _ieSourceEditorName, path, _T("")) == FALSE) return FALSE;

	return TRUE;
}

static BOOL GetRegValue(HKEY hk, const TCHAR *lpszKey, const TCHAR *lpszName, CString &value, DWORD &type)
{
	HKEY			hk2;
	DWORD			dataSize = _MAX_PATH * 2;

	if(RegOpenKeyEx(hk, lpszKey, 0, KEY_QUERY_VALUE, &hk2) != ERROR_SUCCESS) {
		return FALSE;
	}

	LONG lResult = RegQueryValueEx(hk2, lpszName, NULL, &type, 
		(LPBYTE)value.GetBuffer(dataSize), &dataSize);
	value.ReleaseBuffer();
	RegCloseKey(hk2);
	
	if(lResult != ERROR_SUCCESS) return FALSE;

	return TRUE;
}

BOOL CheckIESourceViewer(TCHAR *path)
{
	CString value;
	DWORD type;

	if(GetRegValue(HKEY_LOCAL_MACHINE, _ieSourceEditorName, _T(""), value, type) == FALSE) return FALSE;

	if(type == REG_SZ && value == path) return TRUE;

	return FALSE;
}
