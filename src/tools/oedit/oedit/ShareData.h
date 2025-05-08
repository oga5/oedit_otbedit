/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(_SHARE_DATA_INCLUDED_)
#define _SHARE_DATA_INCLUDED_

#define SHARED_MEMORY_NAME	_T("Oedit_Shared_Memory Ver1.0")
#define MAX_WINDOW_CNT	256

typedef	struct {
	HWND	hwnd;
	TCHAR	file_name[MAX_PATH];
} window_list_t;

typedef struct share_data_st {
	int				window_cnt;
	window_list_t	window_list[MAX_WINDOW_CNT];
} share_data_t;

class CShareData
{
public:
	CShareData();
	~CShareData();

	BOOL Init();
	BOOL RegisterWindow(HWND hwnd);
	BOOL UnregisterWindow(HWND hwnd);
	BOOL UpdateOpenFileName(HWND hwnd, const TCHAR *file_name);

	int GetWindowCnt();
	BOOL GetWindowList(int &cnt, window_list_t *window_list, int list_size);

	BOOL ActiveWindow(int idx);
	BOOL ActiveNextWindow(HWND hwnd);
	BOOL ActivePrevWindow(HWND hwnd);

private:
	HANDLE			m_mutex;
	HANDLE			m_mapped_file;
	share_data_t	*m_share_data;

	BOOL OpenSharedMem();
	void CloseSharedMem();
	void InitData();

	int FindWindow(HWND hwnd);

#ifdef _DEBUG
	void TraceList(TCHAR *function_name);
#endif
};

#endif _SHARE_DATA_INCLUDED_
