/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"

#include "EditDispData.h"
#include "colorutil.h"


CEditDispData::CEditDispData()
{
	m_caret_pos.y = 0;
	m_caret_pos.x = 0;

	m_highlight_pt = CPoint(-1, -1);
	m_highlight_pt2 = CPoint(-1, -1);

	m_bracket_pt = CPoint(-1, -1);
	m_bracket_pt2 = CPoint(-1, -1);
	m_bracket_prev_seq = -1;
	m_bracket_prev_cur_pt = CPoint(-1, -1);

	m_max_disp_idx = 0;

	m_line_mode = EC_LINE_MODE_NORMAL;
	m_line_len = INT_MAX;
	m_line_width = INT_MAX;

	m_select_area.dchandler = NULL;
	m_select_area.next_box_select = FALSE;
	ClearSelected();

	m_dragdrop = FALSE;
	m_drop_myself = FALSE;

	for(int i = 0; i < MAX_EDIT_NOTIFY_WND_CNT; i++) {
		m_notify_wnd_list[i] = NULL;
	}

	m_color[TEXT_COLOR] = RGB(0, 0, 0);
	m_color[KEYWORD_COLOR] = RGB(0, 0, 205);
	m_color[KEYWORD2_COLOR] = RGB(0, 128, 192);
	m_color[COMMENT_COLOR] = RGB(0, 120, 0);
	m_color[BG_COLOR] = RGB(255, 255, 255);
	m_color[PEN_COLOR] = RGB(0, 150, 150);
	m_color[QUOTE_COLOR] = RGB(220, 0, 0);
	m_color[SEARCH_COLOR] = RGB(255, 255, 50);
	m_color[SELECTED_COLOR] = RGB(0, 0, 50);
	m_color[RULER_COLOR] = RGB(0, 100, 0);
	m_color[OPERATOR_COLOR] = RGB(128, 0, 0);
	m_color[IME_CARET_COLOR] = RGB(0x00, 0xa0, 0x00);
	m_color[RULED_LINE_COLOR] = RGB(180, 180, 180);

	m_color[BRACKET_COLOR1] = m_color[OPERATOR_COLOR];
	m_color[BRACKET_COLOR2] = m_color[OPERATOR_COLOR];
	m_color[BRACKET_COLOR3] = m_color[OPERATOR_COLOR];

// NOTE:
//   CEditData(CEditDispDataを使っている)は、内部データとして使う場合もあるので、
//   ここではGGI objectを生成しない (GetMarkPenなどで取得時に初期化する)
//	m_mark_pen.CreatePen(PS_SOLID, 0, GetColor(PEN_COLOR));
//	m_ruler_pen.CreatePen(PS_SOLID, 0, GetColor(RULER_COLOR));

	m_caret_size.cy = -1;
	m_caret_size.cx = -1;
}

CEditDispData::~CEditDispData()
{
	m_mark_pen.DeleteObject();
	m_ruler_pen.DeleteObject();
	m_caret_bitmap.DeleteObject();

	ClearSelected();
}

void CEditDispData::ClearSelected()
{
	m_select_area.pos1 = CPoint(-1, -1);
	m_select_area.pos2 = CPoint(-1, -1);
	m_select_area.start_pt1 = CPoint(-1, -1);
	m_select_area.start_pt2 = CPoint(-1, -1);
	m_select_area.drag_flg = NO_DRAG;
	m_select_area.word_select = FALSE;
	m_select_area.box_select = FALSE;

	if(m_select_area.dchandler) {
		delete m_select_area.dchandler;
		m_select_area.dchandler = NULL;
	}
}

void CEditDispData::SetDCHandler(CWnd *wnd, CFont *font)
{
	if(m_select_area.dchandler) {
		delete m_select_area.dchandler;
		m_select_area.dchandler = NULL;
	}

	m_select_area.dchandler = new CFontWidthHandler(wnd, font);
}

void CEditDispData::SetColor(int type, COLORREF color)
{
	if(GetColor(type) == color) return;

	m_color[type] = color;

	if(type == PEN_COLOR) {
		m_mark_pen.DeleteObject();
		m_mark_pen.CreatePen(PS_SOLID, 0, GetColor(PEN_COLOR));
	}
	if(type == RULER_COLOR) {
		m_ruler_pen.DeleteObject();
		m_ruler_pen.CreatePen(PS_SOLID, 0, GetColor(RULER_COLOR));
	}
	if(type == IME_CARET_COLOR) {
		InvalidateCaretBitmap();
	}
	if(type == BG_COLOR) {
		InvalidateCaretBitmap();
	}
}

void CEditDispData::InvalidateCaretBitmap()
{
	if(m_caret_size.cy != -1) {
		m_caret_size.cy = -1;
	}
}

BOOL CEditDispData::CreateCaretBitmap(int width, int height, BOOL b_ime)
{
	CDC display_dc;
	if(!display_dc.CreateDC(_T("DISPLAY"), NULL, NULL, NULL)) return FALSE;

	InvalidateCaretBitmap();
	m_caret_bitmap.DeleteObject();
	if(!m_caret_bitmap.CreateCompatibleBitmap(&display_dc, width, height)) return FALSE;

	CDC dc;
	if(!dc.CreateCompatibleDC(&display_dc)) return FALSE;

	CBitmap *old_bitmap = dc.SelectObject(&m_caret_bitmap);
	dc.FillSolidRect(0, 0, width, height, 
		make_xor_color(GetColor(IME_CARET_COLOR), GetColor(BG_COLOR)));
	dc.SelectObject(old_bitmap);

	dc.DeleteDC();

	m_caret_ime = b_ime;
	m_caret_size.cy = height;
	m_caret_size.cx = width;

	return TRUE;
}

CBitmap *CEditDispData::GetCaretBitmap(int width, int height, BOOL b_ime)
{
	if(m_caret_ime == b_ime &&
		m_caret_size.cx == width && m_caret_size.cy == height) {
		return &m_caret_bitmap;
	}

	if(!CreateCaretBitmap(width, height, b_ime)) return NULL;

	return &m_caret_bitmap;
}

BOOL CEditDispData::HaveSelected()
{
	if(GetSelectArea()->pos1.y == -1) return FALSE;

	if(GetSelectArea()->pos1.y == GetSelectArea()->pos2.y &&
		GetSelectArea()->pos1.x == GetSelectArea()->pos2.x) return FALSE;

	return TRUE;
}

void CEditDispData::RegistNotifyWnd(HWND hwnd)
{
	for(int i = 0; i < MAX_EDIT_NOTIFY_WND_CNT; i++) {
		if(m_notify_wnd_list[i] == NULL) {
			m_notify_wnd_list[i] = hwnd;
			break;
		}
	}
}

void CEditDispData::UnRegistNotifyWnd(HWND hwnd)
{
	for(int i = 0; i < MAX_EDIT_NOTIFY_WND_CNT; i++) {
		if(m_notify_wnd_list[i] == hwnd) {
			m_notify_wnd_list[i] = NULL;
			break;
		}
	}
}

void CEditDispData::SendNotifyMessage(HWND sender, UINT msg, WPARAM wParam, LPARAM lParam)
{
	for(int i = 0; i < MAX_EDIT_NOTIFY_WND_CNT; i++) {
		if(m_notify_wnd_list[i] != NULL && m_notify_wnd_list[i] != sender) {
			::SendMessage(m_notify_wnd_list[i], msg, wParam, lParam);
		}
	}
}

void CEditDispData::SendNotifyMessage_OneWnd(UINT msg, WPARAM wParam, LPARAM lParam)
{
	for(int i = 0; i < MAX_EDIT_NOTIFY_WND_CNT; i++) {
		if(m_notify_wnd_list[i] != NULL) {
			::SendMessage(m_notify_wnd_list[i], msg, wParam, lParam);
			break;
		}
	}
}

void CEditDispData::SetLineLen(int len)
{
	if(len <= 0) len = 80;
	m_line_len = len;
}

void CEditDispData::SetLineWidth(int width)
{
	if(width <= 0) width = 400;
	m_line_width = width;
}
