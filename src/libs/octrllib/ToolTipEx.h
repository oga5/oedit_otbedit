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
/* �E�B���h�E�ɔC�ӂ�TIP��\������                                          */
/*                                                                          */
/* 1.�����o�ϐ���CToolTipEx��ǉ�                                           */
/*     CToolTipEx m_ToolTipEx;                                              */
/* 2.OnInitialUpdate()��ToolTip���쐬����                                   */
/*     m_ToolTipEx.Create(this->GetSafeHwnd());                             */
/* 3.PreTranslateMessage()���I�[�o�[���C�h���CCToolTipEx::Show()���Ăяo��  */
/*     m_ToolTipEx.Show(Msg *pMsg);                                         */
/* 4.�f�X�g���N�^�ŁCCToolTipEx::Destroy()���Ăяo��                        */
/*     m_ToolTipEx.Destroy();                                               */
/* 5.�\�����镶���t�H���g��ݒ肷��                                         */
/*     m_ToolTipEx.SetFont(CFont *pFont);                                   */
/* 6.�\��������������ݒ肷��                                               */
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
	CString m_msg;		//�\�����������b�Z�[�W
	HWND m_hWnd;		//�����̃E�B���h�E�n���h��

	CRect m_disp_rect;	//�\���͈͂�Rect

	CPoint m_pt;		//�E�B���h�E�̕\���ʒu(����F���_)
	CSize m_size;		//�E�B���h�E�T�C�Y
	COLORREF m_bk_cr;	//�w�i�F
	COLORREF m_text_cr;	//�����F
	HFONT m_font;		//�����t�H���g

	BOOL m_b_show;

	HWND m_parent_wnd;
};

#endif /* __TOOL_TIP_EX_INCLUDE__ */