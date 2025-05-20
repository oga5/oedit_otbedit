/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef __TOOL_TIP_EX_INCLUDE__
#define __TOOL_TIP_EX_INCLUDE__

/****************************************************************************/
/* ウィンドウに任意のTIPを表示する                                          */
/*                                                                          */
/* 1.メンバ変数にCToolTipExを追加                                           */
/*     CToolTipEx m_ToolTipEx;                                              */
/* 2.OnInitialUpdate()でToolTipを作成する                                   */
/*     m_ToolTipEx.Create(this->GetSafeHwnd());                             */
/* 3.PreTranslateMessage()をオーバーライドし，CToolTipEx::Show()を呼び出す  */
/*     m_ToolTipEx.Show(Msg *pMsg);                                         */
/* 4.デストラクタで，CToolTipEx::Destroy()を呼び出す                        */
/*     m_ToolTipEx.Destroy();                                               */
/* 5.表示する文字フォントを設定する                                         */
/*     m_ToolTipEx.SetFont(CFont *pFont);                                   */
/* 6.表示したい文字を設定する                                               */
/*     m_ToolTipEx.SetMssage(CString message);                              */
/*                                                                          */
/****************************************************************************/

#define TIP_CTRL_CLASSNAME	_T("Tip Control Window")
#define TIMER_ID_MOUSE_POS_CHECK	32867

class CToolTipEx {

public:
	CToolTipEx();
	~CToolTipEx();

	BOOL Create();
	BOOL Destroy();
	BOOL Show(HWND parent_wnd, RECT *rect);
	BOOL Hide();

	BOOL IsShow() { return m_b_show; }

	void SetMessage(CString msg);
	HWND GetHwnd() { return m_hWnd; };
	void SetFont(CFont *font);
	void SetPoint(CPoint pt) { m_pt = pt; }
	void SetBkColor(COLORREF cr) { m_bk_cr = cr; }
	void SetTextColor(COLORREF cr) { m_text_cr = cr; }

	HWND GetParentWnd() { return m_parent_wnd; }

private:
	static LRESULT CALLBACK ToolTipExWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	BOOL Create(HINSTANCE hinst);
	void OnPaint();
	void OnTimer(UINT_PTR nIDEvent);
	void CalcSize();

private:
	CString m_msg;		//表示したいメッセージ
	HWND m_hWnd;		//自分のウィンドウハンドル

	CRect m_disp_rect;	//表示範囲のRect

	CPoint m_pt;		//ウィンドウの表示位置(左上：原点)
	CSize m_size;		//ウィンドウサイズ
	COLORREF m_bk_cr;	//背景色
	COLORREF m_text_cr;	//文字色
	HFONT m_font;		//文字フォント

	BOOL m_b_show;

	HWND m_parent_wnd;
};

#endif /* __TOOL_TIP_EX_INCLUDE__ */