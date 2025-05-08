/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef __EDIT_DISP_DATA_H_INCLUDED__
#define __EDIT_DISP_DATA_H_INCLUDED__

#include "DispColorData.h"
#include "IntArray.h"
#include "FontWidth.h"
#include "Renderer.h"

#define NO_DRAG		0
#define PRE_DRAG	1
#define DO_DRAG		2

// 行を折り返して表示するかどうか
#define EC_LINE_MODE_NORMAL		0		// 折り返さない
#define EC_LINE_MODE_RIGHT		1		// 右端で折り返す
#define EC_LINE_MODE_LEN		2		// 指定桁数で折り返す

#define MAX_EDIT_NOTIFY_WND_CNT		4

enum {
	TEXT_COLOR,
	KEYWORD_COLOR,
	COMMENT_COLOR,
	BG_COLOR,
	PEN_COLOR,
	QUOTE_COLOR,
	SEARCH_COLOR,
	SELECTED_COLOR,
	RULER_COLOR,
	OPERATOR_COLOR,
	KEYWORD2_COLOR,
	IME_CARET_COLOR,
	RULED_LINE_COLOR,
	BRACKET_COLOR1,
	BRACKET_COLOR2,
	BRACKET_COLOR3,
	EDIT_CTRL_COLOR_CNT,
};

class CEditDispData
{
public:
	struct edit_data_select_area_st {
		int drag_flg;
		POINT start_pt;
		POINT start_pt1;
		POINT start_pt2;
		POINT pos1;
		POINT pos2;
		int	  box_start_w;
		int   box_end_w;
		BOOL  box_select;
		BOOL  next_box_select;
		BOOL  word_select;
		CFontWidthHandler *dchandler;
	};

private:
	CIntArray	m_disp_row_arr;
	int			m_max_disp_idx;

	BOOL		m_dragdrop;
	BOOL		m_drop_myself;

	int			m_line_mode;
	int			m_line_len;
	int			m_line_width;

	// 範囲選択用
	struct edit_data_select_area_st m_select_area;

	HWND		m_notify_wnd_list[MAX_EDIT_NOTIFY_WND_CNT];
	POINT		m_caret_pos;
	CBitmap		m_caret_bitmap;

	CDispColorDataPool m_disp_color_data_pool;

	POINT		m_highlight_pt;
	POINT		m_highlight_pt2;

	POINT		m_bracket_pt;
	POINT		m_bracket_pt2;
	UINT		m_bracket_prev_seq;
	POINT		m_bracket_prev_cur_pt;

	SIZE		m_caret_size;
	BOOL		m_caret_ime;
	BOOL CreateCaretBitmap(int width, int height, BOOL b_ime);
	void InvalidateCaretBitmap();

	COLORREF	m_color[EDIT_CTRL_COLOR_CNT];

	CPen		m_mark_pen;
	CPen		m_ruler_pen;

	CRenderer		m_renderer;
	CFontWidthData	m_font_width_data;

public:
	CEditDispData();
	~CEditDispData();

	void SetDCHandler(CWnd *wnd, CFont *font);

	void ClearSelected();
	void SetDispPoolSize(int size) { m_disp_color_data_pool.SetPoolSize(size); }

	void SetLineMode(int line_mode) { m_line_mode = line_mode; }
	int GetLineMode() { return m_line_mode; }
	void SetLineLen(int len);
	int GetLineLen() { return m_line_len; }
	void SetLineWidth(int width);
	int GetLineWidth() { return m_line_width; }

	BOOL GetDragDrop() { return m_dragdrop; }
	void SetDragDrop(BOOL dragdrop) { m_dragdrop = dragdrop; }
	BOOL GetDropMyself() { return m_drop_myself; }
	void SetDropMyself(BOOL drop_myself) { m_drop_myself = drop_myself; }

	CIntArray *GetDispRowArr() { return &m_disp_row_arr; }
	int GetMaxDispIdx() { return m_max_disp_idx; }
	void SetMaxDispIdx(int max_disp_idx) { m_max_disp_idx = max_disp_idx; }

	POINT *GetCaretPos() { return &m_caret_pos; }
	CBitmap *GetCaretBitmap(int width, int height, BOOL b_ime);
	struct edit_data_select_area_st *GetSelectArea() { return &m_select_area; }
	BOOL HaveSelected();
	CDispColorDataPool *GetDispColorDataPool() { return &m_disp_color_data_pool; }

	virtual void RegistNotifyWnd(HWND hwnd);
	virtual void UnRegistNotifyWnd(HWND hwnd);
	void SendNotifyMessage(HWND sender, UINT msg, WPARAM wParam, LPARAM lParam);
	void SendNotifyMessage_OneWnd(UINT msg, WPARAM wParam, LPARAM lParam);

	const POINT &GetHighlightPos() const { return m_highlight_pt; }
	const POINT &GetHighlightPos2() const { return m_highlight_pt2; } 
	void SetHighlightPos(POINT pt) { m_highlight_pt = pt; }
	void SetHighlightPos2(POINT pt) { m_highlight_pt2 = pt; }

	const POINT &GetBracketPos() const { return m_bracket_pt; }
	const POINT &GetBracketPos2() const { return m_bracket_pt2; }
	void SetBracketPos(POINT pt) { m_bracket_pt = pt; }
	void SetBracketPos2(POINT pt) { m_bracket_pt2 = pt; }
	UINT GetBracketPrevSequence() const { return m_bracket_prev_seq; }
	void SetBracketPrevSequence(UINT seq) { m_bracket_prev_seq = seq; }
	const POINT &GetBracketPrevCurPt() const { return m_bracket_prev_cur_pt; }
	void SetBracketPrevCurPt(POINT pt) { m_bracket_prev_cur_pt = pt; }

	void SetColor(int type, COLORREF color);
	COLORREF GetColor(int type) { return m_color[type]; }

	CPen *GetMarkPen() { 
		if (m_mark_pen.m_hObject == NULL) {
			m_mark_pen.CreatePen(PS_SOLID, 0, GetColor(PEN_COLOR));
		}
		return &m_mark_pen;
	}
	CPen *GetRulerPen() { 
		if (m_ruler_pen.m_hObject == NULL) {
			m_ruler_pen.CreatePen(PS_SOLID, 0, GetColor(RULER_COLOR));
		}
		return &m_ruler_pen;
	}

	void SetCharSpaceSetting(int char_space) { m_font_width_data.SetCharSpaceSetting(char_space); }
	int GetCharSpaceSetting() { return m_font_width_data.GetCharSpaceSetting(); }

	void InitFontWidth(CDC *pdc) { m_font_width_data.InitFontWidth(pdc); }
	int GetFontWidth(CDC *pdc, unsigned int ch, CFontWidthHandler *pdchandler) { 
		return m_font_width_data.GetFontWidth(pdc, ch, pdchandler);
	}
	int GetCharSpace(int w) { return m_font_width_data.GetCharSpace(w); }
	BOOL IsFullWidthChar(CDC *pdc, unsigned int ch, CFontWidthHandler *pdchandler) { 
		return m_font_width_data.IsFullWidthChar(pdc, ch, pdchandler);
	}
	int GetTabWidth() { return m_font_width_data.GetTabWidth(); }

	CFontWidthData *GetFontData() { return &m_font_width_data; }
	CRenderer *GetRenderer() { return &m_renderer; }
};

#endif __EDIT_DISP_DATA_H_INCLUDED__
