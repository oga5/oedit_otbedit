/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef __COMMON_CALCEL_DLG_H_INCLUDED__
#define __COMMON_CALCEL_DLG_H_INCLUDED__

#define CND_USE_CANCEL_BTN		(0x0001 << 0)
#define CND_USE_PROGRESS_BAR	(0x0001 << 1)
#define CND_USE_STATIC_2		(0x0001 << 2)

class CCommonCancelDlg : public CDialog
{
public:
	CCommonCancelDlg();
	~CCommonCancelDlg();

	void InitData(DWORD ex_style, HWND *p_hWnd, HANDLE hThread);
	INT_PTR GetExitCode() { return m_exit_code; }
	volatile int *GetCancelFlgPt() { return &m_cancel_flg; }

	void SetRowCntMsg(UINT msg);

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	BOOL OnInitDialog();
	BOOL OnCreate(LPCREATESTRUCT lpCreateStruct);

	void OnOK();
	void OnCancel();

private:
	BOOL InitResource();
	BOOL CreateItem();

	BOOL OnExit(INT_PTR exit_code);
	BOOL OnSetStatic(INT_PTR id, TCHAR *msg);
	int OnQueryCancel();
	BOOL OnSetProgressRange(INT_PTR step, INT_PTR range);
	BOOL OnSetProgressPos(INT_PTR pos);
	BOOL OnProgressStepit();

	BOOL OnSetRowCnt(INT_PTR row);
	BOOL OnRowCntMsg(INT_PTR row);

	char			m_res_buffer[sizeof(DLGTEMPLATE) + 40 + ( sizeof(DLGITEMTEMPLATE) + 20 ) * 10];

	DWORD			m_ex_style;

	CStatic			m_static_1;
	CStatic			m_static_2;
	CButton			m_cancel_btn;
	CProgressCtrl	m_progress;

	HWND			*m_p_hWnd;
	HANDLE			m_hThread;
	volatile int	m_cancel_flg;
	INT_PTR			m_exit_code;

	INT_PTR			m_row_cnt;
	UINT			m_row_cnt_msg;
};

#endif __COMMON_CALCEL_DLG_H_INCLUDED__
