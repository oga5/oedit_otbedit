/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#include "stdafx.h"

#include "CommonCancelDlg.h"
#include "octrl_msg.h"

#define ID_STATIC_1		110
#define ID_STATIC_2		111
#define ID_PROGRESS		120

CCommonCancelDlg::CCommonCancelDlg() : m_p_hWnd(NULL), m_hThread(NULL), m_ex_style(0),
	m_row_cnt_msg(0), m_row_cnt(0)
{
	InitResource();
	m_lpDialogTemplate = (LPDLGTEMPLATE)m_res_buffer;
}

CCommonCancelDlg::~CCommonCancelDlg()
{
}

BOOL CCommonCancelDlg::InitResource()
{
	char*  p;
	
	// メモリ res 上にダイアログ・リソースを作成する
	{
		memset( m_res_buffer, 0, sizeof(m_res_buffer) );

		// ダイアログ
		p = m_res_buffer;
		DLGTEMPLATE*  dlg_t = (DLGTEMPLATE*)p;
		{
			dlg_t->style = ( WS_CAPTION | WS_DLGFRAME | WS_POPUP | DS_MODALFRAME );
			dlg_t->x = 0;
			dlg_t->y = 0;
			dlg_t->cx = 220;
			dlg_t->cy = 70;
			dlg_t->dwExtendedStyle = 0;
			//dlg_t->cdit = 2;
			dlg_t->cdit = 0;
			p += sizeof( DLGTEMPLATE );
		}
		*(WORD*)p = 0;  p += sizeof(WORD);  // menu
		*(WORD*)p = 0;  p += sizeof(WORD);  // winClass
		wcscpy( (WCHAR*)p, L"実行中" );  p += 8;  // caption
/*	
		// スタティック
		p = (char*)( (int)(p + 3) & ~0x3 );
		{
			DLGITEMTEMPLATE*  t = (DLGITEMTEMPLATE*)p;
			t->id = ID_STATIC_1;
			t->style = ( WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER );
			t->cx = dlg_t->cx - 10;
			t->cy = 14;
			t->x = (dlg_t->cx / 2) - (t->cx / 2);
			t->y = 5;
			t->dwExtendedStyle = 0;
			p += sizeof( DLGTEMPLATE );
		}
		*(WORD*)p = 0xFFFF;  p += sizeof(WORD);  // atom
		*(WORD*)p = 0x0082;  p += sizeof(WORD);  // Static
		*(WORD*)p = 0x0000;  p += sizeof(WORD);  // caption
		*(WORD*)p = 0;   p += sizeof(WORD);  // nCtrlByte

		// ボタン
		p = (char*)( (int)(p + 3) & ~0x3 );
		{
			DLGITEMTEMPLATE*  t = (DLGITEMTEMPLATE*)p;
			t->id = IDCANCEL;
			t->style = ( WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON );
			t->cx = 50;
			t->cy = 14;
			t->x = (dlg_t->cx / 2) - (t->cx / 2);
			t->y = dlg_t->cy - t->cy - 5;
			t->dwExtendedStyle = 0;
			p += sizeof( DLGTEMPLATE );
		}
		*(WORD*)p = 0xFFFF;  p += sizeof(WORD);  // atom
		*(WORD*)p = 0x0080;  p += sizeof(WORD);  // Button
		wcscpy( (WCHAR*)p, L"&Cancel" );  p += 14 * sizeof(WCHAR);  // caption
		*(WORD*)p = 0;       p += sizeof(WORD);  // nCtrlByte
*/
	}

	return TRUE;
}

void CCommonCancelDlg::InitData(DWORD ex_style, HWND *p_hWnd, HANDLE hThread)
{
	m_p_hWnd = p_hWnd;
	m_hThread = hThread;
	m_ex_style = ex_style;
}

LRESULT CCommonCancelDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
	case WM_CREATE:
		return OnCreate((LPCREATESTRUCT)lParam);
	case WM_INITDIALOG:
		return OnInitDialog();

	case CND_WM_EXIT:
		return OnExit(wParam);
	case CND_WM_STATIC:
		return OnSetStatic(wParam, (TCHAR *)lParam);
	case CND_WM_QUERY_CANCEL:
		return OnQueryCancel();
	case CND_WM_SETRANGE:
		return OnSetProgressRange(wParam, lParam);
	case CND_WM_SETPOS:
		return OnSetProgressPos(wParam);
	case CND_WM_STEPIT:
		return OnProgressStepit();

	case CND_WM_SET_ROW_CNT:
		return OnSetRowCnt(wParam);
	}

	if(m_row_cnt_msg != 0 && message == m_row_cnt_msg) {
		return OnRowCntMsg(lParam);
	}

	return CDialog::WindowProc(message, wParam, lParam);
}

BOOL CCommonCancelDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(CDialog::OnCreate(lpCreateStruct) == FALSE) return FALSE;

	return TRUE;
}

BOOL CCommonCancelDlg::CreateItem()
{
	CRect	client_rect;
	GetClientRect(client_rect);
	
	CRect	static_rect;
	static_rect.left = 10;
	static_rect.right = client_rect.right - 10;
	static_rect.top = 10;
	static_rect.bottom = static_rect.top + 20;

	m_static_1.Create(_T("msg"), WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		static_rect, this, ID_STATIC_1);
	m_static_1.SetFont(this->GetFont());

	if(m_ex_style & CND_USE_PROGRESS_BAR) {
		CRect	progress_rect;
		progress_rect.left = static_rect.left;
		progress_rect.right = static_rect.right;
		progress_rect.top = static_rect.bottom + 10;
		progress_rect.bottom = progress_rect.top + 10;

		m_progress.Create(WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
			progress_rect, this, ID_PROGRESS);
	}
	if(m_ex_style & CND_USE_STATIC_2) {
		CRect	static_rect2;
		static_rect2.left = 10;
		static_rect2.right = client_rect.right - 10;
		static_rect2.top = static_rect.bottom + 10;
		static_rect2.bottom = static_rect2.top + 20;

		m_static_2.Create(_T(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			static_rect2, this, ID_STATIC_2);
		m_static_2.SetFont(this->GetFont());
	}

	CRect	btn_rect;
	btn_rect.left = client_rect.Width() / 2 - 40;
	btn_rect.right = client_rect.Width() / 2 + 40;
	btn_rect.top = client_rect.bottom - 30;
	btn_rect.bottom = btn_rect.top + 20;

	if(m_ex_style & CND_USE_CANCEL_BTN) {
		m_cancel_btn.Create(_T("Cancel"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
			btn_rect, this, IDCANCEL);
		m_cancel_btn.SetFont(this->GetFont());
	}

	return TRUE;
}

BOOL CCommonCancelDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ASSERT(m_p_hWnd != NULL && m_hThread != NULL);

	if(CreateItem() == FALSE) {
		EndDialog(IDCANCEL);
		return FALSE;
	}

	m_cancel_flg = 0;
	*m_p_hWnd = GetSafeHwnd();
	ResumeThread(m_hThread);

	return TRUE;
}

void CCommonCancelDlg::OnOK()
{
	return;
	//CDialog::OnOK();
}

void CCommonCancelDlg::OnCancel()
{
	if(!(m_ex_style & CND_USE_CANCEL_BTN)) return;

	m_cancel_flg = 2; // メッセージボックス表示中
	if(MessageBox(_T("キャンセルしますか"), _T("確認"), MB_ICONQUESTION | MB_YESNO) == IDYES) {
		m_cancel_flg = 1; // キャンセル
	} else {
		m_cancel_flg = 0; // 続行
	}
}

BOOL CCommonCancelDlg::OnExit(INT_PTR exit_code)
{
	m_exit_code = exit_code;
	CDialog::OnOK();
	return TRUE;
}

BOOL CCommonCancelDlg::OnSetStatic(INT_PTR id, TCHAR *msg)
{
	switch(id) {
	case 1:
		m_static_1.SetWindowText(msg);
		break;
	case 2:
		m_static_2.SetWindowText(msg);
		break;
	}
	return TRUE;
}

int CCommonCancelDlg::OnQueryCancel()
{
	return m_cancel_flg;
}

BOOL CCommonCancelDlg::OnSetProgressRange(INT_PTR step, INT_PTR range)
{
	m_progress.SetRange32(0, (int)range);
	m_progress.SetPos(0);
	m_progress.SetStep((int)step);
	return TRUE;
}

BOOL CCommonCancelDlg::OnSetProgressPos(INT_PTR pos)
{
	m_progress.SetPos((int)pos);
	return TRUE;
}

BOOL CCommonCancelDlg::OnProgressStepit()
{
	m_progress.StepIt();
	return TRUE;
}

BOOL CCommonCancelDlg::OnSetRowCnt(INT_PTR row)
{
	m_row_cnt = row;
	return TRUE;
}

void CCommonCancelDlg::SetRowCntMsg(UINT msg)
{
	m_row_cnt_msg = msg;
}

BOOL CCommonCancelDlg::OnRowCntMsg(INT_PTR row)
{
	CString		msg;

	msg.Format(_T("データ取得中"));
	m_static_1.SetWindowText(msg);
	m_progress.SetPos((int)row);
	return TRUE;
}
