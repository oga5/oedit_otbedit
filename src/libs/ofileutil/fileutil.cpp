/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"

#include "fileutil.h"

#include "mbutil.h"

//#include <afxdllx.h>
#include <shlobj.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <time.h>
#ifdef WIN32
#include <direct.h>
#endif

//
// フォルダ選択ダイアログのコールバック
// フォルダの初期値を設定
static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lParam,LPARAM lpData)
{
	if(uMsg == BFFM_INITIALIZED){
		SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)TRUE,lpData);
	}
	return 0;
}

//
// フォルダ選択ダイアログ
//
LRESULT SelectFolder(HWND hWnd, TCHAR *folder, const TCHAR *base_dir, const TCHAR *title)
{
	BROWSEINFO		bR;
	LPITEMIDLIST	pidlBrowse;
	TCHAR			buf[MAX_PATH] = _T("");
	LPMALLOC		pMalloc;
    
	SHGetMalloc(&pMalloc);

	bR.hwndOwner = hWnd;
	bR.pszDisplayName = buf;
	bR.pidlRoot = NULL;
	bR.lpszTitle = title;
	bR.ulFlags = BIF_RETURNONLYFSDIRS;
	bR.lpfn = &BrowseCallbackProc;
	bR.lParam = (LPARAM)base_dir;

	pidlBrowse = SHBrowseForFolder(&bR);

	if (pidlBrowse != NULL ) {
		SHGetPathFromIDList(pidlBrowse, folder);

		pMalloc->Free(pidlBrowse);
		pMalloc->Release();
		return TRUE;
	} else {
		pMalloc->Release();
		return FALSE;
	}
}

/*------------------------------------------------------------------------*/
/*    ディレクトリ名の最後に'\'を付ける                                   */
/*------------------------------------------------------------------------*/
void make_dirname(TCHAR *dir)
{
	make_dirname2(dir);
	_tcscat(dir, _T("\\"));
}

/*------------------------------------------------------------------------*/
/*    ディレクトリ名の最後の'\'('/')を取る                                */
/*------------------------------------------------------------------------*/
void make_dirname2(TCHAR *dir)
{
	TCHAR	*p;

	/* '\'はSJISの2バイト目でも使われるので，文字列の前から調べていく */
	for(p = dir; p[0] != '\0'; p++) {
		/* 2バイト文字はスキップ */
		if(is_lead_byte(*p) == 1) {
			p++;
			if(*p == '\0') break;
			continue;
		}

		/* 文字列の最後が'\'のときは，NULLにする */
		if(p[1] == '\0') {
#ifdef WIN32
			if(p[0] == '\\') {
				p[0] = '\0';				
			}
#else
			if(p[0] == '/') {
				p[0] = '\0';				
			}
#endif
			break;
		}
	}
}

/*------------------------------------------------------------------------*/
/*    親ディレクトリ名を作成                                              */
/*------------------------------------------------------------------------*/
void make_parent_dirname(TCHAR *dir)
{
	TCHAR	*p;
	INT_PTR		last_sepa_pos = 0;

	// 末尾の'\'を取り除く
	make_dirname2(dir);

	/* '\'はSJISの2バイト目でも使われるので，文字列の前から調べていく */
	for(p = dir; p[0] != '\0'; p++) {
		/* 2バイト文字はスキップ */
		if(is_lead_byte(*p) == 1) {
			p++;
			continue;
		}

#ifdef WIN32
		if(p[0] == '\\') last_sepa_pos = p - dir;
#else
		if(p[0] == '/') last_sepa_pos = p - dir;
#endif
	}

	if(last_sepa_pos != 0) dir[last_sepa_pos] = '\0';
}

//
// アプリケーションのパスを取得
//
CString GetAppPath()
{
	DWORD dwRet;
	TCHAR path_buffer[_MAX_PATH];
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	CString path;

	dwRet = GetModuleFileName(NULL, path_buffer, sizeof(path_buffer)/sizeof(path_buffer[0]));
	// path_buffer に 実行モジュールのフルパスが格納されます。
	if(dwRet != 0) {// パスの取得に成功した場合、
		_tsplitpath( path_buffer, drive, dir, NULL, NULL);
		// drive には "c:" のようにドライブ名が
		// dir には "\test\mydir\" のように絶対パスが格納されます。

		path.Format(_T("%s%s"), drive, dir);
		// path に実行モジュールのある絶対パスが出来上がりです。
	}

	return path;
}

CString GetShortAppPath()
{
	CString path = GetAppPath();
	CString short_path;

	GetShortPathName(path, short_path.GetBuffer(_MAX_PATH), _MAX_PATH);
	short_path.ReleaseBuffer();
	return short_path;
}

//
// ロングファイル名を取得
//
BOOL GetLongPath(const TCHAR *short_name, TCHAR *long_name)
{
	HANDLE				hFind;
	WIN32_FIND_DATA		find_data;
	TCHAR				*p1, *p2;
	TCHAR				find_path[_MAX_PATH * 2] = _T("");
	TCHAR				file_name[_MAX_PATH * 2] = _T("");

	if(short_name[0] == '\\' && short_name[1] == '\\') {
		// UNCパス(ネットワークドライブ)
		p1 = (TCHAR *)short_name + 2;
		p1 = my_mbschr(p1, '\\');
		if(p1 == NULL) return FALSE;
		p1++;
		p1 = my_mbschr(p1, '\\');
		if(p1 == NULL) return FALSE;
		_stprintf(find_path, _T("%.*s"), (int)(p1 - short_name), short_name);
		p1++;
	} else if(short_name[0] != '\0' && short_name[1] == ':' && short_name[2] == '\\') {
		p1 = (TCHAR *)short_name + 3;
		// ドライブ名を取得
		_stprintf(find_path, _T("%.*s"), 2, short_name);
	} else {
		return FALSE;
	}
	_tcscpy(long_name, find_path);

	// pathの前の要素から，longnameに変換していく
	for(;;) {
		p2 = my_mbschr(p1, '\\');
		if(p2 == NULL) {
			_tcscpy(file_name, p1);
		} else {
			_stprintf(file_name, _T("%.*s"), (int)(p2 - p1), p1);
		}
		_tcscat(find_path, _T("\\"));
		_tcscat(find_path, file_name);

		hFind = FindFirstFile(find_path, &find_data);
		if(hFind == INVALID_HANDLE_VALUE) return FALSE;

		_tcscat(long_name, _T("\\"));
		_tcscat(long_name, find_data.cFileName);

		if((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
			FindClose(hFind);
			break;
		}

		FindClose(hFind);

		if(p2 == NULL) break;
		p1 = p2 + 1;
	}

	return TRUE;
}

/*------------------------------------------------------------------------------
 ファイルが存在するか確認する
------------------------------------------------------------------------------*/
BOOL is_file_exist(const TCHAR *path)
{
	int			ret_v;
	struct _stat file_stat;

	ret_v = _tstat(path, &file_stat);
	if(ret_v != 0) return FALSE;

	if((file_stat.st_mode & S_IFDIR) != 0) return FALSE;

	return TRUE;
}

/*------------------------------------------------------------------------------
 ディレクトリが存在するか確認する
------------------------------------------------------------------------------*/
BOOL is_directory_exist(const TCHAR *path)
{
	int			ret_v;
	TCHAR		path2[_MAX_PATH];
	struct _stat file_stat;

	if(path == NULL || _tcslen(path) == 0) return FALSE;

	/* ディレクトリ名の最後の \をはずす */
	_tcscpy(path2, path);
	make_dirname2(path2);

	/* drive名のとき，stat()に失敗するのを回避 */
	if(_tcslen(path2) == 2 && path2[1] == ':') {
		_tcscat(path2, _T("\\"));
	}

	ret_v = _tstat(path2, &file_stat);
	if(ret_v != 0) return FALSE;

	if((file_stat.st_mode & S_IFDIR) == 0) return FALSE;

	return TRUE;
}

/*------------------------------------------------------------------------------
 ディレクトリを作成する
------------------------------------------------------------------------------*/
int make_directory(const TCHAR *dir, TCHAR *msg_buf)
{
	TCHAR	buf[_MAX_PATH] = _T("");
	const TCHAR	*p = NULL;

	/* ROOTから順番にディレクトリの存在を確認して，存在しない場合は作成していく */
	p = dir;
	#ifdef WIN32
	/* Windowsの場合，ドライブ名をスキップする */
	p = my_mbschr(p, '\\');
	p++;
	#else
	p = _tcschr(p, '/' );
	p++;
	#endif
	for(; p != NULL;) {
		#ifdef WIN32
		p = my_mbschr(p, '\\');
		#else
		p = _tcschr(p, '/');
		#endif

		if(p != NULL) {
			_stprintf(buf, _T("%.*s"), (int)(p - dir), dir);
		} else {
			_stprintf(buf, _T("%s"), dir);
		}
		if(_tcslen(buf) == 0) break;

		if(is_directory_exist(buf) == FALSE) {
		#ifdef WIN32
			if(_tmkdir(buf) == -1) {
		#else
			if(mkdir(buf, 0777 ) == -1) {
		#endif
				_stprintf(msg_buf, _T("ディレクトリが作成できません(%s)"), dir);
				return 1;
			}
		}

		if(p != NULL) p++;
	}

	return 0;
}

/*------------------------------------------------------------------------------
 ディレクトリを作成する
------------------------------------------------------------------------------*/
BOOL is_valid_path(const TCHAR *path)
{
	TCHAR	dir[_MAX_PATH];

	_tcscpy(dir, path);
	make_parent_dirname(dir);

	return is_directory_exist(dir);
}

/*------------------------------------------------------------------------------
 拡張子を調べる
------------------------------------------------------------------------------*/
BOOL check_ext(const TCHAR *file_name, const TCHAR *ext)
{
	TCHAR ext_name[_MAX_PATH];
	TCHAR lwr_ext[_MAX_PATH];

	_tcscpy(lwr_ext, ext);
	_tcslwr(lwr_ext);

	_tsplitpath(file_name, NULL, NULL, NULL, ext_name);
	_tcslwr(ext_name);

	if(_tcscmp(ext_name, lwr_ext) == 0) return TRUE;
	return FALSE;
}

/*------------------------------------------------------------------------------
 相対パスをフルパスにする
------------------------------------------------------------------------------*/
void make_full_path(TCHAR *path)
{
	if(path == NULL || _tcslen(path) == 0 || _tcslen(path) >= _MAX_PATH) return;
	if((path[0] == '\\' && path[1] == '\\') || path[1] == ':') return;

	TCHAR cur_dir[_MAX_PATH];
	GetCurrentDirectory(_MAX_PATH, cur_dir);

	TCHAR tmp_path[_MAX_PATH];
	_tcscpy(tmp_path, path);
	TCHAR *p = tmp_path;

	if(path[0] == '\\') {
		TCHAR drive_name[_MAX_PATH];
		_tsplitpath(cur_dir, drive_name, NULL, NULL, NULL);
		_stprintf(path, _T("%s%s"), drive_name, p);
		return;
	}

	for(;;) {
		if(_tcsncmp(p, _T("..\\"), 3) != 0) break;
		make_parent_dirname(cur_dir);
		p = p + 3;
	}

	_stprintf(path, _T("%s\\%s"), cur_dir, p);
}

/*------------------------------------------------------------------------------
 リストに定義された拡張子か調べる
------------------------------------------------------------------------------*/
__inline static const TCHAR *GetExt(const TCHAR *file_type, TCHAR *ext)
{
	const TCHAR	*p;

	_tcscpy(ext, _T(""));

	file_type = _tcschr(file_type, '.');
	if(file_type == NULL) return NULL;

	p = _tcschr(file_type, ';');
	if(p == NULL) {
		_tcscpy(ext, file_type);
	} else {
		_tcsncpy(ext, file_type, p - file_type);
		ext[p - file_type] = '\0';
		p++;
	}

	return p;
}

BOOL CheckFileType(const TCHAR *file_name, const TCHAR *file_type)
{
	TCHAR	ext[_MAX_PATH];
	TCHAR	ext2[_MAX_PATH];
	const TCHAR	*p;

	_tsplitpath(file_name, NULL, NULL, NULL, ext);
	_tcslwr(ext);

	for(p = file_type; p != NULL;) {
		p = GetExt(p, ext2);
		if(_tcscmp(ext2, _T(".*")) == 0) return TRUE;
		_tcslwr(ext2);

		if(_tcscmp(ext, ext2) == 0) return TRUE;
	}
	return FALSE;
}

/*------------------------------------------------------------------------------
 ショートカットかチェックする
------------------------------------------------------------------------------*/
BOOL IsShortcut(const TCHAR *file_name)
{
	if(CheckFileType(file_name, _T(".lnk"))) return TRUE;
	return FALSE;
}

/*------------------------------------------------------------------------------
 ショートカットのフルパスを取得
------------------------------------------------------------------------------*/
BOOL GetFullPathFromShortcut(const TCHAR *file_name, CString *str)
{
	HRESULT hres;
	IShellLink *psl = NULL;
	IPersistFile *ppf = NULL;
	TCHAR fullPath[_MAX_PATH] = _T("");

	// COM初期化
	CoInitialize(NULL);

    // IShellLinkインタフェース取得
	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
		IID_IShellLink, (void**)&psl);
	if(!SUCCEEDED(hres)) goto ERR1;

	// IPersistFileインタフェース取得
	hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
	if(!SUCCEEDED(hres)) goto ERR1;

	// ショートカットの読み込み
	hres = ppf->Load(file_name, STGM_READ);
	if(!SUCCEEDED(hres)) goto ERR1;

	// リンク先を解決する
	hres = psl->Resolve(NULL, 0);
	if(!SUCCEEDED(hres)) goto ERR1;

	// リンク先のフルパス取得
	hres = psl->GetPath(fullPath, _MAX_PATH, NULL, 0);
	if(!SUCCEEDED(hres)) goto ERR1;
        
	if(ppf != NULL) ppf->Release();
    if(psl != NULL) psl->Release();
	CoUninitialize();

    *str = fullPath;
	return TRUE;

ERR1:
	if(ppf != NULL) ppf->Release();
    if(psl != NULL) psl->Release();
	CoUninitialize();
    return FALSE;
}

/*------------------------------------------------------------------------------
 ファイル名に使えない文字を'_'に置き換える
------------------------------------------------------------------------------*/
CString GetSafePathName(const TCHAR *file_name)
{
	CString result = file_name;
	result.Replace('\\', '_');
	result.Replace('/', '_');
	result.Replace(':', '_');
	result.Replace('*', '_');
	result.Replace('?', '_');
	result.Replace('"', '_');
	result.Replace('<', '_');
	result.Replace('>', '_');
	result.Replace('|', '_');
	return result;
}
