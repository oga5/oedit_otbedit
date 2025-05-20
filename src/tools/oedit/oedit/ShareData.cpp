/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "sharedata.h"

static const TCHAR* STR_MUTEX_NAME = _T("UNIQUE_STRING_OEDIT_MUTEX");

CShareData::CShareData()
{
	m_mapped_file = NULL;
	m_share_data = NULL;
	m_mutex = NULL;
}

CShareData::~CShareData()
{
	CloseSharedMem();
	if(m_mutex != NULL) CloseHandle(m_mutex);
}

BOOL CShareData::Init()
{
	m_mutex = ::CreateMutex(NULL, FALSE, STR_MUTEX_NAME);
	if(m_mutex == NULL) return FALSE;

	return OpenSharedMem();
}

BOOL CShareData::OpenSharedMem()
{
	if(m_mutex == NULL) return FALSE;
	WaitForSingleObject(m_mutex, INFINITE);

	m_mapped_file = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,
		PAGE_READWRITE | SEC_COMMIT, 0, sizeof(share_data_t),
		SHARED_MEMORY_NAME);
	if(m_mapped_file == NULL) {
		MessageBox(NULL, _T("共有メモリの作成エラー"), _T("Error"), MB_OK | MB_ICONERROR);
		ReleaseMutex(m_mutex);
		return FALSE;
	}

	if(GetLastError() != ERROR_ALREADY_EXISTS ) {
		m_share_data = (share_data_t *)MapViewOfFile(m_mapped_file,
			FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if(m_share_data == NULL) {
			MessageBox(NULL, _T("共有メモリの作成エラー"), _T("Error"), MB_OK | MB_ICONERROR);
			CloseHandle(m_mapped_file);
			ReleaseMutex(m_mutex);
			return FALSE;
		}

		InitData();
	} else {
		m_share_data = (share_data_t *)MapViewOfFile(m_mapped_file,
			FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if(m_share_data == NULL) {
			MessageBox(NULL, _T("共有メモリの作成エラー"), _T("Error"), MB_OK | MB_ICONERROR);
			CloseHandle(m_mapped_file);
			ReleaseMutex(m_mutex);
			return FALSE;
		}
	}

	ReleaseMutex(m_mutex);
	return TRUE;
}

void CShareData::CloseSharedMem()
{
	if(m_mutex == NULL) return;
	WaitForSingleObject(m_mutex, INFINITE);

	if(m_share_data != NULL) {
		UnmapViewOfFile(m_share_data);
		m_share_data = NULL;
	}
	if(m_mapped_file != NULL) {
		CloseHandle(m_mapped_file);
		m_mapped_file = NULL;
	}

	ReleaseMutex(m_mutex);
}

void CShareData::InitData()
{
	if(m_share_data == NULL) return;

	int		i;

	for(i = 0; i < MAX_WINDOW_CNT; i++) {
		m_share_data->window_list[i].hwnd = NULL;
	}
	m_share_data->window_cnt = 0;
}

int CShareData::FindWindow(HWND hwnd)
{
	if(m_share_data == NULL) return -1;

	int		i;

	for(i = 0; i < m_share_data->window_cnt; i++) {
		if(m_share_data->window_list[i].hwnd == hwnd) return i;
	}
	
	return -1;
}

BOOL CShareData::RegisterWindow(HWND hwnd)
{
	if(m_mutex == NULL || m_share_data == NULL) return FALSE;
	WaitForSingleObject(m_mutex, INFINITE);

	if(FindWindow(hwnd) != -1 || m_share_data->window_cnt == MAX_WINDOW_CNT) {
		ReleaseMutex(m_mutex);
		return FALSE;
	}

	m_share_data->window_list[m_share_data->window_cnt].hwnd = hwnd;
	_tcscpy(m_share_data->window_list[m_share_data->window_cnt].file_name, _T(""));

	m_share_data->window_cnt++;

#ifdef _DEBUG
	TraceList(_T("RegisterWindow"));
#endif

	ReleaseMutex(m_mutex);
	return TRUE;
}

BOOL CShareData::UnregisterWindow(HWND hwnd)
{
	if(m_mutex == NULL || m_share_data == NULL) return FALSE;
	WaitForSingleObject(m_mutex, INFINITE);

	int idx = FindWindow(hwnd);
	if(idx == -1) {
		ReleaseMutex(m_mutex);
		return FALSE;
	}

	// 前に詰める
	int i;
	for(i = idx + 1; i < m_share_data->window_cnt; i++) {
		m_share_data->window_list[i - 1].hwnd = m_share_data->window_list[i].hwnd;
		_tcscpy(m_share_data->window_list[i - 1].file_name, m_share_data->window_list[i].file_name);
	}

	m_share_data->window_cnt--;

#ifdef _DEBUG
	TraceList(_T("UnregisterWindow"));
#endif

	ReleaseMutex(m_mutex);
	return TRUE;
}

BOOL CShareData::UpdateOpenFileName(HWND hwnd, const TCHAR *file_name)
{
	if(m_mutex == NULL || m_share_data == NULL) return FALSE;
	WaitForSingleObject(m_mutex, INFINITE);

	int idx = FindWindow(hwnd);
	if(idx == -1) {
		ReleaseMutex(m_mutex);
		return FALSE;
	}

	_tcscpy(m_share_data->window_list[idx].file_name, file_name);

#ifdef _DEBUG
	TraceList(_T("UpdateOpenFileName"));
#endif

	ReleaseMutex(m_mutex);
	return TRUE;
}

BOOL CShareData::GetWindowList(int &cnt, window_list_t *window_list, int list_size)
{
	if(m_mutex == NULL || m_share_data == NULL) return FALSE;
	WaitForSingleObject(m_mutex, INFINITE);

	int		i;

	cnt = min(m_share_data->window_cnt, list_size);

	for(i = 0; i < cnt; i++) {
		window_list[i].hwnd = m_share_data->window_list[i].hwnd;
		_tcscpy(window_list[i].file_name, m_share_data->window_list[i].file_name);
	}

	ReleaseMutex(m_mutex);
	return TRUE;
}

#ifdef _DEBUG
void CShareData::TraceList(TCHAR *function_name)
{
	int		i;

	TRACE(_T("CShareData::TraceList() function:%s\n"), function_name);

	for(i = 0; i < m_share_data->window_cnt; i++) {
		TRACE(_T("%d:%d:%s\n"), i,
			m_share_data->window_list[i].hwnd, 
			m_share_data->window_list[i].file_name);
	}
}
#endif

BOOL CShareData::ActiveWindow(int idx)
{
	if(m_mutex == NULL || m_share_data == NULL) return FALSE;
	WaitForSingleObject(m_mutex, INFINITE);

	if(m_share_data->window_cnt <= idx) {
		ReleaseMutex(m_mutex);
		return FALSE;
	}

	::SetForegroundWindow(m_share_data->window_list[idx].hwnd);

	ReleaseMutex(m_mutex);
	return TRUE;
}

BOOL CShareData::ActiveNextWindow(HWND hwnd)
{
	if(m_mutex == NULL || m_share_data == NULL) return FALSE;
	WaitForSingleObject(m_mutex, INFINITE);

	int idx = FindWindow(hwnd);
	if(idx == -1) {
		ReleaseMutex(m_mutex);
		return FALSE;
	}

	idx++;
	if(idx >= m_share_data->window_cnt) idx = 0;
	::SetForegroundWindow(m_share_data->window_list[idx].hwnd);

	ReleaseMutex(m_mutex);
	return TRUE;
}

BOOL CShareData::ActivePrevWindow(HWND hwnd)
{
	if(m_mutex == NULL || m_share_data == NULL) return FALSE;
	WaitForSingleObject(m_mutex, INFINITE);

	int idx = FindWindow(hwnd);
	if(idx == -1) {
		ReleaseMutex(m_mutex);
		return FALSE;
	}

	idx--;
	if(idx < 0) idx = m_share_data->window_cnt - 1;
	::SetForegroundWindow(m_share_data->window_list[idx].hwnd);

	ReleaseMutex(m_mutex);
	return TRUE;
}

int CShareData::GetWindowCnt()
{
	if(m_share_data == NULL) return 0;
	return m_share_data->window_cnt;
/*
	if(m_mutex == NULL || m_share_data == NULL) return 0;
	WaitForSingleObject(m_mutex, INFINITE);

	int cnt = m_share_data->window_cnt;

	ReleaseMutex(m_mutex);
	return cnt;
*/
}
