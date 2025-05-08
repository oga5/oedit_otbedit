/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

// EditCtrl.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "EditCtrl.h"
#include "colorutil.h"

#include "octrl_util.h"
#include "octrl_inline.h"

#include <imm.h>
#include <math.h>

#define USE_DOUBLE_BUFFERING

#define FULL_SCREEN_NULL_CURSOR_CNT	3

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static const char *THIS_FILE = __FILE__;
#endif

struct invalidate_editdata_st {
	int		old_row_cnt;
	int		old_char_cnt;
	int		old_split_cnt;
	int		start_row;
};

static const unsigned char LINK_CURSOR_XOR_DATA[128] = {
	0x00, 0x00, 0x00, 0x00,
	0x0c, 0x00, 0x00, 0x00,
	0x0c, 0x00, 0x00, 0x00,
	0x0c, 0x00, 0x00, 0x00,
	0x0c, 0x00, 0x00, 0x00,
	0x0c, 0x00, 0x00, 0x00,
	0x0d, 0xb6, 0x00, 0x00,
	0x0d, 0xb6, 0x00, 0x00,
	0x6d, 0xb6, 0x00, 0x00,
	0x6d, 0xb6, 0x00, 0x00,
	0x6f, 0xfe, 0x00, 0x00,
	0x7f, 0xfe, 0x00, 0x00,
	0x7f, 0xfe, 0x00, 0x00,
	0x3f, 0xfe, 0x00, 0x00,
	0x1f, 0xfe, 0x00, 0x00,
	0x1f, 0xfc, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
};
static const unsigned char LINK_CURSOR_AND_DATA[128] = {
	0xf3, 0xff, 0xff, 0xff,
	0xe1, 0xff, 0xff, 0xff,
	0xe1, 0xff, 0xff, 0xff,
	0xe1, 0xff, 0xff, 0xff,
	0xe1, 0xff, 0xff, 0xff,
	0xe0, 0x01, 0xff, 0xff,
	0xe0, 0x00, 0xff, 0xff,
	0x80, 0x00, 0xff, 0xff,
	0x00, 0x00, 0xff, 0xff,
	0x00, 0x00, 0xff, 0xff,
	0x00, 0x00, 0xff, 0xff,
	0x00, 0x00, 0xff, 0xff,
	0x00, 0x00, 0xff, 0xff,
	0x80, 0x00, 0xff, 0xff,
	0x80, 0x00, 0xff, 0xff,
	0xc0, 0x01, 0xff, 0xff,
	0xc0, 0x01, 0xff, 0xff,
	0xc0, 0x01, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff,
};

HCURSOR CEditCtrl::m_link_cursor = NULL;
UINT CEditCtrl::m_caret_blink_time = 0;

static BOOL IsPtInArea(POINT *pt1, POINT *pt2, POINT *pt)
{
	if(pt1->y > pt->y) return FALSE;
	if(pt2->y < pt->y) return FALSE;
	
	if(pt1->y == pt->y && pt1->x > pt->x) return FALSE;
	if(pt2->y == pt->y && pt2->x < pt->x) return FALSE;

	return TRUE;
}

static TCHAR *clickable_url_str[] = {_T("http://"), _T("https://"), _T("ftp://"), _T("www.")};

/////////////////////////////////////////////////////////////////////////////
#define EDITCTRL_CLASSNAME	_T("OGAWA_EditCtrl")

#define IdDragSelectTimer	100
#define IdDoubleClickTimer	101
#define IdHighlightChar		102
#define IdFullScreenTimer	103

#define DEFAULT_FONT_HEIGHT	18

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CEditCtrl, CScrollWnd)
/////////////////////////////////////////////////////////////////////////////
// CEditCtrl

CEditCtrl::CEditCtrl()
{
	RegisterWndClass();

	m_row_space = 0;
	m_row_space_top = 0;
	m_top_space = 1;
	m_left_space = 1;
	m_bottom_space = 0;
	m_right_space = 0;

	m_edit_data = NULL;

	m_read_only = FALSE;

	m_ex_style = 0;

	m_completion_data.cnt = -1;

	m_font_height = DEFAULT_FONT_HEIGHT;
	m_row_height = m_font_height + m_row_space;
	m_char_width = m_row_height / 2;
	m_num_width = 0;
	m_tt_flg = 0;

	m_row_num_digit = 0;
	m_row_header_width = m_left_space;
	m_col_header_height = m_top_space;

	if(m_link_cursor == NULL) {
		m_link_cursor = CreateCursor(AfxGetInstanceHandle(), 4, 0, 32, 32,
			LINK_CURSOR_AND_DATA, LINK_CURSOR_XOR_DATA);
	}
	if(m_caret_blink_time == 0) {
		m_caret_blink_time = GetCaretBlinkTime();
	}
	m_brackets_blink_time = GetCaretBlinkTime();
	m_caret_width = 2;

	m_overwrite = FALSE;

	m_cf_is_box = RegisterClipboardFormat(_T("OGAWA_EditCtrl_CF_IS_BOX_UNICODE"));

	m_dlg_code = DLGC_WANTALLKEYS;

	m_no_quote_color_char = '\0';

	m_ime_rect.left = 0;
	m_ime_rect.right = 0;
	m_ime_rect.top = 0;
	m_ime_rect.bottom = 0;

	m_iPicture = NULL;
	bk_dc_size.cx = -1;
	bk_dc_size.cy = -1;
}

CEditCtrl::~CEditCtrl()
{
	ClearBackGround();
}

BEGIN_MESSAGE_MAP(CEditCtrl, CScrollWnd)
	//{{AFX_MSG_MAP(CEditCtrl)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_CHAR()
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SETFOCUS()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_GETDLGCODE()
	ON_WM_RBUTTONDOWN()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CEditCtrl メッセージ ハンドラ

BOOL CEditCtrl::IsCheckCommentChar(unsigned int ch)
{
	if(m_ex_style & ECS_BRACKET_MULTI_COLOR) {
		if(ch == '(' || ch == ')') return TRUE;
	}

	return (m_edit_data->get_str_token()->isCommentChar(ch) ||
		m_edit_data->get_str_token()->isCommentChar(m_edit_data->get_cur_char()));
}

int CEditCtrl::CalcSplitCnt(int row)
{
	if(!IsWindow(GetSafeHwnd())) return 1;

	CFontWidthHandler dchandler(this, &m_font);
	m_edit_data->calc_disp_width(row, &dchandler);
	return m_edit_data->calc_disp_row_split_cnt(row, &dchandler);
}

void CEditCtrl::MakeDispData(BOOL all_recalc_split, int start_row, int recalc_split_cnt)
{
	// FIXME: 分割ウィンドウのとき，複数回実行しないようにする

	ASSERT(start_row < m_edit_data->get_row_cnt());
	if(start_row >= m_edit_data->get_row_cnt()) return;

	int row, j, idx, split;

	int disp_row = 0;
	idx = 0;

	for(row = 0; row < start_row; row++) {
		disp_row += m_edit_data->get_split_cnt(row);
	}
	if(row != 0) {
		idx = m_edit_data->get_scroll_row(row);
	}

	CIntArray *disp_row_arr = m_edit_data->get_disp_data()->GetDispRowArr();

	if(disp_row_arr->AllocData(m_edit_data->get_row_cnt()) == FALSE) {
		MessageBox(_T("メモリが確保できません"), _T("メッセージ"), MB_ICONINFORMATION | MB_OK);
		return;
	}

	for(row = start_row; row < m_edit_data->get_row_cnt(); row++) {
		CEditRowData *row_data = m_edit_data->get_edit_row_data(row);

		if(all_recalc_split || recalc_split_cnt > 0) {
			split = CalcSplitCnt(row);
			recalc_split_cnt--;
		} else {
			split = row_data->get_split_cnt();
		}
		disp_row += split;

		if(disp_row_arr->GetAllocedSize() <= disp_row) {
			if(disp_row_arr->AllocData(disp_row) == FALSE) {
				MessageBox(_T("メモリが確保できません"), _T("メッセージ"), MB_ICONINFORMATION | MB_OK);
				return;
			}
		}

		row_data->set_scroll_row(idx);
		row_data->set_split_cnt(split);
		
		for(j = 0; j < split; j++) {
			disp_row_arr->SetData(idx, row);
			idx++;
		}
	}

	m_edit_data->recalc_max_disp_width();
	m_edit_data->get_disp_data()->SetMaxDispIdx(disp_row);
}

void CEditCtrl::CheckCommentRow(int start_row, int end_row)
{
	// FIXME: 分割ウィンドウのとき，複数回実行しないようにする

	if(m_ex_style & ECS_NO_COMMENT_CHECK) return;

	/* この関数は実行に時間がかかるため，頻繁に呼び出さないこと */
	BOOL invalidate = FALSE;

	m_edit_data->check_comment_row(start_row, end_row, m_ex_style, &invalidate);

	if(invalidate) {
		Invalidate_AllWnd();
	}
}

QWORD CEditCtrl::SetBracketColorStyle(QWORD ex_style)
{
	TRACE(_T("SetBracketColorStyle: %d, %d, %d, %d, %d\n"), ex_style & ECS_BRACKET_MULTI_COLOR_ENABLE, GetColor(OPERATOR_COLOR), GetColor(BRACKET_COLOR1), GetColor(BRACKET_COLOR2), GetColor(BRACKET_COLOR3));

	if(ex_style & ECS_BRACKET_MULTI_COLOR_ENABLE && (
		GetColor(OPERATOR_COLOR) != GetColor(BRACKET_COLOR1) ||
		GetColor(OPERATOR_COLOR) != GetColor(BRACKET_COLOR2) ||
		GetColor(OPERATOR_COLOR) != GetColor(BRACKET_COLOR3))) {
		ex_style |= ECS_BRACKET_MULTI_COLOR;
	} else {
		ex_style &= (~ECS_BRACKET_MULTI_COLOR);
	}

	return ex_style;
}
/*
void CEditCtrl::SetExStyle(DWORD ex_style)
{
	SetExStyle2(ex_style);
}
*/
void CEditCtrl::SetExStyle2(QWORD ex_style)
{
	TRACE(_T("SetExStyle: %d\n"), ex_style & ECS_BRACKET_MULTI_COLOR_ENABLE);
	ex_style = SetBracketColorStyle(ex_style);

	if(m_ex_style == ex_style) return;

	// 
	m_edit_data->get_disp_data()->SetBracketPos(CPoint(-1, -1));
	m_edit_data->get_disp_data()->SetBracketPos2(CPoint(-1, -1));

	m_ex_style = ex_style;

	if(m_ex_style & ECS_GRID_EDITOR) {
		SetScrollStyle(GetScrollStyle() | NO_HSCROLL_BAR);
	}
	if(m_ex_style & ECS_FULL_SCREEN_MODE) {
		m_null_cursor_cnt = FULL_SCREEN_NULL_CURSOR_CNT;
		SetTimer(IdFullScreenTimer, 1000, NULL);
	} else {
		KillTimer(IdFullScreenTimer);
	}

	SetHeaderSize();
}

void CEditCtrl::PaintColNum(CDC *pdc, CDC *p_paintdc)
{
	if(!(m_ex_style & ECS_SHOW_COL_NUM)) return;

	RECT	rect2, winrect, bk_rect;
	int		i, x, col;
	int		max_right;

	GetDispRect(&winrect);

	max_right = winrect.right - m_right_space;

	rect2.top = 0;
	rect2.bottom = m_col_header_height;
	rect2.left = 0;
	rect2.right = winrect.right;
	if(p_paintdc->RectVisible(&rect2) == FALSE) return;

	rect2.top = m_top_space;
	rect2.bottom = m_col_header_height - 2;
	rect2.left = -(GetScrollPos(SB_HORZ) * m_char_width) + m_row_header_width;
	rect2.right = max_right;

	int line_bottom = rect2.bottom - 1;

	CPen *old_pen = pdc->SelectObject(m_edit_data->get_disp_data()->GetRulerPen());

	RECT caret_rect;
	caret_rect.left = GetCaretPos().x + 1;
	caret_rect.right = caret_rect.left + m_char_width - 1;
	caret_rect.top = m_top_space;
	caret_rect.bottom = line_bottom - 1;
	if(caret_rect.right > max_right) caret_rect.right = max_right;

	int char_space = m_edit_data->get_disp_data()->GetCharSpace(m_num_width);

	// 文字を描く
	CRect char_rect = rect2;
	col = 0;
	for(x = 0;; x += 10) {
		if(char_rect.left >= max_right) break;

		char_rect.right = char_rect.left + m_char_width * 10;
		if(char_rect.right > max_right) char_rect.right = max_right;

		if(char_rect.right > 0 && p_paintdc->RectVisible(&char_rect)) {
			FillBackGroundRect(pdc, &char_rect, GetColor(BG_COLOR));

			if(char_rect.PtInRect(CPoint(caret_rect.left, caret_rect.top)) ||
				char_rect.PtInRect(CPoint(caret_rect.right, caret_rect.top))) {
				RECT invert_rect = caret_rect;
				if(invert_rect.left < char_rect.left) invert_rect.left = char_rect.left;
				if(invert_rect.right > char_rect.right) invert_rect.right = char_rect.right;

				FillBackGroundRect(pdc, &invert_rect, GetColor(SELECTED_COLOR));
			}

			// FIXME: 20バイト固定をやめる
			TCHAR	col_num[20];
			_stprintf(col_num, _T("%d"), col);

			// 1文字ずつ表示する
			int col_num_len = (int)_tcslen(col_num);

			for(i = 0; i < col_num_len; i++) {
				CRect col_num_rect = char_rect;
				pdc->SetTextColor(GetColor(RULER_COLOR));
				pdc->SetBkColor(GetColor(BG_COLOR));

				col_num_rect.left = char_rect.left + 2 + (i * m_char_width);
				col_num_rect.right = col_num_rect.left + m_char_width;
				if(col_num_rect.right > max_right) col_num_rect.right = max_right;
				TextOut2(pdc, p_paintdc, col_num + i, 1, col_num_rect);

				// 文字を反転表示
				if(col_num_rect.PtInRect(CPoint(caret_rect.left, caret_rect.top)) ||
					col_num_rect.PtInRect(CPoint(caret_rect.right, caret_rect.top))) {

					RECT invert_rect = caret_rect;
					if(invert_rect.left < col_num_rect.left) {
						invert_rect.left = col_num_rect.left;
					}
					if(invert_rect.right > col_num_rect.right) {
						invert_rect.right = col_num_rect.right;
					}

					COLORREF new_text_color;
					if(m_ex_style & ECS_NO_INVERT_SELECT_TEXT) {
						new_text_color = GetColor(KEYWORD_COLOR);
					} else {
						new_text_color = RGB(0xff, 0xff, 0xff) ^ GetColor(KEYWORD_COLOR);
					}
					pdc->SetTextColor(new_text_color);
					pdc->SetBkColor(GetColor(SELECTED_COLOR));

					int left = invert_rect.left - (invert_rect.left - col_num_rect.left);
					int top = invert_rect.top;
					pdc->ExtTextOut(left, top, ETO_OPAQUE | ETO_CLIPPED, &invert_rect, col_num + i, 1, NULL);
				}
			}
		}
		if(col > m_edit_data->get_disp_data()->GetLineLen()) break;

		col += 10;
		char_rect.left = char_rect.right;
	}

	// 線を引く
	RECT line_rect = rect2;
	col = 0;
	for(x = 0;; x += 10) {
		if(line_rect.left >= max_right) break;

		line_rect.right = line_rect.left + m_char_width * 10;
		if(line_rect.right > max_right) line_rect.right = max_right;

		if(line_rect.right > 0 && p_paintdc->RectVisible(&line_rect)) {
			if(col + 10 > m_edit_data->get_disp_data()->GetLineLen()) {
				int line_right = line_rect.left + (m_char_width * (m_edit_data->get_disp_data()->GetLineLen() % 10)) + 1;
				if(line_right > max_right) line_right = max_right;
				pdc->MoveTo(line_rect.left, line_bottom);
				pdc->LineTo(line_right, line_bottom);
			} else {
				pdc->MoveTo(line_rect.left, line_bottom);
				pdc->LineTo(line_rect.right, line_bottom);
			}

			int line_x = line_rect.left;
			pdc->MoveTo(line_x, m_top_space);
			pdc->LineTo(line_x, line_bottom);
			for(i = 1; i < 10; i++) {
				if(col + i > m_edit_data->get_disp_data()->GetLineLen()) break;
				if(line_x > max_right) break;

				line_x += m_char_width;
				if(i == 5) {
					pdc->MoveTo(line_x, line_bottom - m_row_height / 2);					
				} else {
					pdc->MoveTo(line_x, line_bottom - m_row_height / 4);
				}
				pdc->LineTo(line_x, line_bottom);
			}
		}
		if(col > m_edit_data->get_disp_data()->GetLineLen()) break;

		col += 10;
		line_rect.left = line_rect.right;
	}

	// 下側の隙間を白く塗る
	bk_rect.top = line_bottom + 1;
	bk_rect.bottom = m_col_header_height;
	bk_rect.left = 0;
	bk_rect.right = winrect.right;
	if(p_paintdc->RectVisible(&bk_rect)) {
		FillBackGroundRect(pdc, &bk_rect, GetColor(BG_COLOR));
	}

	// 左側の隙間を白く塗る
	bk_rect.left = 0;
	bk_rect.right = m_row_header_width;
	bk_rect.top = 0;
	bk_rect.bottom = m_col_header_height;
	if(p_paintdc->RectVisible(&bk_rect)) {
		FillBackGroundRect(pdc, &bk_rect, GetColor(BG_COLOR));

		// 線の左端
		bk_rect.left = m_left_space;
		pdc->MoveTo(bk_rect.left, line_bottom);
		pdc->LineTo(bk_rect.right, line_bottom);
	}

	// 右側を白く塗る
	if(line_rect.left < winrect.right) {
		bk_rect.top = 0;
		bk_rect.bottom = m_col_header_height;
		bk_rect.left = line_rect.left;
		if(bk_rect.left < m_row_header_width) bk_rect.left = m_row_header_width;
		bk_rect.right = winrect.right;	
		if(p_paintdc->RectVisible(&bk_rect)) {
			FillBackGroundRect(pdc, &bk_rect, GetColor(BG_COLOR));
		}
	}

	pdc->SelectObject(old_pen);
}

/*
void CEditCtrl::PaintColNum(CDC *pdc, CDC *p_paintdc)
{
	if(!(m_ex_style & ECS_SHOW_COL_NUM)) return;

	RECT	rect2, winrect, bk_rect;
	int		i, x, col;
	int		max_right;

	GetDispRect(&winrect);

	max_right = winrect.right - m_right_space;

	rect2.top = 0;
	rect2.bottom = m_col_header_height;
	rect2.left = 0;
	rect2.right = winrect.right;
	if(p_paintdc->RectVisible(&rect2) == FALSE) return;

	rect2.top = m_top_space;
	rect2.bottom = m_col_header_height - 2;
	rect2.left = -(GetScrollPos(SB_HORZ) * m_char_width) + m_row_header_width;
	rect2.right = max_right;

	int line_bottom = rect2.bottom - 1;

	CPen *old_pen = pdc->SelectObject(m_edit_data->get_disp_data()->GetRulerPen());

	// 文字を描く
	RECT char_rect = rect2;
	col = 0;
	for(x = 0;; x += 10) {
		if(char_rect.left >= max_right) break;

		char_rect.right = char_rect.left + m_char_width * 10;
		if(char_rect.right > max_right) char_rect.right = max_right;

		if(char_rect.right > 0 && p_paintdc->RectVisible(&char_rect)) {
			FillBackGroundRect(pdc, &char_rect, GetColor(BG_COLOR));

			// FIXME: 20バイト固定をやめる
			TCHAR	col_num[20];
			_stprintf(col_num, _T("%d"), col);

			// 1文字ずつ表示する
			int col_num_len = _tcslen(col_num);

			for(i = 0; i < col_num_len; i++) {
				RECT col_num_rect = char_rect;
				pdc->SetTextColor(GetColor(RULER_COLOR));
				pdc->SetBkColor(GetColor(BG_COLOR));
				col_num_rect.left = char_rect.left + 2 + (i * m_char_width);
				col_num_rect.right = col_num_rect.left + m_char_width;
				if(col_num_rect.right > max_right) col_num_rect.right = max_right;
				TextOut2(pdc, p_paintdc, col_num + i, 1, col_num_rect);
			}
		}
		if(col > m_edit_data->get_disp_data()->GetLineLen()) break;

		col += 10;
		char_rect.left = char_rect.right;
	}

	// 現在のCaret位置を反転する
	{
		RECT caret_rect;
		caret_rect.left = GetCaretPos().x + 1;
		caret_rect.right = caret_rect.left + m_char_width - 2;
		caret_rect.top = m_top_space;
		caret_rect.bottom = line_bottom - 1;
		if(caret_rect.right > max_right) caret_rect.right = max_right;

		POINT pt;
		COLORREF bg_color = GetColor(BG_COLOR);
		COLORREF select_color = GetColor(SELECTED_COLOR);
		COLORREF text_color = GetColor(RULER_COLOR);
		COLORREF new_text_color;
		if(m_ex_style & ECS_NO_INVERT_SELECT_TEXT) {
			new_text_color = GetColor(KEYWORD_COLOR);
		} else {
			new_text_color = RGB(0xff, 0xff, 0xff) ^ GetColor(KEYWORD_COLOR);
		}

		for(pt.y = caret_rect.top; pt.y <= caret_rect.bottom; pt.y++) {
			for(pt.x = caret_rect.left; pt.x <= caret_rect.right; pt.x++) {
				if(pdc->GetPixel(pt) == bg_color) pdc->SetPixel(pt, select_color);
				if(pdc->GetPixel(pt) == text_color) pdc->SetPixel(pt, new_text_color);
			}
		}
	}

	// 線を引く
	RECT line_rect = rect2;
	col = 0;
	for(x = 0;; x += 10) {
		if(line_rect.left >= max_right) break;

		line_rect.right = line_rect.left + m_char_width * 10;
		if(line_rect.right > max_right) line_rect.right = max_right;

		if(line_rect.right > 0 && p_paintdc->RectVisible(&line_rect)) {
			if(col + 10 > m_edit_data->get_disp_data()->GetLineLen()) {
				int line_right = line_rect.left + (m_char_width * (m_edit_data->get_disp_data()->GetLineLen() % 10)) + 1;
				if(line_right > max_right) line_right = max_right;
				pdc->MoveTo(line_rect.left, line_bottom);
				pdc->LineTo(line_right, line_bottom);
			} else {
				pdc->MoveTo(line_rect.left, line_bottom);
				pdc->LineTo(line_rect.right, line_bottom);
			}

			int line_x = line_rect.left;
			pdc->MoveTo(line_x, m_top_space);
			pdc->LineTo(line_x, line_bottom);
			for(i = 1; i < 10; i++) {
				if(col + i > m_edit_data->get_disp_data()->GetLineLen()) break;
				if(line_x > max_right) break;

				line_x += m_char_width;
				if(i == 5) {
					pdc->MoveTo(line_x, line_bottom - m_row_height / 2);					
				} else {
					pdc->MoveTo(line_x, line_bottom - m_row_height / 4);
				}
				pdc->LineTo(line_x, line_bottom);
			}
		}
		if(col > m_edit_data->get_disp_data()->GetLineLen()) break;

		col += 10;
		line_rect.left = line_rect.right;
	}

	// 下側の隙間を白く塗る
	bk_rect.top = line_bottom + 1;
	bk_rect.bottom = m_col_header_height;
	bk_rect.left = 0;
	bk_rect.right = winrect.right;
	if(p_paintdc->RectVisible(&bk_rect)) {
		FillBackGroundRect(pdc, &bk_rect, GetColor(BG_COLOR));
	}

	// 左側の隙間を白く塗る
	bk_rect.left = 0;
	bk_rect.right = m_row_header_width;
	bk_rect.top = 0;
	bk_rect.bottom = m_col_header_height;
	if(p_paintdc->RectVisible(&bk_rect)) {
		FillBackGroundRect(pdc, &bk_rect, GetColor(BG_COLOR));

		// 線の左端
		bk_rect.left = m_left_space;
		pdc->MoveTo(bk_rect.left, line_bottom);
		pdc->LineTo(bk_rect.right, line_bottom);
	}

	// 右側を白く塗る
	if(line_rect.left < winrect.right) {
		bk_rect.top = 0;
		bk_rect.bottom = m_col_header_height;
		bk_rect.left = line_rect.left;
		if(bk_rect.left < m_row_header_width) bk_rect.left = m_row_header_width;
		bk_rect.right = winrect.right;	
		if(p_paintdc->RectVisible(&bk_rect)) {
			FillBackGroundRect(pdc, &bk_rect, GetColor(BG_COLOR));
		}
	}

	pdc->SelectObject(old_pen);
}
*/

//
// テキストを出力する(実際に画面に描いているのはここ)
//
LPBYTE GetBits(HBITMAP hbmp, int x, int y)
{
	BITMAP bm;
	LPBYTE lp;
	
	GetObject(hbmp, sizeof(BITMAP), &bm);

	lp = (LPBYTE)bm.bmBits;
	lp += ((INT_PTR)(bm.bmHeight) - y - 1) * (((INT_PTR)3 * bm.bmWidth + 3) / 4) * 4;
	lp += (INT_PTR)3 * x;

	return lp;
}

int CEditCtrl::TextOutAA(CDC *pdc, CDC *p_paintdc, const TCHAR *p, int len, RECT rect)
{
	HDC	hdc = pdc->GetSafeHdc();

	TEXTMETRIC      tm;
	GetTextMetrics(hdc, &tm);

	COLORREF cr = pdc->GetTextColor();
	COLORREF bk = pdc->GetBkColor();
	int cr_r = GetRValue(cr);
	int cr_g = GetGValue(cr);
	int cr_b = GetBValue(cr);
	int bk_r = GetRValue(bk);
	int bk_g = GetGValue(bk);
	int bk_b = GetBValue(bk);

	BYTE	glyph_buf[1024 * 8];

	for(; len > 0;) {
		GLYPHMETRICS gm;
		CONST MAT2 Mat = {{0,1},{0,0},{0,0},{0,1}};

		// 文字コード取得
		int font_width = GetFontWidth(pdc, get_char(p));
		UINT code = get_char(p);

		RECT bk_rect = { rect.left, rect.top, rect.left + font_width, rect.bottom };
		
		if(p_paintdc->RectVisible(&bk_rect)) {
			FillBackGroundRect(pdc, &bk_rect, bk);

			if(code != ' ') {
				// ビットマップ取得
				DWORD size = GetGlyphOutline(hdc, code, GGO_GRAY8_BITMAP, &gm, 0, NULL, &Mat);

				BYTE *ptr;
				if(size > sizeof(glyph_buf)) {
					ptr = new BYTE[size];
				} else {
					ptr = glyph_buf;
				}

				GetGlyphOutline(hdc, code, GGO_GRAY8_BITMAP, &gm, size, ptr, &Mat);

				int nLine   = ((gm.gmBlackBoxX + 3) / 4) * 4;
				int nWidth  = gm.gmBlackBoxX;
				int nHeight = gm.gmBlackBoxY;

				int x = rect.left + gm.gmptGlyphOrigin.x;
				int y = rect.top + tm.tmAscent - gm.gmptGlyphOrigin.y;

				BYTE *p = ptr;
				for (int i = 0; i < nHeight; i++) {
					p = ptr + ((INT_PTR)i * nLine);
					for (int j = 0; j < nWidth; j++) {
						int alpha = *p;
						p++;

						if (alpha) {
							SetPixel(hdc, x + j, y + i, 
								RGB((cr_r * alpha / 64) + ( bk_r * (64 - alpha) / 64),
									(cr_g * alpha / 64) + ( bk_g * (64 - alpha) / 64),
									(cr_b * alpha / 64) + ( bk_b * (64 - alpha) / 64)));
						}
					}
				}

				// 後片付け
				if(ptr != glyph_buf) delete[] ptr;
			}
		}

		rect.left += font_width;
		len -= get_char_len(p);
		p += get_char_len(p);
	}

	return rect.left;
}

int CEditCtrl::GetFontWidth(CDC *pdc, unsigned int ch, CFontWidthHandler *pdchandler)
{
	int w = m_edit_data->get_disp_data()->GetFontWidth(pdc, ch, pdchandler);
	return w + m_edit_data->get_disp_data()->GetCharSpace(w);
}

int CEditCtrl::TextOut2(CDC *pdc, CDC *p_paintdc, const TCHAR *p, int len, RECT rect, int top_space)
{
	if(m_tt_flg && (m_ex_style & ECS_DRAW_ANTIALIAS)) {
		return TextOutAA(pdc, p_paintdc, p, len, rect);
	}
/*
#ifdef _DEBUG
	if(m_tt_flg) {
		return TextOutAA(pdc, p_paintdc, p, len, rect);
	}
#endif
*/
	UINT textout_options = ETO_OPAQUE;
	if(bk_dc.GetSafeHdc() != NULL && pdc->GetBkColor() == GetColor(BG_COLOR)) {
		textout_options = 0;
	}

	return m_edit_data->get_disp_data()->GetRenderer()->TextOut2(pdc, p_paintdc, p, len, rect,
		m_edit_data->get_disp_data()->GetFontData(), 
		m_row_header_width - GetScrollPos(SB_HORZ) * m_char_width,
		textout_options, top_space);
}

//
// テキストを折り返しながら出力する
//
int CEditCtrl::TextOut(CDispColorData *color_data, CDC *pdc, CDC *p_paintdc,
	const TCHAR *p, int len, RECT &rect, int &line_len, int row,
	int &split, int win_bottom, int h_scr_pos)
{
	int		idx = m_edit_data->get_scroll_row(row);
	int		w = rect.left;

	if(rect.bottom > win_bottom) rect.bottom = win_bottom;

	for(;;) {
		// 折り返し位置か判定する
		if(split < (GetDispRowSplitCnt(row) - 1) && line_len + len > GetDispOffset(idx + split + 1)) {
			int tmp_len = GetDispOffset(idx + split + 1) - line_len;
			if(tmp_len > 0 && is_lead_byte(p[tmp_len - 1])) tmp_len--;

			if(rect.bottom > m_col_header_height) {
				w = TextOut2(pdc, p_paintdc, p, tmp_len, rect, m_row_space_top);
				PaintTextSpace(color_data, pdc, p_paintdc, row, split, rect.top, w);
			}

			p += tmp_len;
			len -= tmp_len;
			line_len += tmp_len;
			split++;

			rect.top = rect.top + m_row_height;
			rect.bottom = rect.top + m_row_height;
			if(rect.bottom > win_bottom) rect.bottom = win_bottom;
			rect.left = -(h_scr_pos * m_char_width) + m_row_header_width;

			// ウィンドウの下端を超えたら，終わり
			if(rect.top > win_bottom) break;
		} else {
			if(rect.bottom > m_col_header_height) {
				w = TextOut2(pdc, p_paintdc, p, len, rect, m_row_space_top);
			}
			line_len += len;
			break;
		}
	}

	return w;
}

__inline void CEditCtrl::SetDispColor(CDispColorData *color_data, COLORREF color, int pos, int len)
{
	for(; len > 0; len--, pos++) {
		color_data->SetColor(pos, color);
	}
}

//
// 行を解析して，テキストの色を決定する
//
void CEditCtrl::MakeDispColorData(CDispColorData *color_data, int row, const TCHAR *p)
{
	int		len;
	BOOL	comment = FALSE;
	BOOL	row_comment = FALSE;
	BOOL	in_quote = FALSE;
	BOOL	in_tag = FALSE;
	BOOL	is_operator = FALSE;
	unsigned int key_word_len;
	int		cur_x;
	int		key_word_type;

	COLORREF disp_color;

	CStrToken	*str_token = m_edit_data->get_str_token();

	if((m_edit_data->get_row_data_flg(row, ROW_DATA_COMMENT)) &&
		(m_ex_style & ECS_NO_COMMENT_CHECK) == 0) comment = TRUE;
	if(m_edit_data->get_row_data_flg(row, ROW_DATA_IN_TAG) &&
		(m_ex_style & ECS_HTML_MODE)) in_tag = TRUE;
	//クォートが閉じられていないとき，以降の行に影響しないようにした
	//if(m_edit_data->get_row_data(row) & ROW_DATA_IN_QUOTE) in_quote = TRUE;

	int bracket_cnt = m_edit_data->get_row_bracket_cnt(row);
	//TRACE(_T("bracket_cnt: %d, %d\n"), row, bracket_cnt);
	COLORREF bracket_color[3] = { GetColor(BRACKET_COLOR1), GetColor(BRACKET_COLOR2), GetColor(BRACKET_COLOR3) };

	cur_x = 0;
	for(;;) {
		len = str_token->get_word_len(p);
		if(len == 0 || *p == '\n') break;

		in_quote = FALSE;
		if(str_token->isQuoteStart(p)) {
			if(comment == TRUE || (m_ex_style & ECS_HTML_MODE && in_tag == FALSE)) {
				len = 1;
			} else {
				in_quote = TRUE;
			}
		}
		if(comment == FALSE) {
			if(in_quote == FALSE) {
				if(str_token->isRowComment(p) == TRUE) {
					row_comment = TRUE;
				} else if(str_token->isOpenComment(p) == TRUE) {
					len = str_token->GetOpenCommentLen();
					comment = TRUE;
				}
			}
		} else if(str_token->isCloseComment(p) == TRUE) {
			len = str_token->GetCloseCommentLen();
		}

		if(m_ex_style & ECS_HTML_MODE) {
			if(comment == FALSE) {
				// タグのチェック
				if(p[0] == '<' && str_token->isOpenComment(p) == FALSE && in_tag == FALSE) {
					in_tag = TRUE;
				}
				//  タグの外にあるキーワードは，色をつけない
				if(in_tag == FALSE) {
					key_word_type = 0;
					in_quote = FALSE;
					is_operator = FALSE;
				} else {
					key_word_type = str_token->GetKeywordType(p, &key_word_len, len);
					is_operator = str_token->isOperatorChar(p[0]);
				}
				// タグのチェック
				if(p[0] == '>' && in_tag == TRUE) {
					in_tag = FALSE;
				}
			}
		} else {
			key_word_type = str_token->GetKeywordType(p, &key_word_len, len);
			is_operator = str_token->isOperatorChar(p[0]);
		}

		if(row_comment == TRUE || comment == TRUE) {
			// コメント
			disp_color = GetColor(COMMENT_COLOR);
		} else if(key_word_type == 1) {
			// キーワード
			len = key_word_len;
			disp_color = GetColor(KEYWORD_COLOR);
		} else if(key_word_type == 2) {
			// キーワード
			len = key_word_len;
			disp_color = GetColor(KEYWORD2_COLOR);
		} else if(in_quote && p[0] != (TCHAR)m_no_quote_color_char) {
			// 囲み文字の中
			disp_color = GetColor(QUOTE_COLOR);
		} else if(is_operator) {
			// 記号
			disp_color = GetColor(OPERATOR_COLOR);

			if(m_ex_style & ECS_BRACKET_MULTI_COLOR) {
				if(*p == '(') {
					disp_color = bracket_color[bracket_cnt % 3];
					bracket_cnt++;
				} else if(*p == ')' && bracket_cnt > 0) {
					bracket_cnt--;
					disp_color = bracket_color[bracket_cnt % 3];
				}
			}
	} else {
			// その他
			disp_color = GetColor(TEXT_COLOR);
		}
		if(disp_color != RGB(0, 0, 0)) {
			// 0で初期化しているので，黒のときは処理をスキップ
			SetDispColor(color_data, disp_color, cur_x, len);
		}

		if(comment == TRUE && str_token->isCloseComment(p) == TRUE) {
			comment = FALSE;
		}

		p = p + len;
		cur_x = cur_x + len;
	}
	color_data->SetMark(cur_x, MARK_END_OF_LINE);
}

//
// 選択範囲を設定する
//
void CEditCtrl::MakeDispSelectedData(CDispColorData *color_data, int row, const TCHAR *p)
{
	if(HaveSelected() == FALSE) return;
	if(IsSelectedRow(row) == FALSE) return;

	// 選択範囲を設定
	int		invert_start_x;
	int		invert_end_x;

	if(GetSelectArea()->box_select) {
		m_edit_data->get_box_x2(row, &invert_start_x, &invert_end_x);
	} else {
		if(row == GetSelectArea()->pos1.y) {
			invert_start_x = GetSelectArea()->pos1.x;
		} else {
			invert_start_x = 0;
		}
		if(row == GetSelectArea()->pos2.y) {
			invert_end_x = GetSelectArea()->pos2.x;
		} else {
			invert_end_x = m_edit_data->get_row_len(row);
		}
	}

	for(int pos = invert_start_x; pos < invert_end_x; pos++) {
		color_data->SetSelected(pos);
	}
}

//
// マークを設定する
//
void CEditCtrl::MakeDispMarkData(CDispColorData *color_data, const TCHAR *p)
{
	const TCHAR *p_start = p;
	unsigned int ch;

	// Tab, 半角空白, 全角空白をマーク
	for(p = p_start; *p != '\0'; p += get_char_len(p)) {
		ch = get_char(p);

		if(ch == L'　') {
			color_data->SetMark((int)(p - p_start), MARK_2BYTE_SPACE);
		} else if(ch == '\t') {
			color_data->SetMark((int)(p - p_start), MARK_TAB);
		} else if(ch == ' ') {
			color_data->SetMark((int)(p - p_start), MARK_SPACE);
		}
	}
}

void CEditCtrl::SetSearchData(CDispColorData *color_data, int pos, int len)
{
	for(; len > 0; len--, pos++) {
		color_data->SetSearched(pos);
	}
}

//
// 検索結果を設定する
//
void CEditCtrl::MakeDispSearchData(CDispColorData *color_data, int row, const TCHAR *p)
{
	if(m_search_data.GetDispSearch() == FALSE) return;

	int		len;
	int		col = 0;

	for(; col <= m_edit_data->get_row_len(row);) {
		col = m_edit_data->strstr_dir_regexp(row, col, 1, &len, m_search_data.GetRegData());
		if(col < 0 || len == 0) break;

		if(col + len > m_edit_data->get_row_len(row)) {
			// FIXME: 検索結果が複数行になる場合に対応する
			int disp_len = m_edit_data->get_row_len(row) - col + 1;
			SetSearchData(color_data, col, disp_len);
			return;
		}

		SetSearchData(color_data, col, len);
		col += len;
	}
}

//
// クリッカブルURLを設定する
//
void CEditCtrl::SetDispClickable(CDispColorData *color_data, int pos, int len)
{
	for(; len > 0; len--, pos++) {
		color_data->SetClickable(pos);
		color_data->SetColor(pos, GetColor(KEYWORD_COLOR));
	}
}

//
// クリッカブルURLを設定する
//
void CEditCtrl::MakeDispClickableDataMain(CDispColorData *color_data, const TCHAR *p, TCHAR *clickable_text)
{
	const TCHAR	*p_start = p;
	int				pos;
	unsigned int	len;

	for(; *p != '\0';) {
		p = _tcsstr(p, clickable_text);
		if(p == NULL) break;

		pos = (int)(p - p_start);
		for(len = 0;; len++, p++) {
			if(*p == '\0' || *p == ' ' || *p == '\t' ||
				*p == '"' || *p == '<' || *p == '>' || 
				*p == '[' || *p == ']' || *p >= 0x80) break;

			// URL中に()が使えるようにする
			if(*p == '(' || *p == ')') {
				BOOL break_flg = FALSE;
				int cnt = 1;
				TCHAR ch;
				for(cnt = 1;; cnt++) {
					ch = *(p + cnt);
					if(ch == '\0' || ch == ' ' || ch == '\t' ||
						ch == '"' || ch == '<' || ch == '>' || 
						ch == '[' || ch == ']' || ch >= 0x80) {
						break_flg = TRUE;
						break;
					}
					if(inline_isalnum(ch) || ch == '.' || ch == '/') break;
				}
				if(break_flg) break;
			}
		}
		if(len > _tcslen(clickable_text)) {
			SetDispClickable(color_data, pos, len);
		}
	}
}

void CEditCtrl::MakeDispClickableDataMail(CDispColorData *color_data, const TCHAR *p)
{
	const TCHAR	*p_start = p;
	const TCHAR	*p_at;
	int				pos, len, pos_at;

	for(; *p != '\0';) {
		p_at = my_mbschr(p, '@');
		if(p_at == NULL) break;

		// 先頭位置を求める
		for(p = p_at - 1; p >= p_start; p--) {
			if(!inline_isalnum(*p) && *p != '_' && *p != '-' && *p != '.' && *p != '+') {
				if(is_low_surrogate(*p)) p++;
				break;
			}
		}
		p++;
		pos = (int)(p - p_start);
		ASSERT(pos >= 0);

		pos_at = (int)(p_at - p_start);
		len = pos_at - pos + 1;
		if(len == 1) {
			p = p_at + 1;
			continue;
		}

		// 終了位置を求める
		p = p_at + 1;
		for(; *p != '\0'; p++, len++) {
			if(is_lead_byte(*p)) break;
			if(!inline_isalnum(*p) && *p != '_' && *p != '-' && *p != '.') break;
		}
		if(p == p_at + 1) continue;

		SetDispClickable(color_data, pos, len);
	}
}

void CEditCtrl::MakeDispClickableData(CDispColorData *color_data, const TCHAR *p)
{
	if(!(m_ex_style & ECS_CLICKABLE_URL)) return;

	for(int i = 0; i < sizeof(clickable_url_str)/sizeof(clickable_url_str[0]); i++) {
		MakeDispClickableDataMain(color_data, p, clickable_url_str[i]);
	}
	MakeDispClickableDataMail(color_data, p);
}

//
// 括弧の強調表示を設定する
//
void CEditCtrl::MakeDispBracketsData(CDispColorData* color_data, int row, const TCHAR* p)
{
	if(m_edit_data->get_disp_data()->GetBracketPos().y == row) {
		color_data->SetBold(m_edit_data->get_disp_data()->GetBracketPos().x);
	}
	if(m_edit_data->get_disp_data()->GetBracketPos2().y == row) {
		color_data->SetBold(m_edit_data->get_disp_data()->GetBracketPos2().x);
	}
}

//
// 指定された色に従って，テキストを出力する
//
void CEditCtrl::PaintTextMain(CDispColorData *color_data, CDC *pdc, CDC *p_paintdc, 
	int h_scr_pos, int rect_top, int row, const TCHAR *p)
{
	RECT	rect, winrect;
	int		disp_bottom, disp_right;
	int		line_len, split;
	CFont	*old_font;

	GetDispRect(&winrect);
	disp_bottom = winrect.bottom - m_bottom_space;
	disp_right = winrect.right - m_right_space;

	rect.top = rect_top;
	rect.bottom = rect.top + m_row_height;
	rect.left = -(h_scr_pos * m_char_width) + m_row_header_width;
	rect.right = disp_right;

	line_len = 0;
	split = 0;

	int		cur_x, len, pos;
	cur_x = 0;
	COLORREF	bk_color_org = pdc->GetBkColor();

	if(rect.top < m_col_header_height) {
		rect.top = m_col_header_height;
		rect.bottom = rect.top + m_row_height;
		cur_x = GetDispOffset(GetScrollPos(SB_VERT));
		p = p + cur_x;
		split = GetScrollPos(SB_VERT) - m_edit_data->get_scroll_row(row);
		line_len = GetDispOffset(m_edit_data->get_scroll_row(row) + split);
	}

	pdc->SetBkColor(GetColor(BG_COLOR));
	for(;;) {
		if(*p == '\0' || *p == '\n') break;

		// 最後の行で，ウィンドウの右端を超えたら，終わり
		if(rect.left > disp_right && split == GetDispRowSplitCnt(row) - 1) break;
		// ウィンドウの下端を超えたら，終わり
		if(rect.top > disp_bottom) break;

		for(len = 1, pos = cur_x;; len++, pos++) {
			// 2byte文字の途中で切れないようにする
			if(is_lead_byte(*(p + len - 1))) {
				if(color_data->DispColorDataCmp(pos, pos + 1) != 0) break;
				len++;
				pos++;
			}
			if(color_data->DispColorDataCmp(pos, pos + 1) != 0) break;
		}

		if(color_data->GetSelected(cur_x)) {
			pdc->SetBkColor(GetColor(SELECTED_COLOR));
			if(m_ex_style & ECS_NO_INVERT_SELECT_TEXT) {
				pdc->SetTextColor((color_data->GetColor(cur_x)));
			} else {
				pdc->SetTextColor(RGB(0xff, 0xff, 0xff) ^ (color_data->GetColor(cur_x)));
			}
		} else if(color_data->GetSearched(cur_x)) {
			pdc->SetBkColor(GetColor(SEARCH_COLOR));
			pdc->SetTextColor((color_data->GetColor(cur_x)));
		} else {
			pdc->SetBkColor(GetColor(BG_COLOR));
			pdc->SetTextColor((color_data->GetColor(cur_x)));
		}

		if(color_data->GetClickable(cur_x)) {
			old_font = pdc->SelectObject(&m_clickable_font);
		} else if(color_data->GetBold(cur_x)) {
			old_font = pdc->SelectObject(&m_bold_font);
		} else {
			old_font = NULL;
		}

		int left;

		if(color_data->GetMark(cur_x) == MARK_SPACE) {
			len = 1;
			left = TextOut(color_data, pdc, p_paintdc, _T(" "), 1, rect, line_len, 
				row, split, disp_bottom, h_scr_pos);
			if(m_ex_style & ECS_SHOW_SPACE) PaintSpace(pdc, winrect, rect.top, rect.left);
			rect.left = left;
		} else if(color_data->GetMark(cur_x) == MARK_TAB) {
			len = 1;
			left = TextOut(color_data, pdc, p_paintdc, _T("\t"), 1, rect, line_len, 
				row, split, disp_bottom, h_scr_pos);
			if(m_ex_style & ECS_SHOW_TAB) PaintTab(pdc, winrect, rect.top, rect.left);
			rect.left = left;
		} else if(color_data->GetMark(cur_x) == MARK_2BYTE_SPACE) {
			len = 1;
			left = TextOut(color_data, pdc, p_paintdc, p, len, rect, line_len, 
				row, split, disp_bottom, h_scr_pos);
			if(m_ex_style & ECS_SHOW_2BYTE_SPACE) Paint2byteSpace(pdc, winrect, rect.top, rect.left);
			rect.left = left;
		} else {
			rect.left = TextOut(color_data, pdc, p_paintdc, p, len, rect, line_len, 
				row, split, disp_bottom, h_scr_pos);
		}

		if(old_font) pdc->SelectObject(old_font);

		p = p + len;
		cur_x = cur_x + len;
	}
	pdc->SetBkColor(bk_color_org);

	// 行番号や右端の空白を描く
	PaintTextSpace(color_data, pdc, p_paintdc, row, split, rect.top, rect.left);

	// 括弧の対応を表示
	if(row == m_edit_data->get_disp_data()->GetHighlightPos().y) {
		InvertPt(pdc, m_edit_data->get_disp_data()->GetHighlightPos());
	}
	if(row == m_edit_data->get_disp_data()->GetHighlightPos2().y) {
		InvertPt(pdc, m_edit_data->get_disp_data()->GetHighlightPos2());
	}
}

void CEditCtrl::InvertPt(CDC *pdc, POINT char_pt)
{
	POINT	disp_pt, caret_pt;
	GetDispDataPoint(char_pt, &disp_pt);
	GetDispCaretPoint(disp_pt, &caret_pt);

	// FIXME: 上下左右のspaceを考慮する

	// 点滅中に編集されて、無効な位置になった場合の処理
	if(!m_edit_data->is_valid_point(char_pt)) {
		return;
	}

	TCHAR ch = m_edit_data->get_char(char_pt.y, char_pt.x);
	
	RECT bk_rect;
	bk_rect.top = caret_pt.y + m_row_space_top;
	bk_rect.bottom = bk_rect.top + m_font_height;
	bk_rect.left = caret_pt.x;
	bk_rect.right = bk_rect.left + GetFontWidth(pdc, ch);

	pdc->InvertRect(&bk_rect);
}

//
// 行のテキストを出力する
//
void CEditCtrl::PaintText(CDC *pdc, CDC *p_paintdc, int h_scr_pos, int rect_top, int row, int *bottom)
{
	RECT	rect, winrect;
	const TCHAR	*p;

	GetDispRect(&winrect);

	rect.top = rect_top;
	rect.bottom = rect.top + GetDispRowSplitCnt(row) * m_row_height;
	rect.left = winrect.left;
	rect.right = winrect.right;
	if(rect.bottom > winrect.bottom - m_bottom_space) rect.bottom = winrect.bottom - m_bottom_space;

	*bottom = rect.bottom;

	if(rect.top > winrect.bottom || rect.bottom < m_col_header_height) return;
	if(!p_paintdc->RectVisible(&rect)) {
		return;
	}

	CDispColorData *p_disp_color_data = m_edit_data->get_disp_data()->GetDispColorDataPool()->GetDispColorData(row);
	if(p_disp_color_data == NULL) {
		p_disp_color_data = m_edit_data->get_disp_data()->GetDispColorDataPool()->GetNewDispColorData(row);

		p = m_edit_data->get_disp_row_text(row);
		MakeDispColorData(p_disp_color_data, row, p);
		MakeDispSelectedData(p_disp_color_data, row, p);
		MakeDispSearchData(p_disp_color_data, row, p);
		MakeDispMarkData(p_disp_color_data, p);
		MakeDispClickableData(p_disp_color_data, p);
		MakeDispBracketsData(p_disp_color_data, row, p);

		p_disp_color_data->SetRowData(m_edit_data->get_edit_row_data(row), row);
	} else {
		p = m_edit_data->get_disp_row_text(row);
		MakeDispSelectedData(p_disp_color_data, row, p);
		MakeDispBracketsData(p_disp_color_data, row, p);
	}

	PaintTextMain(p_disp_color_data, pdc, p_paintdc, h_scr_pos, rect_top, row, p);

	if(IsSelectedRow(row)) p_disp_color_data->ClearSelected();
}

void CEditCtrl::PaintTextSpace(CDispColorData *color_data, CDC *pdc, CDC *p_paintdc,
	int row, int split, int top, int left)
{
	if(top < m_col_header_height) return;

	COLORREF	bk_color_org = pdc->GetBkColor();

	CRect	winrect, bk_rect;

	GetDispRect(&winrect);

	// 右側を白く塗る
	if(left < winrect.right) {
		bk_rect.top = top;
		bk_rect.bottom = bk_rect.top + m_row_height;
		bk_rect.left = left;
		if(bk_rect.left < m_row_header_width) bk_rect.left = m_row_header_width;
		bk_rect.right = winrect.right;
		if(p_paintdc->RectVisible(&bk_rect)) {
			FillBackGroundRect(pdc, &bk_rect, GetColor(BG_COLOR));
			if(left >= m_row_header_width) {
				if(split == GetDispRowSplitCnt(row) - 1) {
					if(HaveSelected() && row >= GetSelectArea()->pos1.y && row < GetSelectArea()->pos2.y &&
						GetSelectArea()->box_select == FALSE) {
						bk_rect.right = bk_rect.left + m_char_width;
						if(bk_rect.right >= winrect.Width() - m_right_space) bk_rect.right = winrect.Width() - m_right_space;
						pdc->InvertRect(&bk_rect);
						FillBackGroundRect(pdc, &bk_rect, GetColor(SELECTED_COLOR));
					} else if(color_data->GetSearched((m_edit_data->get_row_len(row)))) {
						bk_rect.right = bk_rect.left + m_char_width;
						if(bk_rect.right >= winrect.Width() - m_right_space) bk_rect.right = winrect.Width() - m_right_space;
						pdc->InvertRect(&bk_rect);
						FillBackGroundRect(pdc, &bk_rect, GetColor(SEARCH_COLOR));
					}
					if(m_ex_style & ECS_SHOW_LINE_FEED) {
						if(row + 1 == m_edit_data->get_row_cnt()) {
							PaintEOF(pdc, winrect, bk_rect.top, bk_rect.left);
						} else {
							PaintLineFeed(pdc, winrect, bk_rect.top, bk_rect.left);
						}
					}
				} else {
					if(m_ex_style & ECS_SHOW_LINE_END) {
						PaintLineEnd(pdc, winrect, bk_rect.top, bk_rect.left);
					}
				}
			}
		}
	}

	// 左側の隙間を白く塗る
	bk_rect.top = top;
	bk_rect.bottom = bk_rect.top + m_row_height;
	bk_rect.left = 0;
	bk_rect.right = m_row_header_width;
	if(p_paintdc->RectVisible(&bk_rect)) {
		FillBackGroundRect(pdc, &bk_rect, GetColor(BG_COLOR));

		if(m_left_space > 1) {
			// ブックマーク
			if(split == 0 && m_edit_data->get_row_data_flg(row, ROW_DATA_BOOK_MARK)) {
				RECT	book_mark_rect = bk_rect;
				book_mark_rect.left = 1;
				book_mark_rect.right = 1 + m_left_space / 2;
				if(p_paintdc->RectVisible(&book_mark_rect)) {
					FillBackGroundRect(pdc, &book_mark_rect, GetColor(QUOTE_COLOR));
				}
			}

			// 編集マーク
			if(m_edit_data->is_edit_row(row) && (m_ex_style & ECS_SHOW_EDIT_ROW)) {
				RECT	book_mark_rect = bk_rect;
				book_mark_rect.left = 1 + m_left_space / 2;
				book_mark_rect.right = m_left_space;
				if(p_paintdc->RectVisible(&book_mark_rect)) {
					FillBackGroundRect(pdc, &book_mark_rect, GetColor(KEYWORD_COLOR));
				}
			}
		}

		// 現在行を反転する
		BOOL invert_flg = FALSE;

		if(row == m_edit_data->get_cur_row() && split == m_edit_data->get_cur_split()) {
			invert_flg = TRUE;
		}

		// 行番号
		if(m_ex_style & ECS_SHOW_ROW_NUM) {
			if(invert_flg) {
				bk_rect.left = m_left_space;
				bk_rect.right = m_row_header_width - m_num_width;
				FillBackGroundRect(pdc, &bk_rect, GetColor(SELECTED_COLOR));
			}
				
			if(split == 0) {
				COLORREF old_color = pdc->GetTextColor();
				if(row == m_edit_data->get_cur_row() && invert_flg) {
					pdc->SetBkColor(GetColor(SELECTED_COLOR));
					if(m_ex_style & ECS_NO_INVERT_SELECT_TEXT) {
						pdc->SetTextColor(GetColor(KEYWORD_COLOR));
					} else {
						pdc->SetTextColor(RGB(0xff, 0xff, 0xff) ^ GetColor(KEYWORD_COLOR));
					}
				} else {
					pdc->SetBkColor(GetColor(BG_COLOR));
					pdc->SetTextColor(GetColor(RULER_COLOR));
				}

				// FIXME: 20バイト固定をやめる
				TCHAR	row_num[20];
				_stprintf(row_num, _T("%d"), row + 1);
				int num_len = (int)_tcslen(row_num);

				bk_rect.left = m_left_space + (m_row_num_digit - num_len + 1) * (m_num_width);

				CFont *old_font = pdc->SelectObject(&m_font);
				TextOut2(pdc, p_paintdc, row_num, num_len, bk_rect, m_row_space_top);
				pdc->SelectObject(old_font);

				pdc->SetTextColor(old_color);
			}

			bk_rect.right = m_row_header_width - m_char_width / 2 - 1;
			CPen *old_pen = pdc->SelectObject(m_edit_data->get_disp_data()->GetRulerPen());
			pdc->MoveTo(bk_rect.right, bk_rect.top);
			pdc->LineTo(bk_rect.right, bk_rect.bottom);
			pdc->SelectObject(old_pen);
		}
	}

	pdc->SetBkColor(bk_color_org);
}

BOOL CEditCtrl::RectVisibleMark(const RECT &client_rect, const RECT &mark_rect)
{
	if(mark_rect.bottom < m_col_header_height || mark_rect.right < m_row_header_width) return FALSE;
	if(mark_rect.left >= client_rect.right - m_right_space || 
		mark_rect.top >= client_rect.bottom - m_bottom_space) return FALSE;

	return TRUE;
}

void CEditCtrl::PaintEOF(CDC *pdc, const RECT &client_rect, int top, int left)
{
	RECT	rect;

	int width = GetFontWidth(pdc, L'x');

	rect.top = top;
	rect.bottom = rect.top + m_font_height;
	rect.left = left;
	rect.right = rect.left + width * 2;

	if(!RectVisibleMark(client_rect, rect)) return;

	CPen *old_pen = pdc->SelectObject(m_edit_data->get_disp_data()->GetMarkPen());

	int		y, x1, x2;
	y = rect.top + m_font_height / 2;
	x1 = rect.left + width * 2 - m_char_width / 4 - 3;
	x2 = rect.left + width / 4 + 1;

	pdc->MoveTo(x1, y);
	pdc->LineTo(x2, y);

	pdc->MoveTo(x2, y);
	pdc->LineTo(x2 + width / 2, y + m_font_height / 4);
	pdc->MoveTo(x2, y);
	pdc->LineTo(x2 + width / 2, y - m_font_height / 4);

	pdc->SelectObject(old_pen);
}

void CEditCtrl::PaintLineEnd(CDC *pdc, const RECT &client_rect, int top, int left)
{
	RECT	rect;

	int width = GetFontWidth(pdc, L'x');

	rect.top = top;
	rect.bottom = top + m_font_height;
	rect.left = left;
	rect.right = rect.left + width;

	if(!RectVisibleMark(client_rect, rect)) return;

	CPen *old_pen = pdc->SelectObject(m_edit_data->get_disp_data()->GetMarkPen());

	int		y, x1, x2;
	y = rect.top + m_font_height / 2;
	x2 = rect.left + width - m_char_width / 4 - 1;
	x1 = rect.left + width / 4;

	pdc->MoveTo(x1, y);
	pdc->LineTo(x2, y + m_font_height / 4);
	pdc->MoveTo(x1, y);
	pdc->LineTo(x2, y - m_font_height / 4);

	pdc->SelectObject(old_pen);
}

void CEditCtrl::PaintLineFeed(CDC *pdc, const RECT &client_rect, int top, int left)
{
	RECT	rect;

	int width = GetFontWidth(pdc, L'x');

	rect.top = top;
	rect.bottom = rect.top + m_font_height;
	rect.left = left;
	rect.right = rect.left + width * 2;

	if(!RectVisibleMark(client_rect, rect)) return;

	CPen *old_pen = pdc->SelectObject(m_edit_data->get_disp_data()->GetMarkPen());

	int		x, y1, y2;
	x = rect.left + (width / 2) + 1;
	y1 = rect.top + m_font_height / 8 + 2;
	y2 = rect.bottom - m_font_height / 8 - 2;

	// 縦線
	pdc->MoveTo(x, y1);
	pdc->LineTo(x, y2);

	// 矢印
	pdc->MoveTo(x, y2);
	pdc->LineTo(x + width / 2, y2 - m_font_height / 4);
	pdc->MoveTo(x, y2);
	pdc->LineTo(x - width / 2, y2 - m_font_height / 4);

	pdc->SelectObject(old_pen);
}

void CEditCtrl::PaintTab(CDC *pdc, const RECT &client_rect, int top, int left)
{
	RECT	rect;

	int width = GetFontWidth(pdc, L'x');

	rect.top = top;
	rect.bottom = top + m_font_height;
	rect.left = left;
	rect.right = rect.left + width;

	if(!RectVisibleMark(client_rect, rect)) return;

	CPen *old_pen = pdc->SelectObject(m_edit_data->get_disp_data()->GetMarkPen());

	int		y, x1, x2;
	y = rect.top + m_font_height / 2;
	x1 = rect.left + width - width / 4 - 1;
	x2 = rect.left + width / 4;
	
	pdc->MoveTo(x1, y);
	pdc->LineTo(x2, y + m_font_height / 4);
	pdc->MoveTo(x1, y);
	pdc->LineTo(x2, y - m_font_height / 4);

	pdc->SelectObject(old_pen);
}

void CEditCtrl::PaintSpace(CDC *pdc, const RECT &client_rect, int top, int left)
{
	RECT	rect;

	rect.top = top;
	rect.bottom = top + m_font_height - 1;
	rect.left = left;
	rect.right = rect.left + GetFontWidth(pdc, ' ') - 2;

	if(!RectVisibleMark(client_rect, rect)) return;

	CPen *old_pen = pdc->SelectObject(m_edit_data->get_disp_data()->GetMarkPen());

	POINT	poly[4] = {
		{rect.left, rect.bottom - 1},
		{rect.left, rect.bottom},
		{rect.right, rect.bottom},
		{rect.right, rect.bottom - 2} };
	
	pdc->Polyline(poly, 4);

	pdc->SelectObject(old_pen);
}

void CEditCtrl::Paint2byteSpace(CDC *pdc, const RECT &client_rect, int top, int left)
{
	RECT	rect;

	rect.top = top + 2;
	rect.bottom = top + m_font_height - 2;
	rect.left = left + 1;
	rect.right = rect.left + GetFontWidth(pdc, L'　') - 3;

	if(!RectVisibleMark(client_rect, rect)) return;

	CPen *old_pen = pdc->SelectObject(m_edit_data->get_disp_data()->GetMarkPen());

	POINT	poly[5] = {
		{rect.left, rect.top},
		{rect.left, rect.bottom},
		{rect.right, rect.bottom},
		{rect.right, rect.top},
		{rect.left, rect.top} };
	
	pdc->Polyline(poly, 5);

	pdc->SelectObject(old_pen);
}

void CEditCtrl::PaintRowLine(CDC *pdc, CDC *p_paintdc)
{
	RECT	winrect, bk_rect;
	GetDispRect(&winrect);

	bk_rect.left = m_row_header_width;
	bk_rect.right = winrect.right;
	bk_rect.top = (m_edit_data->get_disp_data()->GetCaretPos()->y - GetScrollPos(SB_VERT)) * 
		m_row_height + m_row_height + m_col_header_height - 1;
	bk_rect.bottom = bk_rect.top + 1;

	if(bk_rect.top <= m_col_header_height || bk_rect.top > winrect.bottom ||
		!p_paintdc->RectVisible(&bk_rect)) return;

	if(m_edit_data->get_disp_data()->GetLineMode() == EC_LINE_MODE_LEN) {
		int right = (m_edit_data->get_disp_data()->GetLineLen() - GetScrollPos(SB_HORZ)) * m_char_width + m_row_header_width;
		if(bk_rect.right > right) bk_rect.right = right;
	}

	CPen *old_pen;
	old_pen = pdc->SelectObject(m_edit_data->get_disp_data()->GetRulerPen());
	pdc->MoveTo(bk_rect.left, bk_rect.top);
	pdc->LineTo(bk_rect.right, bk_rect.top);
	pdc->SelectObject(old_pen);
}

BOOL CEditCtrl::SetBackGroundImage(const TCHAR *file_name, BOOL screen_size_flg)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	HGLOBAL hGlobal = NULL;
	IStream *iStream = NULL;
	void *mem = NULL;
	ULONG nFileSize;
	ULONG nReadByte;

	ClearBackGround();

	if(m_iPicture != NULL) {
		m_iPicture->Release();
		m_iPicture = NULL;
	}
	if(file_name == NULL || _tcslen(file_name) == 0) {
		return TRUE;
	}

	hFile = CreateFile(file_name, 
		GENERIC_READ, 0, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE) goto ERR1;

	nFileSize = GetFileSize(hFile, NULL);
	
	hGlobal = GlobalAlloc(GPTR, nFileSize);
	if(hGlobal == NULL) goto ERR1;

	mem = GlobalLock(hGlobal);

	nReadByte;
	ReadFile(hFile, mem, nFileSize, &nReadByte, NULL);

	iStream = NULL;
	if(CreateStreamOnHGlobal(hGlobal, TRUE, &iStream) != S_OK) goto ERR1;

	if(OleLoadPicture(iStream, nFileSize, TRUE, IID_IPicture, (void **)&m_iPicture) != S_OK) goto ERR1;

	CloseHandle(hFile);
	iStream->Release();
	GlobalFree(mem);
	GlobalFree(hGlobal);

	m_iPicture->get_Width(&m_bk_width_hi);
	m_iPicture->get_Height(&m_bk_height_hi);

	m_bg_screen_size = screen_size_flg;
	bk_dc_size.cx = -1;
	bk_dc_size.cy = -1;

	SetScrollStyle(GetScrollStyle() | LOCK_WINDOW);

	return TRUE;

ERR1:
	if(hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
	if(iStream != NULL) iStream->Release();
	if(mem != NULL) GlobalFree(mem);
	if(hGlobal != NULL) GlobalFree(hGlobal);
	return FALSE;
}

void CEditCtrl::FillBackGroundRect(CDC *pdc, RECT *rect, COLORREF cr)
{
//	if(m_iPicture != NULL && cr == GetColor(BG_COLOR)) {
	if(bk_dc.GetSafeHdc() != NULL && cr == GetColor(BG_COLOR)) {
		CBitmap *old_bk_bmp = (CBitmap *)bk_dc.SelectObject(&bk_bmp);
		pdc->BitBlt(rect->left, rect->top, rect->right, rect->bottom, &bk_dc, 
			rect->left, rect->top, SRCCOPY);
		bk_dc.SelectObject(old_bk_bmp);
	} else {
		pdc->FillSolidRect(rect, cr);
	}
}

void CEditCtrl::PaintMain(CDC *pdc, CDC *p_paintdc)
{
	int		bottom = 0;
	CFont	*old_font;

	pdc->SetBkColor(GetColor(BG_COLOR));

	// 列番号を出力
	old_font = pdc->SelectObject(&m_font);
	PaintColNum(pdc, p_paintdc);
	pdc->SelectObject(old_font);

	// テキストを出力
	old_font = pdc->SelectObject(&m_font);
	int row_bottom = GetDispRowBottom();
	int row = GetDispRowTop();
	int rect_top = GetDispRowPos(row);
	int h_scr_pos = GetScrollPos(SB_HORZ);
	for(; row <= row_bottom; row++) {
		PaintText(pdc, p_paintdc, h_scr_pos, rect_top, row, &bottom);
		rect_top = bottom;
	}
	pdc->SelectObject(old_font);

	RECT	winrect, bk_rect;
	GetDispRect(&winrect);

	// 現在行に線を出力
	if(m_ex_style & ECS_SHOW_ROW_LINE) {
		PaintRowLine(pdc, p_paintdc);
	}

	// 下側を白く塗る
	bk_rect.left = 0;
	bk_rect.right = winrect.right;
	bk_rect.top = bottom;
	bk_rect.bottom = winrect.bottom;
	if(bk_rect.top < bk_rect.bottom && p_paintdc->RectVisible(&bk_rect)) {
		FillBackGroundRect(pdc, &bk_rect, GetColor(BG_COLOR));
	}

	// 上側を白く塗る
	bk_rect.left = 0;
	bk_rect.right = winrect.right;
	bk_rect.top = 0;
	bk_rect.bottom = m_top_space;
	if(p_paintdc->RectVisible(&bk_rect)) {
		FillBackGroundRect(pdc, &bk_rect, GetColor(BG_COLOR));
	}
}

void CEditCtrl::ClearBackGround()
{
	if(bk_dc.GetSafeHdc() != NULL) {
		bk_dc.DeleteDC();
		bk_bmp.DeleteObject();
	}
	bk_dc_size.cx = -1;
	bk_dc_size.cy = -1;
}

void CEditCtrl::PaintBackGround(CDC *pdc, int width, int height)
{
	if(bk_dc_size.cx == width && bk_dc_size.cy == height) return;

	ClearBackGround();
	bk_dc.CreateCompatibleDC(pdc);
	bk_bmp.CreateCompatibleBitmap(pdc, width, height);

	CBitmap *old_bk_bmp2 = (CBitmap *)bk_dc.SelectObject(&bk_bmp);

	bk_dc.FillSolidRect(CRect(0, 0, width, height), GetColor(BG_COLOR));

	// 背景画像
	if(m_iPicture != NULL) {
		if(!m_bg_screen_size) {
			int dpi_y = bk_dc.GetDeviceCaps(LOGPIXELSY);
			int dpi_x = bk_dc.GetDeviceCaps(LOGPIXELSX);
			int HPI = 2540;

			LONG img_width = ( dpi_x * m_bk_width_hi + HPI/2 ) / HPI;
			LONG img_height = ( dpi_y * m_bk_height_hi + HPI/2 ) / HPI;

			m_iPicture->Render(bk_dc.GetSafeHdc(), 0, 0, img_width, img_height,
				0, m_bk_height_hi, m_bk_width_hi, -m_bk_height_hi, NULL);
		} else {
			m_iPicture->Render(bk_dc.GetSafeHdc(), 0, 0, width, height,
				0, m_bk_height_hi, m_bk_width_hi, -m_bk_height_hi, NULL);
		}
	}

	// 罫線
	if(m_ex_style & ECS_SHOW_RULED_LINE) {
		CBrush b;
		b.CreateSolidBrush(GetColor(RULED_LINE_COLOR));

		int	r;
		int show_row = GetShowRow();
		int right = width - m_right_space;
		if(m_edit_data->get_disp_data()->GetLineMode() == EC_LINE_MODE_LEN) {
			right = m_edit_data->get_disp_data()->GetLineWidth() + m_row_header_width;
		}

		for(r = 0; r < show_row; r++) {
			RECT rect;
//			rect.top = ((r + 1) * m_row_height) + m_col_header_height - m_row_space + m_row_space_top + 1;
			rect.top = r * m_row_height + m_row_height + m_col_header_height - 1;
			rect.bottom = rect.top + 1;
			rect.left = m_left_space;
			rect.right = right;

			if(rect.bottom > height - m_bottom_space) break;

			if(pdc->RectVisible(&rect)) {
				bk_dc.FillRect(&rect, &b);
			}
		}
	}

	bk_dc.SelectObject(old_bk_bmp2);
	bk_dc_size.cx = width;
	bk_dc_size.cy = height;
}

void CEditCtrl::OnPaint() 
{
	CPaintDC dc(this); // 描画用のデバイス コンテキスト

#ifdef USE_DOUBLE_BUFFERING
	// ダブルバッファリングで，画面のちらつきを抑える
	RECT rcClient;
	GetDispRect(&rcClient);

	if(!dc.RectVisible(&rcClient)) return;

	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	memDC.SetBkMode(TRANSPARENT);

	CBitmap cBmp;
	cBmp.CreateCompatibleBitmap(&dc, rcClient.right, rcClient.bottom);
	CBitmap *pOldBmp = (CBitmap *)memDC.SelectObject(&cBmp);

	if(m_iPicture != NULL || (m_ex_style & ECS_SHOW_RULED_LINE)) {
		PaintBackGround(&dc, rcClient.right, rcClient.bottom);

		CBitmap *old_bk_bmp = (CBitmap *)bk_dc.SelectObject(&bk_bmp);
		memDC.BitBlt(0, 0, rcClient.right, rcClient.bottom, &bk_dc, 0, 0, SRCCOPY);
		bk_dc.SelectObject(old_bk_bmp);
	}

	PaintMain(&memDC, &dc);

	if(m_ex_style & ECS_FULL_SCREEN_MODE) {
		if(m_null_cursor_cnt > 0) {
#define BOX_SPACE	10
			int right = rcClient.right - m_right_space;
			if(m_edit_data->get_disp_data()->GetLineMode() == EC_LINE_MODE_LEN) {
				right = m_edit_data->get_disp_data()->GetLineWidth() + m_left_space;
			}

			RECT rect;
			rect.top = m_top_space - BOX_SPACE;
			rect.left = m_left_space - BOX_SPACE;
			rect.bottom = rcClient.bottom - m_bottom_space + BOX_SPACE;
			rect.right = right + BOX_SPACE;
			CBrush b;
			b.CreateSolidBrush(GetColor(RULED_LINE_COLOR));
			memDC.FrameRect(&rect, &b);
		}
	}

	dc.BitBlt(0, 0, rcClient.right, rcClient.bottom, &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(pOldBmp);

	memDC.DeleteDC();
	cBmp.DeleteObject();
#else
	PaintMain(&dc, &dc);
#endif
}

void CEditCtrl::CalcShowRow()
{
	// Windowサイズ変更，フォント変更時に再計算
	CRect	winrect;
	GetDispRect(winrect);
	m_show_row = ((winrect.Height() - m_col_header_height - m_bottom_space) / m_row_height) + 1;
}

int CEditCtrl::GetShowRow()
{
/* 実行される回数が多いので，CalcShowRow()で事前に計算しておくように変更
	int		result;
	CRect	winrect;

	GetDispRect(winrect);

	result = (winrect.Height() / m_row_height) + 1;

	return result;
*/
	return m_show_row;
}

int CEditCtrl::GetShowCol()
{
	int		result;
	CRect	winrect;

	GetDispRect(winrect);
	result = ((winrect.Width() - m_row_header_width - m_right_space) / m_char_width) + 1;

	return result;
}

BOOL CEditCtrl::HaveSelected()
{
	return m_edit_data->get_disp_data()->HaveSelected();
}

BOOL CEditCtrl::HaveSelectedFullOneLine()
{
	// 1行全体を選択しているとき，TRUEを返す
	if(HaveSelected() == FALSE) return FALSE;

	if(GetSelectArea()->pos1.y != GetSelectArea()->pos2.y) return FALSE;

	if(GetSelectArea()->pos1.x != 0) return FALSE;
	if(GetSelectArea()->pos2.x != m_edit_data->get_row_len(GetSelectArea()->pos2.y)) return FALSE;

	return TRUE;
}

BOOL CEditCtrl::HaveSelectedMultiLine()
{
	if(HaveSelected() == FALSE) return FALSE;

	if(GetSelectArea()->pos1.y == GetSelectArea()->pos2.y) return FALSE;

	return TRUE;
}

void CEditCtrl::SetSelectedPoint(POINT pt1, POINT pt2, BOOL box_select)
{
	// 範囲選択の開始位置と終了位置が同じときは、カーソルを移動して終了
	if(!box_select && inline_pt_cmp(&pt1, &pt2) == 0) {
		m_edit_data->set_cur(pt1.y, pt2.x);
		SetCaret(TRUE, 1);
		return;
	}

	GetSelectArea()->next_box_select = box_select;
	StartSelect(pt1);
	EndSelect(pt2);
}

void CEditCtrl::GetSelectedPoint(POINT *pt1, POINT *pt2)
{
	*pt1 = GetSelectArea()->pos1;
	*pt2 = GetSelectArea()->pos2;
}

void CEditCtrl::ClearSelected(BOOL redraw)
{
	if(HaveSelected() == TRUE) {
		if(redraw) InvalidateRows_AllWnd(GetSelectArea()->pos1.y, GetSelectArea()->pos2.y);
		m_edit_data->get_disp_data()->ClearSelected();

		// 親ウィンドウに通知
		GetParent()->SendMessage(EC_WM_CHANGE_SELECTED_TEXT, 0, 0);
	}

	// キーワード補完の状態をクリア
	ClearKeywordCompletion();
}

BOOL CEditCtrl::IsSelectedRow(int row)
{
	if(HaveSelected() && 
		GetSelectArea()->pos1.y <= row && GetSelectArea()->pos2.y >= row) return TRUE;
	return FALSE;
}

static int CALLBACK MyEnumFontFamExProc(const struct tagLOGFONTW *lpelfe, 
	const struct tagTEXTMETRICW *lpntme, ULONG FontType, LPARAM lParam)
{
	int *pi = (int *)lParam;

	if(FontType & TRUETYPE_FONTTYPE) {
		*pi = 1;
	} else {
		*pi = 0;
	}

	return 0;
}

void CEditCtrl::PostSetFont()
{
	LOGFONT lf;
	m_font.GetLogFont(&lf);

	CDC *pdc = GetDC();

	EnumFontFamiliesEx(pdc->GetSafeHdc(), &lf, &MyEnumFontFamExProc, (LPARAM)&m_tt_flg, 0);
	
	TEXTMETRIC tm;
	pdc->SetMapMode(MM_TEXT);
	CFont *pOldFont = pdc->SelectObject(&m_font);
	pdc->GetTextMetrics(&tm);

	m_edit_data->get_disp_data()->InitFontWidth(pdc);

	m_num_width = GetFontWidth(pdc, '0');

	pdc->SelectObject(pOldFont);
	ReleaseDC(pdc);

	m_font_height = tm.tmHeight + tm.tmExternalLeading;
	m_row_height = m_font_height + m_row_space;
	m_char_width = tm.tmAveCharWidth + m_edit_data->get_disp_data()->GetCharSpace(m_num_width);

	SetHeaderSize();

	if(m_edit_data->get_disp_data()->GetLineMode() == EC_LINE_MODE_RIGHT) {
		CRect	rect;
		GetDispRect(rect);
		SetLineModeRight(rect.Width());
		MakeDispData();
	} else if(m_edit_data->get_disp_data()->GetLineMode() == EC_LINE_MODE_LEN) {
		SetLineMode(EC_LINE_MODE_LEN, m_edit_data->get_disp_data()->GetLineLen(), FALSE);
		MakeDispData();
	} else {
		MakeDispData();
	}

	CalcShowRow();
	CheckScrollBar();

	if(GetFocus() == this) {
		CreateCaret(TRUE);
		SetCaret(FALSE);
	}

	HIMC hIMC = ImmGetContext(GetSafeHwnd());
	if(hIMC != NULL) {
		// Set font for the conversion window
		ImmSetCompositionFont(hIMC, &lf);
		ImmReleaseContext(GetSafeHwnd(), hIMC);	
	}

	ClearBackGround();
	Invalidate();
}

void CEditCtrl::SetFont(CFont *font)
{
	LOGFONT lf;
	font->GetLogFont(&lf);
	CreateFonts(&lf);
	PostSetFont();
}

void CEditCtrl::CreateFonts(LOGFONT *lf)
{
	m_font.DeleteObject();
	m_clickable_font.DeleteObject();
	m_bold_font.DeleteObject();

//	lf->lfQuality = ANTIALIASED_QUALITY;
//	lf->lfQuality = PROOF_QUALITY;
//B	lf->lfQuality = CLEARTYPE_QUALITY;

	m_font.CreateFontIndirect(lf);

	lf->lfUnderline = TRUE;
	m_clickable_font.CreateFontIndirect(lf);

	lf->lfUnderline = FALSE;
	if(lf->lfWeight < FW_BOLD) {
		lf->lfWeight = FW_BOLD;			// Bold
	} else if(lf->lfWeight == FW_BOLD) {
		lf->lfWeight = FW_EXTRABOLD;
	} else if(lf->lfWeight == FW_EXTRABOLD) {
		lf->lfWeight = FW_HEAVY;
	}
	m_bold_font.CreateFontIndirect(lf);
}

void CEditCtrl::SetHeaderSize()
{
	m_row_num_digit = (int)ceil(log10(m_edit_data->get_row_cnt() + 1.0));
	if(m_row_num_digit < 2) m_row_num_digit = 2;

	if(m_ex_style & ECS_SHOW_ROW_NUM) {
		m_row_header_width = (m_row_num_digit + 2) * m_num_width + m_left_space;
	} else {
		m_row_header_width = m_left_space;
	}

	if(m_ex_style & ECS_SHOW_COL_NUM) {
		m_col_header_height = m_row_height - m_row_space_top + m_top_space + 4;
	} else {
		m_col_header_height = m_top_space;
	}
}

void CEditCtrl::InvalidateEditData_AllWnd(int old_row_cnt, int old_char_cnt, 
	int old_split_cnt, int start_row)
{
	InvalidateEditData(old_row_cnt, old_char_cnt, old_split_cnt, start_row, TRUE);

	if(IsSplitterMode()) {
		struct invalidate_editdata_st data;
		data.old_row_cnt = old_row_cnt;
		data.old_char_cnt = old_char_cnt;
		data.old_split_cnt = old_split_cnt;
		data.start_row = start_row;

		m_edit_data->get_disp_data()->SendNotifyMessage(GetSafeHwnd(), 
			EC_WM_INVALIDATE_EDITDATA, (WPARAM)&data, 0); 
	}
}

void CEditCtrl::InvalidateEditData(int old_row_cnt, int old_char_cnt, int old_split_cnt, 
	int start_row, BOOL b_make_edit_data)
{
	ASSERT(start_row < m_edit_data->get_row_cnt());

	// FIXME: old_char_cntは有効に使えてない
	// 以下のロジックにするか
	// (1)old_char_cntの代わりに、max_disp_widthを保持しておく
	// (2)is_max_disp_width_row()がTRUEなら、disp_widthを再計算
	// (3)(1)で保持していた値と、(2)の結果が異なる場合、全体を再描画

	// 行が変わったときは全体を再描画
	// 1行の最大の幅が変更されたときも全体を再描画
	if(old_row_cnt != m_edit_data->get_row_cnt() || 
		m_edit_data->is_max_disp_width_row(m_edit_data->get_cur_row())) {

		if(b_make_edit_data) {
			int recalc_split_cnt = max(m_edit_data->get_row_cnt() - old_row_cnt + 1, 2);
			MakeDispData(FALSE, start_row, recalc_split_cnt);
		}

		SetHeaderSize();

		// スクロールバー
		CheckScrollBar();

		// 再描画
		m_edit_data->get_disp_data()->GetDispColorDataPool()->ClearAllData();
		Invalidate();
	} else {
		if(CalcSplitCnt(m_edit_data->get_cur_row()) != old_split_cnt) {
			if(b_make_edit_data) MakeDispData(FALSE, start_row, 1);

			// 再描画
			//m_edit_data->get_disp_data()->GetDispColorDataPool()->ClearAllData();
			Invalidate();
		}

		// スクロールバー
		CheckScrollBar();

		InvalidateRow(m_edit_data->get_cur_row());
	}
}

void CEditCtrl::InvalidateRow_AllWnd(int row)
{
	if(IsSplitterMode()) {
		m_edit_data->get_disp_data()->SendNotifyMessage(GetSafeHwnd(), EC_WM_INVALIDATE_ROW, row, 0); 
	}
	InvalidateRow(row);
}

void CEditCtrl::InvalidateRow(int row)
{
	// 画面の範囲外は計算しない
	if(row < GetDispRowTop() || row > GetDispRowBottom()) return;
	if(row >= m_edit_data->get_row_cnt()) return;

	RECT		rect, winrect;

	GetDispRect(&winrect);
	rect.left = m_row_header_width;
	rect.right = winrect.right;
	rect.top = GetDispRowPos(row);
	rect.bottom = rect.top + GetDispRowSplitCnt(row) * m_row_height;
	if(rect.top < m_col_header_height) rect.top = m_col_header_height;
	if(rect.bottom > winrect.bottom) rect.bottom = winrect.bottom;
	InvalidateRect(&rect);

	// 編集マークを再描画
	rect.left = 0;
	rect.right = m_left_space;
	InvalidateRect(&rect);
}

void CEditCtrl::InvalidateRowHeader_AllWnd(int idx)
{
	if(IsSplitterMode()) {
		m_edit_data->get_disp_data()->SendNotifyMessage(GetSafeHwnd(), 
			EC_WM_INVALIDATE_ROW_HEADER, idx, 0); 
	}
	InvalidateRowHeader(idx);
}

void CEditCtrl::InvalidateRowHeader(int idx)
{
	int idx1 = GetScrollPos(SB_VERT);
	int idx2 = idx1 + GetShowRow() - 1;

	// 画面の範囲外は計算しない
	// FIXME: 要テスト
	if(idx < idx1 || idx > idx2) {
		return;
	}

	RECT		rect, winrect;
	GetDispRect(&winrect);
	rect.left = 0;
	rect.right = m_row_header_width;
	rect.top = (idx - idx1) * m_row_height + m_col_header_height;
	rect.bottom = rect.top + m_row_height;
	if(rect.top < m_col_header_height) rect.top = m_col_header_height;
	if(rect.bottom > winrect.bottom) rect.bottom = winrect.bottom;

	InvalidateRect(&rect);
}

void CEditCtrl::InvalidateColHeader_AllWnd(int x)
{
	if(IsSplitterMode()) {
		m_edit_data->get_disp_data()->SendNotifyMessage(GetSafeHwnd(), 
			EC_WM_INVALIDATE_COL_HEADER, x, 0); 
	}
	InvalidateColHeader(x);
}

void CEditCtrl::InvalidateColHeader(int x)
{
	RECT		winrect;
	GetDispRect(&winrect);

	x = x - m_row_header_width;

	// 画面の範囲外は計算しない
	if(x < 0 || x > winrect.right) return;
	
	RECT		rect;

	rect.left = x + m_row_header_width;
	rect.right = rect.left + m_char_width;
	rect.top = 0;
	rect.bottom = m_col_header_height;
	InvalidateRect(&rect);
}

void CEditCtrl::InvalidateRows_AllWnd(int row1, int row2)
{
	if(IsSplitterMode()) {
		m_edit_data->get_disp_data()->SendNotifyMessage(GetSafeHwnd(), 
			EC_WM_INVALIDATE_ROWS, row1, row2); 
	}
	InvalidateRows(row1, row2);
}

void CEditCtrl::InvalidateRows(int row1, int row2)
{
	// 画面の範囲外は，ループを回さないようにする
	int row = max(row1, GetDispRowTop());
	int loop_cnt = min(row2, GetDispRowBottom());

	for(; row <= loop_cnt; row++) {
		InvalidateRow(row);
	}
}

void CEditCtrl::ShowPoint(CPoint point)
{
	CFontWidthHandler dchandler(this, &m_font);
	const TCHAR *p = m_edit_data->get_disp_row_text(GetDispRow(point.y)) + GetDispOffset(point.y);

	int w = CEditRowData::get_disp_width_str(p, m_edit_data->get_disp_data(),
		&dchandler, m_edit_data->get_tabstop(), point.x);

	point.x = w / m_char_width;

	SIZE scr_size = {0, 0};
	if(point.x < GetScrollPos(SB_HORZ)) {
		scr_size.cx = point.x - GetScrollPos(SB_HORZ);
	} else if((GetScrollPos(SB_HORZ) + GetShowCol() - 2) < point.x) {
		scr_size.cx = (point.x - GetShowCol() + 2) - GetScrollPos(SB_HORZ);
	}

	int row_offset = 2;
	if((m_ex_style & ECS_V_SCROLL_EX) && GetShowRow() > 2) row_offset = 3;
	if(GetShowRow() <= 1) row_offset = 1;

	if(point.y < GetScrollPos(SB_VERT)) {
		scr_size.cy = point.y - GetScrollPos(SB_VERT);
	} else if((GetScrollPos(SB_VERT) + GetShowRow() - row_offset) < point.y) {
		scr_size.cy = (point.y - GetShowRow() + row_offset) - GetScrollPos(SB_VERT);
	}
	if(scr_size.cx == 0 && scr_size.cy == 0) return;

	OnScrollBy(scr_size, TRUE);
}

BOOL CEditCtrl::VisiblePoint(const POINT *pt)
{
	if(pt->y < m_col_header_height || pt->x < m_row_header_width) {
		return FALSE;
	}

	CRect	win_rect;
	GetDispRect(&win_rect);
	if(pt->y > win_rect.Height() - m_bottom_space) return FALSE;
	if(pt->x > win_rect.Width() - m_right_space) return FALSE;

	return TRUE;
}


void CEditCtrl::SetCaret(BOOL show_scroll, int focus_mode, BOOL b_line_end)
{
	// focus_modeが0かつfocusがないウィンドウの場合は，処理しない
	// focus_modeが1のときは，常に処理する
	if(focus_mode == 0 && GetFocus() != this) return;

	POINT		disp_data_pt;
	GetDispDataPoint(m_edit_data->get_cur_pt(), &disp_data_pt, b_line_end);

	if(GetFocus() == this) {
		InvalidateColHeader_AllWnd(GetCaretPos().x);
	}

	if(show_scroll == TRUE) {
		ShowPoint(disp_data_pt);
	}

	if(GetFocus() == this) {
		POINT	caret_point;
		GetDispCaretPoint(disp_data_pt, &caret_point);

		caret_point.y += m_row_space_top;

		// カレットが行番号・列番号の位置に表示されないようにする
		if(!VisiblePoint(&caret_point)) caret_point.y = -m_row_height;

		POINT	prev_caret_pos = GetCaretPos();
		{
			// カレットの残像をなくす処理
			RECT	rect;
			rect.top = prev_caret_pos.y;
			rect.bottom = rect.top + m_font_height;
			rect.left = prev_caret_pos.x;
			rect.right = rect.left + m_caret_width;
			InvalidateRect(&rect);
		}

		if(m_overwrite) {
			if(m_caret_width != GetFontWidth(NULL, m_edit_data->get_cur_char())) CreateCaret(TRUE);
		}

		SetCaretPos(caret_point);
		SetConversionWindow(caret_point.x, caret_point.y);

		InvalidateColHeader_AllWnd(caret_point.x);

		// カレットの位置を親ウィンドウに通知する
		CFontWidthHandler dchandler(this, &m_font);
		int caret_x = m_edit_data->get_cur_caret_col(&dchandler);
		GetParent()->SendMessage(EC_WM_CHANGE_CARET_POS, caret_x, m_edit_data->get_cur_row());
	}

	CaretMoved();

	if(m_edit_data->get_disp_data()->GetCaretPos()->y != disp_data_pt.y) {
		InvalidateRowHeader_AllWnd(m_edit_data->get_disp_data()->GetCaretPos()->y);
		InvalidateRowHeader_AllWnd(disp_data_pt.y);

		if(m_ex_style & ECS_SHOW_ROW_LINE) {
			InvalidateRow_AllWnd(GetDispRow(m_edit_data->get_disp_data()->GetCaretPos()->y));
			InvalidateRow_AllWnd(GetDispRow(disp_data_pt.y));
		}

		m_edit_data->get_disp_data()->GetCaretPos()->y = disp_data_pt.y;
	}
	if(m_edit_data->get_disp_data()->GetCaretPos()->x != disp_data_pt.x) {
		m_edit_data->get_disp_data()->GetCaretPos()->x = disp_data_pt.x;
	}

	if(m_ex_style & ECS_SHOW_BRACKETS_BOLD) {
		SetBracketsPt();
	}
}

void CEditCtrl::SetBracketsPt()
{
	// ファイルオープン直後、max_disp_idxなどが設定されていないときは処理しない
	if(m_edit_data->get_disp_data()->GetMaxDispIdx() == 1 && m_edit_data->get_row_cnt() > 1) {
		return;
	}

	// カーソルが画面内にない場合は，実行しない
	if(m_edit_data->get_cur_row() < GetDispRowTop()) return;
	if(m_edit_data->get_cur_row() > GetDispRowBottom()) return;

	CPoint search_end_pt;
	search_end_pt.x = 0;

	CPoint open_pt = CPoint(-1, -1);
	CPoint close_pt = CPoint(-1, -1);
	CPoint tmp_pt;

	const TCHAR *brackets_str = m_edit_data->get_str_token()->GetBracketStr();
	UINT searched_char = '\0';

	// 最初の開き括弧を探す
	search_end_pt.y = GetDispRowTop();
	search_end_pt.x = GetDispOffset(GetScrollPos(SB_VERT));

	for(; *brackets_str; brackets_str++) {
		UINT close_char = *brackets_str;
		UINT open_char = m_edit_data->get_str_token()->getOpenBrackets(close_char);

		if(m_edit_data->search_brackets_ex(
			open_char, close_char,
			1, &search_end_pt, '\0', -1, &tmp_pt)) {
			if(open_pt.y == -1 || inline_pt_cmp(&tmp_pt, &open_pt) > 0) {
				searched_char = close_char;
				open_pt = tmp_pt;
				// 検索範囲を狭くする
				search_end_pt = tmp_pt;
			}
		}
	}

	// 閉じ括弧を探す

	// 検索範囲を最小にする
	int last_idx = (GetScrollPos(SB_VERT) + GetShowRow());
	if(last_idx < 0 || last_idx >= m_edit_data->get_disp_data()->GetMaxDispIdx()) {
		search_end_pt = m_edit_data->get_end_of_text_pt();
	} else {
		search_end_pt.y = GetDispRow(last_idx);
		search_end_pt.x = GetDispOffset(last_idx);
	}

	if(searched_char != '\0') {
		UINT open_char = m_edit_data->get_str_token()->getOpenBrackets(searched_char);

		// 現在位置が閉じ括弧のとき，それを使う
		if(m_edit_data->get_cur_char() == searched_char
			&& m_edit_data->check_char_type() == CHAR_TYPE_NORMAL) {
			close_pt = m_edit_data->get_cur_pt();
		} else {
			// 現在位置が開き括弧のとき，条件を調節する
			int start_cnt = -1;
			if(m_edit_data->get_cur_char() == open_char
				&& m_edit_data->check_char_type() == CHAR_TYPE_NORMAL) {
				start_cnt--;
			}
			if(m_edit_data->search_brackets_ex(
				open_char, searched_char,
				start_cnt, &search_end_pt, '\0', 1, &tmp_pt)) {
				close_pt = tmp_pt;
			}
		}
	} else {
		// 開き括弧が見つからないとき，最初の閉じ括弧を探す
		// 現在位置が閉じ括弧のとき，それを使う
		if(m_edit_data->get_str_token()->isCloseBrackets(m_edit_data->get_cur_char())
			&& m_edit_data->check_char_type() == CHAR_TYPE_NORMAL) {
			close_pt = m_edit_data->get_cur_pt();
		} else {
			brackets_str = m_edit_data->get_str_token()->GetBracketStr();

			for(; *brackets_str; brackets_str++) {
				UINT close_char = *brackets_str;
				UINT open_char = m_edit_data->get_str_token()->getOpenBrackets(close_char);

				// 現在位置が開き括弧のとき，条件を調節する
				int start_cnt = -1;
				if(m_edit_data->get_cur_char() == open_char
					&& m_edit_data->check_char_type() == CHAR_TYPE_NORMAL) {
					start_cnt--;
				}
				if(m_edit_data->search_brackets_ex(
					open_char, close_char,
					start_cnt, &search_end_pt, '\0', 1, &tmp_pt)) {
					if(close_pt.y == -1 || inline_pt_cmp(&tmp_pt, &close_pt) < 0) {
						close_pt = tmp_pt;
						// 検索範囲を狭くする
						search_end_pt = tmp_pt;
					}
				}
			}
		}
	}

	// 前回の結果と今回の結果が同じときは，再描画処理を省略する
	if(inline_pt_cmp(&open_pt, &m_edit_data->get_disp_data()->GetBracketPos()) != 0) {
		// 前回のデータをクリア
		if(m_edit_data->get_disp_data()->GetBracketPos().y != -1) {
			POINT pt = m_edit_data->get_disp_data()->GetBracketPos();
			CDispColorData *p_disp_color_data = 
				m_edit_data->get_disp_data()->GetDispColorDataPool()->GetDispColorData(pt.y);
			if(p_disp_color_data) p_disp_color_data->UnsetBold(pt.x);
			InvalidateRow_AllWnd(pt.y);
		}
		m_edit_data->get_disp_data()->SetBracketPos(open_pt);
		if(open_pt.y != -1) InvalidateRow_AllWnd(open_pt.y);
	}

	if(inline_pt_cmp(&close_pt, &m_edit_data->get_disp_data()->GetBracketPos2()) != 0) {
		// 前回のデータをクリア
		if(m_edit_data->get_disp_data()->GetBracketPos2().y != -1) {
			POINT pt = m_edit_data->get_disp_data()->GetBracketPos2();
			CDispColorData *p_disp_color_data = 
				m_edit_data->get_disp_data()->GetDispColorDataPool()->GetDispColorData(pt.y);
			if(p_disp_color_data) p_disp_color_data->UnsetBold(pt.x);
			InvalidateRow_AllWnd(pt.y);
		}
		m_edit_data->get_disp_data()->SetBracketPos2(close_pt);
		if(close_pt.y != -1) InvalidateRow_AllWnd(close_pt.y);
	}
}

void CEditCtrl::MoveCaretRow(int row, BOOL extend, BOOL page_scr)
{
	int col = m_edit_data->get_row_col();
	if(col == -1) col = GetCaretPos().x;

	POINT	pt;

	CPoint	point;

	point.y = (m_edit_data->get_disp_data()->GetCaretPos()->y - GetScrollPos(SB_VERT) + row) * m_row_height + m_col_header_height;
	point.x = col;

	GetDataPoint(point, &pt, TRUE);

	// row, colを同時に移動すると，col位置がずれてしまう
	// (col位置を保存しながら行移動するため)
	{	// 行の端にカーソルがあるとき，カーソルが行頭と行末を交互に表示されるのを回避
		PreMoveCaret(extend);
		m_edit_data->move_cur_row(pt.y - m_edit_data->get_cur_row());
		m_edit_data->move_cur_col(pt.x - m_edit_data->get_cur_col());
		PostMoveCaret(extend, FALSE, FALSE);
	}

	BOOL b_line_end = FALSE;
	if(pt.y == m_edit_data->get_cur_row() &&
		col >= m_row_header_width + m_char_width * 2) b_line_end = TRUE;

	if(page_scr) {
		SIZE scr_size = {0, row};
		OnScrollBy(scr_size, TRUE);
	}
   
	SetCaret(TRUE, 0, b_line_end);

	m_edit_data->set_row_col(col);
}

void CEditCtrl::MoveCaretPos(int row, int col, BOOL extend)
{
	MoveCaret(row - m_edit_data->get_cur_row(), 0, extend, FALSE);
	MoveCaret(0, col - m_edit_data->get_cur_col(), extend);
}

void CEditCtrl::PreMoveCaret(BOOL extends)
{
	if(extends) {
		if(HaveSelected() == FALSE) {
			StartSelect(CPoint(m_edit_data->get_cur_col(), m_edit_data->get_cur_row()));
		}
	} else {
		ClearSelected();
	}
	ClearKeywordCompletion();
}

void CEditCtrl::PostMoveCaret(BOOL extends, BOOL show_scroll, BOOL set_caret)
{
	if(extends) {
		POINT	pt;
		pt.y = m_edit_data->get_cur_row();
		pt.x = m_edit_data->get_cur_col();
		SelectText(pt, FALSE);
	}

	if(set_caret) SetCaret(show_scroll, 0);
}

void CEditCtrl::MoveCaret(int row, int col, BOOL extend, BOOL show_scroll)
{
	PreMoveCaret(extend);

	if(row != 0) m_edit_data->move_cur_row(row);
	if(col != 0) m_edit_data->move_cur_col(col);

	PostMoveCaret(extend, show_scroll);
}

// DeleteWordAfterCaret(), DeleteWordBeforeCaret()の削除範囲を取得する
CPoint CEditCtrl::GetNextWordPt(int allow)
{
	// 現在のカーソル位置を保存
	CPoint cur_pt = m_edit_data->get_cur_pt();

	if(allow < 0) m_edit_data->skip_space(-1);
	m_edit_data->move_word(allow);
	if(allow > 0) m_edit_data->skip_space(1);

	CPoint word_pt = m_edit_data->get_cur_pt();

	// カーソル位置を元に戻す
	m_edit_data->set_cur(cur_pt.y, cur_pt.x);

	return word_pt;
}

void CEditCtrl::MoveNextSpace(int allow, BOOL extend)
{
	unsigned int ch_arr[3];
	ch_arr[0] = L' ';
	ch_arr[1] = L'\t';
	ch_arr[2] = L'　';

	PreMoveCaret(extend);

	m_edit_data->move_chars(allow, ch_arr, 3);

	PostMoveCaret(extend, TRUE);
}

void CEditCtrl::MoveWord(int allow, BOOL extend, BOOL skip_space)
{
	PreMoveCaret(extend);

	if(skip_space && allow < 0) m_edit_data->skip_space(-1);
	m_edit_data->move_word(allow);
	if(skip_space && allow > 0) m_edit_data->skip_space(1);

	PostMoveCaret(extend, TRUE);
}

void CEditCtrl::SetConversionWindow(int x, int y)
{
	if(GetFocus() != this) return;

	HIMC	hIMC;
	COMPOSITIONFORM cf;

	hIMC = ImmGetContext(this->GetSafeHwnd());
//	TRACE(_T("%d:%d,%d\n"), x, m_ime_rect.right, m_ime_rect.left);
	if (x >= 0) {
		// IMEウィンドウが消えないようにする
		// Note(2019/2/12)
		//  x > m_ime_rect.rightを x >= m_ime_rect.rightに修正
		//  指定桁数で折り返し表示のとき、折り返し位置からIME入力を開始したとき、次の行に表示する
		if(x >= m_ime_rect.right) {
			x = m_ime_rect.left;
			y += m_row_height;
		}

		cf.dwStyle = CFS_POINT;
		cf.ptCurrentPos.x = x;
		cf.ptCurrentPos.y = y;
	} else {
		cf.dwStyle = CFS_DEFAULT;
	}
	ImmSetCompositionWindow(hIMC, &cf);

	cf.dwStyle = CFS_RECT;
	cf.rcArea = m_ime_rect;
	ImmSetCompositionWindow(hIMC, &cf);
/*
	// Set font for the conversion window
	LOGFONT lf;
	m_font.GetLogFont(&lf);
	ImmSetCompositionFont(hIMC, &lf);
*/
	ImmReleaseContext(this->GetSafeHwnd(), hIMC);	
}

void CEditCtrl::SelectText(CPoint pt, BOOL mouse_select)
{
	POINT		cur_pos1, cur_pos2;

	if(GetSelectArea()->box_select && !mouse_select) {
		CFontWidthHandler dchandler(this, &m_font);
		GetSelectArea()->box_end_w = m_edit_data->get_disp_width_pt(pt, &dchandler);
	}

	// バグ検出
	// 選択範囲がある場合，pos1かpos2が，start_ptと一致していなければならない
	ASSERT(HaveSelected() == FALSE ||
		(GetSelectArea()->start_pt.y == GetSelectArea()->pos1.y && GetSelectArea()->start_pt.x == GetSelectArea()->pos1.x) ||
		(GetSelectArea()->start_pt.y == GetSelectArea()->pos2.y && GetSelectArea()->start_pt.x == GetSelectArea()->pos2.x));

	cur_pos1 = GetSelectArea()->pos1;
	cur_pos2 = GetSelectArea()->pos2;

	// ダブルクリックで選択したときの処理
	if(GetSelectArea()->start_pt2.y != -1) {
		if(IsPtInArea(&GetSelectArea()->start_pt1, &GetSelectArea()->start_pt2, &pt)) {
			// 選択開始の範囲内のときは，選択範囲を変更しない
			GetSelectArea()->start_pt = GetSelectArea()->start_pt1;
			pt = GetSelectArea()->start_pt2;
		} else {
			// 選択開始の範囲の前後で，選択範囲の開始位置を変更する
			if(GetSelectArea()->start_pt1.y > pt.y ||
				(GetSelectArea()->start_pt.y == pt.y && GetSelectArea()->start_pt.x > pt.x)) {
				GetSelectArea()->start_pt = GetSelectArea()->start_pt2;
			} else {
				GetSelectArea()->start_pt = GetSelectArea()->start_pt1;
			}
		}
	}

	if(GetSelectArea()->start_pt.y > pt.y || 
		(GetSelectArea()->start_pt.y == pt.y && GetSelectArea()->start_pt.x > pt.x)) {
		GetSelectArea()->pos1 = pt;
		GetSelectArea()->pos2 = GetSelectArea()->start_pt;
	} else {
		GetSelectArea()->pos1 = GetSelectArea()->start_pt;
		GetSelectArea()->pos2 = pt;
	}

	int		i, loop_cnt;
	if(cur_pos1.y == -1) {	// 選択開始のとき
		i = max(GetSelectArea()->pos1.y, GetDispRowTop());
		loop_cnt = min(GetSelectArea()->pos2.y, GetDispRowBottom());
		for(; i <= loop_cnt; i++) {
			InvalidateRow_AllWnd(i);
		}
	} else if(GetSelectArea()->box_select) {
		// 選択前後で重ならない行を再描画
		i = max(cur_pos1.y, GetDispRowTop());
		loop_cnt = min(cur_pos2.y, GetDispRowBottom());
		for(; i <= loop_cnt; i++) {
			if(i < GetSelectArea()->pos1.y || i > GetSelectArea()->pos2.y) {
				InvalidateRow_AllWnd(i);
			}
		}

		// 選択範囲を再描画
		InvalidateRows_AllWnd(max(GetSelectArea()->pos1.y, GetDispRowTop()),
			min(GetSelectArea()->pos2.y, GetDispRowBottom()));
	} else {
		if(IsSplitterMode()) {
			// 分割ウィンドウに通知
			m_edit_data->get_disp_data()->SendNotifyMessage(GetSafeHwnd(), 
				EC_WM_INVALIDATE_ROWS, 
				min(cur_pos1.y, GetSelectArea()->pos1.y),
				max(cur_pos2.y, GetSelectArea()->pos2.y)); 
		}

		// 選択前後で重ならない行を再描画
		i = max(cur_pos1.y, GetDispRowTop());
		loop_cnt = min(cur_pos2.y, GetDispRowBottom());
		for(; i <= loop_cnt; i++) {
			if(i < GetSelectArea()->pos1.y || i > GetSelectArea()->pos2.y) {
				InvalidateRow(i);
			}
		}

		i = max(GetSelectArea()->pos1.y, GetDispRowTop());
		loop_cnt = min(GetSelectArea()->pos2.y, GetDispRowBottom());
		for(; i <= loop_cnt; i++) {
			if(i < cur_pos1.y || i > cur_pos2.y) {
				InvalidateRow(i);
			}
		}

		// FIXME: 現在のカーソル行がちらつかないようにする
		// 選択前後の端の行を再描画
		if(cur_pos1.y == GetSelectArea()->start_pt.y) {
			InvalidateRow(cur_pos2.y);
		} else {
			InvalidateRow(cur_pos1.y);
		}
		if(GetSelectArea()->pos1.y == GetSelectArea()->start_pt.y) {
			InvalidateRow(GetSelectArea()->pos2.y);
		} else {
			InvalidateRow(GetSelectArea()->pos1.y);
		}

		// 選択範囲の上下が変わったとき，選択開始行を再描画
		if(cur_pos1.y < GetSelectArea()->start_pt.y && GetSelectArea()->pos2.y > GetSelectArea()->start_pt.y ||
			cur_pos2.y > GetSelectArea()->start_pt.y && GetSelectArea()->pos1.y < GetSelectArea()->start_pt.y) {
			InvalidateRow(GetSelectArea()->start_pt.y);
		}

		// ダブルクリックで選択したときで，選択範囲の上下が変わったときの処理
		if(GetSelectArea()->start_pt2.y != -1) {
			if(cur_pos2.y == GetSelectArea()->start_pt2.y && cur_pos2.x != GetSelectArea()->start_pt2.x &&
				GetSelectArea()->pos1.y < GetSelectArea()->start_pt1.y) {
				InvalidateRow(GetSelectArea()->start_pt2.y);
			}
		}
	}
	GetParent()->SendMessage(EC_WM_CHANGE_SELECTED_TEXT, 0, 0);
}

void CEditCtrl::PreDragSelected(BOOL word_select)
{
	GetSelectArea()->drag_flg = PRE_DRAG;
	if(HaveSelected() == FALSE) {
		SetSelectInfo(m_edit_data->get_cur_pt());
		GetSelectArea()->word_select = word_select;
	}

	SetCapture();
	SetTimer(IdDragSelectTimer, 100, NULL);
}

void CEditCtrl::StartDragSelected()
{
	GetSelectArea()->drag_flg = DO_DRAG;
}

void CEditCtrl::DoDragSelected(CPoint point, BOOL show_scroll)
{
	POINT		pt;
	GetDataPoint(point, &pt, TRUE);

	POINT		cur_pt = pt;

	// 行番号を選択中
	if(pt.y > GetSelectArea()->start_pt.y && HitTestRowHeader(point) != -1) {
		if(pt.y == m_edit_data->get_row_cnt() - 1) {
			pt.x = m_edit_data->get_row_len(pt.y);
		} else {
			pt.y++;
			pt.x = 0;
		}
		cur_pt.x = pt.x;
	}

	m_edit_data->set_cur(cur_pt.y, cur_pt.x);

	if(m_ex_style & ECS_WORD_SELECT_MODE) {
		if(GetSelectArea()->word_select) {
			int allow = 1;
			if(GetSelectArea()->start_pt.y > pt.y ||
				(GetSelectArea()->start_pt.y == pt.y && GetSelectArea()->start_pt.x > pt.x)) {
				allow = -1;
			}
			if(!m_edit_data->get_str_token()->isBreakChar(m_edit_data->get_cur_char()) &&
				!m_edit_data->get_str_token()->isBreakChar(m_edit_data->get_prev_char())) {
				if(!m_edit_data->is_first_of_line() && !m_edit_data->is_end_of_line()) {
					m_edit_data->move_word(allow);
				}
			}
			pt = m_edit_data->get_cur_pt();
		}
	}

	SetCaret(show_scroll);

	if(GetSelectArea()->box_select) {
		GetSelectArea()->box_end_w = point.x - m_row_header_width + (m_char_width * GetScrollPos(SB_HORZ));
	}
	SelectText(pt, TRUE);
}

void CEditCtrl::EndDragSelected()
{
	KillTimer(IdDragSelectTimer);
	ReleaseCapture();
	GetSelectArea()->drag_flg = NO_DRAG;
	if(HaveSelected() && m_ex_style & ECS_DRAG_SELECT_COPY) {
		CopyMain();
	}
}

int CEditCtrl::FilterText2(const TCHAR *search_text,
	BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp, BOOL b_selected_area,
	int *preplace_cnt, BOOL b_del_flg)
{
	if(preplace_cnt != NULL) *preplace_cnt = 0;

	CRegData reg_data;
	if(!reg_data.Compile2(search_text, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp)) {
		MessageBox(_T("不正な正規表現です"), _T("メッセージ"), MB_ICONINFORMATION | MB_OK);
		return 1;
	}

	if(b_selected_area) {
		if(HaveSelected()) {
			int		start_row = GetSelectArea()->pos1.y;
			int		end_row = GetSelectArea()->pos2.y;
			if(start_row != end_row && GetSelectArea()->pos2.x == 0) end_row--;

			m_edit_data->filter_str(start_row, end_row,
				preplace_cnt, reg_data.GetRegData(), b_del_flg);
			ValidateSelectedArea();
		}
	} else {
		m_edit_data->filter_str(-1, -1, preplace_cnt, reg_data.GetRegData(), b_del_flg);
		ClearSelected(FALSE);
	}

	Redraw_AllWnd();

	return 0;
}

int CEditCtrl::FilterText(const TCHAR *search_text, 
	BOOL b_distinct_lwr_upr, BOOL b_regexp, BOOL b_selected_area, 
	int *preplace_cnt, BOOL b_del_flg)
{
	return FilterText2(search_text, b_distinct_lwr_upr, TRUE, b_regexp, b_selected_area, preplace_cnt, b_del_flg);
}

int CEditCtrl::ReplaceTextAll(const TCHAR *search_text, const TCHAR *replace_text, 
	BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp, BOOL b_selected_area,
	int *preplace_cnt)
{
	CRegData reg_data;
	if(!reg_data.Compile2(search_text, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp)) {
		MessageBox(_T("不正な正規表現です"), _T("メッセージ"), MB_ICONINFORMATION | MB_OK);
		return 1;
	}

	if(b_selected_area) {
		if(HaveSelected()) {
			if(GetSelectArea()->box_select) {
				CFontWidthHandler dchandler(this, &m_font);
				m_edit_data->replace_str_all_regexp_box(&dchandler, replace_text,
					b_regexp, preplace_cnt, reg_data.GetRegData());
			} else {
				m_edit_data->replace_str_all_regexp(replace_text,
					b_regexp, &GetSelectArea()->pos1, &GetSelectArea()->pos2,
					preplace_cnt, reg_data.GetRegData());
			}

			ValidateSelectedArea();
		}
	} else {
		m_edit_data->replace_str_all_regexp(replace_text,
			b_regexp, NULL, NULL, preplace_cnt, reg_data.GetRegData());
		ClearSelected(FALSE);
	}

	Redraw_AllWnd();

	return 0;
}

int CEditCtrl::ReplaceText2(const TCHAR *search_text, const TCHAR *replace_text, int dir,
	BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp, BOOL b_all, BOOL b_selected_area,
	int *preplace_cnt, BOOL *b_looped)
{
	if(preplace_cnt != NULL) *preplace_cnt = 0;

	if(b_all) {
		return ReplaceTextAll(search_text, replace_text, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp, 
			b_selected_area, preplace_cnt);
	}

	// reg_dataは，m_search_dataでキャッシュしてるので，この関数内で開放しない
	HREG_DATA reg_data = m_search_data.MakeRegData2(search_text, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp);
	if(reg_data == NULL) {
		MessageBox(_T("不正な正規表現です"), _T("メッセージ"), MB_ICONINFORMATION | MB_OK);
		return 1;
	}
	SaveSearchData2(search_text, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp);

	POINT find_pt, start_pt;
	if(HaveSelected() == FALSE) {
		start_pt.y = m_edit_data->get_cur_row();
		start_pt.x = m_edit_data->get_cur_col();
	} else {
		start_pt = GetSelectArea()->pos1;
	}

	int len = m_edit_data->search_text_regexp(&find_pt, dir,
		TRUE, &start_pt, (m_ex_style & ECS_SEARCH_LOOP_MSG) ? GetSafeHwnd() : NULL,
		reg_data, b_looped);

	if(len == -1 || find_pt.y != start_pt.y || find_pt.x != start_pt.x) {
		return PostSearchText(len, &find_pt);
	}
	if(len > 0 && HaveSelected() == FALSE) {
		return PostSearchText(len, &find_pt);
	}

	POINT end_pt = find_pt;
	m_edit_data->calc_find_last_pos(&end_pt, len);
	if(GetSelectArea()->pos2.y != end_pt.y || GetSelectArea()->pos2.x != end_pt.x) {
		return PostSearchText(len, &find_pt);
	}

	if(b_regexp) {
		CString replace = m_edit_data->make_back_ref_str(&find_pt, len, replace_text, reg_data);
		Paste(replace);
	} else {
		Paste(replace_text);
	}

	return SearchText2(search_text, dir, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp, b_looped);
}

int CEditCtrl::ReplaceText(const TCHAR *search_text, const TCHAR *replace_text, int dir,
	BOOL b_distinct_lwr_upr, BOOL b_regexp, BOOL b_all, BOOL b_selected_area,
	int *preplace_cnt, BOOL *b_looped)
{
	return ReplaceText2(search_text, replace_text, dir, b_distinct_lwr_upr, TRUE, b_regexp, b_all,
		b_selected_area, preplace_cnt, b_looped);
}

int CEditCtrl::PostSearchText(int len, POINT *pt)
{
	if(len == -1) {
		if(m_ex_style & ECS_SEARCH_LOOP_MSG) {
			MessageBox(_T("見つかりません"), _T("検索"), MB_OK | MB_ICONINFORMATION);
		}
		return 1;
	}

	if(len < 0) return 1;

	POINT end_pt = *pt;
	m_edit_data->calc_find_last_pos(&end_pt, len);

	POINT disp_pt;
	GetDispDataPoint(*pt, &disp_pt);

	ShowPoint(disp_pt);
	StartSelect(*pt);
	EndSelect(end_pt);

	return 0;
}

int CEditCtrl::SearchText2(const TCHAR *search_text, int dir, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii,
	BOOL b_regexp, BOOL *b_looped, BOOL enable_loop)
{
	CWaitCursor	wait_cursor;
	POINT		find_pt;

	if(_tcscmp(search_text, _T("")) == 0) {
		ClearSearchText();
		return 1;
	}
	if(dir != 1 && dir != -1) return 1;

	// 正規表現で"^"など検索結果の長さが0になる検索を実行したとき，次の検索ができないバグを修正
	if(b_regexp && dir == 1 &&
		GetSelectArea()->pos1.y == m_edit_data->get_cur_row() &&
		GetSelectArea()->pos1.x == m_edit_data->get_cur_col() &&
		GetSelectArea()->pos1.y == GetSelectArea()->pos2.y &&
		GetSelectArea()->pos1.x == GetSelectArea()->pos2.x) {

		if(m_edit_data->is_end_of_text()) {
			m_edit_data->set_cur(0, 0);
		} else {
			m_edit_data->move_cur_col(1);
		}
	} else if(HaveSelected()) {
		if(dir == 1) {
			m_edit_data->set_cur(GetSelectArea()->pos1.y, GetSelectArea()->pos1.x);
			m_edit_data->move_cur_col(1);
		} else {
			if(HaveSelectedMultiLine()) {
				m_edit_data->set_cur(GetSelectArea()->pos1.y, GetSelectArea()->pos1.x);
			} else {
				m_edit_data->set_cur(GetSelectArea()->pos2.y, GetSelectArea()->pos2.x);
				m_edit_data->move_cur_col(-1);
			}
		}
	}

	// reg_dataは，m_search_dataでキャッシュしてるので，この関数内で開放しない
	HREG_DATA reg_data = m_search_data.MakeRegData2(search_text, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp);
	if(reg_data == NULL) {
		MessageBox(_T("不正な正規表現です"), _T("メッセージ"), MB_ICONINFORMATION | MB_OK);
		return 1;
	}
	SaveSearchData2(search_text, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp);

	int len = m_edit_data->search_text_regexp(&find_pt, dir,
		enable_loop, NULL, (m_ex_style & ECS_SEARCH_LOOP_MSG) ? GetSafeHwnd() : NULL,
		reg_data, b_looped);

	if(HaveSelected() && len < 0) {
		m_edit_data->set_cur(GetSelectArea()->pos2.y, GetSelectArea()->pos2.x);
		SetCaret(TRUE);
	}

	return PostSearchText(len, &find_pt);
}

int CEditCtrl::SearchText(const TCHAR *search_text, int dir, BOOL b_distinct_lwr_upr, 
	BOOL b_regexp, BOOL *b_looped, BOOL enable_loop)
{
	return SearchText2(search_text, dir, b_distinct_lwr_upr, TRUE, b_regexp, b_looped, enable_loop);
}

void CEditCtrl::RegisterWndClass()
{
	WNDCLASS wndcls;
	HINSTANCE hInst = AfxGetInstanceHandle();

	if (::GetClassInfo(hInst, EDITCTRL_CLASSNAME, &wndcls)) return;

	// otherwise we need to register a new class
	wndcls.style            = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW |
		CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
	wndcls.lpfnWndProc      = ::DefWindowProc;
	wndcls.cbClsExtra       = wndcls.cbWndExtra = 0;
	wndcls.hInstance        = hInst;
	wndcls.hIcon            = NULL;
	wndcls.hCursor          = AfxGetApp()->LoadStandardCursor(IDC_IBEAM);
	wndcls.hbrBackground    = (HBRUSH) COLOR_WINDOW;
	wndcls.lpszMenuName     = NULL;
	wndcls.lpszClassName    = EDITCTRL_CLASSNAME;

	if (!AfxRegisterClass(&wndcls)) {
		AfxThrowResourceException();
	}
}

int CEditCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CScrollWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_edit_data->get_disp_data()->RegistNotifyWnd(GetSafeHwnd());
	
	if(m_droptarget.Register(this) == -1) return -1;

	LOGFONT		lf;
	lf.lfHeight = -DEFAULT_FONT_HEIGHT;
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = 400;
	lf.lfItalic = 0;
	lf.lfUnderline = 0;
	lf.lfStrikeOut = 0;
	lf.lfCharSet = 128;
	lf.lfOutPrecision = OUT_CHARACTER_PRECIS;
	lf.lfClipPrecision = CLIP_CHARACTER_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
	_tcscpy(lf.lfFaceName, _T("Terminal"));

	CreateFonts(&lf);
	PostSetFont();

	return 0;
}

void CEditCtrl::IndentMain(BOOL del, BOOL space_flg, int space_cnt, UINT input_char)
{
	if(!HaveSelectedMultiLine() && !HaveSelectedFullOneLine() && 
		(input_char != '\t' || !del)) {
		InputChar(input_char);
		return;
	}

	if(del) {
		BOOL haveSelected = HaveSelected();
		int old_col = m_edit_data->get_cur_col();
		int old_line_len = m_edit_data->get_row_len(m_edit_data->get_cur_row());
		CString	search_str;

		CUndoSetMode undo_obj(m_edit_data->get_undo());
		m_edit_data->save_cursor_pos();

		if(m_edit_data->get_disp_data()->GetSelectArea()->box_select) {
			search_str.Format(_T("\\=^((\\t)|(\\s{1,%d}))"), space_cnt);
		} else {
			InfrateSelectedArea();
			search_str.Format(_T("^((\\t)|(\\s{1,%d}))"), space_cnt);
		}
		ReplaceText(search_str.GetBuffer(0), _T(""), 1, 
			TRUE, TRUE, TRUE, TRUE, NULL, NULL);

		if(!haveSelected) {
			ClearSelected();

			if(m_edit_data->is_blank_row(m_edit_data->get_cur_row())) {
				// 空白のみの行の場合、カーソル位置を元に戻す
				int new_col = m_edit_data->get_cur_col();
				int new_line_len = m_edit_data->get_row_len(m_edit_data->get_cur_row());
				int new_pos = old_col - (old_line_len - new_line_len);

				if(new_pos < 0) {
					LineStart(FALSE);
				} else {
					m_edit_data->move_cur_col(new_pos - new_col);
				}
				
				SetCaret(TRUE);
			} else {
				// 先頭の単語に移動
				LineStart(FALSE);
			}
		}
	} else {
		CString	search_str;
		CString replace_str;
		replace_str.Format(_T("%c"), input_char);

		if(space_flg) {
			replace_str = _T("");
			for(int i = 0; i < space_cnt; i++) replace_str += _T(" ");
		}

		if(m_edit_data->get_disp_data()->GetSelectArea()->box_select) {
			search_str = "\\=^";
		} else {
			InfrateSelectedArea();
			search_str = "^";
		}
		ReplaceText(search_str.GetBuffer(0), 
			replace_str.GetBuffer(0), 1, 
			TRUE, TRUE, TRUE, TRUE, NULL, NULL);
	}
}

void CEditCtrl::InsertTab(BOOL del)
{
	IndentMain(del, m_edit_data->get_tab_as_space(), m_edit_data->get_tabstop(), '\t');
}

void CEditCtrl::IndentSpace(BOOL del)
{
	IndentMain(del, TRUE, 1, ' ');
}

void CEditCtrl::InputMain(UINT nChar)
{
	if(nChar == '\t') {
		CFontWidthHandler dchandler(this, &m_font);
		m_edit_data->input_tab(&dchandler);
	} else {
		m_edit_data->input_char(nChar);
	}
}

void CEditCtrl::InputChar(UINT nChar)
{
	if(m_read_only == TRUE) return;

	// ECS_GRID_EDITORのときは，改行とタブを受け付けない
	if(nChar == VK_RETURN && m_ex_style & ECS_GRID_EDITOR) return;
	if(nChar == VK_TAB && m_ex_style & ECS_GRID_EDITOR) return;

	if(nChar == VK_ESCAPE) return;
	if(nChar == VK_BACK) return;

	if(nChar == VK_TAB) {
		if(HaveSelected() == TRUE && GetSelectArea()->pos1.y != GetSelectArea()->pos2.y) {
			// shiftキーのときは，tab削除
			BOOL	del = FALSE;
			if(GetKeyState(VK_SHIFT) < 0) del = TRUE;

			InsertTab(del);
			return;
		}
	}

	int cur_row = 0;
	int old_row_cnt = m_edit_data->get_row_cnt();
	int old_split_cnt = GetDispRowSplitCnt(m_edit_data->get_cur_row());
	int old_char_cnt = m_edit_data->get_edit_row_data(m_edit_data->get_cur_row())->get_char_cnt();

	if(HaveSelected() == TRUE) {
		if(HaveSelectedMultiLine()) {
			old_row_cnt = -1; // 全データを再描画
		}
		cur_row = min(GetSelectArea()->pos1.y, GetSelectArea()->pos2.y);

		CUndoSetMode undo_obj(m_edit_data->get_undo());

		if(GetSelectArea()->box_select) {
			m_edit_data->del_box(FALSE);
		} else {
			m_edit_data->del(&GetSelectArea()->pos1, &GetSelectArea()->pos2, FALSE);
		}
		InputMain(nChar);

		ClearSelected();

		// コメントの色分け対応
		CheckCommentRow(cur_row, m_edit_data->get_cur_row());
	} else {
		cur_row = m_edit_data->get_cur_row();

		if(m_overwrite && !m_edit_data->is_end_of_text()) {
			// 上書きモードのときは、Enterキーで次の行に移動する
			if(nChar == VK_RETURN) {
				// 最終行のときは移動しない
				if(!m_edit_data->is_end_line()) {
					LineDown(FALSE);
					LineStart(FALSE, TRUE);
				}
				return;
			}
			CUndoSetMode undo_obj(m_edit_data->get_undo());
			m_edit_data->delete_key();
			InputMain(nChar);
		} else {
			InputMain(nChar);
		}

		// コメントの色分け対応
		if(IsCheckCommentChar(nChar) || old_row_cnt != m_edit_data->get_row_cnt()) {
			CheckCommentRow(cur_row, m_edit_data->get_cur_row());
		}
	}

	ClearKeywordCompletion();

	InvalidateEditData_AllWnd(old_row_cnt, old_char_cnt, old_split_cnt, cur_row);

	SetCaret(TRUE);

	// 括弧の対応を表示する
	if(m_ex_style & ECS_INVERT_BRACKETS) {
		if(m_edit_data->get_str_token()->isCloseBrackets(nChar)) {
			// カーソルの位置を保存
			CEditDataSaveCurPt	saved_pt(m_edit_data);
			// 入力前の位置にする
			m_edit_data->move_cur_col(-1);
			InvertBrackets(nChar);
		}
	}
}

void CEditCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// コントロールキーが押されているときは、文字の入力を受け付けない
	if(GetKeyState(VK_CONTROL) < 0) {
		if(m_ex_style & ECS_ON_DIALOG) {
			switch(nChar) {
			case 0x03: // Ctrl + C
				Copy();
				break;
			case 0x16: // Ctrl + V
				Paste();
				break;
			case 0x18: // Ctrl + X
				Cut();
				break;
			case 0x1a: // Ctrl + Z
				Undo();
				break;
			case 0x19: // Ctrl + Y
				Redo();
				break;
			case 0x01: // Ctrl + A
				SelectAll();
				break;
			}
		}
		return;
	}

	InputChar(nChar);

	CWnd::OnChar(nChar, nRepCnt, nFlags);
}

BOOL CEditCtrl::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;
}

void CEditCtrl::ClearKeywordCompletion()
{
	m_completion_data.cnt = -1;
}

void CEditCtrl::KeywordCompletion(BOOL reverse)
{
	if(HaveSelected() == TRUE) {
		ClearSelected();
		return;
	}

	if(m_read_only == TRUE) return;

	// 現在位置が単語の末尾でないときは，補完しない
	unsigned int ch = m_edit_data->get_cur_char();
	if(!(m_edit_data->get_str_token()->isBreakChar(ch))) return;

	POINT	pt1, pt2;

	if(m_completion_data.cnt != -1) { // 連続実行
		// 元の文字に戻す
		// Undo()後に，m_completion_data.cntがクリアされるのを回避
		int cnt = m_completion_data.cnt;
		Undo();
		m_completion_data.cnt = cnt;
	}

	// 単語の末尾位置を取得
	pt2.y = m_edit_data->get_cur_row();
	pt2.x = m_edit_data->get_cur_col();
	if(pt2.x == 0) return;

	// 単語の先頭位置を取得
	m_edit_data->move_word(-1);
	pt1.y = m_edit_data->get_cur_row();
	pt1.x = m_edit_data->get_cur_col();
	m_edit_data->set_cur(pt2.y, pt2.x);

	if(m_completion_data.cnt == -1) { // 最初の実行
		// 単語の長さを計算
		int bufsize = m_edit_data->calc_buf_size(&pt1, &pt2);
		if(bufsize > sizeof(m_completion_data.org_str)) return;

		m_edit_data->copy_text_data(&pt1, &pt2, m_completion_data.org_str, bufsize);
	}

	// キーワード補完
	TCHAR buf[EDIT_CTRL_MAX_WORD_LEN];

	m_completion_data.cnt = m_edit_data->get_str_token()->KeywordCompletion(
		m_completion_data.org_str,
		m_completion_data.cnt,
		buf, sizeof(buf),
		reverse);
	if(m_completion_data.cnt != -1) {
		m_edit_data->replace_str(&pt1, &pt2, buf);
	}

	CheckScrollBar();

	InvalidateRow_AllWnd(m_edit_data->get_cur_row());
	SetCaret(TRUE);
}

void CEditCtrl::BackSpace()
{
	if(m_read_only == TRUE) return;

	if(HaveSelected()) {
		Delete();
		return;
	}

	// テキストの先頭では、backspaceしない
	if (m_edit_data->get_cur_row() == 0 && m_edit_data->get_cur_col() == 0) return;

	int old_row_cnt = m_edit_data->get_row_cnt();
	int old_split_cnt = GetDispRowSplitCnt(m_edit_data->get_cur_row());
	int old_char_cnt = m_edit_data->get_edit_row_data(m_edit_data->get_cur_row())->get_char_cnt();

	unsigned int ch = m_edit_data->get_prev_char();

	m_edit_data->back_space();

	ClearKeywordCompletion();
	InvalidateEditData_AllWnd(old_row_cnt, old_char_cnt, old_split_cnt, m_edit_data->get_cur_row());

	// コメントの色分け対応
	if(IsCheckCommentChar(ch) || old_row_cnt != m_edit_data->get_row_cnt()) {
		CheckCommentRow(m_edit_data->get_cur_row(), m_edit_data->get_cur_row());
	}

	SetCaret(TRUE);
}

void CEditCtrl::DeleteKey()
{
	if(m_read_only == TRUE) return;

	if(HaveSelected() == TRUE) {
		Delete();
		return;
	}

	// テキストの最後では、deleteしない
	if(m_edit_data->get_cur_row() == m_edit_data->get_row_cnt() - 1 &&
		m_edit_data->get_cur_col() == m_edit_data->get_row_len(m_edit_data->get_cur_row())) return;

	int cur_row = m_edit_data->get_cur_row();
	int old_row_cnt = m_edit_data->get_row_cnt();
	int old_split_cnt = GetDispRowSplitCnt(m_edit_data->get_cur_row());
	int old_char_cnt = m_edit_data->get_edit_row_data(m_edit_data->get_cur_row())->get_char_cnt();

	unsigned int ch = m_edit_data->get_cur_char();

	m_edit_data->delete_key();

	ClearKeywordCompletion();
	InvalidateEditData_AllWnd(old_row_cnt, old_char_cnt, old_split_cnt, cur_row);

	// コメントの色分け対応
	if(IsCheckCommentChar(ch) || old_row_cnt != m_edit_data->get_row_cnt()) {
		CheckCommentRow(cur_row, cur_row);
	}
	SetCaret(TRUE);
}

void CEditCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	BOOL	ctrl = FALSE;
	BOOL	shift = FALSE;

	if(GetKeyState(VK_SHIFT) < 0) shift = TRUE;
	if(GetKeyState(VK_CONTROL) < 0) ctrl = TRUE;

	if((m_ex_style & ECS_DISABLE_KEY_DOWN)) {
		switch(nChar) {
		case VK_ESCAPE:
			if(HaveSelected() == TRUE) {
				ClearSelected();
			}
			break;
		}
	} else {
		switch(nChar) {
		case VK_ESCAPE:
			if(HaveSelected() == TRUE) {
				ClearSelected();
			} else {
				KeywordCompletion(shift);
			}
			break;
		case VK_LEFT:
			if(ctrl) {
				WordLeft(shift);
			} else {
				CharLeft(shift);
			}
			break;
		case VK_RIGHT:
			if(ctrl) {
				WordRight(shift);
			} else {
				CharRight(shift);
			}
			break;
		case VK_UP:
			if(ctrl == TRUE) {
				ScrollUp();
			} else {
				LineUp(shift);
			}
			break;
		case VK_DOWN:
			if(ctrl == TRUE) {
				ScrollDown();
			} else {
				LineDown(shift);
			}
			break;
		case VK_HOME:
			if(ctrl == TRUE) {
				DocumentStart(shift);
			} else {
				LineStart(shift);
			}
			break;
		case VK_END:
			if(ctrl == TRUE) {
				DocumentEnd(shift);
			} else {
				LineEnd(shift);
			}
			break;
		case VK_PRIOR:
			if(ctrl == TRUE) {
				ScrollPageUp();
			} else {
				PageUp(shift);
			}
			break;
		case VK_NEXT:
			if(ctrl == TRUE) {
				ScrollPageDown();
			} else {
				PageDown(shift);
			}
			break;
		case VK_BACK:
			BackSpace();
			break;
		case VK_DELETE:
			DeleteKey();
			break;
		}
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CEditCtrl::CreateCaret(BOOL destroy)
{
	if(destroy) {
		HideCaret();
		DestroyCaret();
	}

	if(m_overwrite) {
		m_caret_width = GetFontWidth(NULL, m_edit_data->get_cur_char());
		if(m_caret_width < 2) m_caret_width = 2;
	} else {
		m_caret_width = 2;
	}

	if(m_ex_style & ECS_IME_CARET_COLOR) {
		HIMC hIMC = ImmGetContext(GetSafeHwnd());
		BOOL b_ime = ImmGetOpenStatus(hIMC);
		ImmReleaseContext(GetSafeHwnd(), hIMC);

		CBitmap *caret_bmp = NULL;
		if(b_ime) {
			caret_bmp = m_edit_data->get_disp_data()->GetCaretBitmap(
				m_caret_width, m_font_height, b_ime);
			if(caret_bmp != NULL) {
				CWnd::CreateCaret(caret_bmp);
			} else {
				CreateGrayCaret(m_caret_width, m_font_height);
			}
		} else {
			CreateSolidCaret(m_caret_width, m_font_height);
		}
	} else {
		CreateSolidCaret(m_caret_width, m_font_height);
	}

	if(m_ex_style & ECS_NO_BLINK_CARET) SetCaretBlinkTime(86400000);	// 24H

	ShowCaret();
}

void CEditCtrl::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd::OnSetFocus(pOldWnd);
	
	CreateCaret(FALSE);
	SetCaret(FALSE);

	if(m_font.m_hObject == NULL) return;

	HIMC hIMC = ImmGetContext(GetSafeHwnd());

	// Set font for the conversion window
	LOGFONT lf;
	m_font.GetLogFont(&lf);
	ImmSetCompositionFont(hIMC, &lf);

	ImmReleaseContext(GetSafeHwnd(), hIMC);	

	GetParent()->SendMessage(EC_WM_SETFOCUS, (WPARAM)pOldWnd, 0);
}

void CEditCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	CWnd::OnKillFocus(pNewWnd);

	HideCaret();
	DestroyCaret();
	SetCaretBlinkTime(m_caret_blink_time);

	if(GetSelectArea()->drag_flg != NO_DRAG) {
		EndDragSelected();
	}

	GetParent()->SendMessage(EC_WM_KILLFOCUS, (WPARAM)pNewWnd, 0);
}

void CEditCtrl::SelectWord(POINT pt)
{
	POINT	pt1, pt2;
	
	m_edit_data->move_word(1);
	pt2.x = m_edit_data->get_cur_col();
	pt2.y = m_edit_data->get_cur_row();

	m_edit_data->move_word(-1);
	pt1.x = m_edit_data->get_cur_col();
	pt1.y = m_edit_data->get_cur_row();

	StartSelect(pt1);
	EndSelect(pt2);
}

void CEditCtrl::SelectWord()
{
	POINT pt;
	pt.y = m_edit_data->get_cur_row();
	pt.x = m_edit_data->get_cur_col();

	SelectWord(pt);
}

void CEditCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if(HitTestClickable(point)) {
		DoClickable();
		return;
	}

	if(HaveSelected()) {
		ClearSelected();
		return;
	}

	POINT pt;
	pt.y = m_edit_data->get_cur_row();
	pt.x = m_edit_data->get_cur_col();

	m_double_click_pos = pt;
	SetTimer(IdDoubleClickTimer, GetDoubleClickTime(), NULL);

	PreDragSelected(TRUE);
	SelectWord(pt);

	GetParent()->SendMessage(EC_WM_LBUTTONDBLCLK, (WPARAM)pt.x, (LPARAM)pt.y);
	
	CWnd::OnLButtonDblClk(nFlags, point);
}

void CEditCtrl::SelectRow(int row, BOOL b_move_caret)
{
	if(row == m_edit_data->get_row_cnt() - 1) {
		StartSelect(CPoint(m_edit_data->get_row_len(row), row));
	} else {
		StartSelect(CPoint(0, row + 1));
	}
	EndSelect(CPoint(0, row), b_move_caret);
}

void CEditCtrl::SelectRow()
{
	SelectRow(m_edit_data->get_cur_row(), FALSE);
}

// データの位置から、表示上の位置を取得 (文字単位)
void CEditCtrl::GetDispDataPoint(CPoint data_pt, POINT *disp_pt, BOOL b_line_end)
{
	disp_pt->x = data_pt.x;
	disp_pt->y = m_edit_data->get_scroll_row(data_pt.y);

	int		split = 0;
	int		row = GetDispRow(disp_pt->y);
	// Y座標を計算
	for(; split < (GetDispRowSplitCnt(row) - 1); split++) {
		if(disp_pt->x < GetDispOffset(disp_pt->y + 1)) break;
		(disp_pt->y)++;
	}
	// X座標を計算
	disp_pt->x -= GetDispOffset(disp_pt->y);

	// 右端の処理
	if(b_line_end && split != 0 && disp_pt->x == 0) {
		(disp_pt->y)--;
		disp_pt->x = data_pt.x - GetDispOffset(disp_pt->y);
	}
}

int CEditCtrl::GetDispWidthStr(const TCHAR *str)
{
	CFontWidthHandler dchandler(this, &m_font);

	int w = CEditRowData::get_disp_width_str(str, m_edit_data->get_disp_data(),
		&dchandler, m_edit_data->get_tabstop(), -1);

	return w;
}

// 表示上の文字の位置から、カーソルの位置を取得
void CEditCtrl::GetDispCaretPoint(CPoint disp_pt, POINT *caret_pt)
{
	caret_pt->y = (disp_pt.y - GetScrollPos(SB_VERT)) * m_row_height + m_col_header_height;

	CFontWidthHandler dchandler(this, &m_font);
	const TCHAR *p = m_edit_data->get_disp_row_text(GetDispRow(disp_pt.y)) + GetDispOffset(disp_pt.y);
	int w = CEditRowData::get_disp_width_str(p, m_edit_data->get_disp_data(),
		&dchandler, m_edit_data->get_tabstop(), disp_pt.x);

	caret_pt->x = m_row_header_width + w - (GetScrollPos(SB_HORZ) * m_char_width);
}

void CEditCtrl::ConvertMousePtToCharPt(CPoint mouse_pt, POINT *char_pt, BOOL enable_minus)
{
	// 表示上の位置から，データ上の位置を取得
	int mouse_row = (long)floor((double)((double)mouse_pt.y - m_col_header_height) / m_row_height);

	if(enable_minus == FALSE) {
		if(mouse_row < 0) mouse_row = 0;
	}

	int idx = GetScrollPos(SB_VERT) + mouse_row;
	if(idx < 0) idx = 0;
	if(idx >= m_edit_data->get_disp_data()->GetMaxDispIdx()) {
		idx = m_edit_data->get_disp_data()->GetMaxDispIdx() - 1;
	}
	int row = GetDispRow(idx);

	int x;
	{
		int mouse_x = mouse_pt.x - m_row_header_width + m_char_width * GetScrollPos(SB_HORZ);
		int split_idx;
		int disp_offset = GetDispOffset(idx , &split_idx);
		const TCHAR *p = m_edit_data->get_disp_row_text(row) + disp_offset;
		CFontWidthHandler dchandler(this, &m_font);

		x = CEditRowData::get_x_from_disp_width_str(p, m_edit_data->get_disp_data(),
			&dchandler, m_edit_data->get_tabstop(), mouse_x);
		x += disp_offset;

		// 折り返し表示の右側をクリックしたとき、表示行の最後にする
		if(split_idx < GetDispRowSplitCnt(row) - 1) {
			int next_disp_offset = m_edit_data->get_edit_row_data(row)->get_split_pos(split_idx + 1);
			if(x > next_disp_offset) x = next_disp_offset;
		}
	}

	char_pt->y = GetDispRow(idx);
	char_pt->x = x;
}

void CEditCtrl::GetDataPoint(CPoint point, POINT *pt, BOOL enable_minus)
{
	ConvertMousePtToCharPt(point, pt, enable_minus);
}

int CEditCtrl::HitTestRowHeader(CPoint point)
{
	if((m_ex_style & ECS_SHOW_ROW_NUM) == 0) return -1;

	if(point.y < m_col_header_height) return -1;
	
	//if(point.x < 0 || point.x > m_row_header_width) return -1;
	// 行頭を選択しやすいようにする
	if(point.x < 0 || point.x > m_row_header_width - m_char_width * 1.5) return -1;

	POINT	pt;
	GetDataPoint(point, &pt);

	return pt.y;
}

void CEditCtrl::SetCaretPoint(CPoint point, BOOL extend)
{
	POINT	pt;
	GetDataPoint(point, &pt);

	BOOL b_line_end = FALSE;
	if(point.x >= m_row_header_width + m_char_width * 2) b_line_end = TRUE;

	// row, colを同時に移動すると，col位置がずれてしまう
	// (col位置を保存しながら行移動するため)
	{
		PreMoveCaret(extend);
		m_edit_data->move_cur_row(pt.y - m_edit_data->get_cur_row());
		m_edit_data->move_cur_col(pt.x - m_edit_data->get_cur_col());
		PostMoveCaret(extend, FALSE, FALSE);
	}

	SetCaret(TRUE, 0, b_line_end);

	// 行番号をクリックしたときの処理
	if(!extend && HitTestRowHeader(point) != -1) {
		SelectRow(m_edit_data->get_cur_row(), FALSE);
	}
}

BOOL CEditCtrl::HitTestSelectedArea(CPoint point)
{
	if(HaveSelected() == FALSE) return FALSE;
	if(point.y < m_col_header_height) return FALSE;
	if(point.x < m_row_header_width) return FALSE;

	POINT	pt;
	GetDataPoint(point, &pt);

	if(GetSelectArea()->box_select) {
		if(pt.y < GetSelectArea()->pos1.y || pt.y > GetSelectArea()->pos2.y) return FALSE;

		int start_x, end_x;

		m_edit_data->get_box_x2(pt.y, &start_x, &end_x);
		if(pt.x < start_x || pt.x > end_x) return FALSE;

		return TRUE;
	}
		
		
	if(GetSelectArea()->pos1.y == GetSelectArea()->pos2.y) {
		if(IsPtInArea(&GetSelectArea()->pos1, &GetSelectArea()->pos2, &pt)) {
			return TRUE;
		}
		return FALSE;
	}

	if(GetSelectArea()->pos1.y < pt.y && GetSelectArea()->pos2.y > pt.y) return TRUE;
	if(GetSelectArea()->pos1.y == pt.y && GetSelectArea()->pos1.x <= pt.x) return TRUE;
	if(GetSelectArea()->pos2.y == pt.y && GetSelectArea()->pos2.x > pt.x) return TRUE;

	return FALSE;
}

BOOL CEditCtrl::DragStart()
{
	if(!(m_ex_style & ECS_DRAG_DROP_EDIT)) return FALSE;

	int bufsize = GetSelectedBytes();

	HGLOBAL hData = GlobalAlloc(GHND, bufsize);
	TCHAR *pstr = (TCHAR *)GlobalLock(hData);
	if(GetSelectArea()->box_select) {
		m_edit_data->copy_text_data_box(pstr, bufsize);
	} else {
		m_edit_data->copy_text_data(&GetSelectArea()->pos1, &GetSelectArea()->pos2, pstr, bufsize);
	}
	GlobalUnlock(hData);

	HGLOBAL hData2 = GlobalAlloc(GHND, 20);
	TCHAR *is_box = (TCHAR *)GlobalLock(hData2);
	if(is_box != NULL) {
		if(GetSelectArea()->box_select) {
			_stprintf(is_box, _T("TRUE"));
		} else {
			_stprintf(is_box, _T("FALSE"));
		}
	}
	GlobalUnlock(hData2);

	COleDataSource dataSource;
	dataSource.CacheGlobalData(CF_UNICODETEXT, hData);
	dataSource.CacheGlobalData(m_cf_is_box, hData2);

	m_edit_data->get_disp_data()->SetDragDrop(TRUE);
	m_edit_data->get_disp_data()->SetDropMyself(FALSE);
	DROPEFFECT result = dataSource.DoDragDrop(DROPEFFECT_COPY | DROPEFFECT_MOVE, NULL, NULL);
	m_edit_data->get_disp_data()->SetDragDrop(FALSE);

	if(result == DROPEFFECT_NONE) return FALSE;

	if(result == DROPEFFECT_MOVE && m_edit_data->get_disp_data()->GetDropMyself() == FALSE) {
		Delete();
	}

	return TRUE;
}

void CEditCtrl::ValidateSelectedArea()
{
	if(GetSelectArea()->start_pt.y != GetSelectArea()->pos1.y ||
		GetSelectArea()->start_pt.x != GetSelectArea()->pos1.x) {
		GetSelectArea()->start_pt = GetSelectArea()->pos2;
	}
	if(GetSelectArea()->start_pt2.y != -1) {
		GetSelectArea()->start_pt2 = GetSelectArea()->pos2;
	}
}

void CEditCtrl::DrawDragRect(CPoint point, BOOL first)
{
	POINT	data_pt, disp_pt, caret_pt;
	GetDataPoint(point, &data_pt, TRUE);
	GetDispDataPoint(data_pt, &disp_pt);
	GetDispCaretPoint(disp_pt, &caret_pt);

	RECT	rect;
	rect.left = caret_pt.x;
	rect.right = rect.left + 2;
	rect.top = caret_pt.y;
	rect.bottom = rect.top + m_row_height;

	CRect	win_rect;
	GetDispRect(&win_rect);
	if(rect.top < m_col_header_height || rect.bottom > (win_rect.Height() - m_row_height - m_bottom_space) ||
		rect.left < m_row_header_width || rect.right > (win_rect.Width() - m_char_width - m_right_space)) {
		rect.top = -1;
		rect.bottom = -1;
		rect.left = -1;
		rect.right = -1;
	}

	SIZE	size;
	size.cx = 1;
	size.cy = 1;

	CDC *pDC = GetDC();
	if(first) {
		pDC->DrawDragRect(&rect, size, NULL, size);
	} else {
		pDC->DrawDragRect(&rect, size, &m_old_dragrect, size);
	}
	ReleaseDC(pDC);

	m_old_dragrect = rect;
	m_old_drag_pt = disp_pt;

	ShowPoint(disp_pt);
}

DROPEFFECT CEditCtrl::DragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	if(!(m_ex_style & ECS_DRAG_DROP_EDIT) && !(m_ex_style & ECS_TEXT_DROP_EDIT)) return FALSE;

	if(pDataObject->IsDataAvailable(CF_UNICODETEXT) || pDataObject->IsDataAvailable(CF_TEXT)) {
		DrawDragRect(point, TRUE);

		if(GetAsyncKeyState(VK_CONTROL) < 0) return DROPEFFECT_COPY;
		return DROPEFFECT_MOVE;
	}

	return DROPEFFECT_NONE;
}

DROPEFFECT CEditCtrl::DragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	if(!(m_ex_style & ECS_DRAG_DROP_EDIT) && !(m_ex_style & ECS_TEXT_DROP_EDIT)) return FALSE;

	if(pDataObject->IsDataAvailable(CF_UNICODETEXT) || pDataObject->IsDataAvailable(CF_TEXT)) {
		DrawDragRect(point, FALSE);

		if(GetAsyncKeyState(VK_CONTROL) < 0) return DROPEFFECT_COPY;
		return DROPEFFECT_MOVE;
	}

	return DROPEFFECT_NONE;
}

void CEditCtrl::DropPaste(POINT &pt1, POINT &pt2, TCHAR *pstr, BOOL is_box)
{
	m_edit_data->set_cur(pt1.y, pt1.x);

	if(is_box) {
		ClearSelected();
		ReplaceStrBox(pstr);
	} else {
		m_edit_data->paste(pstr);
	}
	pt2.y = m_edit_data->get_cur_row();
	pt2.x = m_edit_data->get_cur_col();
}

static TCHAR *GetCFTEXT(HGLOBAL hData, CString *str)
{
	const char *p = (const char *)GlobalLock(hData);
	if(p == NULL) return NULL;

	int len = (int)mbstowcs(NULL, p, -1);

	TCHAR *buf = str->GetBuffer(len + 1);
	mbstowcs(buf, p, len);
	buf[len] = '\0';
	str->ReleaseBuffer();

	return str->GetBuffer(0);
}

BOOL CEditCtrl::Drop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	if(!(m_ex_style & ECS_DRAG_DROP_EDIT) && !(m_ex_style & ECS_TEXT_DROP_EDIT)) return FALSE;

	if(pDataObject->IsDataAvailable(CF_UNICODETEXT) || pDataObject->IsDataAvailable(CF_TEXT)) {
		if(m_edit_data->get_disp_data()->GetDragDrop() && HitTestSelectedArea(point) && dropEffect != DROPEFFECT_COPY) {
			SetCaretPoint(point, FALSE);
			return FALSE;
		}
		BOOL is_box = FALSE;
		HGLOBAL hData2 = pDataObject->GetGlobalData(m_cf_is_box);
		if(hData2 != NULL) {
			TCHAR *buf = (TCHAR *)GlobalLock(hData2);
			if(_tcscmp(buf, _T("TRUE")) == 0) {
				is_box = TRUE;
			}
			GlobalUnlock(hData2);
		}

		POINT		pt1, pt2;
		TCHAR		*pstr;
		CString		str_buf;

		{
			CUndoSetMode undo_obj(m_edit_data->get_undo());

			GetDataPoint(point, &pt1);

			HGLOBAL hData = pDataObject->GetGlobalData(CF_UNICODETEXT);
			if(hData == NULL) {
				hData = pDataObject->GetGlobalData(CF_OEMTEXT);
				if(hData == NULL) hData = pDataObject->GetGlobalData(CF_TEXT);
				if(hData == NULL) return FALSE;
				
				// 文字コード変換
				pstr = GetCFTEXT(hData, &str_buf);
			} else {
				pstr = (TCHAR *)GlobalLock(hData);
			}

			// MOVEモードで自分自身にドロップするときは，ペースト前に削除する
			if(m_edit_data->get_disp_data()->GetDragDrop() && dropEffect != DROPEFFECT_COPY) {
				int box_start_x, box_end_x;

				if(GetSelectArea()->box_select) {
					m_edit_data->get_box_x2(GetSelectArea()->pos1.y, &box_start_x, &box_end_x);
					m_edit_data->del_box(TRUE);
				} else {
					m_edit_data->del(&GetSelectArea()->pos1, &GetSelectArea()->pos2, TRUE);
				}

				// カーソル位置を調節する
				if(!is_box && GetSelectArea()->pos2.y == pt1.y && GetSelectArea()->pos1.y != GetSelectArea()->pos2.y) {
					pt1.x += (GetSelectArea()->pos1.x - GetSelectArea()->pos2.x);
				} else if(!is_box && GetSelectArea()->pos1.y == GetSelectArea()->pos2.y &&
					GetSelectArea()->pos1.y == pt1.y && GetSelectArea()->pos1.x <= pt1.x) {
					pt1.x -= (GetSelectArea()->pos2.x - GetSelectArea()->pos1.x);
				} else if(is_box && GetSelectArea()->pos1.y <= pt1.y &&
					GetSelectArea()->pos2.y >= pt1.y && GetSelectArea()->pos1.x <= pt1.x) {
					pt1.x -= (box_end_x - box_start_x);
				}
				if(GetSelectArea()->pos2.y <= pt1.y && !is_box) {
					pt1.y -= GetSelectArea()->pos2.y - GetSelectArea()->pos1.y;
				}
				m_edit_data->set_valid_point(&pt1);
			}
			DropPaste(pt1, pt2, pstr, is_box);

			GlobalUnlock(hData);
		}

		ClearSelected(FALSE);
		Redraw_AllWnd(TRUE);

		m_edit_data->set_valid_point(&pt1);
		m_edit_data->set_valid_point(&pt2);
		SetSelectedPoint(pt1, pt2, is_box);

		m_edit_data->get_disp_data()->SetDropMyself(m_edit_data->get_disp_data()->GetDragDrop());

		SetForegroundWindow();

		return TRUE;
	}

	return FALSE;
}

void CEditCtrl::DragLeave()
{
	InvalidateRect(&m_old_dragrect);
}

BOOL CEditCtrl::CompleteComposition()
{
	HIMC hIMC = ImmGetContext(GetSafeHwnd());

	if(ImmGetOpenStatus(hIMC)) {
		if(ImmGetCompositionString(hIMC, GCS_COMPSTR, NULL, 0) != 0) {
			ImmNotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
			ImmReleaseContext(GetSafeHwnd(), hIMC);
			return TRUE;
		}
	}

	ImmReleaseContext(GetSafeHwnd(), hIMC);

	return FALSE;
}

void CEditCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if(GetFocus() != this) SetFocus();
	if(CompleteComposition()) return;

	// FULL SCREEN MODEのとき、余白部分は反応しない
	if(m_ex_style & ECS_FULL_SCREEN_MODE) {
		CRect rect;
		GetDispRect(&rect);

		int margin = 40;
		if(point.y < m_top_space - margin) return;
		if(point.y > rect.bottom - m_bottom_space + margin) return;
		if(point.x < m_left_space - margin) return;
		if(point.x > rect.right - m_right_space + margin) return;
	}

	BOOL shift = (GetKeyState(VK_SHIFT) < 0);
	BOOL ctrl = (GetKeyState(VK_CONTROL) < 0);

	// トリプルクリックの処理
	if(KillTimer(IdDoubleClickTimer) == TRUE) {
		POINT	pt;
		GetDataPoint(point, &pt);
		if(m_double_click_pos.y == pt.y && m_double_click_pos.x == pt.x) {
			ClearSelected();
			PreDragSelected();
			SelectRow(pt.y, FALSE);
			return;
		}
	}

	if(m_ex_style & ECS_DRAG_DROP_EDIT && HitTestSelectedArea(point)) {
		if(DragStart() == TRUE) return;
		SetCaretPoint(point, shift);
		return;
	}

	if(ctrl) {
		NextBoxSelect();
	}

	SetCaretPoint(point, shift);

	// ドラッグ選択の準備
	PreDragSelected();

	CWnd::OnLButtonDown(nFlags, point);
}

void CEditCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if(GetSelectArea()->drag_flg != NO_DRAG) {
		EndDragSelected();
	}
	
	CWnd::OnLButtonUp(nFlags, point);
}

void CEditCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	switch(GetSelectArea()->drag_flg) {
	case NO_DRAG:
		if(m_ex_style & ECS_FULL_SCREEN_MODE) {
			if(m_null_cursor_cnt == 0 && (m_null_cursor_pt.x != point.x || m_null_cursor_pt.y != point.y)) {
				m_null_cursor_cnt = FULL_SCREEN_NULL_CURSOR_CNT;
				Invalidate();
			}
			m_null_cursor_pt = point;
		}
		RefleshCursor();
		break;
	case PRE_DRAG:
		StartDragSelected();
		break;
	case DO_DRAG:
		DoDragSelected(point);
		break;
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CEditCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);

	if(cx == 0) return;

	if(m_edit_data->get_disp_data()->GetLineMode() == EC_LINE_MODE_RIGHT) {
		SetLineModeRight(cx);
		MakeDispData();
	}

	CalcShowRow();
	CheckScrollBar();
	SetCaret(FALSE, 1);
	SetImeRect();
}

void CEditCtrl::SetImeRect()
{
	GetDispRect(&m_ime_rect);
	m_ime_rect.top = m_col_header_height;
	m_ime_rect.left = m_row_header_width;
	if(m_edit_data->get_disp_data()->GetLineMode() == EC_LINE_MODE_LEN) {
		m_ime_rect.right = m_char_width * m_edit_data->get_disp_data()->GetLineLen() + m_row_header_width;
	}
}

void CEditCtrl::OnTimer(UINT_PTR nIDEvent) 
{
	switch(nIDEvent) {
	case IdDoubleClickTimer:
		KillTimer(IdDoubleClickTimer);
		break;
	case IdDragSelectTimer:
		{
			POINT		pt;
			CRect		rect;

			GetCursorPos(&pt);
			ScreenToClient(&pt);
			GetDispRect(rect);

			rect.left = m_row_header_width;
			rect.top = m_col_header_height;
			rect.right -= m_right_space;
			rect.bottom -= m_bottom_space;
			
			if(rect.PtInRect(pt) != 0) return;

			switch(GetSelectArea()->drag_flg) {
			case PRE_DRAG:
				StartDragSelected();
				break;
			case DO_DRAG:
				DoDragSelected(pt, TRUE);
				break;
			}
		}
		break;
	case IdHighlightChar:
		KillTimer(IdHighlightChar);
		ClearHighlightChar();
		break;
	case IdFullScreenTimer:
		if(m_null_cursor_cnt > 0) {
			m_null_cursor_cnt--;
			if(m_null_cursor_cnt == 0) {
				RefleshCursor();
				Invalidate();
			}
		}
		break;
	}
	
	CWnd::OnTimer(nIDEvent);
}

void CEditCtrl::ClearHighlightChar()
{
	if(m_edit_data->get_disp_data()->GetHighlightPos().y != -1) {
		InvalidateRow_AllWnd(m_edit_data->get_disp_data()->GetHighlightPos().y);
		m_edit_data->get_disp_data()->SetHighlightPos(CPoint(-1, -1));
	}
	if(m_edit_data->get_disp_data()->GetHighlightPos2().y != -1) {
		InvalidateRow_AllWnd(m_edit_data->get_disp_data()->GetHighlightPos2().y);
		m_edit_data->get_disp_data()->SetHighlightPos2(CPoint(-1, -1));
	}
}

void CEditCtrl::Cut()
{
	if(m_read_only == TRUE) return;
	if(HaveSelected() == FALSE) return;

	CopyMain();
	Delete();
}

CString CEditCtrl::GetSelectedText()
{
	if(HaveSelected() == FALSE) return _T("");

	if(GetSelectArea()->box_select) {
		return m_edit_data->get_selected_text_box();
	}
	return m_edit_data->get_point_text(GetSelectArea()->pos1, GetSelectArea()->pos2);
}

void CEditCtrl::ToUpper()
{
	if(HaveSelected() == FALSE) return;

	POINT pos1 = GetSelectArea()->pos1;
	POINT pos2 = GetSelectArea()->pos2;
	BOOL box_select = GetSelectArea()->box_select;
	CString	buf = GetSelectedText();

	buf.MakeUpper();

	Paste(buf, GetSelectArea()->box_select);
	SetSelectedPoint(pos1, pos2, box_select);
}

void CEditCtrl::ToLower()
{
	if(HaveSelected() == FALSE) return;

	POINT pos1 = GetSelectArea()->pos1;
	POINT pos2 = GetSelectArea()->pos2;
	BOOL box_select = GetSelectArea()->box_select;
	CString	buf = GetSelectedText();

	buf.MakeLower();

	Paste(buf, GetSelectArea()->box_select);
	SetSelectedPoint(pos1, pos2, box_select);
}

void CEditCtrl::ToLowerWithOutCommentAndLiteral()
{
	if(HaveSelected() == FALSE) return;
	if(GetSelectArea()->box_select) {
		MessageBox(_T("矩形選択には対応していません"), _T("メッセージ"), MB_ICONINFORMATION | MB_OK);
		return;
	}

	if(m_edit_data->to_lower_without_comment_and_literal(&GetSelectArea()->pos1, &GetSelectArea()->pos2) != 0) return;

	ValidateSelectedArea();
	Redraw_AllWnd();
}

void CEditCtrl::ToUpperWithOutCommentAndLiteral()
{
	if(HaveSelected() == FALSE) return;
	if(GetSelectArea()->box_select) {
		MessageBox(_T("矩形選択には対応していません"), _T("メッセージ"), MB_ICONINFORMATION | MB_OK);
		return;
	}

	if(m_edit_data->to_upper_without_comment_and_literal(&GetSelectArea()->pos1, &GetSelectArea()->pos2) != 0) return;

	ValidateSelectedArea();
	Redraw_AllWnd();
}

void CEditCtrl::HankakuToZenkaku(BOOL b_alpha, BOOL b_kata)
{
	ZenkakuHankakuConvert(b_alpha, b_kata, TRUE);
}

void CEditCtrl::ZenkakuToHankaku(BOOL b_alpha, BOOL b_kata)
{
	ZenkakuHankakuConvert(b_alpha, b_kata, FALSE);
}

void CEditCtrl::ZenkakuHankakuConvert(BOOL b_alpha, BOOL b_kata, BOOL b_hankaku_to_zenkaku)
{
	if(HaveSelected() == FALSE) return;
	if(GetSelectArea()->box_select) {
		MessageBox(_T("矩形選択には対応していません"), _T("メッセージ"), MB_ICONINFORMATION | MB_OK);
		return;
	}

	if(b_hankaku_to_zenkaku) {
		if(m_edit_data->hankaku_to_zenkaku(&GetSelectArea()->pos1, &GetSelectArea()->pos2,
			b_alpha, b_kata) != 0) return;
	} else {
		if(m_edit_data->zenkaku_to_hankaku(&GetSelectArea()->pos1, &GetSelectArea()->pos2,
			b_alpha, b_kata) != 0) return;
	}

	ValidateSelectedArea();
	Redraw_AllWnd();
}

int CEditCtrl::GetSelectedBytes()
{
	if(!HaveSelected()) return 0;

	int bufsize;

	if(GetSelectArea()->box_select) {
		bufsize = m_edit_data->calc_buf_size_box();
	} else {
		bufsize = m_edit_data->calc_buf_size(&GetSelectArea()->pos1, &GetSelectArea()->pos2);
	}

	return bufsize;
}

int CEditCtrl::GetSelectedChars()
{
	if(!HaveSelected()) return 0;

	int bufsize;

	if(GetSelectArea()->box_select) {
		bufsize = m_edit_data->calc_char_cnt_box();
	} else {
		bufsize = m_edit_data->calc_char_cnt(&GetSelectArea()->pos1, &GetSelectArea()->pos2);
	}

	return bufsize;
}

void CEditCtrl::CopyMain(POINT *pt1, POINT *pt2, BOOL box)
{
	int bufsize;

	if(box) {
		bufsize = m_edit_data->calc_buf_size_box();
	} else {
		bufsize = m_edit_data->calc_buf_size(pt1, pt2);
	}

	HGLOBAL hData = GlobalAlloc(GHND, bufsize);
	TCHAR *pstr = (TCHAR *)GlobalLock(hData);

	if(box) {
		m_edit_data->copy_text_data_box(pstr, bufsize);
	} else {
		m_edit_data->copy_text_data(pt1, pt2, pstr, bufsize);
	}

	GlobalUnlock(hData);

	HGLOBAL hData2 = GlobalAlloc(GHND, 20);
	TCHAR *is_box = (TCHAR *)GlobalLock(hData2);
	if(is_box != NULL) {
		if(GetSelectArea()->box_select) {
			_stprintf(is_box, _T("TRUE"));
		} else {
			_stprintf(is_box, _T("FALSE"));
		}
	}
	GlobalUnlock(hData2);

	if ( !OpenClipboard() ) {
		AfxMessageBox( _T("Cannot open the Clipboard") );
		return;
	}
	// Remove the current Clipboard contents
	if( !EmptyClipboard() ) {
		AfxMessageBox( _T("Cannot empty the Clipboard") );
		return;
	}
	// ...
	// Get the currently selected data
	// ...
	// For the appropriate data formats...
	if ( ::SetClipboardData( CF_UNICODETEXT, hData ) == NULL ) {
		AfxMessageBox( _T("Unable to set Clipboard data") );
		CloseClipboard();
		return;
	}

	if(::SetClipboardData(m_cf_is_box, hData2) == NULL) {
		AfxMessageBox( _T("Unable to set Clipboard data") );
		CloseClipboard();
		return;
	}

	// ...
	CloseClipboard();
}

void CEditCtrl::CopyMain()
{
	POINT	pt1, pt2;

	if(HaveSelected() == TRUE) {
		pt1 = GetSelectArea()->pos1;
		pt2 = GetSelectArea()->pos2;
		CopyMain(&pt1, &pt2, GetSelectArea()->box_select);
	} else {
		// 選択範囲がないときは，現在行をコピーする
		if(m_ex_style & ECS_ROW_COPY_AT_NO_SEL) CopyRowMain();
	}
}

void CEditCtrl::Copy()
{
	CopyMain();

	if(HaveSelected()) {
		// コピーしたら選択解除の設定時の動作
		if(m_ex_style & ECS_CLEAR_AFTER_COPY) {
			ClearSelected();
		}
	}
}

void CEditCtrl::ReplaceStrBox(const TCHAR *pstr)
{
	ASSERT(!HaveSelected());

	if(!HaveSelected()) {
		// start_w, end_wを設定する
		GetSelectArea()->next_box_select = TRUE;
		StartSelect(m_edit_data->get_cur_pt());
		EndSelect(m_edit_data->get_cur_pt(), FALSE);
	}

	m_edit_data->replace_str_box(pstr);

	// CEditCtrl::ClearSelected()では、HaveSelected()がFALSEを返すので選択範囲がclearされない
	m_edit_data->get_disp_data()->ClearSelected();
}

void CEditCtrl::Paste(const TCHAR *pstr, BOOL is_box, BOOL set_caret_flg)
{
	//CWaitCursor		wait_cursor;

	int old_row_cnt = m_edit_data->get_row_cnt();
	int old_split_cnt = GetDispRowSplitCnt(m_edit_data->get_cur_row());
	int old_char_cnt = m_edit_data->get_edit_row_data(m_edit_data->get_cur_row())->get_char_cnt();

	int	cur_row = m_edit_data->get_cur_row();

	if(is_box && !HaveSelected()) {
		ReplaceStrBox(pstr);
		old_row_cnt = -1;	// 全データを再描画
	} else if(HaveSelected() == TRUE) {
		if(GetSelectArea()->box_select) {
			m_edit_data->replace_str_box(pstr);
		} else {
			m_edit_data->replace_str(&GetSelectArea()->pos1, &GetSelectArea()->pos2, pstr);
		}
		if(HaveSelectedMultiLine()) {
			old_row_cnt = -1; // 全データを再描画
			cur_row = min(GetSelectArea()->pos1.y, GetSelectArea()->pos2.y);
		}
		ClearSelected();
	} else {
		m_edit_data->paste(pstr);
	}
	ClearKeywordCompletion();

	InvalidateEditData_AllWnd(old_row_cnt, old_char_cnt, old_split_cnt, cur_row);
	// コメントの色分け対応
	CheckCommentRow(cur_row, m_edit_data->get_cur_row());

	if(set_caret_flg) {
		SetCaret(TRUE, 1);
	}
}

void CEditCtrl::Paste(BOOL data_only, BOOL box_paste)
{
	if(m_read_only == TRUE) return;
	
	COleDataObject	obj;

	if(obj.AttachClipboard() == FALSE) return;
	if(obj.IsDataAvailable(CF_TEXT) == FALSE && 
	   obj.IsDataAvailable(CF_OEMTEXT) == FALSE &&
	   obj.IsDataAvailable(CF_UNICODETEXT) == FALSE) return;

	TCHAR *pstr;
	CString str_buf;

	HGLOBAL hData = obj.GetGlobalData(CF_UNICODETEXT);
	// CF_UNICODETEXTが取れない場合，CF_OEMTEXT, CF_TEXTの取得を試みる
	if(hData == NULL) {
		hData = obj.GetGlobalData(CF_OEMTEXT);
		if(hData == NULL) hData = obj.GetGlobalData(CF_TEXT);
		if(hData == NULL) return;

		// 文字コード変換
		pstr = GetCFTEXT(hData, &str_buf);
	} else {
		pstr = (TCHAR *)GlobalLock(hData);
		if(pstr == NULL) return;
	}

	BOOL is_box = box_paste;
	HGLOBAL hData2 = obj.GetGlobalData(m_cf_is_box);
	if(hData2 != NULL) {
		TCHAR *buf = (TCHAR *)GlobalLock(hData2);
		if(buf != NULL && _tcscmp(buf, _T("TRUE")) == 0) {
			is_box = TRUE;
		}
		GlobalUnlock(hData2);
	}

	if(data_only) {
		m_edit_data->paste(pstr, TRUE);
	} else {
		Paste(pstr, is_box);
	}

	GlobalUnlock(hData);
}

void CEditCtrl::Undo()
{
	if(m_read_only == TRUE) return;

	m_edit_data->undo();

	ClearSelected();
	Redraw_AllWnd(TRUE);
}

BOOL CEditCtrl::CanRedo()
{
	if(m_read_only == TRUE) return FALSE;

	return m_edit_data->can_redo();
}

BOOL CEditCtrl::CanUndo()
{
	if(m_read_only == TRUE) return FALSE;

	return m_edit_data->can_undo();
}

BOOL CEditCtrl::CanPaste()
{
	if(m_read_only == TRUE) return FALSE;

	COleDataObject	obj;

	if(obj.AttachClipboard() == FALSE) return FALSE;
	if(obj.IsDataAvailable(CF_UNICODETEXT) == FALSE && obj.IsDataAvailable(CF_TEXT) == FALSE) return FALSE;

	return TRUE;
}

BOOL CEditCtrl::CanCopy()
{
	if(!(m_ex_style & ECS_ROW_COPY_AT_NO_SEL)) return HaveSelected();
	return TRUE;
}

BOOL CEditCtrl::CanCut()
{
	if(m_read_only == TRUE) return FALSE;
	
	return HaveSelected();
}

void CEditCtrl::Redo()
{
	if(m_read_only == TRUE) return;

	m_edit_data->redo();

	ClearSelected();
	Redraw_AllWnd(TRUE);
}

void CEditCtrl::ClearSearchText()
{
	if(m_search_data.GetDispSearch() == FALSE) return;
	
	m_search_data.SetDispSearch(FALSE);
	m_edit_data->get_disp_data()->GetDispColorDataPool()->ClearAllSearched();
	Invalidate_AllWnd();
}

void CEditCtrl::SaveSearchData2(const TCHAR *search_text, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp)
{
	m_search_data.SetDispSearch(TRUE);
	m_search_data.MakeRegData2(search_text, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp);

	m_edit_data->get_disp_data()->GetDispColorDataPool()->ClearAllData();
	Invalidate_AllWnd();
}

void CEditCtrl::SaveSearchData(const TCHAR *search_text, BOOL b_distinct_lwr_upr, BOOL b_regexp)
{
	SaveSearchData2(search_text, b_distinct_lwr_upr, TRUE, b_regexp);
}

int CEditCtrl::SearchNext2(const TCHAR *search_text, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii,
	BOOL b_regexp, BOOL *b_looped, BOOL enable_loop)
{
	return SearchText2(search_text, 1, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp, b_looped, enable_loop);
}

int CEditCtrl::SearchNext(const TCHAR *search_text, BOOL b_distinct_lwr_upr,
	BOOL b_regexp, BOOL *b_looped, BOOL enable_loop)
{
	return SearchText(search_text, 1, b_distinct_lwr_upr, b_regexp, b_looped, enable_loop);
}

int CEditCtrl::SearchPrev2(const TCHAR *search_text, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii,
	BOOL b_regexp, BOOL *b_looped, BOOL enable_loop)
{
	return SearchText2(search_text, -1, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp, b_looped, enable_loop);
}

int CEditCtrl::SearchPrev(const TCHAR *search_text, BOOL b_distinct_lwr_upr,
	BOOL b_regexp, BOOL *b_looped, BOOL enable_loop)
{
	return SearchText(search_text, -1, b_distinct_lwr_upr, b_regexp, b_looped, enable_loop);
}

void CEditCtrl::Redraw_AllWnd(BOOL b_show_caret)
{
	Redraw2(b_show_caret, TRUE);

	if(IsSplitterMode()) {
		m_edit_data->get_disp_data()->SendNotifyMessage(GetSafeHwnd(), EC_WM_REDRAW,
			(BOOL)b_show_caret, 0); 
	}
}

void CEditCtrl::Redraw2(BOOL b_show_caret, BOOL b_make_disp_data)
{
	if(IsWindow(m_hWnd) == FALSE) return;

	m_edit_data->get_disp_data()->GetDispColorDataPool()->ClearAllData();

	SetHeaderSize();
	if(b_make_disp_data) {
		MakeDispData();
		CheckCommentRow();
	}
	CalcShowRow();
	CheckScrollBar();

	SetCaret(b_show_caret);
	ClearBackGround();
	SetImeRect();
	Invalidate();
}

void CEditCtrl::Redraw(BOOL b_show_caret)
{
	if(IsWindow(m_hWnd) == FALSE) return;

	m_edit_data->get_disp_data()->GetDispColorDataPool()->ClearAllData();

	SetHeaderSize();
	MakeDispData();
	CheckCommentRow();
	CalcShowRow();
	CheckScrollBar();
	SetCaret(b_show_caret);
	ClearBackGround();
	Invalidate();
}

void CEditCtrl::SelectAll()
{
	StartSelect(CPoint(m_edit_data->get_row_len(m_edit_data->get_row_cnt() - 1),
		m_edit_data->get_row_cnt() - 1));
	EndSelect(CPoint(0, 0), FALSE);

	Invalidate_AllWnd();
}

void CEditCtrl::Invalidate_AllWnd(BOOL bErase)
{
	if(IsSplitterMode()) {
		m_edit_data->get_disp_data()->SendNotifyMessage(GetSafeHwnd(), EC_WM_INVALIDATE,
			(BOOL)bErase, 0); 
	}
	Invalidate(bErase);
}

void CEditCtrl::Invalidate(BOOL bErase)
{
	if(IsWindow(m_hWnd) == FALSE) return;

	CWnd::Invalidate(bErase);

	GetParent()->SendMessage(EC_WM_INVALIDATED);
}

BOOL CEditCtrl::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, 
	const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	return CWnd::Create(EDITCTRL_CLASSNAME, lpszWindowName, dwStyle, rect, pParentWnd, 
		nID, pContext);
}

void CEditCtrl::OnDestroy() 
{
	CWnd::OnDestroy();

	ClearBackGround();

	m_edit_data->get_disp_data()->UnRegistNotifyWnd(GetSafeHwnd());
	
	m_font.DeleteObject();
	m_clickable_font.DeleteObject();
	m_bold_font.DeleteObject();
}

void CEditCtrl::SetColor(int type, COLORREF color)
{
	m_edit_data->get_disp_data()->SetColor(type, color);

	if(type == OPERATOR_COLOR || type == BRACKET_COLOR1 || type == BRACKET_COLOR2 || type == BRACKET_COLOR3) {
		m_ex_style = SetBracketColorStyle(m_ex_style);
	}
}

UINT CEditCtrl::OnGetDlgCode() 
{
	return m_dlg_code;
	//	return CWnd::OnGetDlgCode();
}


BOOL CEditCtrl::SearchMatchBrackets(UINT nChar, POINT *pt)
{
	if(!m_edit_data->get_str_token()->isOpenBrackets(nChar) &&
		!m_edit_data->get_str_token()->isCloseBrackets(nChar)) return FALSE;
	if(m_edit_data->check_char_type() != CHAR_TYPE_NORMAL) return FALSE;

	BOOL b_result = FALSE;

	if(m_edit_data->get_str_token()->isOpenBrackets(nChar)) {
		b_result = m_edit_data->search_brackets_ex(nChar, m_edit_data->get_str_token()->getCloseBrackets(nChar), 
			-1, NULL, '\0', 1, pt);
	} else {
		b_result = m_edit_data->search_brackets_ex(
			m_edit_data->get_str_token()->getOpenBrackets(nChar), nChar,
			1, NULL, '\0', -1, pt);
	}

	return b_result;
}

void CEditCtrl::InvertBrackets(UINT nChar)
{
	if(m_brackets_blink_time <= 0) return;

	ClearHighlightChar();
	KillTimer(IdHighlightChar);

	POINT	pt;
	BOOL b_result = SearchMatchBrackets(nChar, &pt);

	if(b_result) {
		m_edit_data->get_disp_data()->SetHighlightPos(pt);
		m_edit_data->get_disp_data()->SetHighlightPos2(m_edit_data->get_cur_pt());

		InvalidateRow_AllWnd(m_edit_data->get_disp_data()->GetHighlightPos().y);
		if(m_edit_data->get_disp_data()->GetHighlightPos().y != m_edit_data->get_disp_data()->GetHighlightPos2().y) {
			InvalidateRow_AllWnd(m_edit_data->get_disp_data()->GetHighlightPos2().y);
		}

		SetTimer(IdHighlightChar, m_brackets_blink_time, NULL);
	}
}

void CEditCtrl::InvertBrackets()
{
	InvertBrackets(m_edit_data->get_cur_char());
}

void CEditCtrl::GoMatchBrackets()
{
	POINT	pt;

	if(!SearchMatchBrackets(m_edit_data->get_cur_char(), &pt)) return;
	
	m_edit_data->set_cur(pt.y, pt.x);
	SetCaret(TRUE, 1);
}

int CEditCtrl::GetScrollLeftMargin()
{
	return m_row_header_width;
}

int CEditCtrl::GetScrollRightMargin()
{
	return m_right_space;
}

int CEditCtrl::GetScrollTopMargin()
{
	return m_col_header_height;
}

int CEditCtrl::GetScrollBottomMargin()
{
	return m_bottom_space;
}

int CEditCtrl::GetHScrollSizeLimit()
{
	return m_char_width * (GetMaxDispCol() + 1) + m_row_header_width;
}

int CEditCtrl::GetVScrollSizeLimit()
{
	if(m_ex_style & ECS_V_SCROLL_EX) {
		return m_row_height * (m_edit_data->get_disp_data()->GetMaxDispIdx() + 1) + m_col_header_height;
	}
	return m_row_height * m_edit_data->get_disp_data()->GetMaxDispIdx() + m_col_header_height;
}

int CEditCtrl::GetHScrollSize(int xOrig, int x)
{
	return (xOrig - x) * m_char_width;
}

int CEditCtrl::GetVScrollSize(int yOrig, int y)
{
	return (yOrig - y) * m_row_height;
}

int CEditCtrl::GetVScrollMin()
{
	return 0;
}

int CEditCtrl::GetVScrollMax()
{
	if(m_ex_style & ECS_V_SCROLL_EX) {
		return m_edit_data->get_disp_data()->GetMaxDispIdx();
	}
	return m_edit_data->get_disp_data()->GetMaxDispIdx() - 1;
}

int CEditCtrl::GetVScrollPage()
{
	return GetShowRow() - 1;
}

int CEditCtrl::GetHScrollMin()
{
	return 0;
}

int CEditCtrl::GetHScrollMax()
{
	return GetMaxDispCol();
}

int CEditCtrl::GetHScrollPage()
{
	return GetShowCol() - 1;
}

void CEditCtrl::RefleshCursor()
{
	if(m_ex_style & ECS_FULL_SCREEN_MODE) {
		if(m_null_cursor_cnt == 0) {
			SetCursor(NULL);
			return;
		}
	} else {
		if(!(m_ex_style & ECS_CLICKABLE_URL)) return;
	}

	CPoint	point;
	GetCursorPos(&point);
	ScreenToClient(&point);

	CRect	rect;
	GetDispRect(&rect);

	rect.left = m_row_header_width;
	rect.top = m_col_header_height;
	if(rect.PtInRect(point) == FALSE) return;

	if(HitTestClickable(point)) {
		SetCursor(m_link_cursor);
	} else {
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_IBEAM));
	}
}

void CEditCtrl::Scrolled(CSize sizeScroll, BOOL bThumbTrack)
{
	// ウィンドウを分割表示中に，カーソルを移動して横スクロールしたとき，
	// アクティブでない方のウィンドウが正しく描画されないバグを修正。
	if(IsSplitterMode() && sizeScroll.cx != 0) {
		m_edit_data->get_disp_data()->SendNotifyMessage(GetSafeHwnd(), EC_WM_INVALIDATE, (BOOL)TRUE, 0); 
	}

	SetCaret(FALSE);
	RefleshCursor();
	GetParent()->SendMessage(EC_WM_SCROLL);
}

void CEditCtrl::ClearAll()
{
	m_edit_data->del_all();
	m_edit_data->get_disp_data()->GetDispColorDataPool()->ClearAllData();
	ClearSelected();
	Redraw_AllWnd(TRUE);
}

void CEditCtrl::SetSelectInfo(POINT start_pt)
{
	GetSelectArea()->start_pt = start_pt;
	GetSelectArea()->box_select = GetSelectArea()->next_box_select;
	GetSelectArea()->next_box_select = FALSE;

	if(GetSelectArea()->box_select) {
		m_edit_data->get_disp_data()->SetDCHandler(this, &m_font);
		GetSelectArea()->box_start_w = m_edit_data->get_disp_width_pt(GetSelectArea()->start_pt, GetSelectArea()->dchandler);
	}
}

void CEditCtrl::StartSelect(CPoint point)
{
	ASSERT(point.y >= 0 && point.y < m_edit_data->get_row_cnt());
	ASSERT(point.x >= 0 && point.x < m_edit_data->get_edit_row_data(point.y)->get_char_cnt());

	ClearSelected();

	SetSelectInfo(point);

	GetSelectArea()->start_pt1 = point;
	GetSelectArea()->start_pt2 = CPoint(-1, -1);
	GetSelectArea()->pos1 = point;
	GetSelectArea()->pos2 = point;
}

void CEditCtrl::EndSelect(CPoint point, BOOL b_move_caret)
{
	ASSERT(point.y >= 0 && point.y < m_edit_data->get_row_cnt());
	ASSERT(point.x >= 0 && point.x < m_edit_data->get_edit_row_data(point.y)->get_char_cnt());

	SelectText(point, FALSE);
	GetSelectArea()->start_pt2 = point;

	// 正規化
	if(GetSelectArea()->start_pt1.y > GetSelectArea()->start_pt2.y ||
		(GetSelectArea()->start_pt1.y == GetSelectArea()->start_pt2.y && 
		GetSelectArea()->start_pt1.x > GetSelectArea()->start_pt2.x)) {
		POINT	pt_tmp;
		pt_tmp = GetSelectArea()->start_pt1;
		GetSelectArea()->start_pt1 = GetSelectArea()->start_pt2;
		GetSelectArea()->start_pt2 = pt_tmp;
	}

	if(b_move_caret) {
		m_edit_data->set_cur(point.y, point.x);
		SetCaret(TRUE, 1);
	}
}

void CEditCtrl::ReverseSelectedRows()
{
	if(HaveSelected() == FALSE) return;

	CWaitCursor		wait_cursor;

	int		start_row = GetSelectArea()->pos1.y;
	int		end_row = GetSelectArea()->pos2.y;
	// 選択範囲の末尾が行頭の場合，その行は対象にしない
	if(end_row > start_row && GetSelectArea()->pos2.x == 0) end_row--;

	m_edit_data->reverse_rows(start_row, end_row);

	ClearSelected();

	// 最終行以外のときは，次の行の先頭に移動
	if(end_row == m_edit_data->get_row_cnt() - 1) {
		m_edit_data->set_cur(end_row, m_edit_data->get_row_len(end_row));
	} else {
		m_edit_data->set_cur(end_row + 1, 0);
	}

	MakeDispData();
	CheckCommentRow();
	SetCaret(TRUE, 1);
	Invalidate_AllWnd();
}

void CEditCtrl::SplitSelectedRows()
{
	if(HaveSelected() == FALSE) return;

	CWaitCursor		wait_cursor;

	POINT start_pt = GetSelectArea()->pos1;
	POINT end_pt = GetSelectArea()->pos2;

	m_edit_data->split_rows(&start_pt, &end_pt);

	SetHeaderSize();
	MakeDispData();
	CheckCommentRow();

	SetSelectedPoint(start_pt, end_pt);
	m_edit_data->set_cur(start_pt.y, start_pt.x);

	SetCaret(TRUE, 1);
	Invalidate_AllWnd();
}

void CEditCtrl::SortSelectedRows(BOOL desc)
{
	if(HaveSelected() == FALSE) return;

	CWaitCursor		wait_cursor;

	int		start_row = GetSelectArea()->pos1.y;
	int		end_row = GetSelectArea()->pos2.y;
	// 選択範囲の末尾が行頭の場合，その行は対象にしない
	if(end_row > start_row && GetSelectArea()->pos2.x == 0) end_row--;

	m_edit_data->sort_rows(GetSelectArea()->pos1.y, end_row, desc);

	ClearSelected();

	// 最終行以外のときは，次の行の先頭に移動
	if(end_row == m_edit_data->get_row_cnt() - 1) {
		m_edit_data->set_cur(end_row, m_edit_data->get_row_len(end_row));
	} else {
		m_edit_data->set_cur(end_row + 1, 0);
	}

	MakeDispData();
	CheckCommentRow();
	SetCaret(TRUE, 1);
	Invalidate_AllWnd();
}

void CEditCtrl::SetSpaces(int row_space, int char_space, int top_space, int left_space,
	int width, int height)
{
	m_row_space = row_space;
	m_row_space_top = (row_space + 1) / 2;

	m_edit_data->get_disp_data()->SetCharSpaceSetting(char_space);
	m_top_space = top_space;
	m_left_space = left_space;
	m_bottom_space = 0;
	m_right_space = 0;

	if(width > 0 || height > 0) {
		CRect rect;
		GetDispRect(&rect);
		if(height > 0) {
			m_bottom_space = rect.Height() - m_top_space - height;
			if(m_bottom_space < 0) m_bottom_space = 0;
		}
		if(width > 0) {
			m_right_space = rect.Width() - m_left_space - width;
			if(m_right_space < 0) m_right_space = 0;
		}
	}

	PostSetFont();
}

int CEditCtrl::GetDispRowTop()
{
	return GetDispRow(GetScrollPos(SB_VERT));
}

int CEditCtrl::GetDispRowBottom()
{
	int idx = (GetScrollPos(SB_VERT) + GetShowRow() - 1);
	
	if(idx < 0 || idx >= m_edit_data->get_disp_data()->GetMaxDispIdx()) {
		return m_edit_data->get_row_cnt() - 1;
	}

	return GetDispRow(idx);
}

int CEditCtrl::GetDispRowPos(int row)
{
	ASSERT(row < m_edit_data->get_row_cnt());
	if(row >= m_edit_data->get_row_cnt()) return 0;

	return (m_edit_data->get_scroll_row(row) - GetScrollPos(SB_VERT)) * m_row_height + m_col_header_height;
}

int CEditCtrl::GetMaxDispCol()
{
	int max_char_len = m_edit_data->get_max_disp_width() / m_char_width + 1;
	
	if(max_char_len < m_edit_data->get_disp_data()->GetLineLen()) {
		return max_char_len;
	}

	return m_edit_data->get_disp_data()->GetLineLen();
}

void CEditCtrl::SetLineModeRight(int win_width)
{
	int len = (win_width - m_row_header_width - m_right_space) / m_char_width - 1;
	if(m_edit_data->is_confinement_mode()) len -= 2;
	if(len < 20) len = 20;
	
	int width = len * m_char_width;

	m_edit_data->get_disp_data()->SetLineLen(len);
	m_edit_data->get_disp_data()->SetLineWidth(width);
}

void CEditCtrl::SetLineMode(int mode, int line_len, BOOL redraw)
{
	m_edit_data->get_disp_data()->SetLineMode(mode);

	switch(mode) {
	case EC_LINE_MODE_NORMAL:
		m_edit_data->get_disp_data()->SetLineLen(INT_MAX);
		m_edit_data->get_disp_data()->SetLineWidth(INT_MAX);
		break;
	case EC_LINE_MODE_RIGHT:
		{
			CRect rect;
			GetDispRect(&rect);
			SetLineModeRight(rect.Width());
		}
		break;
	case EC_LINE_MODE_LEN:
		m_edit_data->get_disp_data()->SetLineLen(line_len);
		m_edit_data->get_disp_data()->SetLineWidth(line_len * m_char_width);
		break;
	}

	if(redraw) Redraw_AllWnd();

	SetImeRect();
}

BOOL CEditCtrl::CanDelete()
{
	if(m_read_only == TRUE) return FALSE;
	
	return HaveSelected();
}

void CEditCtrl::DeleteMain(POINT *pt1, POINT *pt2, BOOL box_select, BOOL b_set_caret)
{
	if(m_read_only == TRUE) return;

	BOOL is_box = FALSE;
	if(HaveSelected() && GetSelectArea()->box_select) is_box = TRUE;

	int old_row_cnt = m_edit_data->get_row_cnt();
	int old_split_cnt = GetDispRowSplitCnt(m_edit_data->get_cur_row());
	int old_char_cnt = m_edit_data->get_edit_row_data(m_edit_data->get_cur_row())->get_char_cnt();

	if(is_box) {
		m_edit_data->del_box(TRUE);
	} else {
		m_edit_data->del(pt1, pt2, TRUE);
	}

	if(is_box) {
		MakeDispData(FALSE, pt1->y, pt2->y - pt1->y);
		Invalidate_AllWnd();
	}

	InvalidateEditData_AllWnd(old_row_cnt, old_char_cnt, old_split_cnt, m_edit_data->get_cur_row());

	ClearSelected();
	if(b_set_caret) SetCaret(TRUE);

	// コメントの色分け対応
	CheckCommentRow(m_edit_data->get_cur_row(), m_edit_data->get_cur_row());
}

void CEditCtrl::Delete()
{
	if(HaveSelected() == FALSE) return;
	DeleteMain(&GetSelectArea()->pos1, &GetSelectArea()->pos2, GetSelectArea()->box_select, TRUE);
}

BOOL CEditCtrl::HitDispData(CPoint point)
{
	if(point.y < m_col_header_height) return FALSE;
	if(point.x < m_row_header_width) return FALSE;

	int mouse_row = (long)floor(((double)point.y - m_col_header_height) / m_row_height);

	int idx = GetScrollPos(SB_VERT) + mouse_row;
	if(idx < 0 || idx >= m_edit_data->get_disp_data()->GetMaxDispIdx()) return FALSE;

	int row = GetDispRow(idx);

	{
		int mouse_x = point.x - m_row_header_width + m_char_width * GetScrollPos(SB_HORZ);
		int split_idx;
		int disp_offset = GetDispOffset(idx , &split_idx);
		const TCHAR *p = m_edit_data->get_disp_row_text(row) + disp_offset;
		CFontWidthHandler dchandler(this, &m_font);

		int w = CEditRowData::get_disp_width_str(p, m_edit_data->get_disp_data(), 
			&dchandler, m_edit_data->get_tabstop(), -1);
		if(w < mouse_x) return FALSE;
	}

	return TRUE;
}

BOOL CEditCtrl::GetClickablePT(POINT *pt1, POINT *pt2)
{
	if(!(m_ex_style & ECS_CLICKABLE_URL)) return FALSE;

	POINT pt;
	pt.y = m_edit_data->get_cur_row();
	pt.x = m_edit_data->get_cur_col();

	CDispColorData *p_disp_color_data = m_edit_data->get_disp_data()->GetDispColorDataPool()->GetDispColorData(pt.y);
	if(p_disp_color_data == NULL) {
		p_disp_color_data = m_edit_data->get_disp_data()->GetDispColorDataPool()->GetNewDispColorData(pt.y);
		const TCHAR *p = m_edit_data->get_disp_row_text(pt.y);
		MakeDispClickableData(p_disp_color_data, p);
	}

	if(!(p_disp_color_data->GetClickable(pt.x))) return FALSE;

	int		start, end;

	// 先頭位置を計算
	for(start = pt.x; start > 0 && p_disp_color_data->GetClickable(start - 1);) start--;

	// 終わりの位置を計算
	for(end = pt.x; p_disp_color_data->GetClickable(end);) end++;

	if(pt1 != NULL && pt2 != NULL) {
		pt1->y = pt.y;
		pt1->x = start;
		pt2->y = pt.y;
		pt2->x = end;
	}

	return TRUE;
}

BOOL CEditCtrl::CopyClickable()
{
	if(!(m_ex_style & ECS_CLICKABLE_URL)) return FALSE;

	POINT disp_pt1, disp_pt2;
	if(GetClickablePT(&disp_pt1, &disp_pt2) == FALSE) return FALSE;

	CopyMain(&disp_pt1, &disp_pt2, FALSE);

	return TRUE;
}

CString CEditCtrl::GetClickableURL()
{
	CString url = _T("");

	if(!(m_ex_style & ECS_CLICKABLE_URL)) return url;

	POINT pt1, pt2;
	if(GetClickablePT(&pt1, &pt2) == FALSE) return url;

	const TCHAR *p = m_edit_data->get_disp_row_text(pt1.y);
	p = p + pt1.x;
	url.Format(_T("%.*s"), pt2.x - pt1.x, p);

	return url;
}

BOOL CEditCtrl::IsURL(const TCHAR *p)
{
	for(int i = 0; i < sizeof(clickable_url_str)/sizeof(clickable_url_str[0]); i++) {
		if(_tcsncmp(p, clickable_url_str[i], _tcslen(clickable_url_str[i])) == 0) {
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CEditCtrl::DoClickable()
{
	if(!(m_ex_style & ECS_CLICKABLE_URL)) return FALSE;

	CString url = GetClickableURL();
	if(url == _T("")) return FALSE;

	if(IsURL(url) == FALSE) {
		url = "mailto:" + url;
	}

	ShellExecute(NULL, NULL, url, NULL, NULL, SW_SHOWNORMAL);

	return TRUE;
}

BOOL CEditCtrl::HitTestClickableURL()
{
	CString url = GetClickableURL();
	return IsURL(url);
}

BOOL CEditCtrl::HitTestClickable(CPoint point)
{
	if(!(m_ex_style & ECS_CLICKABLE_URL)) return FALSE;
	if(HitDispData(point) == FALSE) return FALSE;

	POINT	pt;
	GetDataPoint(point, &pt, TRUE);

	CDispColorData *p_disp_color_data = m_edit_data->get_disp_data()->GetDispColorDataPool()->GetDispColorData(pt.y);
	if(p_disp_color_data == NULL) {
		p_disp_color_data = m_edit_data->get_disp_data()->GetDispColorDataPool()->GetNewDispColorData(pt.y);
		const TCHAR *p = m_edit_data->get_disp_row_text(pt.y);
		MakeDispClickableData(p_disp_color_data, p);
	}

	if(p_disp_color_data->GetClickable(pt.x)) return TRUE;

	return FALSE;
}

void CEditCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	if(GetFocus() != this) SetFocus();

	BOOL shift = FALSE;
	if(GetKeyState(VK_SHIFT) < 0) shift = TRUE;

	if(GetSelectArea()->drag_flg != NO_DRAG) {
		EndDragSelected();
	}

	if(HaveSelected()) {
		POINT	pt;
		GetDataPoint(point, &pt);
		if(!IsPtInArea(&GetSelectArea()->pos1, 
			&GetSelectArea()->pos2, &pt)) {
			SetCaretPoint(point, shift);
		}
	} else {
		SetCaretPoint(point, shift);
	}
	
	CScrollWnd::OnRButtonDown(nFlags, point);
}

void CEditCtrl::InfrateSelectedArea()
{
	if(HaveSelected()) {
		POINT	pt1, pt2;
		GetSelectedPoint(&pt1, &pt2);
		pt1.x = 0;
/*
		if(m_edit_data->get_cur_row() == pt2.y - 1 && m_edit_data->get_cur_col() == 0 && pt2.x == 0) {
			// 行選択のとき，下の行まで選択しない
			pt2.y--;
		}
*/
		if(pt2.x == 0) {
			// 行の先頭までが選択範囲のとき，その行まで選択しない
			pt2.y--;
		}

		// 行全体を選択する
		pt2.x = m_edit_data->get_row_len(pt2.y);

		SetSelectedPoint(pt2, pt1);
	} else {
		// 選択範囲がないとき，現在の行を選択する
		POINT	pt1, pt2;
		pt1.x = 0;
		pt1.y = m_edit_data->get_cur_row();
		pt2.y = pt1.y;
		pt2.x = m_edit_data->get_row_len(pt2.y);

		SetSelectedPoint(pt1, pt2);
	}
}

void CEditCtrl::LineStart(BOOL extend, BOOL start_flg)
{
	PreMoveCaret(extend);

	int col = -m_edit_data->get_cur_col();
	int cur_col = m_edit_data->get_cur_col();
	m_edit_data->move_cur_col(col);

	if(!start_flg) {
		m_edit_data->skip_space(1);
		if(cur_col == m_edit_data->get_cur_col()) {
			m_edit_data->move_cur_col(col);
		}
	}

	PostMoveCaret(extend, TRUE);
}

void CEditCtrl::LineEnd(BOOL extend)
{
	PreMoveCaret(extend);

	m_edit_data->move_line_end();

	PostMoveCaret(extend, TRUE);
}

void CEditCtrl::DocumentStart(BOOL extend)
{
	PreMoveCaret(extend);

	m_edit_data->move_cur_row(-m_edit_data->get_cur_row());
	m_edit_data->move_cur_col(-m_edit_data->get_cur_col());

	PostMoveCaret(extend, TRUE);
}

void CEditCtrl::DocumentEnd(BOOL extend)
{
	PreMoveCaret(extend);

	m_edit_data->move_document_end();

	PostMoveCaret(extend, TRUE);
}

void CEditCtrl::SetBookMark()
{
	int row = m_edit_data->get_cur_row();

	m_edit_data->toggle_row_data_flg(row, ROW_DATA_BOOK_MARK);

	InvalidateRowHeader_AllWnd(row);
}

void CEditCtrl::SetBreakPoint()
{
	int row = m_edit_data->get_cur_row();

	m_edit_data->toggle_row_data_flg(row, ROW_DATA_BREAK_POINT);

	InvalidateRowHeader_AllWnd(row);
}

BOOL CEditCtrl::JumpBookMarkLoop(int start, int end, int allow, BOOL extend)
{
	int		i;

	for(i = start; i != end; i += allow) {
		if(m_edit_data->get_row_data_flg(i, ROW_DATA_BOOK_MARK)) {
			MoveCaretPos(i, 0, extend);
			return TRUE;
		}
	}

	return FALSE;
}

void CEditCtrl::JumpBookMark(BOOL reverse, BOOL extend)
{
	if(reverse == FALSE) {
		if(JumpBookMarkLoop(m_edit_data->get_cur_row() + 1, m_edit_data->get_row_cnt(), 1, extend)) return;
		if(JumpBookMarkLoop(0, m_edit_data->get_cur_row() + 1, 1, extend)) return;
	} else {
		if(JumpBookMarkLoop(m_edit_data->get_cur_row() - 1, -1, -1, extend)) return;
		if(JumpBookMarkLoop(m_edit_data->get_row_cnt() - 1, m_edit_data->get_cur_row() - 1, -1, extend)) return;
	}
}

void CEditCtrl::ClearAllBookMark()
{
	int		i, loop_cnt;

	loop_cnt = m_edit_data->get_row_cnt();
	for(i = 0; i < loop_cnt; i++) {
		m_edit_data->unset_row_data_flg(i, ROW_DATA_BOOK_MARK);
	}
	Invalidate_AllWnd();
}

void CEditCtrl::ToggleOverwrite()
{
	m_overwrite = !m_overwrite;
	ResetCaret();
}

void CEditCtrl::ResetCaret()
{
	if(GetFocus() == this) {
		CreateCaret(TRUE);
		SetCaret(FALSE);
	}
}

int CEditCtrl::SpaceToTab()
{
	POINT		pt1, pt2;
	GetSelectedPoint(&pt1, &pt2);

	m_edit_data->space_to_tab(&pt1, &pt2);

	pt2.x = m_edit_data->get_row_len(pt2.y);
	SetSelectedPoint(pt1, pt2);

	return 0;
}

int CEditCtrl::TabToSpace()
{
	POINT		pt1, pt2;
	GetSelectedPoint(&pt1, &pt2);

	m_edit_data->tab_to_space(&pt1, &pt2);

	pt2.x = m_edit_data->get_row_len(pt2.y);
	SetSelectedPoint(pt1, pt2);

	return 0;
}

void CEditCtrl::CharLeft(BOOL extend)
{
	if(extend == FALSE && HaveSelected() &&
		((GetSelectArea()->pos1.y == m_edit_data->get_cur_row() && GetSelectArea()->pos1.x == m_edit_data->get_cur_col()) ||
		 (GetSelectArea()->pos2.y == m_edit_data->get_cur_row() && GetSelectArea()->pos2.x == m_edit_data->get_cur_col()))) {
		POINT	pt = GetSelectArea()->pos1;
		ClearSelected();
		MoveCaretPos(pt.y, pt.x);
		return;
	}
	MoveCaret(0, -1, extend);
}

void CEditCtrl::CharRight(BOOL extend)
{
	if(extend == FALSE && HaveSelected() &&
		((GetSelectArea()->pos1.y == m_edit_data->get_cur_row() && GetSelectArea()->pos1.x == m_edit_data->get_cur_col()) ||
		 (GetSelectArea()->pos2.y == m_edit_data->get_cur_row() && GetSelectArea()->pos2.x == m_edit_data->get_cur_col()))) {
		POINT	pt = GetSelectArea()->pos2;
		ClearSelected();
		MoveCaretPos(pt.y, pt.x);
		return;
	}

	MoveCaret(0, 1, extend);
}

CSearchData::CSearchData()
{
	m_b_disp_search = FALSE;
	m_text_buf = NULL;
	m_text_buf_size = 0;
	m_b_regexp = FALSE;
	m_b_distinct_lwr_upr = FALSE;
	m_b_distinct_width_ascii = FALSE;
}

CSearchData::~CSearchData()
{
	if(m_text_buf != NULL) {
		free(m_text_buf);
		m_text_buf = NULL;
		m_text_buf_size = 0;
	}
}

BOOL CSearchData::AllocTextBuf(int size)
{
	if(m_text_buf_size > size) return TRUE;

	size += 1024 - (size % 1024);	// 1k単位で確保する

	m_text_buf = (TCHAR *)realloc(m_text_buf, size);
	if(m_text_buf == NULL) return FALSE;

	m_text_buf_size = size;

	return TRUE;
}

void CSearchData::MakeTextBuf(const TCHAR *p)
{
	_tcscpy(m_text_buf, p);
	if(m_b_distinct_lwr_upr == FALSE) {
		my_mbslwr(m_text_buf);
	}
}

HREG_DATA CSearchData::MakeRegData2(const CString &search_text, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp)
{
	if(m_reg_data.GetRegData() != NULL &&
		m_search_text == search_text &&
		m_b_distinct_lwr_upr == b_distinct_lwr_upr &&
		m_b_distinct_width_ascii == b_distinct_width_ascii &&
		m_b_regexp == b_regexp) {
		return m_reg_data.GetRegData();
	}

	if(!m_reg_data.Compile2(search_text, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp)) return NULL;

	m_search_text = search_text;
	m_b_distinct_lwr_upr = b_distinct_lwr_upr;
	m_b_distinct_width_ascii = b_distinct_width_ascii;
	m_b_regexp = b_regexp;

	return m_reg_data.GetRegData();
}

HREG_DATA CSearchData::MakeRegData(const CString &search_text, BOOL b_distinct_lwr_upr, BOOL b_regexp)
{
	return MakeRegData2(search_text, b_distinct_lwr_upr, TRUE, b_regexp);
}

void CEditCtrl::DeleteRowMain()
{
	POINT	pt1, pt2;
	m_edit_data->calc_del_rows_pos(
		m_edit_data->get_cur_row(), m_edit_data->get_cur_row(),
		&pt1, &pt2);
	DeleteMain(&pt1, &pt2, FALSE, FALSE);
}

void CEditCtrl::DeleteRow()
{
	if(m_read_only == TRUE) return;
	if(m_edit_data->is_empty()) return;

	// カーソル位置を保存
	CFontWidthHandler dchandler(this, &m_font);
	int w = m_edit_data->get_disp_width_pt(m_edit_data->get_cur_pt(), &dchandler);

	CUndoSetMode undo_obj(m_edit_data->get_undo());
	m_edit_data->save_cursor_pos();

	DeleteRowMain();

	// カーソル位置を元に戻す
	POINT pt;
	pt.y = m_edit_data->get_cur_row();
	pt.x = m_edit_data->get_x_from_width_pt(pt.y, w, &dchandler);
	m_edit_data->set_cur(pt.y, pt.x);
	SetCaret(TRUE);

	m_edit_data->save_cursor_pos();
}

void CEditCtrl::DeleteAndSaveCursor(POINT pt1, POINT pt2)
{
	if(m_read_only == TRUE) return;
	if(pt1.y == pt2.y && pt1.x == pt2.x) return;

	CUndoSetMode undo_obj(m_edit_data->get_undo());
	m_edit_data->save_cursor_pos();
	DeleteMain(&pt1, &pt2, FALSE, TRUE);
}

void CEditCtrl::DeleteAfterCaret()
{
	int row = m_edit_data->get_cur_row();
	DeleteAndSaveCursor(
		CPoint(m_edit_data->get_cur_col(), row),
		CPoint(m_edit_data->get_row_len(row), row));
}

void CEditCtrl::DeleteBeforeCaret()
{
	int row = m_edit_data->get_cur_row();
	DeleteAndSaveCursor(
		CPoint(0, row),
		CPoint(m_edit_data->get_cur_col(), row));
}

void CEditCtrl::DeleteWordAfterCaret()
{
	CPoint pt1 = m_edit_data->get_cur_pt();
	CPoint pt2 = GetNextWordPt(1);
	DeleteAndSaveCursor(pt1, pt2);
}

void CEditCtrl::DeleteWordBeforeCaret()
{
	CPoint pt1 = m_edit_data->get_cur_pt();
	CPoint pt2 = GetNextWordPt(-1);
	DeleteAndSaveCursor(pt2, pt1);
}

void CEditCtrl::MoveLastEditPos()
{
	if(HaveSelected()) ClearSelected();
	m_edit_data->move_last_edit_pos();
	SetCaret(TRUE);
}

void CEditCtrl::CutRow()
{
	if(m_read_only == TRUE) return;

	CopyRowMain();
	DeleteRow();
}

void CEditCtrl::JoinRow()
{
	if(m_read_only == TRUE) return;

	if(HaveSelectedMultiLine()) {
		ReplaceText(_T("\n"), _T(""), 1, 
			FALSE, TRUE, TRUE, TRUE, NULL, NULL);
	} else {
		CUndoSetMode undo_obj(m_edit_data->get_undo());

		m_edit_data->save_cursor_pos();
		LineEnd(FALSE);
		DeleteKey();

		SetCaret(TRUE);
	}
}

void CEditCtrl::CopyRowMain()
{
	POINT	pt1, pt2;

	// 選択範囲がないときは，選択中の行をコピーする
	pt1.y = m_edit_data->get_cur_row();
	pt1.x = 0;
	if(pt1.y == m_edit_data->get_row_cnt() - 1) {
		pt2.y = m_edit_data->get_cur_row();
		pt2.x = m_edit_data->get_row_len(m_edit_data->get_cur_row());
	} else {
		pt2.y = m_edit_data->get_cur_row() + 1;
		pt2.x = 0;
	}

	CopyMain(&pt1, &pt2, FALSE);
}

void CEditCtrl::CopyRow()
{
	CopyRowMain();

	if(HaveSelected()) {
		// コピーしたら選択解除の設定時の動作
		if(m_ex_style & ECS_CLEAR_AFTER_COPY) {
			ClearSelected();
		}
	}
}

void CEditCtrl::PasteRow(BOOL b_up_flg)
{
	if(m_read_only == TRUE) return;

	CUndoSetMode undo_obj(m_edit_data->get_undo());
	m_edit_data->save_cursor_pos();

	if(HaveSelected()) ClearSelected();

	if(b_up_flg) {
		// 現在行の上に行を追加
		int row = m_edit_data->get_cur_row();
		m_edit_data->set_cur(row, 0);
		m_edit_data->paste(_T("\n"));
		m_edit_data->set_cur(row, 0);
	} else {
		// 現在行の下に行を追加
		int row = m_edit_data->get_cur_row();
		m_edit_data->set_cur(row, m_edit_data->get_row_len(row));
		m_edit_data->paste(_T("\n"));
	}

	// クリップボードの内容を貼り付ける
	Paste(TRUE);

	// 余計な改行を削除
	if(m_edit_data->get_cur_col() == 0) m_edit_data->back_space();

	m_edit_data->set_cur(m_edit_data->get_cur_row(), 0);

	Redraw_AllWnd(TRUE);
}

void CEditCtrl::SplitStart(BOOL extend)
{
	PreMoveCaret(extend);

	if(m_edit_data->get_disp_data()->GetLineMode() == EC_LINE_MODE_NORMAL || 
		GetDispRowSplitCnt(m_edit_data->get_cur_row()) == 1) {
		LineStart(extend);
		return;
	}

	CPoint caret_pt = GetCaretPos();
	caret_pt.x = m_row_header_width;

	POINT data_pt;
	GetDataPoint(caret_pt, &data_pt);

	if(data_pt.x == 0) {
		LineStart(extend);
		return;
	}
	
	int col = data_pt.x - m_edit_data->get_cur_col();
	m_edit_data->move_cur_col(col);

	PostMoveCaret(extend, FALSE);
	SetCaret(TRUE, 0, FALSE);
}

void CEditCtrl::SplitEnd(BOOL extend)
{
	PreMoveCaret(extend);

	if(m_edit_data->get_disp_data()->GetLineMode() == EC_LINE_MODE_NORMAL || 
		GetDispRowSplitCnt(m_edit_data->get_cur_row()) == 1) {
		LineEnd(extend);
		return;
	}

	CRect rect;
	GetDispRect(&rect);

	CPoint caret_pt = GetCaretPos();
	caret_pt.x = rect.Width() + m_char_width;

	POINT data_pt;
	GetDataPoint(caret_pt, &data_pt);
	
	int col = data_pt.x - m_edit_data->get_cur_col();
	m_edit_data->move_cur_col(col);

	PostMoveCaret(extend, FALSE);
	SetCaret(TRUE, 0, TRUE);
}

void CEditCtrl::SetEditData(CEditData *edit_data)
{
	m_edit_data = edit_data;
	MakeDispData();	// CIntArrayがNULLにならないようにする
}

// FIXME: imm.hに定義してある？
/*
typedef struct tagRECONVERTSTRING {
    DWORD dwSize;
    DWORD dwVersion;
    DWORD dwStrLen;
    DWORD dwStrOffset;
    DWORD dwCompStrLen;
    DWORD dwCompStrOffset;
    DWORD dwTargetStrLen;
    DWORD dwTargetStrOffset;
} RECONVERTSTRING;
*/

#ifndef WM_IME_REQUEST
#define WM_IME_REQUEST	0x0288
#endif
#ifndef IMR_RECONVERTSTRING
#define IMR_RECONVERTSTRING 0x0004
#endif

LRESULT CEditCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message) {
	case WM_IME_NOTIFY:
		if(wParam == IMN_SETOPENSTATUS && (m_ex_style & ECS_IME_CARET_COLOR)) {
			if(GetFocus() == this) CreateCaret(TRUE);
		}
		break;
	case WM_IME_CHAR:
		InputChar((UINT)wParam);
		return 0;
		break;
	case WM_IME_STARTCOMPOSITION:
		m_edit_data->clear_cur_operation();
		break;
	case WM_IME_ENDCOMPOSITION:
		m_edit_data->clear_cur_operation();
		break;
	case WM_IME_REQUEST:
		if(wParam == IMR_RECONVERTSTRING) {
			return (LRESULT)IMEReconvert((void *)lParam);
		}
		break;
	case EC_WM_INVALIDATE_ROW:
		InvalidateRow((int)wParam);
		break;
	case EC_WM_INVALIDATE_ROWS:
		InvalidateRows((int)wParam, (int)lParam);
		break;
	case EC_WM_INVALIDATE_EDITDATA:
		{
			struct invalidate_editdata_st *pdata = (struct invalidate_editdata_st *)wParam;
			InvalidateEditData(pdata->old_row_cnt, pdata->old_char_cnt, 
				pdata->old_split_cnt, pdata->start_row, FALSE);
		}
		break;
	case EC_WM_INVALIDATE:
		Invalidate((BOOL)wParam);
		break;
	case EC_WM_INVALIDATE_ROW_HEADER:
		InvalidateRowHeader((int)wParam);
		break;
	case EC_WM_INVALIDATE_COL_HEADER:
		InvalidateColHeader((int)wParam);
		break;
	case EC_WM_REDRAW:
		Redraw2((BOOL)wParam, FALSE);
		break;
	case EC_WM_REDRAW_ALLWND:
		Redraw_AllWnd((BOOL)wParam);
		break;
	case EC_WM_PASTE_TEXT:
		Paste((const TCHAR *)wParam);
		break;
	case EC_WM_NOTICE_UPDATE:
		NoticeUpdate();
		break;
	}
	
	return CScrollWnd::WindowProc(message, wParam, lParam);
}

void *CEditCtrl::IMEReconvert(void *param)
{
	if(HaveSelected() == FALSE || HaveSelectedMultiLine() == TRUE) return 0;

	int str_len = GetSelectArea()->pos2.x - GetSelectArea()->pos1.x;
	int rec_size = (int)sizeof(RECONVERTSTRING) + (str_len + 1) * (int)sizeof(TCHAR);

	if(param == NULL) {
		return (void *)((INT_PTR)rec_size);
	}

	m_edit_data->set_cur(GetSelectArea()->pos1.y, GetSelectArea()->pos1.x);
	SetCaret(TRUE, 1);

	RECONVERTSTRING	*rec = (RECONVERTSTRING	*)param;

	rec->dwSize = rec_size;
	rec->dwVersion = 0;

	rec->dwStrOffset = sizeof(RECONVERTSTRING);
	rec->dwStrLen = str_len;

	TCHAR *p = (TCHAR *)((char *)param + rec->dwStrOffset);
	_tcsncpy(p, 
		m_edit_data->get_row_buf(m_edit_data->get_cur_row()) + GetSelectArea()->pos1.x,
		rec->dwStrLen);
	p[rec->dwStrLen] = '\0';

	rec->dwCompStrOffset = 0;
	rec->dwCompStrLen = rec->dwStrLen;

	rec->dwTargetStrOffset = rec->dwCompStrOffset;
	rec->dwTargetStrLen = rec->dwCompStrLen;

	return param;
}

void CEditCtrl::ResetDispInfo()
{
	ClearSelected(FALSE);
	SetScrollPos(SB_HORZ, 0);
	SetScrollPos(SB_VERT, 0);
}

void CEditCtrl::CheckScrollBars()
{
	CheckScrollBar();
}

void CEditCtrl::InsertDateTime()
{
	CTime t = CTime::GetCurrentTime();
	CString str = t.Format(_T("%H:%M %Y/%m/%d"));
	Paste(str);
}

void CEditCtrl::PageUp(BOOL extend)
{
	int move_row = GetShowRow() - 1;
	if(move_row == 0) move_row = 1;
	MoveCaretRow(-move_row, extend, TRUE);
}

void CEditCtrl::PageDown(BOOL extend)
{
	int move_row = GetShowRow() - 1;
	if(move_row == 0) move_row = 1;
	MoveCaretRow(move_row, extend, TRUE);
}

int CEditCtrl::GetCaretCol(POINT pt)
{
	CFontWidthHandler dchandler(this, &m_font);
	return GetEditData()->get_caret_col(pt.y, pt.x, &dchandler);
}

int CEditCtrl::GetColFromCaretCol(POINT pt)
{
	CFontWidthHandler dchandler(this, &m_font);
	return GetEditData()->get_col_from_caret_col(pt.y, pt.x, &dchandler);
}

void CEditCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	HScroll(nSBCode);
}

void CEditCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	VScroll(nSBCode);
}

void CEditCtrl::SetModifiedBkColor(COLORREF color)
{
	m_modified_bk_color = color;
	m_original_bk_color = GetColor(BG_COLOR);
}

void CEditCtrl::NoticeUpdate()
{
	if(m_ex_style & ECS_MODIFIED_BK_COLOR) {
		if(m_edit_data->is_edit_data()) {
			SetColor(BG_COLOR, m_modified_bk_color);
		} else {
			SetColor(BG_COLOR, m_original_bk_color);
		}
	}
}
