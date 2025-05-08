/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

// FileListBar.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "oedit.h"
#include "fileutil.h"
#include "FileListBar.h"
#include <process.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static const char *THIS_FILE = __FILE__;
#endif

#define TREE_CTRL_ID				401

/////////////////////////////////////////////////////////////////////////////
// CFileListBar

CFileListBar::CFileListBar()
{
	m_b_init = FALSE;
}

CFileListBar::~CFileListBar()
{
}


BEGIN_MESSAGE_MAP(CFileListBar, CSizingControlBarG)
	//{{AFX_MSG_MAP(CFileListBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileListBar メッセージ ハンドラ

int CFileListBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CSizingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: この位置に固有の作成用コードを追加してください
	CRect rect(0, 0, lpCreateStruct->cx, lpCreateStruct->cy);

	m_tree_ctrl.Create(WS_CHILD | 
		TVS_HASLINES | TVS_LINESATROOT | 
		TVS_HASBUTTONS | TVS_SHOWSELALWAYS,
		rect, this, TREE_CTRL_ID);
	//m_tree_ctrl.DisplayTree(NULL, TRUE);
	
	return 0;
}

void CFileListBar::OnSize(UINT nType, int cx, int cy) 
{
	CSizingControlBar::OnSize(nType, cx, cy);
	
	// TODO: この位置にメッセージ ハンドラ用のコードを追加してください
	m_tree_ctrl.SetWindowPos(&wndBottom, 0, 0, cx, cy, SWP_NOACTIVATE);
}

void CFileListBar::SetFilter(LPCTSTR filter)
{
	m_tree_ctrl.SetFilter(filter);
	if(m_b_init) {
		m_tree_ctrl.SetSelPath(m_tree_ctrl.GetFullPath());
	}
}

void CFileListBar::Init()
{
	if(m_b_init) return;

	m_tree_ctrl.DisplayTree(NULL, TRUE);
	m_tree_ctrl.ShowWindow(SW_SHOW);
	m_b_init = TRUE;

//	char	cur_dir[_MAX_PATH];
//	GetCurrentDirectory(sizeof(cur_dir), cur_dir);
//	m_tree_ctrl.SetSelPath(cur_dir);
}

BOOL CFileListBar::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
	switch(wParam) {
	case TREE_CTRL_ID:
		NMHDR *p_nmhdr = (NMHDR *)lParam;
		switch(p_nmhdr->code) {
		case NM_DBLCLK:
			OnTreeDblClick();
			break;
		}
		break;
	}

	return CSizingControlBar::OnNotify(wParam, lParam, pResult);
}

void CFileListBar::OnTreeDblClick()
{
	CString file_name;

	file_name = m_tree_ctrl.GetFullPath();

	if(is_file_exist(file_name.GetBuffer(0)) == FALSE) return;

	AfxGetApp()->OpenDocumentFile(file_name);
}

static unsigned int _stdcall file_list_bar_init_thr(void *lpvThreadParam)
{
	CFileListBar *p_bar = (CFileListBar *)lpvThreadParam;

	p_bar->Init();

	return 0;
}

static uintptr_t st_h_thread = NULL;

int file_list_bar_init(CFileListBar *file_list_bar, LPCTSTR filter)
{
	file_list_bar->SetFilter(filter);

	UINT thread_addr;
	uintptr_t h_thread = _beginthreadex(NULL, 0, file_list_bar_init_thr, 
		(void *)file_list_bar, 0, &thread_addr);
	if(h_thread == NULL) {
		return 1;
	}

//	CloseHandle((void *)h_thread);
	st_h_thread = h_thread;

	return 0;
}

void CFileListBar::OnPaint() 
{
	CPaintDC dc(this); // 描画用のデバイス コンテキスト
	
	// TODO: この位置にメッセージ ハンドラ用のコードを追加してください
	if(m_b_init == FALSE) {
		dc.TextOut(2, 2, "initializing ...");
	}
	
	// 描画用メッセージとして CSizingControlBar::OnPaint() を呼び出してはいけません
}

void CFileListBar::WaitForInitializeThread()
{
	if(st_h_thread != NULL) {
		WaitForSingleObject((void *)st_h_thread, 10 * 1000);
		CloseHandle((void *)st_h_thread);
		st_h_thread = NULL;
	}
}

BOOL CFileListBar::IsFileTreeActive()
{
	if(!(g_file_list_bar.GetStyle() & WS_VISIBLE)) return FALSE;
	if(GetFocus() == &m_tree_ctrl) return TRUE;
	return FALSE;
}


