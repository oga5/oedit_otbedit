/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

// GridCtrl.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "GridCtrl.h"

#include "ostrutil.h"
#include "octrl_util.h"
#include "get_char.h"
#include "oregexp.h"

#include "ColorUtil.h"

#define USE_DOUBLE_BUFFERING

#include <math.h>
#include <imm.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CGridData CGridCtrl::m_null_grid_data;

/////////////////////////////////////////////////////////////////////////////
#define GR_DEF_DRAGRECT_SIZE 2

#define IdDragHeaderTimer	100
#define IdDragSelectTimer	101
#define IdToolTipTimer		102

#define NO_DRAG_HEADER		0
#define PRE_DRAG_HEADER		1
#define DO_DRAG_HEADER		2

#define NO_DRAG		0
#define PRE_DRAG	1
#define DO_DRAG		2

#define SELECT_MODE_NORMAL	0
#define SELECT_MODE_ROW		1
#define SELECT_MODE_COL		2

#define DEFAULT_FONT_HEIGHT	18

#define EDIT_CELL_ID	2001

/////////////////////////////////////////////////////////////////////////////

// 編集用エディットコントロールの，サブクラス後のウィンドウプロシージャ
static LRESULT CALLBACK Edit_SubclassWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
	case WM_KEYDOWN:
		PostMessage(GetParent(hwnd), GC_WM_EDIT_KEY_DOWN, wParam, lParam);
		break;
	default:
		break;
	}
	return (CallWindowProc((WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA),
		hwnd, message, wParam, lParam));
}

IMPLEMENT_DYNAMIC(CGridCtrl, CScrollWnd)
/////////////////////////////////////////////////////////////////////////////
// CGridCtrl

CGridCtrl::CGridCtrl()
{
	RegisterWndClass();

	m_grid_data = &m_null_grid_data;
	m_gridStyle = 0;
	m_row_height = 1;
	m_col_header_row = 1;

	m_font_height = DEFAULT_FONT_HEIGHT;
	m_font_width = m_font_height / 2;
	m_num_width = 0;

	m_droprect.left = 0;

	ClearFixColMode();

	m_min_colwidth = 5;
	m_adjust_min_col_chars = 0;

	m_last_active_wnd = FALSE;

	m_edit_cell = NULL;
	m_edit_data = NULL;

	m_end_key_flg = FALSE;

	m_last_tool_tip_col = -1;

	m_null_text = "";
	m_null_text_len = 0;

	m_change_active_row_text_color = FALSE;

	m_cell_padding_top = 1;
	m_cell_padding_bottom = 1;
	m_cell_padding_left = 2;
	m_cell_padding_right = 2;

/*
m_cell_padding_top = 5;
	m_cell_padding_bottom = 10;
	m_cell_padding_left = 10;
	m_cell_padding_right = 20;
*/
	InitData();
}

CGridCtrl::~CGridCtrl()
{
	FreeEditData();
}

void CGridCtrl::InitData()
{
	m_grid_data->clear_selected_cell();

	m_edit_cell_focused = FALSE;

	m_drag_header.drag_flg = NO_DRAG_HEADER;

	m_grid_data->GetSelectArea()->select_mode = SELECT_MODE_NORMAL;
	m_grid_data->GetSelectArea()->pos1.y = -1;
	m_grid_data->GetSelectArea()->pos1.x = -1;
	m_grid_data->GetSelectArea()->pos2.y = -1;
	m_grid_data->GetSelectArea()->pos2.x = -1;
	m_grid_data->GetSelectArea()->drag_flg = NO_DRAG;
}

BEGIN_MESSAGE_MAP(CGridCtrl, CScrollWnd)
	//{{AFX_MSG_MAP(CGridCtrl)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_GETDLGCODE()
	ON_WM_CHAR()
	ON_WM_RBUTTONDOWN()
	ON_WM_SYSKEYDOWN()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGridCtrl メッセージ ハンドラ

void CGridCtrl::SetCellPadding(int top, int bottom, int left, int right)
{
	m_cell_padding_top = top;
	m_cell_padding_bottom = bottom;
	m_cell_padding_left = left;
	m_cell_padding_right = right;

	PostSetFont();
}

BOOL CGridCtrl::IsSelectedCellColor(int y, int x)
{
	// FIXME: 選択ダイアログ表示中は，現在のセルを普通の背景色にする

	if(IsSelectedCell(y, x)) {
		if(y != m_grid_data->get_cur_row() || x != m_grid_data->get_cur_col() || 
			(m_gridStyle & GRS_LINE_SELECT)) {
			return TRUE;
		}
		// 分割表示で，フォーカスが無い場合
		if(!IsActiveSplitter()) {
			return TRUE;
		}
	} else if(!IsActiveSplitter()) {
		if(y == m_grid_data->get_cur_row() && x == m_grid_data->get_cur_col()) {
			return TRUE;
		}
	}

	return FALSE;
}

COLORREF CGridCtrl::GetCellBkColor(int y, int x)
{
	if(IsSelectedCellColor(y, x)) {
		return GetColor(GRID_SELECT_COLOR);
	}

	if(m_grid_data->IsCustomizeBkColor()) {
		return m_grid_data->GetCellBkColor(y, x);
	}

	if(m_grid_data->IsEditable() == FALSE) {
		if((m_gridStyle & GRS_SHOW_NULL_CELL) && m_grid_data->IsColDataNull(y, x)) {
			return GetColor(GRID_NULL_CELL_COLOR);
		}
		return GetColor(GRID_BG_COLOR);
	}

	int data_row = (m_gridStyle & GRS_SWAP_ROW_COL_MODE) ? x : y;

	if(m_grid_data->IsDeleteRow(data_row)) {
		return GetColor(GRID_DELETE_COLOR);
	} else if(m_grid_data->IsUpdateCell(y, x) && !m_grid_data->IsInsertRow(data_row)) {
		return GetColor(GRID_UPDATE_COLOR);
	} else if(m_grid_data->IsEditableCell(y, x) == FALSE) {
		return GetColor(GRID_UNEDITABLE_COLOR);
	} else if(m_grid_data->IsInsertRow(data_row)) {
		return GetColor(GRID_INSERT_COLOR);
	} else if((m_gridStyle & GRS_SHOW_NULL_CELL) && m_grid_data->IsColDataNull(y, x)) {
		return GetColor(GRID_NULL_CELL_COLOR);
	}

	return GetColor(GRID_BG_COLOR);
}

int CGridCtrl::GetFontWidth(CDC *pdc, unsigned int ch, CFontWidthHandler *pdchandler)
{
	int w = m_grid_data->GetDispData()->GetFontWidth(pdc, ch, pdchandler);
	return w + m_grid_data->GetDispData()->GetCharSpace(w);
}

void CGridCtrl::PaintSpace(CDC *pdc, CDC *p_paintdc, int top, int left, int right)
{
	if(top < m_col_header_height) return;
	
	RECT	rect;
	rect.top = top;
	rect.bottom = top + m_font_height - 1;
	rect.left = left;
	rect.right = rect.left + GetFontWidth(pdc, ' ') - 2;

	CPen	pen;
	pen.CreatePen(PS_SOLID, 0, GetColor(GRID_MARK_COLOR));
	CPen *old_pen = pdc->SelectObject(&pen);

	if(rect.right < right) {
		POINT	poly[4] = {
			{rect.left, rect.bottom - 1},
			{rect.left, rect.bottom},
			{rect.right, rect.bottom},
			{rect.right, rect.bottom -2} };
		pdc->Polyline(poly, 4);
	} else if(rect.left < right) {
		pdc->MoveTo(rect.left, rect.bottom - 1);
		pdc->LineTo(rect.left, rect.bottom);
		pdc->LineTo(right, rect.bottom);
	}

	pdc->SelectObject(old_pen);
}

void CGridCtrl::PaintTab(CDC *pdc, CDC *p_paintdc, int top, int left, int right)
{
	if(top < m_col_header_height) return;

	int width = GetFontWidth(pdc, L'x');

	RECT	rect;

	rect.top = top;
	rect.bottom = top + m_font_height;
	rect.left = left;

	int		y, x1, x2;
	y = rect.top + m_font_height / 2;
	x1 = rect.left + width - width / 4 - 1;
	x2 = rect.left + width / 4;

	if(x1 > right) return;
	
	CPen	pen;
	pen.CreatePen(PS_SOLID, 0, GetColor(GRID_MARK_COLOR));
	CPen *old_pen = pdc->SelectObject(&pen);

	pdc->MoveTo(x1, y);
	pdc->LineTo(x2, y + m_font_height / 4);
	pdc->MoveTo(x1, y);
	pdc->LineTo(x2, y - m_font_height / 4);

	pdc->SelectObject(old_pen);
}

void CGridCtrl::Paint2byteSpace(CDC *pdc, CDC *p_paintdc, int top, int left, int right)
{
	if(top < m_col_header_height) return;
	
	RECT	rect;

	rect.top = top + 2;
	rect.bottom = top + m_font_height - 2;
	rect.left = left + 1;
	rect.right = rect.left + GetFontWidth(pdc, L'　') - 3;

	CPen	pen;
	pen.CreatePen(PS_SOLID, 0, GetColor(GRID_MARK_COLOR));
	CPen *old_pen = pdc->SelectObject(&pen);

	if(rect.right < right) {
		POINT	poly[5] = {
			{rect.left, rect.top},
			{rect.left, rect.bottom},
			{rect.right, rect.bottom},
			{rect.right, rect.top},
			{rect.left, rect.top} };
		pdc->Polyline(poly, 5);
	} else if(rect.left < right) {
		pdc->MoveTo(rect.left, rect.top);
		pdc->LineTo(rect.left, rect.bottom);
		pdc->LineTo(right, rect.bottom);
		pdc->MoveTo(right - 1, rect.top);
		pdc->LineTo(rect.left, rect.top);
	}

	pdc->SelectObject(old_pen);
}

void CGridCtrl::PaintSpaceMain(CDC *pdc, CDC *p_paintdc, RECT &rect, const TCHAR *data)
{
	const TCHAR *p = data;
	unsigned int ch;
	int			left = rect.left;

	for(p = data; *p != '\0'; p += get_char_len(p)) {
		ch = get_char(p);

		if(ch == '\n' || ch == '\r') break;
		if(ch == ' ' && (m_gridStyle & GRS_SHOW_SPACE)) {
			PaintSpace(pdc, p_paintdc, rect.top, left, rect.right);
		} else if(ch == ZENKAKU_SPACE && (m_gridStyle & GRS_SHOW_2BYTE_SPACE)) {
			Paint2byteSpace(pdc, p_paintdc, rect.top, left, rect.right);
		} else if(ch == '\t' && (m_gridStyle & GRS_SHOW_TAB)) {
			PaintTab(pdc, p_paintdc, rect.top, left, rect.right);
		}

		left += GetFontWidth(pdc, ch);
		if(left > rect.right) break;
	}
}

void CGridCtrl::PaintCell(CDC *pdc, CDC *p_paintdc, RECT &rect, int y, int x)
{
	RECT	rect2 = rect;

	rect2.bottom -= 1;
	rect2.right -= 1;

	if(!p_paintdc->RectVisible(&rect2)) return;

//TRACE("PaintCell:%d, %d\n", y, x);

	COLORREF bk_color = GetCellBkColor(y, x);
	pdc->FillSolidRect(&rect2, bk_color);

	rect2 = rect;
	rect2.top += m_cell_padding_top;
	rect2.bottom -= m_cell_padding_bottom;
	rect2.left += m_cell_padding_left;
	rect2.right -= m_cell_padding_right;

	//pdc->SetBkColor(bk_color);
	PaintCellData(pdc, p_paintdc, rect2, y, x);
}

static int search_str_for_paint_text(const TCHAR *data, int start_col,
	INT_PTR *result_start, INT_PTR *search_len,
	const TCHAR *search_text, HREG_DATA reg_data)
{
	if(oreg_exec_str(reg_data, data, start_col, result_start, search_len) == OREGEXP_FOUND) {
		return 1;
	}
	return 0;
}

int CGridCtrl::TextOut2(CDC *pdc, CDC *p_paintdc, const TCHAR *p, int len, RECT rect, 
	int row_left, UINT textout_options, int left_offset)
{
	if(rect.left > rect.right) return rect.left;

	return m_grid_data->GetDispData()->GetRenderer()->TextOut2(pdc, p_paintdc, p, len, rect,
		m_grid_data->GetDispData()->GetFontData(), row_left, textout_options, 0, left_offset);
}

int CGridCtrl::PaintTextMain(CDC *pdc, CDC *p_paintdc, RECT rect, const TCHAR *data, int len, int left_offset)
{
	if(!m_search_data.GetDispSearch() || GetColor(GRID_BG_COLOR) == GetColor(GRID_SEARCH_COLOR)) {
		return TextOut2(pdc, p_paintdc, data, len, rect, rect.left, ETO_CLIPPED, left_offset);
	}

	INT_PTR		result_start = 0;
	INT_PTR		search_len = 0;
	int		start_col = 0;
	const TCHAR *p = data;

	for(; len > 0;) {
		if(search_str_for_paint_text(data, start_col, &result_start, &search_len, 
			m_search_data.GetSearchText(),
			m_search_data.GetRegData()) != 1) break;
		if(search_len == 0) break;

		if(result_start != start_col) {
			int tmp_len = (int)(result_start - start_col);
			rect.left = TextOut2(pdc, p_paintdc, p, tmp_len, rect, rect.left, ETO_CLIPPED, left_offset);

			p += tmp_len;
			len -= tmp_len;
			left_offset = 0;
		}

		if(search_len > len) search_len = len;
		if(search_len > 0) {
			RECT tmp_rect = rect;
			tmp_rect.bottom = rect.bottom - 1;

			if(tmp_rect.right > rect.left) {
				COLORREF bk_cr = pdc->SetBkColor(GetColor(GRID_SEARCH_COLOR));

				// 検索結果の背景で塗る
				rect.left = TextOut2(pdc, p_paintdc, p, (int)search_len, tmp_rect, tmp_rect.left, 
					ETO_CLIPPED | ETO_OPAQUE, left_offset);

				p += search_len;
				len -= (int)search_len;
				left_offset = 0;

				pdc->SetBkColor(bk_cr);
			}
		}

		start_col += (int)(result_start - start_col) + (int)search_len;
	}
	if(len > 0) {
		rect.left = TextOut2(pdc, p_paintdc, p, len, rect, rect.left, ETO_CLIPPED, left_offset);
	}

	return rect.left;
}

void CGridCtrl::PaintLineFeed(CDC *pdc, CDC *p_paintdc, int top, int left)
{
	RECT	rect;

	int width = GetFontWidth(pdc, L'x');

	rect.top = top;
	rect.bottom = rect.top + m_font_height;
	rect.left = left;
	rect.right = rect.left + width * 2;

	CPen	pen;
	pen.CreatePen(PS_SOLID, 0, GetColor(GRID_MARK_COLOR));
	CPen *old_pen = pdc->SelectObject(&pen);

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

void CGridCtrl::PaintCellData(CDC *pdc, CDC *p_paintdc, RECT rect, int y, int x)
{
	const TCHAR *data = GetDispData(y, x);

	if(m_null_text_len > 0 && m_grid_data->IsColDataNull(y, x)) {
		pdc->SetTextColor(GetColor(GRID_MARK_COLOR));
		PaintTextMain(pdc, p_paintdc, rect, m_null_text, m_null_text_len);
		return;
	}

	COLORREF text_color = m_grid_data->GetCellTextColor(y, x);

	if(m_change_active_row_text_color) {
		if(!(m_gridStyle & GRS_SWAP_ROW_COL_MODE)) {
			if(y == m_grid_data->get_cur_row()) {
				text_color = GetColor(GRID_CUR_ROW_TEXT_COLOR);
			}
		} else {
			if(x == m_grid_data->get_cur_col()) {
				text_color = GetColor(GRID_CUR_ROW_TEXT_COLOR);
			}
		}
	}

	if((m_gridStyle & GRS_INVERT_SELECT_TEXT) && IsSelectedCellColor(y, x)) {
		text_color = RGB(0xff, 0xff, 0xff) - text_color;
	}
	pdc->SetTextColor(text_color);

	int len = (int)_tcslen(data);
	if(m_grid_data->Get_ColDataType(y, x) == GRID_DATA_NUMBER_TYPE) {
		// 右寄せで表示
		int w = m_grid_data->GetColWidth(data, rect.right - rect.left, pdc, NULL, NULL);
		int left_offset = (rect.right - rect.left) - w;
		PaintTextMain(pdc, p_paintdc, rect, data, len, left_offset);
	} else {
		int tmp_len = len;
		TCHAR *tmp_p = inline_strchr2(data, '\r', '\n');
		if(tmp_p != NULL) {
			tmp_len = (int)(tmp_p - data);
		}

		int left = PaintTextMain(pdc, p_paintdc, rect, data, tmp_len);

		if(m_gridStyle & (GRS_SHOW_SPACE | GRS_SHOW_2BYTE_SPACE | GRS_SHOW_TAB)) {
			PaintSpaceMain(pdc, p_paintdc, rect, data);
		}
		if(tmp_p != NULL && (m_gridStyle & GRS_SHOW_LINE_FEED)) {
			if(left < rect.right) PaintLineFeed(pdc, p_paintdc, rect.top, left);
		}

		rect.left = left;
	}
}

void CGridCtrl::PaintData(CDC *pdc, CDC *p_paintdc, int show_row, int show_col, CPoint &scr_pt)
{
	int			x, y;
	RECT		rect;
	CRect		winrect;
	COLORREF	bk_cr;

	bk_cr = pdc->GetBkColor();
	GetDispRect(winrect);

	// データ
	rect.bottom = m_col_header_height;
	for(y = scr_pt.y; y < scr_pt.y + show_row; y++) {
		rect.top = rect.bottom;
		rect.bottom = rect.top + m_row_height;
		rect.right = m_row_header_width;

		for(x = m_fix_start_col; x < m_fix_end_col; x++) {
			rect.left = rect.right;
			rect.right = rect.left + GetDispColWidth(x);
			PaintCell(pdc, p_paintdc, rect, y, x);
		}

		for(x = scr_pt.x; x < scr_pt.x + show_col; x++) {
			rect.left = rect.right;
			rect.right = rect.left + GetDispColWidth(x);

			PaintCell(pdc, p_paintdc, rect, y, x);
		}

		if(rect.right < winrect.Width()) {
			rect.left = rect.right;
			rect.right = winrect.Width();
			if(p_paintdc->RectVisible(&rect)) {
				pdc->FillSolidRect(&rect, GetColor(GRID_BG_COLOR));
			}
		}
	}
	if(rect.bottom < winrect.Height()) {
		rect.left = 0;
		rect.right = winrect.Width();
		rect.top = rect.bottom;
		rect.bottom = winrect.Height();
		if(p_paintdc->RectVisible(&rect)) {
			pdc->FillSolidRect(&rect, GetColor(GRID_BG_COLOR));
		}
	}

	pdc->SetBkColor(bk_cr);
}

void CGridCtrl::PaintRowNumber(CDC *pdc, CDC *p_paintdc, int show_row, CPoint &scr_pt, CRect &winrect)
{
	if(!(m_gridStyle & GRS_ROW_HEADER)) return;

	CRect		rect;
	int			y;
	CPen		pen;
	pen.CreatePen(PS_SOLID, 0, GetColor(GRID_HEADER_LINE_COLOR));

	const TCHAR	*buf;
	rect.left = 0;
	rect.right = m_row_header_width;
	rect.bottom = m_col_header_height;
	for(y = scr_pt.y; y < scr_pt.y + show_row; y++) {
		rect.top = rect.bottom;
		rect.bottom = rect.top + m_row_height;
		if(p_paintdc->RectVisible(rect)) {
			buf = m_grid_data->GetRowHeader(y);

			pdc->SetBkColor(GetColor(GRID_HEADER_BG_COLOR));
			if((m_gridStyle & GRS_HIGHLIGHT_HEADER) && y == m_grid_data->get_cur_row()) {
				pdc->SetBkColor(make_highlight_color(GetColor(GRID_HEADER_BG_COLOR), 0.8));
			}
			pdc->SetTextColor(GetColor(GRID_TEXT_COLOR));

			// 空白は数字の文字幅で描画する
			int space_cnt = 0;
			int buf_len = (int)_tcslen(buf);
			for(; *buf == ' '; buf++) {
				space_cnt++;
				buf_len--;
			}
			pdc->FillSolidRect(&rect, pdc->GetBkColor());

			pdc->ExtTextOut(rect.left + m_cell_padding_left + m_num_width * space_cnt, 
				rect.top + m_cell_padding_top,
				ETO_CLIPPED, rect, buf, buf_len, NULL);

			CPen *pOldPen = pdc->SelectObject(&pen);
			pdc->MoveTo(rect.left, rect.bottom - 1);
			pdc->LineTo(rect.right, rect.bottom - 1);
			pdc->SelectObject(pOldPen);
		}
	}
	if(rect.bottom < winrect.Height()) {
		rect.top = rect.bottom;
		rect.bottom = winrect.Height();
		if(p_paintdc->RectVisible(rect)) {
			pdc->FillSolidRect(rect, GetColor(GRID_BG_COLOR));
		}
	}
	rect.top = 0;
	rect.bottom = winrect.Height();
	rect.left = m_row_header_width - 1;
	rect.right = m_row_header_width + 1;
	if(p_paintdc->RectVisible(rect)) {
		CPen *pOldPen = pdc->SelectObject(&pen);
		pdc->MoveTo(m_row_header_width - 1, rect.top);
		pdc->LineTo(m_row_header_width - 1, rect.bottom);
		pdc->SelectObject(pOldPen);
	}
}

void CGridCtrl::PaintNoDispColLine(CDC *pdc, CDC *p_paintdc, int left, int top, int bottom)
{
	CPen	pen;
	pen.CreatePen(PS_SOLID, 0, GetColor(GRID_NO_DISP_CELL_LINE_COLOR));

	CPen *old_pen = pdc->SelectObject(&pen);

	// 前のカラムが非表示のとき
	pdc->MoveTo(left, top);
	pdc->LineTo(left, bottom);

	pdc->SelectObject(old_pen);
}

CString CGridCtrl::GetDispColNameWithNotNullFlg(int col)
{
	const TCHAR *data = m_grid_data->Get_DispColName(col);

	if((m_gridStyle & GRS_SHOW_NOTNULL_COL) && !m_grid_data->IsNullableCol(col)) {
		CString cname;
		cname.Format(_T("%s(*)"), data);
		return cname;
	}

	return data;
}

void CGridCtrl::PaintColHeaderMain(CDC *pdc, CDC *p_paintdc, int x, CRect &rect)
{
	if(p_paintdc->RectVisible(rect)) {
		CString cname = GetDispColNameWithNotNullFlg(x);
		const TCHAR *data = cname.GetBuffer(0);

		pdc->SetBkColor(GetColor(GRID_HEADER_BG_COLOR));
		if((m_gridStyle & GRS_HIGHLIGHT_HEADER) && x == m_grid_data->get_cur_col()) {
			pdc->SetBkColor(make_highlight_color(GetColor(GRID_HEADER_BG_COLOR), 0.8));
		}
		pdc->SetTextColor(m_grid_data->GetCellHeaderTextColor(x));

		pdc->FillSolidRect(rect, GetColor(GRID_HEADER_BG_COLOR));

		int		len;
		CRect	text_rect = rect;
		text_rect.bottom = rect.top + m_row_height;
		for(int row_cnt = 0; row_cnt < m_col_header_row; row_cnt++) {
			const TCHAR *p = _tcschr(data, '\n');
			len = (p != NULL) ? (int)(p - data) : (int)_tcslen(data);

			CRect textout_rect = text_rect;
			textout_rect.top += m_cell_padding_top;
			textout_rect.bottom -= m_cell_padding_bottom;
			textout_rect.left += m_cell_padding_left;
			textout_rect.right -= m_cell_padding_right;
			pdc->ExtTextOut(textout_rect.left, textout_rect.top,
				ETO_CLIPPED | ETO_OPAQUE, textout_rect,
				data, len, NULL);

			data = (p != NULL) ? (p + 1) : _T("");
			text_rect.top = text_rect.bottom;
			text_rect.bottom += m_row_height;
		}

		CPen		pen;
		pen.CreatePen(PS_SOLID, 0, GetColor(GRID_HEADER_LINE_COLOR));
		CPen *pOldPen = pdc->SelectObject(&pen);
		pdc->MoveTo(rect.right - 1, rect.top);
		pdc->LineTo(rect.right - 1, rect.bottom);
		pdc->SelectObject(pOldPen);

		if(x != 0 && m_grid_data->GetDispFlg(x - 1) == FALSE) {
			// 前のカラムが非表示のとき
			PaintNoDispColLine(pdc, p_paintdc, rect.left, rect.top, rect.bottom);
		}
	}
}

void CGridCtrl::PaintColHeader(CDC *pdc, CDC *p_paintdc, int show_col, CPoint &scr_pt, CRect &winrect)
{
	CRect		rect;
	int			x;

	// 項目名
	if(m_gridStyle & GRS_COL_HEADER) {
		rect.right = m_row_header_width;
		rect.top = 0;
		rect.bottom = m_col_header_height;

		for(x = m_fix_start_col; x < m_fix_end_col; x++) {
			rect.left = rect.right;
			rect.right = rect.left + GetDispColWidth(x);
			PaintColHeaderMain(pdc, p_paintdc, x, rect);
		}

		for(x = scr_pt.x; x < scr_pt.x + show_col; x++) {
			rect.left = rect.right;
			rect.right = rect.left + GetDispColWidth(x);
			PaintColHeaderMain(pdc, p_paintdc, x, rect);
		}

		if(rect.right < winrect.Width()) {
			rect.top = 0;
			rect.bottom = m_col_header_height;
			rect.left = rect.right;
			rect.right = winrect.Width();
			if(p_paintdc->RectVisible(rect)) {
				pdc->FillSolidRect(rect, GetColor(GRID_BG_COLOR));

				if(m_grid_data->Get_ColCnt() > 0 &&
						m_grid_data->GetDispFlg(m_grid_data->Get_ColCnt() - 1) == FALSE) {
					// 最終カラムが非表示のとき
					PaintNoDispColLine(pdc, p_paintdc, rect.left, rect.top, rect.bottom);
				}
			}
		}

		rect.top = 0;
		rect.bottom = m_col_header_height;
		rect.left = 0;
		rect.right = GetLineRight(scr_pt, winrect);
		if(p_paintdc->RectVisible(rect)) {
			CPen		pen;
			pen.CreatePen(PS_SOLID, 0, GetColor(GRID_HEADER_LINE_COLOR));
			CPen *pOldPen = pdc->SelectObject(&pen);
			pdc->MoveTo(rect.left, rect.bottom - 1);
			pdc->LineTo(rect.right, rect.bottom - 1);
			pdc->SelectObject(pOldPen);
		}
	}
}

int CGridCtrl::GetLineRight(CPoint &scr_pt, CRect &winrect)
{
/*
	// 横線が，右端のセルからはみださないようにする
	int		result = m_row_header_width;

	for(int x = scr_pt.x; x < m_grid_data->Get_ColCnt(); x++) {
		result = result + GetDispColWidth(x);
		if(result > winrect.right) return winrect.right;
	}

	return result;
*/
	return winrect.Width();
}

void CGridCtrl::PaintGrid(CDC *pdc, CDC *p_paintdc, int show_row, int show_col, CPoint &scr_pt, CRect &winrect)
{
	int		x, y;
	CPen	*pOldPen;
	CRect	rect;

	CPen	pen;
	pen.CreatePen(PS_SOLID, 0, GetColor(GRID_LINE_COLOR));

	// 横線
	pOldPen = pdc->SelectObject(&pen);

	rect.bottom = m_col_header_height;
	rect.left = m_row_header_width;
	rect.right = GetLineRight(scr_pt, winrect);
	for(y = scr_pt.y; y < scr_pt.y + show_row; y++) {
		rect.top = rect.bottom;
		rect.bottom = rect.top + m_row_height;
		if(p_paintdc->RectVisible(rect)) {
			pdc->MoveTo(rect.left, rect.bottom - 1);
			pdc->LineTo(rect.right, rect.bottom - 1);
		}
	}

	// 縦線
	rect.top = m_col_header_height;
	rect.bottom = (min(show_row, m_grid_data->Get_RowCnt() - scr_pt.y)) * m_row_height + rect.top;
	rect.right = m_row_header_width;
	if(IsFixColMode()) {
		for(x = m_fix_start_col; x < m_fix_end_col; x++) {
			rect.left = rect.right;
			rect.right = rect.left + GetDispColWidth(x);
			if(p_paintdc->RectVisible(rect)) {
				if(x == m_fix_end_col - 1) pdc->SelectObject(pOldPen);
				pdc->MoveTo(rect.right - 1, rect.top);
				pdc->LineTo(rect.right - 1, rect.bottom);
				if(x == m_fix_end_col - 1) pdc->SelectObject(&pen);
			}
		}
	}
	for(x = scr_pt.x; x < scr_pt.x + show_col; x++) {
		rect.left = rect.right;
		rect.right = rect.left + GetDispColWidth(x);
		if(p_paintdc->RectVisible(rect)) {
			pdc->MoveTo(rect.right - 1, rect.top);
			pdc->LineTo(rect.right - 1, rect.bottom);
		}
	}

	pdc->SelectObject(pOldPen);
}

void CGridCtrl::OnPaint() 
{
	CPaintDC dc(this);

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

	PaintMain(&memDC, &dc);

	dc.SetBkMode(TRANSPARENT);
	dc.BitBlt(0, 0, rcClient.right, rcClient.bottom, &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(pOldBmp);

	memDC.DeleteDC();
	cBmp.DeleteObject();
#else
	PaintMain(&dc, &dc);
#endif
}

void CGridCtrl::PaintMain(CDC *pdc, CDC *p_paintdc)
{
	CFont		*pOldFont;
	CRect		rect, winrect;
	CPoint		scr_pt;
	COLORREF	bk_cr;
	int			show_row, show_col;

	GetDispRect(winrect);
	scr_pt.x = GetScrollPos(SB_HORZ);
	scr_pt.y = GetScrollPos(SB_VERT);

	show_col = min(GetShowCol() + 1, m_grid_data->Get_ColCnt() - scr_pt.x);
	show_row = min(GetShowRow() + 1, m_grid_data->Get_RowCnt() - scr_pt.y);

	pOldFont = pdc->SelectObject(&m_font);

	bk_cr = pdc->GetBkColor();

	// 左上の空白
	if(m_gridStyle & GRS_COL_HEADER && m_gridStyle & GRS_ROW_HEADER) {
		rect.top = 0;
		rect.left = 0;
		rect.right = m_row_header_width;
		rect.bottom = m_col_header_height;
		if(p_paintdc->RectVisible(rect)) {
			if(m_gridStyle & GRS_DONT_CHANGE_TOPLEFT_CELL_COLOR) {
				pdc->FillSolidRect(rect, m_grid_data->GetDispData()->GetTopLeftCellColor());
			} else {
				if(GetFocus() == this || 
					(IsEnterEdit() && GetFocus() == m_edit_cell) || 
					(IsSplitterMode() && m_grid_data->IsWindowActive())) {
					pdc->FillSolidRect(rect, m_grid_data->GetDispData()->GetTopLeftCellColor());
				} else {
					pdc->FillSolidRect(rect, GetColor(GRID_HEADER_BG_COLOR));
				}
			}
		}
	}
	
	// 項目名
	PaintColHeader(pdc, p_paintdc, show_col, scr_pt, winrect);

	// 行番号
	PaintRowNumber(pdc, p_paintdc, show_row, scr_pt, winrect);

	pdc->SetBkColor(bk_cr);

	// データ
	PaintData(pdc, p_paintdc, show_row, show_col, scr_pt);

	pdc->SelectObject(pOldFont);

	// 枠
	PaintGrid(pdc, p_paintdc, show_row, show_col, scr_pt, winrect);

	// 選択枠
	if(!(m_gridStyle & GRS_LINE_SELECT)) {
		if(IsFixColMode() ||
			(m_grid_data->get_cur_col() >= scr_pt.x && m_grid_data->get_cur_col() <= scr_pt.x + show_col && 
			m_grid_data->get_cur_row() >= scr_pt.y && m_grid_data->get_cur_row() <= scr_pt.y + show_row)) {

			GetCellRect(m_grid_data->get_cur_cell(), &rect);

			if(p_paintdc->RectVisible(rect)) {
				CPen	*pOldPen;
				CPen	pen;
				pen.CreatePen(PS_SOLID, 0, GetColor(GRID_SELECTED_CELL_LINE_COLOR));

				pOldPen = pdc->SelectObject(&pen);

				pdc->MoveTo(rect.left, rect.top);
				pdc->LineTo(rect.right, rect.top);
				pdc->LineTo(rect.right, rect.bottom);
				pdc->LineTo(rect.left, rect.bottom);
				pdc->LineTo(rect.left, rect.top);

				pdc->SelectObject(pOldPen);
			}
		}
	}
}

BOOL CGridCtrl::SetFont(CFont *font)
{
	LOGFONT lf;
	BOOL	b;

	font->GetLogFont(&lf);

	m_font.DeleteObject();
	b = m_font.CreateFontIndirect(&lf);

	PostSetFont();

	return b;
}

BOOL CGridCtrl::CreateEditData()
{
	if(m_edit_data) return TRUE;

	m_edit_data = new CEditData;
	m_edit_cell = new CEditCtrl;

	m_edit_data->set_undo_cnt(INT_MAX);

	m_edit_cell->SetEditData(m_edit_data);
	m_edit_cell->Create(NULL, NULL,
		WS_CHILD,
		CRect(0, 0, 100, 10),
		this, EDIT_CELL_ID);
	m_edit_cell->SetExStyle2(ECS_GRID_EDITOR | ECS_NO_COMMENT_CHECK);

	// サブクラス化
	// 古いウィンドウプロシージャを保存する
	HWND hwnd = m_edit_cell->GetSafeHwnd();
	::SetWindowLongPtr (hwnd, GWLP_USERDATA, GetWindowLongPtr(hwnd, GWLP_WNDPROC));
	// ウィンドウプロシージャを切り替える
	::SetWindowLongPtr (hwnd, GWLP_WNDPROC, (LONG_PTR)Edit_SubclassWndProc);

	return TRUE;
}

void CGridCtrl::FreeEditData()
{
	delete m_edit_cell;
	delete m_edit_data;
}

void CGridCtrl::RegisterWndClass()
{
    WNDCLASS wndcls;
    HINSTANCE hInst = AfxGetInstanceHandle();

    if (::GetClassInfo(hInst, GRIDCTRL_CLASSNAME, &wndcls)) return;

    // otherwise we need to register a new class
	wndcls.style            = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW |
		CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
    wndcls.lpfnWndProc      = ::DefWindowProc;
    wndcls.cbClsExtra       = wndcls.cbWndExtra = 0;
    wndcls.hInstance        = hInst;
    wndcls.hIcon            = NULL;
    wndcls.hCursor          = NULL;
    wndcls.hbrBackground    = (HBRUSH) COLOR_WINDOW;
    wndcls.lpszMenuName     = NULL;
    wndcls.lpszClassName    = GRIDCTRL_CLASSNAME;

    if (!AfxRegisterClass(&wndcls)) {
        AfxThrowResourceException();
    }
}

int CGridCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CScrollWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_grid_data->RegistNotifyWnd(GetSafeHwnd());
	
	if(m_grid_data->IsEditable()) CreateEditData();

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
	m_font.CreateFontIndirect(&lf);

	PostSetFont();

	return 0;
}

int CGridCtrl::GetShowCol()
{
	int		width, col, result;
	CRect	winrect;

	GetDispRect(winrect);
	width = GetScrollLeftMargin();
	col = GetScrollPos(SB_HORZ);

	for(result = 0; width < winrect.Width() && col < m_grid_data->Get_ColCnt(); col++, result++) {
		width = width + GetDispColWidth(col);
	}
	if(width > winrect.Width()) result--;

	return result;
}

void CGridCtrl::CalcShowRow()
{
	// Windowサイズ変更，フォント変更時に再計算
	CRect	winrect;
	GetDispRect(winrect);
	m_show_row = ((winrect.Height() - m_col_header_height) / m_row_height);
}

int CGridCtrl::GetShowRow()
{
	return m_show_row;
}

int CGridCtrl::GetScrSize(int old_col, int new_col)
{
	int result = 0;
	int i;

	if(old_col > new_col) {
		for(i = new_col; i < old_col; i++) {
			result = result + GetDispColWidth(i);
		}
	} else {
		for(i = old_col; i < new_col; i++) {
			result = result - GetDispColWidth(i);
		}
	}

	return result;
}

void CGridCtrl::SetHeaderSize()
{
	if(m_gridStyle & GRS_COL_HEADER) {
		m_col_header_height = m_row_height * m_col_header_row;
	} else {
		m_col_header_height = 0;
	}
	
	if(m_gridStyle & GRS_ROW_HEADER) {
		if(m_gridStyle & GRS_SWAP_ROW_COL_MODE && IsWindow(GetSafeHwnd())) {
			int max_w = 0;
			CFontWidthHandler dchandler(this, &m_font);
			for(int c = 0; c < m_grid_data->Get_RowCnt(); c++) {
				int col_name_width = m_grid_data->GetColWidth(
					m_grid_data->GetRowHeader(c), INT_MAX, NULL, &dchandler, NULL);

				if(max_w < col_name_width) max_w = col_name_width;
			}
			m_row_header_width = max_w + 5;
		} else {
			m_row_header_width = m_num_width * m_grid_data->GetRowHeaderLen() + m_cell_padding_left + m_cell_padding_right + 1;
		}
	} else {
		m_row_header_width = 0;
	}
}

void CGridCtrl::SetCharSpace(int space)
{
	m_grid_data->GetDispData()->SetCharSpaceSetting(space);
}

void CGridCtrl::PostSetFont()
{
	TEXTMETRIC tm;
	CDC *pdc = GetDC();
	CFont *pOldFont = pdc->SelectObject(&m_font);

	pdc->GetTextMetrics(&tm);

	m_grid_data->GetDispData()->InitFontWidth(pdc);
	m_num_width = GetFontWidth(pdc, '0');

	pdc->SelectObject(pOldFont);
	ReleaseDC(pdc);

	m_font_height = tm.tmHeight + tm.tmExternalLeading;
	m_font_width = tm.tmAveCharWidth + m_grid_data->GetDispData()->GetCharSpace(m_num_width);

	m_row_height = tm.tmHeight + m_cell_padding_top + m_cell_padding_bottom + 1;

	SetHeaderSize();

	if(m_edit_cell) m_edit_cell->SetFont(&m_font);

	HIMC hIMC = ImmGetContext(this->GetSafeHwnd());
	if(hIMC != NULL) {
		// Set font for the conversion window
		LOGFONT lf;
		m_font.GetLogFont(&lf);
		ImmSetCompositionFont(hIMC, &lf);
		ImmReleaseContext(this->GetSafeHwnd(), hIMC);	
	}

	m_grid_data->GetDispData()->GetToolTip()->SetFont(&m_font);

	Update();
}

void CGridCtrl::SetGridData(CGridData *griddata)
{
	if(griddata == NULL) griddata = &m_null_grid_data;

	m_grid_data->UnRegistNotifyWnd(GetSafeHwnd());
	
	m_grid_data = griddata;
	m_grid_data->RegistNotifyWnd(GetSafeHwnd());
	InitData();

	if(m_grid_data->Get_RowCnt() != 0 && m_grid_data->Get_ColCnt() != 0) {
		SetCell(0, 0);
	}

	if(::IsWindow(m_hWnd)) {
		if(m_grid_data->IsEditable()) CreateEditData();
		Update();
	}
}

void CGridCtrl::SetGridStyle(DWORD dwGridStyle, BOOL b_update_window)
{
	m_gridStyle = dwGridStyle;

	if(b_update_window) Update();

	if(m_edit_cell && IsWindow(m_edit_cell->GetSafeHwnd())) {
		QWORD		style = ECS_GRID_EDITOR | ECS_NO_COMMENT_CHECK;
		if(m_gridStyle & GRS_SHOW_SPACE) {
			style |= ECS_SHOW_SPACE;
		}
		if(m_gridStyle & GRS_SHOW_TAB) {
			style |= ECS_SHOW_TAB;
		}
		if(m_gridStyle & GRS_SHOW_2BYTE_SPACE) {
			style |= ECS_SHOW_2BYTE_SPACE;
		}
		if(m_gridStyle & GRS_SHOW_LINE_FEED) {
			style |= ECS_SHOW_LINE_FEED;
		}
		if(!(m_gridStyle & GRS_INVERT_SELECT_TEXT)) {
			style |= ECS_NO_INVERT_SELECT_TEXT;
		}
		if(m_gridStyle & GRS_IME_CARET_COLOR) {
			style |= ECS_IME_CARET_COLOR;
		}
		if(m_gridStyle & GRS_ON_DIALOG) {
			style |= ECS_ON_DIALOG;
		}
		m_edit_cell->SetExStyle2(style);
	}
}

void CGridCtrl::Update_AllWnd()
{
	if(IsSplitterMode()) {
		m_grid_data->SendNotifyMessage(GetSafeHwnd(), GC_WM_UPDATE, 0, 0); 
	}

	Update();
}

void CGridCtrl::Update()
{
	if(!IsWindow(GetSafeHwnd())) return;

	SetHeaderSize();
	CheckScrollBar();

	CalcShowRow();

	if(m_grid_data->GetDispData()->GetToolTip()->IsShow()) {
		m_grid_data->GetDispData()->GetToolTip()->Hide();
	}
	LeaveEdit();
	SetEditCellPos();

	Invalidate();

	UpdateWindow();
}

void CGridCtrl::InitSelectedCell()
{
	m_grid_data->init_cur_cell();

	m_drag_header.drag_flg = NO_DRAG_HEADER;

	m_grid_data->GetSelectArea()->pos1.y = -1;
	m_grid_data->GetSelectArea()->drag_flg = NO_DRAG;
	m_grid_data->GetSelectArea()->select_mode = SELECT_MODE_NORMAL;
}

int CGridCtrl::HitColSeparator(CPoint point)
{
	int col = -1;
	int x = m_row_header_width;

	if(point.y < m_col_header_height) {
		if(IsFixColMode()) {
			for(col = m_fix_start_col; col < m_fix_end_col; col++) {
				x += GetDispColWidth(col);
				if(point.x > x - 3 && point.x < x + 3) break;
			}
			if(col == m_fix_end_col - 1 && GetScrollPos(SB_HORZ) == GetHScrollMin() &&
				GetDispColWidth(m_fix_end_col) == 0 && point.x > x) col++;
		}

		if(!IsFixColMode() || col == m_fix_end_col) {
			int pos = GetScrollPos(SB_HORZ);
			int show_col = GetShowCol();

			for(; pos > 0 && GetDispColWidth(pos - 1) == 0; ) pos--;

			for(col = pos; col < pos + show_col; col++) {
				x += GetDispColWidth(col);
				if(point.x > x - 3 && point.x < x + 3) break;
			}

			if(col == pos + show_col) col = -1;
		}
	}

	if(col != -1) {
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));

		if(col >= m_fix_end_col && point.x > x) {
			for(;col < m_grid_data->Get_ColCnt() - 1; col++) {
				x += GetDispColWidth(col + 1);
				if(point.x < x - 3) break;
			}
		}

		return col;
	}

	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));

	return -1;
}

void CGridCtrl::HitCell(CPoint point, POINT *pt, BOOL out_of_window)
{
	pt->x = pt->y = -1;

	// col
	if(point.y >= m_col_header_height || out_of_window == TRUE) {
		int pos = GetScrollPos(SB_HORZ);
		int show_col = GetShowCol();
		int x = m_row_header_width;
		if(point.x >= m_row_header_width) {
			for(pt->x = m_fix_start_col; pt->x < m_fix_end_col; (pt->x)++) {
				if(point.x >= x && point.x < x + GetDispColWidth(pt->x)) break;
				x = x + GetDispColWidth(pt->x);
			}
			if(pt->x == m_fix_end_col) {
				for(pt->x = pos; pt->x < pos + show_col; (pt->x)++) {
					if(point.x >= x && point.x < x + GetDispColWidth(pt->x)) break;
					x = x + GetDispColWidth(pt->x);
				}
			}
		} else {
			for(pt->x = pos - 1; pt->x >= 0; (pt->x)--) {
				if(point.x - m_row_header_width <= x && 
					point.x - m_row_header_width > x - GetDispColWidth(pt->x)) break;
				x = x - GetDispColWidth(pt->x);
			}
		}
	}

	// row
	if(point.x >= m_row_header_width || out_of_window == TRUE) {
		pt->y = GetScrollPos(SB_VERT) + (long)floor((double)((double)point.y - m_col_header_height) / m_row_height);
	}
}

BOOL CGridCtrl::HitAllSelectArea(CPoint point)
{
	if(point.y < 0 || point.y > m_col_header_height) return FALSE;
	if(point.x < 0 || point.x > m_row_header_width) return FALSE;

	return TRUE;
}

int CGridCtrl::HitRowHeader(CPoint point)
{
	if(point.y < m_col_header_height) return -1;

	if(point.x < m_row_header_width) {
		int row = GetScrollPos(SB_VERT) + (point.y - m_col_header_height) / m_row_height;
		if(row >= m_grid_data->Get_RowCnt()) return -1;
		return row;
	}

	return -1;
}

int CGridCtrl::HitColHeader(CPoint point)
{
	if(point.x < m_row_header_width) return -1;
	if(point.y < m_col_header_height) {
		int col;
		int x = m_row_header_width;

		for(col = m_fix_start_col; col < m_fix_end_col; col++) {
			if(x < point.x && x + GetDispColWidth(col) > point.x) return col;
			x = x + GetDispColWidth(col);
		}

		int pos = GetScrollPos(SB_HORZ);
		int show_col = min(GetShowCol() + 1, m_grid_data->Get_ColCnt() - GetScrollPos(SB_HORZ));

		for(col = pos; col < pos + show_col; col++) {
			if(x < point.x && x + GetDispColWidth(col) > point.x) return col;
			x = x + GetDispColWidth(col);
		}
	}

	return -1;
}

void CGridCtrl::ShowToolTip()
{
	int col = HitColHeader(m_tool_tip_pt);
	if(col == -1) return;

	// ウィンドウ分割を解除したとき、フォントの設定が初期化される問題を回避
	m_grid_data->GetDispData()->GetToolTip()->SetFont(&m_font);

	m_grid_data->GetDispData()->GetToolTip()->SetMessage(m_grid_data->Get_DispColName(col));
	CRect rect;
	GetCellWidth(col, &rect);
	rect.top = 0;
	rect.bottom = m_col_header_height;
	ClientToScreen(&rect);
	rect.top -= 1;

	m_grid_data->GetDispData()->GetToolTip()->Show(GetSafeHwnd(), &rect);

	m_last_tool_tip_col = col;
}

void CGridCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	switch(m_drag_header.drag_flg) {
	case NO_DRAG_HEADER:
		HitColSeparator(point);
		break;
	case PRE_DRAG_HEADER:
		StartDragHeader(point);
		break;
	case DO_DRAG_HEADER:
		DrawDragRect(point, FALSE);
		break;
	}

	switch(m_grid_data->GetSelectArea()->drag_flg) {
	case PRE_DRAG:
		StartDragSelected();
		break;
	case DO_DRAG:
		DoDragSelected(point);
		break;
	}

	if((m_gridStyle & GRS_COLUMN_NAME_TOOL_TIP) &&
		m_drag_header.drag_flg == NO_DRAG_HEADER && 
		m_grid_data->GetSelectArea()->drag_flg == NO_DRAG &&
		HitColSeparator(point) == -1) {

		int col = HitColHeader(point);
		if(col != -1) {
			if(col != m_last_tool_tip_col) {
				KillTimer(IdToolTipTimer);
				m_tool_tip_pt = point;
				SetTimer(IdToolTipTimer, 200, NULL);

				m_last_tool_tip_col = -1;
			}
		} else {
			m_last_tool_tip_col = -1;
		}
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CGridCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if(IsEnterEdit()) {
		if(LeaveEdit() == FALSE) return;
	}

	if(GetFocus() != this) SetFocus();
	
	int	col;
	int row;

	// 列ヘッダの境界
	col = HitColSeparator(point);
	if(col != -1) {
		PreDragHeader(point, col);
		return;
	}

	// 行ヘッダ
	row = HitRowHeader(point);
	if(row != -1) {
		SelectRow(row);
		PreDragSelected(SELECT_MODE_ROW);
		return;
	}

	// 列ヘッダ
	col = HitColHeader(point);
	if(col != -1) {
		SelectCol(col);
		PreDragSelected(SELECT_MODE_COL);
		return;
	}

	// 全選択
	if(HitAllSelectArea(point)) {
		SelectAll();
		return;
	}

	POINT	pt;
	HitCell(point, &pt);
	if(pt.x == -1 || pt.y == -1) return;

	m_grid_data->GetSelectArea()->select_mode = SELECT_MODE_NORMAL;
	SelChanged(&pt);

	// ドラッグ選択の準備
	PreDragSelected(SELECT_MODE_NORMAL);

	CWnd::OnLButtonDown(nFlags, point);
}

void CGridCtrl::EqualAllColWidth()
{
	CWaitCursor		wait_cursor;

	m_grid_data->SetDefaultColWidth();

	Update_AllWnd();
}

int CGridCtrl::AdjustAllColWidth(BOOL use_col_name, BOOL force_window_width, 
	BOOL update_window, volatile int *cancel_flg)
{
	CWaitCursor		wait_cursor;
	int		i;
	int		ret_v;

	if(!use_col_name && m_grid_data->Get_RowCnt() == 0) use_col_name = TRUE;

	for(i = 0; i < m_grid_data->Get_ColCnt(); i++) {
		ret_v = AdjustColWidth(i, use_col_name, force_window_width, cancel_flg);
		if(ret_v != 0) return ret_v;
	}

	if(update_window) {
		Update_AllWnd();
	}

	return 0;
}

int CGridCtrl::GetMaxColWidth()
{
	CRect winrect;
	GetDispRect(&winrect);

	int win_width = winrect.Width() - m_row_header_width;

	return win_width;
}

int CGridCtrl::AdjustColWidth(int col, BOOL use_col_name, BOOL force_window_width,
	volatile int *cancel_flg)
{
	if(!m_grid_data->GetDispFlg(col)) {
		SetDispColWidth(col, 0);
		return 0;
	}

	CRect winrect;
	GetDispRect(&winrect);

	int row_cnt = m_grid_data->Get_RowCnt();

	int win_width = winrect.Width() - m_row_header_width;
	if(force_window_width) win_width = INT_MAX;

	int max_width;
	CFontWidthHandler dchandler(this, &m_font);

	if((m_gridStyle & GRS_ADJUST_COL_WIDTH_TRIM_CHAR_DATA) &&
		m_grid_data->Get_ColDataType(0, col) == GRID_DATA_CHAR_TYPE) {
		max_width = m_grid_data->GetMaxColWidthNoLastSpace(col, win_width, &dchandler, cancel_flg);
	} else {
		max_width = m_grid_data->GetMaxColWidth(col, win_width, &dchandler, cancel_flg);
	}
	if(cancel_flg && *cancel_flg) return GRID_CTRL_ADJUST_COL_WIDTH_CANCEL;

	// m_adjust_min_col_charsより小さくならないようにする
	if(m_adjust_min_col_chars * m_font_width > max_width) max_width= m_adjust_min_col_chars * m_font_width;

	// カラム名より小さくならないようにする
	if(use_col_name) {
		int col_name_width = m_grid_data->GetColWidth(
			GetDispColNameWithNotNullFlg(col), win_width, NULL, &dchandler, NULL);
		if(col_name_width > max_width) max_width = col_name_width;
	}

	SetDispColWidth(col, max_width + m_cell_padding_left + m_cell_padding_right);

	// ウィンドウより幅広にならないようにする
	if(!force_window_width && GetDispColWidth(col) > winrect.Width() - m_row_header_width - 10) {
		SetDispColWidth(col, winrect.Width() - m_row_header_width - 10);
	}

	return 0;
}

void CGridCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	int	col;

	// 列ヘッダの境界
	col = HitColSeparator(point);
	if(col != -1) {
		BOOL use_colname = !(m_gridStyle & GRS_ADJUST_COL_WIDTH_NO_USE_COL_NAME);

		if(HaveSelectedCol(col)) {
			// 選択範囲のセルの幅を調整
			int start_x = min(m_grid_data->GetSelectArea()->pos1.x, m_grid_data->GetSelectArea()->pos2.x);
			int end_x = max(m_grid_data->GetSelectArea()->pos1.x, m_grid_data->GetSelectArea()->pos2.x);
			for(col = start_x; col <= end_x; col++) AdjustColWidth(col, use_colname, FALSE);
		} else {
			AdjustColWidth(col, use_colname, FALSE);
		}
		Update_AllWnd();
		return;
	}

	POINT	pt;
	HitCell(point, &pt);
	if(pt.x >= 0 && pt.y >= 0) {
		// ダブルクリックでセルを編集モードにする
		if(EnterEdit(TRUE)) {
			// マウスダブルクリックした文字の位置にカーソルを移動する
			INPUT input[2];
			ZeroMemory(input, sizeof(input));
			if(GetSystemMetrics(SM_SWAPBUTTON)) {
				// マウスの設定で「主と副のボタンを切り替える」が有効な場合の対応
				input[0].type = INPUT_MOUSE;
				input[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
				input[0].mi.dwExtraInfo = GetMessageExtraInfo();
				input[1].type = INPUT_MOUSE;
				input[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;
				input[1].mi.dwExtraInfo = GetMessageExtraInfo();
			} else {
				input[0].type = INPUT_MOUSE;
				input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
				input[0].mi.dwExtraInfo = GetMessageExtraInfo();
				input[1].type = INPUT_MOUSE;
				input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
				input[1].mi.dwExtraInfo = GetMessageExtraInfo();
			}
			SendInput(2, input, sizeof(INPUT));
			//mouse_event(MOUSEEVENTF_LEFTDOWN, point.x, point.y, 0, 0);
			//mouse_event(MOUSEEVENTF_LEFTUP, point.x, point.y, 0, 0);
		}
	}

	if(!(m_gridStyle & GRS_SWAP_ROW_COL_MODE) && HitColHeader(point) != -1) {
		if(HaveSelected()) {
			int start_x = min(m_grid_data->GetSelectArea()->pos1.x, m_grid_data->GetSelectArea()->pos2.x);
			int end_x = max(m_grid_data->GetSelectArea()->pos1.x, m_grid_data->GetSelectArea()->pos2.x);
			GetParent()->SendMessage(GC_WM_DBL_CLK_COL_HEADER, start_x, end_x);
		} else {
			int x = HitColHeader(point);
			GetParent()->SendMessage(GC_WM_DBL_CLK_COL_HEADER, x, x);
		}
	}
	if((m_gridStyle & GRS_SWAP_ROW_COL_MODE) && HitRowHeader(point) != -1) {
		if(HaveSelected()) {
			int start_x = min(m_grid_data->GetSelectArea()->pos1.y, m_grid_data->GetSelectArea()->pos2.y);
			int end_x = max(m_grid_data->GetSelectArea()->pos1.y, m_grid_data->GetSelectArea()->pos2.y);
			GetParent()->SendMessage(GC_WM_DBL_CLK_COL_HEADER, start_x, end_x);
		} else {
			int x = HitRowHeader(point);
			GetParent()->SendMessage(GC_WM_DBL_CLK_COL_HEADER, x, x);
		}
	}

	CWnd::OnLButtonDblClk(nFlags, point);
}

void CGridCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if(m_drag_header.drag_flg != NO_DRAG_HEADER) {
		EndDragHeader(point);
	}
	if(m_grid_data->GetSelectArea()->drag_flg != NO_DRAG) {
		EndDragSelected();
	}

	HitColSeparator(point);
	
	CWnd::OnLButtonUp(nFlags, point);
}

int CGridCtrl::DrawDragRect(CPoint point, BOOL first)
{
	CRect	winrect, rect;
	SIZE	size;

	if(m_droprect.left == point.x) return 0;

	GetDispRect(winrect);
	rect.top    = GR_DEF_DRAGRECT_SIZE;
	rect.bottom = winrect.Height() - GR_DEF_DRAGRECT_SIZE;
	rect.left   = point.x;
	rect.right  = rect.left + GR_DEF_DRAGRECT_SIZE;

	size.cx = GR_DEF_DRAGRECT_SIZE;
	size.cy = GR_DEF_DRAGRECT_SIZE;

	CDC *pDC = GetDC();
	if(first == TRUE) {
		pDC->DrawDragRect(rect, size, NULL, size);
	} else {
		pDC->DrawDragRect(rect, size, &m_droprect, size);
	}
	ReleaseDC(pDC);
	m_droprect = rect;

	int old_colsize = GetDispColWidth(m_drag_header.col);
	SetDispColWidth(m_drag_header.col,
		GetDispColWidth(m_drag_header.col) - (m_drag_header.pt.x - point.x));
	if(GetDispColWidth(m_drag_header.col) < m_min_colwidth) {
		SetDispColWidth(m_drag_header.col, m_min_colwidth);
	}
	InvalidateRect(CRect(0, 0, winrect.Width(), m_row_height));

	SetDispColWidth(m_drag_header.col, old_colsize);

	return 0;
}

void CGridCtrl::SelChanged(POINT *pt, BOOL b_area_select, BOOL b_no_clear_selected_area)
{
	LeaveEdit();

	// 非表示のセルに移動しないようにする
	if(m_grid_data->GetDispFlg(pt->x) == FALSE) {
		int col = pt->x;
		if(col < m_grid_data->get_cur_col()) {
			for(; col > 0 && m_grid_data->GetDispFlg(col) == FALSE; col--) ;
		} else {
			for(; col < m_grid_data->Get_ColCnt() && m_grid_data->GetDispFlg(col) == FALSE; col++) ;
		}

		if(m_grid_data->GetDispFlg(col)) {
			pt->x = col;
		} else {
			pt->x = m_grid_data->get_cur_col();
		}
	}

	if(pt->x < 0) pt->x = 0;
	if(pt->y < 0) pt->y = 0;
	if(pt->x >= m_grid_data->Get_ColCnt()) pt->x = m_grid_data->Get_ColCnt() - 1;
	if(pt->y >= m_grid_data->Get_RowCnt()) pt->y = m_grid_data->Get_RowCnt() - 1;

	BOOL	shift = FALSE;
	if(GetAsyncKeyState(VK_SHIFT) < 0) shift = TRUE;

	if(b_area_select && (shift == TRUE || m_grid_data->GetSelectArea()->drag_flg == DO_DRAG)) {
		if(HaveSelected() == FALSE) {
			m_grid_data->GetSelectArea()->select_mode = SELECT_MODE_NORMAL;
			CPoint pt = *m_grid_data->get_cur_cell();
			StartSelect(pt);
		}
	} else {
		if(!b_no_clear_selected_area) ClearSelected();
	}

	if(m_grid_data->get_cur_col() == pt->x && m_grid_data->get_cur_row() == pt->y) return;

	ClearIME();

	POINT	scr_pt, show;

	scr_pt.x = GetScrollPos(SB_HORZ);
	scr_pt.y = GetScrollPos(SB_VERT);

	show.x = GetShowCol();
	show.y = GetShowRow();

	// 行番号を再描画
	if(((m_gridStyle & GRS_SHOW_CUR_ROW) || (m_gridStyle & GRS_HIGHLIGHT_HEADER)) &&
			m_grid_data->get_cur_row() != pt->y) {
		InvalidateRowHeader_AllWnd(m_grid_data->get_cur_row());
		InvalidateRowHeader_AllWnd(pt->y);
	}

	// カラム名を再描画
	if((m_gridStyle & GRS_HIGHLIGHT_HEADER) && m_grid_data->get_cur_col() != pt->x) {
		InvalidateCellHeader_AllWnd(m_grid_data->get_cur_col());
		InvalidateCellHeader_AllWnd(pt->x);
	}

	if(m_change_active_row_text_color) {
		if(m_gridStyle & GRS_SWAP_ROW_COL_MODE) {
			// FIXME: InvalidateCol, InvalidateCol_AllWndを実装する？
			Invalidate_AllWnd();
		} else {
			// 行全体を再描画する
			InvalidateRow_AllWnd(m_grid_data->get_cur_row());
			InvalidateRow_AllWnd(pt->y);
		}
	}

	// 移動前のセル
	if(m_grid_data->get_cur_col() >= scr_pt.x && m_grid_data->get_cur_col() <= scr_pt.x + show.x && 
		m_grid_data->get_cur_row() >= scr_pt.y && m_grid_data->get_cur_row() <= scr_pt.y + show.y) {
		InvalidateCell_AllWnd(m_grid_data->get_cur_cell());
	} else if(IsSplitterMode()) {
		InvalidateCell_AllWnd(m_grid_data->get_cur_cell());
	}

	// 移動後のセル
	if(pt->x >= scr_pt.x && pt->x < scr_pt.x + show.x && 
		pt->y >= scr_pt.y && pt->y < scr_pt.y + show.y) {
		InvalidateCell_AllWnd(pt);
	} else {
		ShowCell(pt);
		InvalidateCell_AllWnd(pt);
	}

	if(pt->y < 0 || pt->x < 0) {
		m_grid_data->clear_selected_cell();
		m_grid_data->set_cur_cell(pt->y, pt->x);
	} else {
		m_grid_data->set_cur_cell(pt->y, pt->x);
	}

	if(b_area_select && (shift == TRUE || m_grid_data->GetSelectArea()->drag_flg == DO_DRAG)) {
		SelectCell(*pt);
	}

	SetEditCellPos();

	// セルの位置を親ウィンドウに通知する
	GetParent()->SendMessage(GC_WM_CHANGE_CELL_POS, pt->x, pt->y);
}

void CGridCtrl::SetEditCellPos()
{
	RECT	rect;
	POINT	pt = *m_grid_data->get_cur_cell();

	GetCellRect(&pt, &rect);

	rect.top += m_cell_padding_top;
	rect.bottom -= m_cell_padding_bottom;
	rect.left += m_cell_padding_left;
	rect.right -= m_cell_padding_right;

	if(m_cell_padding_bottom > 0) {
		rect.bottom += 1;
	}
	if(m_cell_padding_right > 0) {
		rect.right += 1;
	}

	if(IsEnterEdit()) m_edit_cell->MoveWindow(&rect);
	SetConversionWindow(rect.left + 1, rect.top + 1);
}

void CGridCtrl::MoveCell(int x, int y)
{
	POINT	pt;
	pt.x = m_grid_data->get_cur_col() + x;
	pt.y = m_grid_data->get_cur_row() + y;

	SelChanged(&pt);
}

void CGridCtrl::DocumentStart()
{
	POINT pt;
	pt.x = 0;
	pt.y = 0;
	SelChanged(&pt);
}

void CGridCtrl::DocumentEnd()
{
	POINT pt;
	pt.x = m_grid_data->Get_ColCnt() - 1;
	pt.y = m_grid_data->Get_RowCnt() - 1;
	if(m_gridStyle & GRS_LINE_SELECT) pt.x = 0;
	SelChanged(&pt);
}

void CGridCtrl::LineStart()
{
	POINT pt;
	pt.x = 0;
	pt.y = m_grid_data->get_cur_row();
	SelChanged(&pt);
}

void CGridCtrl::LineEnd()
{
	POINT pt;
	pt.x = m_grid_data->Get_ColCnt() - 1;
	pt.y = m_grid_data->get_cur_row();
	SelChanged(&pt);
}

void CGridCtrl::ColumnStart()
{
	POINT pt;
	pt.x = m_grid_data->get_cur_col();
	pt.y = 0;
	SelChanged(&pt);
}

void CGridCtrl::ColumnEnd()
{
	POINT pt;
	pt.x = m_grid_data->get_cur_col();
	pt.y = m_grid_data->Get_RowCnt() - 1;
	SelChanged(&pt);
}

void CGridCtrl::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	BOOL	menu = FALSE;
	if(GetKeyState(VK_MENU) < 0) menu = TRUE;

	switch(nChar) {
	case VK_HOME:
		ColumnStart();
		break;
	case VK_END:
		ColumnEnd();
		break;
	}
	
	CScrollWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);
}

void CGridCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	BOOL	ctrl = FALSE;
	BOOL	shift = FALSE;
	if(GetKeyState(VK_CONTROL) < 0) ctrl = TRUE;
	if(GetKeyState(VK_SHIFT) < 0) shift = TRUE;

	switch(nChar) {
	case VK_ESCAPE:
		ClearSelected();
		break;
	case VK_TAB:
		if(shift) {
			MoveCell(-1, 0);
			ClearSelected();
		} else {
			MoveCell(1, 0);
		}
		break;
	case VK_RETURN:
		if(shift) {
			MoveCell(0, -1);
			ClearSelected();
		} else {
			MoveCell(0, 1);
		}
		break;
	case VK_UP:
		if(ctrl) {
			ScrollUp();
		} else {
			if(m_end_key_flg) {
				ColumnStart();
			} else {
				MoveCell(0, -1);
			}
		}
		break;
	case VK_DOWN:
		if(ctrl) {
			ScrollDown();
		} else {
			if(m_end_key_flg) {
				ColumnEnd();
			} else {
				MoveCell(0, 1);
			}
		}
		break;
	case VK_LEFT:
		if(ctrl) {
			ScrollLeft();
		} else {
			if(m_end_key_flg) {
				LineStart();
			} else {
				MoveCell(-1, 0);
			}
		}
		break;
	case VK_RIGHT:
		if(ctrl) {
			ScrollRight();
		} else {
			if(m_end_key_flg) {
				LineEnd();
			} else {
				MoveCell(1, 0);
			}
		}
		break;
	case VK_PRIOR:
		if(ctrl) {
			ScrollPageUp();
		} else {
			MoveCell(0, -GetShowRow());
		}
		break;
	case VK_NEXT:
		if(ctrl) {
			ScrollPageDown();
		} else {
			MoveCell(0, GetShowRow());
		}
		break;
	case VK_HOME:
		if(ctrl) {
			DocumentStart();
		} else {
			LineStart();
		}
		break;
	case VK_END:
		if(!ctrl && m_gridStyle & GRS_END_KEY_LIKE_EXCEL) {
			m_end_key_flg = !m_end_key_flg;
		} else {
			if(ctrl) {
				DocumentEnd();
			} else {
				LineEnd();
			}
		}
		break;
	case VK_DELETE:
		DeleteKey();
		break;
	case VK_F2:
		if(m_gridStyle & GRS_ON_DIALOG || m_gridStyle & GRS_ALLOW_F2_ENTER_EDIT) {
			EnterEdit();
		}
		break;
	default:
		break;
	}

	if(m_end_key_flg && nChar != VK_END && nChar != VK_SHIFT && nChar != VK_CONTROL) {
		m_end_key_flg = FALSE;
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CGridCtrl::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	return CWnd::Create(GRIDCTRL_CLASSNAME, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

void CGridCtrl::GetCellWidth(int col, RECT *rect)
{
	int		i;

	if(col >= m_fix_end_col && col < GetScrollPos(SB_HORZ)) {
		rect->left = 0;
		rect->right = 0;
		return;
	}

	if(col < m_fix_end_col) {
		rect->left = m_row_header_width;
		for(i = m_fix_start_col; i < col; i++) {
			rect->left += GetDispColWidth(i);
		}
	} else {
		rect->left = GetScrollLeftMargin();
		for(i = GetScrollPos(SB_HORZ); i < col; i++) {
			rect->left += GetDispColWidth(i);
		}
	}
	rect->right = rect->left + GetDispColWidth(i);

	if(rect->left != 0) rect->left--;
	if(rect->left != rect->right) rect->right--;
}

void CGridCtrl::GetCellRect(POINT *pt, RECT *rect)
{
	if((pt->x >= m_fix_end_col && pt->x < GetScrollPos(SB_HORZ)) || pt->y < GetScrollPos(SB_VERT)) {
		rect->top = 0;
		rect->bottom = 0;
		rect->left = 0;
		rect->right = 0;
		return;
	}

	rect->top = m_col_header_height + (pt->y - GetScrollPos(SB_VERT)) * m_row_height;
	rect->bottom = rect->top + m_row_height - 1;

	GetCellWidth(pt->x, rect);

	if(rect->top != 0) rect->top--;
}

void CGridCtrl::InvalidateCell_AllWnd(POINT *pt)
{
	if(IsSplitterMode()) {
		m_grid_data->SendNotifyMessage(GetSafeHwnd(), GC_WM_INVALIDATE_CELL, pt->y, pt->x); 
	}
	InvalidateCell(pt);
}

void CGridCtrl::InvalidateCellHeader_AllWnd(int col)
{
	if(IsSplitterMode()) {
		m_grid_data->SendNotifyMessage(GetSafeHwnd(), GC_WM_INVALIDATE_CELL_HEADER, col, 0); 
	}
	InvalidateCellHeader(col);
}

void CGridCtrl::InvalidateCellHeader(int col)
{
	RECT	rect;

	POINT pt;
	
	pt.y = GetScrollPos(SB_VERT);
	pt.x = col;

	GetCellRect(&pt, &rect);

	rect.top = 0;
	rect.bottom = m_col_header_height;

	rect.right++;
	rect.bottom++;

	InvalidateRect(&rect);
}

void CGridCtrl::InvalidateCell(POINT *pt)
{
	// 画面の範囲外は計算しない
	if(pt->y < GetScrollPos(SB_VERT) || pt->y > GetScrollPos(SB_VERT) + GetShowRow()) return;

	RECT	rect;

	GetCellRect(pt, &rect);

	if(m_gridStyle & GRS_LINE_SELECT) {
		RECT winrect;
		GetDispRect(&winrect);
		rect.left = 0;
		rect.right = winrect.right;
	}

	rect.right++;
	rect.bottom++;
	InvalidateRect(&rect);
}

void CGridCtrl::InvalidateRowHeader_AllWnd(int row)
{
	if(IsSplitterMode()) {
		m_grid_data->SendNotifyMessage(GetSafeHwnd(), GC_WM_INVALIDATE_ROW_HEADER, row, 0); 
	}
	InvalidateRowHeader(row);
}

void CGridCtrl::InvalidateRowHeader(int row)
{
	// 画面の範囲外は計算しない
	if(row < GetScrollPos(SB_VERT) || row > GetScrollPos(SB_VERT) + GetShowRow()) return;

	RECT	rect;

	rect.top = m_col_header_height + (row - GetScrollPos(SB_VERT)) * m_row_height;
	rect.bottom = rect.top + m_row_height;

	RECT winrect;
	GetDispRect(&winrect);
	rect.left = 0;
	rect.right = m_row_header_width;

//	InflateRect(&rect, 1, 1);
	InvalidateRect(&rect);
}

void CGridCtrl::InvalidateRow_AllWnd(int row)
{
	if(IsSplitterMode()) {
		m_grid_data->SendNotifyMessage(GetSafeHwnd(), GC_WM_INVALIDATE_ROW, row, 0); 
	}
	InvalidateRow(row);
}

void CGridCtrl::InvalidateRow(int row)
{
	// 画面の範囲外は計算しない
	if(row < GetScrollPos(SB_VERT) || row > GetScrollPos(SB_VERT) + GetShowRow()) return;

	RECT	rect;

	rect.top = m_col_header_height + (row - GetScrollPos(SB_VERT)) * m_row_height;
	rect.bottom = rect.top + m_row_height;

	RECT winrect;
	GetDispRect(&winrect);
	rect.left = 0;
	rect.right = winrect.right;

	InflateRect(&rect, 1, 1);
	InvalidateRect(&rect);
}

void CGridCtrl::Invalidate_AllWnd(BOOL b_self)
{
	if(IsSplitterMode()) {
		m_grid_data->SendNotifyMessage(GetSafeHwnd(), GC_WM_INVALIDATE, 0, 0); 
	}
	if(b_self) Invalidate();
}

void CGridCtrl::InvalidateRange_AllWnd(POINT *pt1, POINT *pt2)
{
	if(IsSplitterMode()) {
		m_grid_data->SendNotifyMessage(GetSafeHwnd(), GC_WM_INVALIDATE_RANGE, 
			(WPARAM)pt1, (LPARAM)pt2); 
	}
	InvalidateRange(pt1, pt2);
}

void CGridCtrl::InvalidateRange(POINT *pt1, POINT *pt2)
{
	int start_y = max(min(pt1->y, pt2->y), GetScrollPos(SB_VERT));
	int end_y = min(max(pt1->y, pt2->y), GetScrollPos(SB_VERT) + GetShowRow());
	int start_x = max(min(pt1->x, pt2->x), GetScrollPos(SB_HORZ));
	int end_x = min(max(pt1->x, pt2->x), GetScrollPos(SB_HORZ) + GetShowCol());

	if(IsFixColMode()) start_x = m_fix_start_col;

	POINT cell_pt;

	for(int y = start_y; y <= end_y; y++) {
		cell_pt.y = y;
		for(int x = start_x; x <= end_x; x++) {
			cell_pt.x = x;
			InvalidateCell(&cell_pt);
		}
	}
}

void CGridCtrl::InvalidateTopLeft()
{
	if(!(m_gridStyle & GRS_ROW_HEADER) && !(m_gridStyle & GRS_COL_HEADER)) return;

	RECT	rect;
	rect.top = 0;
	rect.left = 0;
	rect.right = m_row_header_width;
	rect.bottom = m_col_header_height;

	InvalidateRect(&rect);
}

void CGridCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	CWnd::OnKillFocus(pNewWnd);
	
	if(IsSplitterMode()) {
		InvalidateCell(m_grid_data->get_cur_cell());
		m_grid_data->SendNotifyMessage(GetSafeHwnd(), GC_WM_INVALIDATE_TOP_LEFT, 0, 0);
	}

	InvalidateTopLeft();

	GetParent()->SendMessage(GC_WM_KILLFOCUS, (WPARAM)pNewWnd, 0);
}

void CGridCtrl::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd::OnSetFocus(pOldWnd);

	InvalidateTopLeft();

	SetEditCellPos();

	if(IsSplitterMode()) {
		InvalidateCell(m_grid_data->get_cur_cell());
		LeaveEdit();
	}

	// セルの位置を親ウィンドウに通知する
	GetParent()->SendMessage(GC_WM_CHANGE_CELL_POS, m_grid_data->get_cur_col(), m_grid_data->get_cur_row());

	if(m_edit_cell_focused && IsEnterEdit()) {
		m_edit_cell->SetFocus();
		m_edit_cell_focused = FALSE;
	}

	m_last_active_wnd = TRUE;
	if(IsSplitterMode()) {
		m_grid_data->SendNotifyMessage(GetSafeHwnd(), GC_WM_CLEAR_ACTIVE_WND_FLG, 0, 0); 
	}

	GetParent()->SendMessage(GC_WM_SETFOCUS, (WPARAM)pOldWnd, 0);
}

void CGridCtrl::NoticeChangeCellPos()
{
	GetParent()->PostMessage(GC_WM_CHANGE_CELL_POS, m_grid_data->get_cur_col(), m_grid_data->get_cur_row());
}

void CGridCtrl::NoticeUpdateGridData()
{
	GetParent()->PostMessage(GC_WM_UPDATE_GRID_DATA, m_grid_data->get_cur_col(), m_grid_data->get_cur_row());
}

void CGridCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	Update();
}

int CGridCtrl::CalcCopyDataSize(int copy_format, int y_start, int y_end, int x_start, int x_end, int copy_option)
{
	int		sepa_size;
	const TCHAR	*pdata = NULL;
	char	quote_char;

	int x, y;
	int result = 0;
	BOOL b_escape = FALSE;

	// セパレータのサイズを設定
	switch(copy_format) {
	case GR_COPY_FORMAT_TAB:
	case GR_COPY_FORMAT_TAB_CNAME:
		if(m_gridStyle & GRS_COPY_ESCAPE_DBL_QUOTE) {
			sepa_size = 3;	// tabと先頭・末尾の'"' ('"'は使わないときもある)
			b_escape = TRUE;
		} else {
			sepa_size = 1;
		}
		quote_char = '"';
		break;
	case GR_COPY_FORMAT_CSV:
	case GR_COPY_FORMAT_CSV_CNAME:
		sepa_size = 3;	// "data",にするので，'"'2つと，","の3バイトを追加
		b_escape = TRUE;
		quote_char = '"';
		break;
	case GR_COPY_FORMAT_FIX_LEN:
	case GR_COPY_FORMAT_FIX_LEN_CNAME:
		sepa_size = 0;
		break;
	case GR_COPY_FORMAT_COLUMN_NAME:
		sepa_size = 3;
		break;
	case GR_COPY_FORMAT_SQL:
		sepa_size = 3;	// 'data',にするので，"'"2つと，","の3バイトを追加
		b_escape = TRUE;
		quote_char = '\'';
		break;
	case GR_COPY_FORMAT_WHERE_CLAUSE:
		sepa_size = 10;	// 'data'\r\nにするので，"'"2つと，"\n"の3バイトを追加
						// is null\r\nの場合もあるため、10にする
		b_escape = TRUE;
		quote_char = '\'';
		break;
	case GR_COPY_FORMAT_IN_CLAUSE:
		sepa_size = 5;	// 'data',にするので，"'"2つと，","の3バイトを追加, 改行も追加する場合があるので5バイトにする
		b_escape = TRUE;
		quote_char = '\'';
		break;
	default:
		return 0;
	}

	// 固定長
	if(copy_format == GR_COPY_FORMAT_FIX_LEN || copy_format == GR_COPY_FORMAT_FIX_LEN_CNAME) {
		int		row;
		row = y_end - y_start + 1;
		if(copy_format == GR_COPY_FORMAT_FIX_LEN_CNAME) {
			if(!(m_gridStyle & GRS_SWAP_ROW_COL_MODE)) {
				row++;
			} else {
				result += m_grid_data->GetRowHeaderLen() * row;
			}
		}
		for(x = x_start; x <= x_end; x++) {
			result += m_grid_data->Get_ColMaxSize(x) * row;
		}
		// 改行コード
		result += 2 * row;
		result++; // \0

		goto RESULT;
	}

	// カラム名
	if(copy_format == GR_COPY_FORMAT_TAB_CNAME || copy_format == GR_COPY_FORMAT_CSV_CNAME ||
		copy_format == GR_COPY_FORMAT_COLUMN_NAME || copy_format == GR_COPY_FORMAT_WHERE_CLAUSE ||
		copy_format == GR_COPY_FORMAT_IN_CLAUSE) {
		if(!(m_gridStyle & GRS_SWAP_ROW_COL_MODE)) {
			for(x = x_start; x <= x_end; x++) {
				pdata = m_grid_data->Get_DispColName(x);
				result = result + (int)_tcslen(pdata) + sepa_size;
				if(b_escape) {
					// "は，""に変換するので，その分を計算
					result += ostr_str_cnt(pdata, quote_char);
				}
			}
			result = result + 2;	// 改行コード
		} else {
			for(y = y_start; y <= y_end; y++) {
				pdata = m_grid_data->GetRowHeader(y);
				result = result + (int)_tcslen(pdata) + sepa_size;
				if(b_escape) {
					// "は，""に変換するので，その分を計算
					result += ostr_str_cnt(pdata, quote_char);
				}
			}
			result = result + 2;	// 改行コード
		}
	}

	if(copy_format == GR_COPY_FORMAT_WHERE_CLAUSE) {
		int col_cnt = x_end - x_start + 1;
		int row_cnt = y_end - y_start + 1;
		if(m_gridStyle & GRS_SWAP_ROW_COL_MODE) {
			int tmp_cnt = col_cnt;
			col_cnt = row_cnt;
			row_cnt = tmp_cnt;
		}
		result += col_cnt * 12;	// where/and/=などの分
		result *= row_cnt;		// レコード数を掛ける
	}

	if(copy_format == GR_COPY_FORMAT_COLUMN_NAME) {
		result += 2;	// 改行コード
		result++;	// '\0'
		goto RESULT;
	}

	ASSERT(y_start >= 0 && x_start >= 0);
	if(y_start < 0 || x_start < 0) goto RESULT;

	// データ
	for(y = y_start; y <= y_end; y++) {
		for(x = x_start; x <= x_end; x++) {
			if(copy_option & GR_COPY_OPTION_USE_NULL && m_grid_data->IsColDataNull(y, x)) {
				// NULLを出力
				pdata = _T("");
				result += 4 + sepa_size;
			} else {
				result += m_grid_data->Get_CopyDataBufSize(y, x, b_escape, quote_char) + sepa_size;
/*
				pdata = GetCopyData(y, x);
				if(pdata != NULL) {
					result = result + (int)_tcslen(pdata) + sepa_size;
					if(b_escape) {
						// "は，""に変換するので，その分を計算
						result += ostr_str_cnt(pdata, quote_char);
					}
				}
*/
			}
		}
		if(pdata != NULL && copy_option & GR_COPY_OPTION_CONVERT_CRLF) {
			// 改行コードをchr(10)などに変換する
			result += (ostr_str_cnt(pdata, '\r') * 20);
			result += (ostr_str_cnt(pdata, '\n') * 20);
		}
		result += 2;	// 改行コード
	}
	result++;	// '\0'

	goto RESULT;

RESULT:
	if(result == 0) result = 10;
	return result * sizeof(TCHAR);
}

static TCHAR *str_csv_cpy(TCHAR *buf, const TCHAR *pdata, const int len)
{
	TCHAR *p = buf;

	*p = '\"';
	p++;
	for(; *pdata != '\0'; pdata++) {
		*p = *pdata;
		p++;
		if(*pdata == '\"') {
			*p = '\"';
			p++;
		}
	}
	*p = '\"';
	p++;

	return p;
}

static TCHAR *str_sql_cpy(TCHAR *buf, const TCHAR *pdata, const int len)
{
	TCHAR *p = buf;

	*p = '\'';
	p++;
	for(; *pdata != '\0'; pdata++) {
		*p = *pdata;
		p++;
		if(*pdata == '\'') {
			*p = '\'';
			p++;
		}
	}
	*p = '\'';
	p++;

	return p;
}

static TCHAR *_add_chr(TCHAR *p, int ch, int *str_cnt)
{
	TCHAR buf[20];
	_stprintf(buf, _T("' || chr(%d) || '"), ch);
	_tcscpy(p, buf);
	p += _tcslen(buf);
	return p;
}

static TCHAR *str_sql_cpy_chr(TCHAR *buf, const TCHAR *pdata, const int len)
{
	TCHAR *p = buf;
	int str_cnt = 0;

	*p = '\'';
	p++;
	for(; *pdata != '\0'; pdata++) {
		if(*pdata == '\r' || *pdata == '\n') {
			p = _add_chr(p, *pdata, &str_cnt);
			continue;
		}

		*p = *pdata;
		p++;
		if(*pdata == '\'') {
			*p = '\'';
			p++;
		}
	}
	*p = '\'';
	p++;

	return p;
}

static TCHAR *str_tsv_cpy_escape_dbl_quote(TCHAR *buf, const TCHAR *pdata, const int len)
{
	if(*pdata == '\"') {
		return str_csv_cpy(buf, pdata, len);
	}

	const TCHAR *p2;
	for(p2 = pdata; *p2 != '\0'; p2++) {
		if(*p2 == '\t' || *p2 == '\n' || *p2 == '\r') {
			return str_csv_cpy(buf, pdata, len);
		}
	}

	TCHAR *p;
	for(p = buf; *pdata != '\0'; pdata++) {
		*p = *pdata;
		p++;
	}

	return p;
}

static TCHAR *str_tsv_cpy(TCHAR *buf, const TCHAR *pdata, const int len)
{
	TCHAR *p;
	for(p = buf; *pdata != '\0'; pdata++) {
		*p = *pdata;
		p++;
	}

	return p;
}

static TCHAR *str_normal_fix_len(TCHAR *buf, const TCHAR *pdata, const int len)
{
	TCHAR *p = buf;
	int cnt = 0;

	for(; cnt < len - 1 && *pdata != '\0'; pdata++) {
		*p = *pdata;
		p++;
		cnt++;
	}
	for(;cnt < len;) {
		*p = ' ';
		p++;
		cnt++;
	}

	return p;
}

static int _need_object_name_quote(const TCHAR *object_name)
{
	return ostr_need_object_name_quote_for_oracle(object_name);
}

void CGridCtrl::GetCopyData(TCHAR *buf, int copy_format, 
	int y_start, int y_end, int x_start, int x_end, int copy_option)
{
	const TCHAR *pdata;
	int sepa;
	TCHAR *(*str_cpy_func)(TCHAR *buf, const TCHAR *pdata, const int len);
	int x, y;
	TCHAR *p = buf;
	BOOL b_fix = FALSE;
	BOOL b_copy_colname = FALSE;
	BOOL b_line_sepa_is_crlf = TRUE;
	BOOL x_crlf_cnt = -1;

	if(copy_format == GR_COPY_FORMAT_TAB_CNAME || 
		copy_format == GR_COPY_FORMAT_CSV_CNAME ||
		copy_format == GR_COPY_FORMAT_FIX_LEN_CNAME ||
		copy_format == GR_COPY_FORMAT_COLUMN_NAME) {
		b_copy_colname = TRUE;
	}

	switch(copy_format) {
	case GR_COPY_FORMAT_TAB:
	case GR_COPY_FORMAT_TAB_CNAME:
		sepa = '\t';
		str_cpy_func = str_tsv_cpy;
		if(m_gridStyle & GRS_COPY_ESCAPE_DBL_QUOTE) str_cpy_func = str_tsv_cpy_escape_dbl_quote;
		break;
	case GR_COPY_FORMAT_CSV:
	case GR_COPY_FORMAT_CSV_CNAME:
		sepa = ',';
		str_cpy_func = str_csv_cpy;
		break;
	case GR_COPY_FORMAT_FIX_LEN:
	case GR_COPY_FORMAT_FIX_LEN_CNAME:
		b_fix = TRUE;
		str_cpy_func = str_normal_fix_len;
		break;
	case GR_COPY_FORMAT_COLUMN_NAME:
		sepa = ',';
		str_cpy_func = str_tsv_cpy;
		if(m_gridStyle & GRS_COPY_ESCAPE_DBL_QUOTE) str_cpy_func = str_tsv_cpy_escape_dbl_quote;
		break;
	case GR_COPY_FORMAT_SQL:
		sepa = ',';
		str_cpy_func = str_sql_cpy;
		if(m_gridStyle & GRS_SWAP_ROW_COL_MODE) b_line_sepa_is_crlf = FALSE;
		break;
	case GR_COPY_FORMAT_WHERE_CLAUSE:
		break;
	case GR_COPY_FORMAT_IN_CLAUSE:
		sepa = ',';
		str_cpy_func = str_sql_cpy;
		b_line_sepa_is_crlf = FALSE;
		x_crlf_cnt = 5;		// 改行を入れる位置を指定
		break;
	default:
		return;
	}

	if(copy_format == GR_COPY_FORMAT_WHERE_CLAUSE) {
		const TCHAR *pcname;

		_tcscpy(p, _T("where "));
		p += 6;

		int row, col;
		int	y, x;

		int row_start = y_start;
		int row_end = y_end;
		int col_start = x_start;
		int col_end = x_end;

		if(m_gridStyle & GRS_SWAP_ROW_COL_MODE) {
			row_start = x_start;
			row_end = x_end;
			col_start = y_start;
			col_end = y_end;
		}

		for(row = row_start; row <= row_end; row++) {
			if(row_start != row_end) {
				*p = '(';
				p++;
			}
			for(col = col_start; col <= col_end; col++) {
				if(m_gridStyle & GRS_SWAP_ROW_COL_MODE) {
					pcname = m_grid_data->GetRowHeader(col);
					y = col;
					x = row;
				} else {
					pcname = m_grid_data->Get_DispColName(col);
					y = row;
					x = col;
				}
				pdata = m_grid_data->Get_CopyData(y, x);

				if((copy_option & GR_COPY_OPTION_QUOTED_NAME) || _need_object_name_quote(pcname)) {
					p = str_tsv_cpy(p, _T("\""), 0);
					p = str_tsv_cpy(p, pcname, 0);
					p = str_tsv_cpy(p, _T("\""), 0);
				} else {
					p = str_tsv_cpy(p, pcname, 0);
				}

				if(m_grid_data->IsColDataNull(y, x)) {
					_tcscpy(p, _T(" is null "));
					p += 10;
				} else {
					_tcscpy(p, _T(" = "));
					p += 3;

					if(copy_option & GR_COPY_OPTION_CONVERT_CRLF) {
						p = str_sql_cpy_chr(p, pdata, 0);
					} else {
						p = str_sql_cpy(p, pdata, 0);
					}
				}

				if(col != col_end) {
					_tcscpy(p, _T(" and "));
					p += 5;
				}
			}
			if(row_start != row_end) {
				_tcscpy(p, _T(")\r\n"));
				p += 3;
				if(row != row_end) {
					_tcscpy(p, _T("   or "));
					p += 6;
				}
			}
		}
		*p = '\0';
		return;
	}

	if(b_copy_colname && !(m_gridStyle & GRS_SWAP_ROW_COL_MODE)) {
		for(x = x_start; x <= x_end; x++) {
			pdata = m_grid_data->Get_DispColName(x);
			p = str_cpy_func(p, pdata, m_grid_data->Get_ColMaxSize(x));

			if(x != x_end && b_fix == FALSE) {
				*p = sepa;
				p++;
			}
		}
		if(copy_format == GR_COPY_FORMAT_COLUMN_NAME) {
			*p = '\0';
			return;
		}

		*p = '\r';
		p++;
		*p = '\n';
		p++;
	}

	ASSERT(y_start >= 0 && x_start >= 0);
	if(y_start < 0 || x_start < 0) goto RESULT;

	int output_data_cnt = 0;
	for(y = y_start; y <= y_end; y++) {
		if(b_copy_colname && (m_gridStyle & GRS_SWAP_ROW_COL_MODE)) {
			pdata = m_grid_data->GetRowHeader(y);
			p = str_cpy_func(p, pdata, m_grid_data->GetRowHeaderLen());
			if(b_fix == FALSE) {
				*p = sepa;
				p++;
			}
			if(copy_format == GR_COPY_FORMAT_COLUMN_NAME) {
				if(y == y_end) {
					// 最後のsepaは不要
					*p--;
					*p = '\r';
					p++;
					*p = '\n';
					p++;
					*p = '\0';
					return;
				}
				continue;
			}
		}

		for(x = x_start; x <= x_end; x++) {
			if(copy_option & GR_COPY_OPTION_USE_NULL && m_grid_data->IsColDataNull(y, x)) {
				// NULLを出力
				pdata = _T("");
				p = str_tsv_cpy(p, _T("NULL"), 0);
			} else {
				pdata = m_grid_data->Get_CopyData(y, x);
				p = str_cpy_func(p, pdata, m_grid_data->Get_ColMaxSize(x));
			}
			output_data_cnt++;

			if(x != x_end && b_fix == FALSE) {
				*p = sepa;
				p++;
			}
		}

		if (b_line_sepa_is_crlf) {
			*p = '\r';
			p++;
			*p = '\n';
			p++;
		}
		// IN CLAUSE用の処理
		if (!b_line_sepa_is_crlf) {
			// 最後のデータの後は','を出力しない
			if (y != y_end) {
				*p = sepa;
				p++;
			}
			// x_crlf_cntで指定した個数出力した場合、改行する
			// 最終データの後も改行する
			if ((y == y_end) ||
				(x_crlf_cnt > 0 && (output_data_cnt % x_crlf_cnt) == 0)) {
				*p = '\r';
				p++;
				*p = '\n';
				p++;
			}
		}
	}

RESULT:
	*p = '\0';
	if(p == buf) return;

	// 単一行のコピーのときは，改行しない
	if(!(m_gridStyle & GRS_SWAP_ROW_COL_MODE) && y_start == y_end) {
		*(p - 2) = '\0';
	}
	if((m_gridStyle & GRS_SWAP_ROW_COL_MODE) && x_start == x_end) {
		*(p - 2) = '\0';
	}
}

CString CGridCtrl::GetCopyString(int copy_format, int y_start, int y_end, int x_start, int x_end, int copy_option)
{
	CString str;

	TCHAR *pstr = str.GetBuffer(
		CalcCopyDataSize(copy_format, y_start, y_end, x_start, x_end, copy_option));

	GetCopyData(pstr, copy_format, y_start, y_end, x_start, x_end, copy_option);

	str.ReleaseBuffer();

	return str;
}

void CGridCtrl::Copy(int copy_format, int y_start, int y_end, int x_start, int x_end, int copy_option)
{
	// コピーするセルが多いときはwait cursorにする
	int copy_cell_cnt = (abs(y_end - y_start) + 1) * (abs(x_end - x_start) + 1);

	if(copy_cell_cnt > 100) {
		CWaitCursor wait_cursor;
		CopyToClipboard(copy_format, y_start, y_end, x_start, x_end, copy_option);
	} else {
		CopyToClipboard(copy_format, y_start, y_end, x_start, x_end, copy_option);
	}
}

void CGridCtrl::CopyToClipboard(int copy_format, int y_start, int y_end, int x_start, int x_end, int copy_option)
{
	if(copy_format == GR_COPY_FORMAT_COLUMN_NAME) {
		if(!CanCopyColumnName()) return;
	} else {
		if(m_grid_data->is_selected_cell() == FALSE) return;
	}

	if(IsEnterEdit()) {
		m_edit_cell->Copy();
		return;
	}

	HGLOBAL hData = GlobalAlloc(GHND, 
		CalcCopyDataSize(copy_format, y_start, y_end, x_start, x_end, copy_option));
	if(hData == NULL) {
		AfxMessageBox( _T("メモリ確保に失敗しました") );
		return;
	}
	TCHAR *pstr = (TCHAR *)GlobalLock(hData);
	if(pstr == NULL) {
		AfxMessageBox( _T("メモリ確保に失敗しました") );
		return;
	}

	GetCopyData(pstr, copy_format, y_start, y_end, x_start, x_end, copy_option);

	GlobalUnlock(hData);

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
	// ...
	CloseClipboard();
}

void CGridCtrl::Copy(int copy_format, int copy_option) 
{
	int y_start, y_end, x_start, x_end;

	if(HaveSelected() == FALSE) {
		y_start = m_grid_data->get_cur_row();
		y_end = m_grid_data->get_cur_row();
		x_start = m_grid_data->get_cur_col();
		x_end = m_grid_data->get_cur_col();
	} else if(m_grid_data->GetSelectArea()->pos1.x <= m_grid_data->GetSelectArea()->pos2.x) {
		y_start = m_grid_data->GetSelectArea()->pos1.y;
		y_end = m_grid_data->GetSelectArea()->pos2.y;
		x_start = m_grid_data->GetSelectArea()->pos1.x;
		x_end = m_grid_data->GetSelectArea()->pos2.x;
	} else {
		y_start = m_grid_data->GetSelectArea()->pos1.y;
		y_end = m_grid_data->GetSelectArea()->pos2.y;
		x_start = m_grid_data->GetSelectArea()->pos2.x;
		x_end = m_grid_data->GetSelectArea()->pos1.x;
	}

	Copy(copy_format, y_start, y_end, x_start, x_end, copy_option);
}

void CGridCtrl::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == IdDragHeaderTimer) {
		POINT		pt;
		CRect		rect;

		GetCursorPos(&pt);
		ScreenToClient(&pt);
		GetDispRect(rect);
		if(rect.PtInRect(pt) != 0) return;

		if(m_drag_header.drag_flg == PRE_DRAG_HEADER) {
			StartDragHeader(pt);
		} else {
			DoDragHeader(pt);
		}
	}

	if(nIDEvent == IdDragSelectTimer) {
		POINT		pt;
		CRect		rect;

		GetCursorPos(&pt);
		ScreenToClient(&pt);
		GetDispRect(rect);

		rect.left = m_row_header_width;
		rect.top = m_col_header_height;

		if(rect.PtInRect(pt) != 0) return;

		switch(m_grid_data->GetSelectArea()->drag_flg) {
		case PRE_DRAG:
			StartDragSelected();
			break;
		case DO_DRAG:
			DoDragSelected(pt);
			break;
		}
	}

	if(nIDEvent == IdToolTipTimer) {
		KillTimer(IdToolTipTimer);
		CPoint pt;
		::GetCursorPos(&pt);
		ScreenToClient(&pt);
		if(m_tool_tip_pt == pt) ShowToolTip();
	}
	
	CWnd::OnTimer(nIDEvent);
}

void CGridCtrl::PreDragHeader(CPoint point, int col)
{
	m_drag_header.drag_flg = PRE_DRAG_HEADER;
	m_drag_header.col = col;
	m_drag_header.pt.x = point.x;
	m_drag_header.pt.y = point.y;
	SetCapture();
	SetTimer(IdDragHeaderTimer, 100, NULL);
}

void CGridCtrl::StartDragHeader(CPoint point)
{
	m_drag_header.drag_flg = DO_DRAG_HEADER;
	DrawDragRect(point, TRUE);
	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
}

void CGridCtrl::DoDragHeader(CPoint point)
{
	DrawDragRect(point, FALSE);
}

void CGridCtrl::EndDragHeader(CPoint point)
{
	if(m_drag_header.drag_flg == DO_DRAG_HEADER) {
		CRect winrect;
		GetDispRect(winrect);

		SetDispColWidth(m_drag_header.col, 
			GetDispColWidth(m_drag_header.col) - (m_drag_header.pt.x - point.x));
		if(GetDispColWidth(m_drag_header.col) < m_min_colwidth) {
			SetDispColWidth(m_drag_header.col, m_min_colwidth);
		}
		Update_AllWnd();

		POINT	pt;
		pt.x = m_drag_header.col;
		pt.y = GetScrollPos(SB_VERT);
		ShowCell(&pt);
	}
	KillTimer(IdDragHeaderTimer);
	ReleaseCapture();
	m_drag_header.drag_flg = NO_DRAG_HEADER;

	SetEditCellPos();
}

int CGridCtrl::SearchText2(const TCHAR *search_text, int dir, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii,
	BOOL b_regexp, BOOL b_loop, BOOL *b_looped, BOOL b_cur_cell)
{
	if(b_looped != NULL) *b_looped = FALSE;
	if(_tcscmp(search_text, _T("")) == 0) return 1;

	HREG_DATA reg_data = m_search_data.MakeRegData2(search_text, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp);
	if(reg_data == NULL) {
		MessageBox(_T("不正な正規表現です"), _T("メッセージ"), MB_ICONINFORMATION | MB_OK);
		return 1;
	}
	SaveSearchData2(search_text, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp);

	POINT		pt1, pt2;

	pt1 = *m_grid_data->get_cur_cell();

	BOOL b_selected_area = FALSE;
	if((m_gridStyle & GRS_SEARCH_SELECTED_AREA) && HaveSelected()) b_selected_area = TRUE;

	if(m_grid_data->SearchDataRegexp(pt1, &pt2, dir,
		b_loop, b_looped, b_cur_cell, b_selected_area, reg_data) == 0) {
		//MessageBox("見つかりません", "検索", MB_OK | MB_ICONINFORMATION);
		return 1;
	}

	SelChanged(&pt2, FALSE, b_selected_area);

	return 0;
}

int CGridCtrl::SearchNext2(const TCHAR *search_text, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp, BOOL *b_looped)
{
	return SearchText2(search_text, 1, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp, TRUE, b_looped, FALSE);
}

int CGridCtrl::SearchPrev2(const TCHAR *search_text, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp, BOOL *b_looped)
{
	return SearchText2(search_text, -1, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp, TRUE, b_looped, FALSE);
}

int CGridCtrl::SearchText(const TCHAR *search_text, int dir, BOOL b_distinct_lwr_upr, BOOL b_regexp,
	BOOL b_loop, BOOL *b_looped, BOOL b_cur_cell)
{
	return SearchText2(search_text, dir, b_distinct_lwr_upr, TRUE, b_regexp, b_loop, b_looped, b_cur_cell);
}

int CGridCtrl::SearchNext(const TCHAR *search_text, BOOL b_distinct_lwr_upr, BOOL b_regexp, BOOL *b_looped)
{
	return SearchText(search_text, 1, b_distinct_lwr_upr, b_regexp, TRUE, b_looped, FALSE);
}

int CGridCtrl::SearchPrev(const TCHAR *search_text, BOOL b_distinct_lwr_upr, BOOL b_regexp, BOOL *b_looped)
{
	return SearchText(search_text, -1, b_distinct_lwr_upr, b_regexp, TRUE, b_looped, FALSE);
}

BOOL CGridCtrl::IsUpdateDispRowAndDispCol(int disp_row, int disp_col)
{
	int data_row = disp_row;
	if(m_gridStyle & GRS_SWAP_ROW_COL_MODE) data_row = disp_col;

	if(!m_grid_data->IsUpdateRow(data_row)) return FALSE;

	return m_grid_data->IsUpdateCell(disp_row, disp_col);
}

int CGridCtrl::SearchUpdateCellNext(BOOL *b_looped)
{
	POINT		pt1;
	int		row, col;

	int		top, bottom, left, right;
	BOOL	b_selected_area;

	if((m_gridStyle & GRS_SEARCH_SELECTED_AREA) && HaveSelected()) {
		b_selected_area = TRUE;
		top = m_grid_data->GetSelectArea()->pos1.y;
		bottom = m_grid_data->GetSelectArea()->pos2.y;
		left = m_grid_data->GetSelectArea()->pos1.x;
		right = m_grid_data->GetSelectArea()->pos2.x;
	} else {
		b_selected_area = FALSE;
		top = 0;
		bottom = m_grid_data->Get_RowCnt() - 1;
		left = 0;
		right = m_grid_data->Get_ColCnt() - 1;
	}

	*b_looped = FALSE;

	// 現在の位置を取得
	pt1 = *m_grid_data->get_cur_cell();

	// 現在のセルの右側を探す
	for(row = pt1.y, col = pt1.x + 1; col <= right; col++) {
		if(IsUpdateDispRowAndDispCol(row, col)) goto FOUND;
	}

	// 現在の行の次から最終行まで探す
	for(row = pt1.y + 1; row <= bottom; row++) {
		for(col = left; col <= right; col++) {
			if(IsUpdateDispRowAndDispCol(row, col)) goto FOUND;
		}
	}

	*b_looped = TRUE;

	// 先頭から現在の行の前まで探す
	for(row = 0; row < pt1.y; row++) {
		for(col = left; col <= right; col++) {
			if(IsUpdateDispRowAndDispCol(row, col)) goto FOUND;
		}
	}

	// 現在のセルの左側を探す
	for(row = pt1.y, col = left; col <= pt1.x; col++) {
		if(IsUpdateDispRowAndDispCol(row, col)) goto FOUND;
	}

	// 見つからなかった
	return 0;

FOUND:
	// 見つかった
	pt1.y = row;
	pt1.x = col;

	SelChanged(&pt1, FALSE, b_selected_area);

	return 1;
}

int CGridCtrl::SearchUpdateCellPrev(BOOL *b_looped)
{
	POINT		pt1;
	int		row, col;

	int		top, bottom, left, right;
	BOOL	b_selected_area;

	if((m_gridStyle & GRS_SEARCH_SELECTED_AREA) && HaveSelected()) {
		b_selected_area = TRUE;
		top = m_grid_data->GetSelectArea()->pos1.y;
		bottom = m_grid_data->GetSelectArea()->pos2.y;
		left = m_grid_data->GetSelectArea()->pos1.x;
		right = m_grid_data->GetSelectArea()->pos2.x;
	} else {
		b_selected_area = FALSE;
		top = 0;
		bottom = m_grid_data->Get_RowCnt() - 1;
		left = 0;
		right = m_grid_data->Get_ColCnt() - 1;
	}

	*b_looped = FALSE;

	// 現在の位置を取得
	pt1 = *m_grid_data->get_cur_cell();

	// 現在のセルの左側を探す
	for(row = pt1.y, col = pt1.x - 1; col >= left; col--) {
		if(IsUpdateDispRowAndDispCol(row, col)) goto FOUND;
	}

	// 現在の行の前から先頭行まで探す
	for(row = pt1.y - 1; row >= top; row--) {
		for(col = right; col >= left; col--) {
			if(IsUpdateDispRowAndDispCol(row, col)) goto FOUND;
		}
	}

	*b_looped = TRUE;

	// 末尾から現在の行の次まで探す
	for(row = bottom; row > pt1.y; row--) {
		for(col = right; col >= left; col--) {
			if(IsUpdateDispRowAndDispCol(row, col)) goto FOUND;
		}
	}

	// 現在のセルの右側を探す
	for(row = pt1.y, col = right; col >= pt1.x; col--) {
		if(IsUpdateDispRowAndDispCol(row, col)) goto FOUND;
	}

	// 見つからなかった
	return 0;

FOUND:
	// 見つかった
	pt1.y = row;
	pt1.x = col;

	SelChanged(&pt1, FALSE, b_selected_area);

	return 1;
}

void CGridCtrl::OnDestroy() 
{
	CWnd::OnDestroy();

	m_grid_data->UnRegistNotifyWnd(GetSafeHwnd());

	m_font.DeleteObject();
}

BOOL CGridCtrl::IsValidCurPt()
{
	return m_grid_data->IsValidCurPt();
}

BOOL CGridCtrl::HaveSelected()
{
	if(IsEnterEdit()) {
		return m_edit_cell->HaveSelected();
	}

	return m_grid_data->HaveSelected();
}

void CGridCtrl::ClearSelected(BOOL b_notify_parent_wnd)
{
	if(HaveSelected() == TRUE) {
		int		x1, x2;
		POINT	pt;
		if(m_grid_data->GetSelectArea()->pos1.x < m_grid_data->GetSelectArea()->pos2.x) {
			x1 = m_grid_data->GetSelectArea()->pos1.x;
			x2 = m_grid_data->GetSelectArea()->pos2.x;
		} else {
			x1 = m_grid_data->GetSelectArea()->pos2.x;
			x2 = m_grid_data->GetSelectArea()->pos1.x;
		}

		// 再描画の範囲を，表示中の範囲にする
		if(IsSplitterMode()) {
			POINT pt1, pt2;
			pt1 = m_grid_data->GetSelectArea()->pos1;
			pt2 = m_grid_data->GetSelectArea()->pos2;
			InvalidateRange_AllWnd(&pt1, &pt2);
		} else {
			pt.y = max(m_grid_data->GetSelectArea()->pos1.y, GetScrollPos(SB_VERT));
			int loop = min(m_grid_data->GetSelectArea()->pos2.y, GetScrollPos(SB_VERT) + GetShowRow());

			// 再描画実行
			for(; pt.y <= loop; (pt.y)++) {
				for(pt.x = x1; pt.x <= x2; (pt.x)++) {
					InvalidateCell_AllWnd(&pt);
				}
			}
		}

		m_grid_data->GetSelectArea()->pos1.y = -1;
		m_grid_data->GetSelectArea()->pos2.y = -1;
		m_grid_data->GetSelectArea()->select_mode = SELECT_MODE_NORMAL;

		if(b_notify_parent_wnd) {
			// 選択範囲が変わったことを親ウィンドウに通知する
			GetParent()->SendMessage(GC_WM_CHANGE_SELECT_AREA, 0, 0);
		}
	}
}

void CGridCtrl::SelectCell(CPoint pt)
{
	if(!(m_gridStyle & GRS_MULTI_SELECT)) return;

	if(pt.x < 0 || pt.x >= m_grid_data->Get_ColCnt() ||
		pt.y < 0 || pt.y >= m_grid_data->Get_RowCnt()) return;

	POINT		cur_pos1, cur_pos2;

	cur_pos1 = m_grid_data->GetSelectArea()->pos1;
	cur_pos2 = m_grid_data->GetSelectArea()->pos2;

	if(m_grid_data->GetSelectArea()->start_pt.y > pt.y || 
		(m_grid_data->GetSelectArea()->start_pt.y == pt.y && m_grid_data->GetSelectArea()->start_pt.x > pt.x)) {
		m_grid_data->GetSelectArea()->pos1 = pt;
		m_grid_data->GetSelectArea()->pos2 = m_grid_data->GetSelectArea()->start_pt;
	} else {
		m_grid_data->GetSelectArea()->pos1 = m_grid_data->GetSelectArea()->start_pt;
		m_grid_data->GetSelectArea()->pos2 = pt;
	}

	if(m_grid_data->GetSelectArea()->select_mode == SELECT_MODE_ROW) {
		m_grid_data->GetSelectArea()->pos1.x = 0;
		m_grid_data->GetSelectArea()->pos2.x = m_grid_data->Get_ColCnt() - 1;
	} else if(m_grid_data->GetSelectArea()->select_mode == SELECT_MODE_COL) {
		m_grid_data->GetSelectArea()->pos1.y = 0;
		m_grid_data->GetSelectArea()->pos2.y = m_grid_data->Get_RowCnt() - 1;
	}

	// 選択範囲を再描画
	if(cur_pos1.y == -1) {
		if(m_grid_data->GetSelectArea()->pos2.y - m_grid_data->GetSelectArea()->pos1.y > 50) {
			Invalidate_AllWnd();
		} else {
			for(int i = m_grid_data->GetSelectArea()->pos1.y; i <= m_grid_data->GetSelectArea()->pos2.y; i++) {
				InvalidateRow_AllWnd(i);
			}
		}
	} else {
		CRect	rect1(cur_pos1, cur_pos2);						// 以前の選択範囲
		CRect	rect2(m_grid_data->GetSelectArea()->pos1, m_grid_data->GetSelectArea()->pos2);	// 現在の選択範囲
		CRect	rect3(0, 0, 0, 0);
		CRect	rect4(0, 0, 0, 0);

		rect1.NormalizeRect();
		rect2.NormalizeRect();
		{	// rect1, rect2を含む，最大の四角形を求める
			rect3.left = min(rect1.left, rect2.left);
			rect3.right = max(rect1.right, rect2.right);
			rect3.top = min(rect1.top, rect2.top);
			rect3.bottom = max(rect1.bottom, rect2.bottom);
		}
		
		{	// rect1, rect2で重なる部分を求める
			rect4.left = max(rect1.left, rect2.left);
			rect4.right = min(rect1.right, rect2.right);
			rect4.top = max(rect1.top, rect2.top);
			rect4.bottom = min(rect1.bottom, rect2.bottom);

			// normalizerectじゃないとき，空にする
			if(rect4.left > rect4.right || rect4.top > rect4.bottom) {
				rect4.left = 0;
				rect4.right = 0;
				rect4.top = 0;
				rect4.bottom = 0;
			} else {
				// CRect::PtInRectで処理できるように，bottomとrightを広げる
				rect4.right = rect4.right + 1;
				rect4.bottom = rect4.bottom + 1;
			}
		}

		if(IsSplitterMode()) {
			POINT pt1, pt2;
			pt1.y = rect3.top;
			pt1.x = rect3.left;
			pt2.y = rect3.bottom;
			pt2.x = rect3.right;
			InvalidateRange_AllWnd(&pt1, &pt2);
		} else {
			POINT pt;
			for(pt.y = rect3.top; pt.y <= rect3.bottom; pt.y++) {
				for(pt.x = rect3.left; pt.x <= rect3.right; pt.x++) {
					if(rect4.PtInRect(pt) == FALSE) {
						InvalidateCell_AllWnd(&pt);
					}
				}
			}
		}
	}

	// 選択範囲が変わったことを親ウィンドウに通知する
	GetParent()->SendMessage(GC_WM_CHANGE_SELECT_AREA, 0, 0);
}

BOOL CGridCtrl::IsSelectedCell(int row, int col)
{
	if(m_gridStyle & GRS_LINE_SELECT) {
		if(m_grid_data->get_cur_row() == row) return TRUE;
		if(m_gridStyle & GRS_MULTI_SELECT) {
			if(m_grid_data->GetSelectArea()->pos1.y <= row &&
				m_grid_data->GetSelectArea()->pos2.y >= row) return TRUE;
		}
		return FALSE;
	}

	if(!(m_gridStyle & GRS_MULTI_SELECT)) return FALSE;

	return m_grid_data->IsInSelectedArea(row, col);
}

void CGridCtrl::StartSelect(CPoint point)
{
	m_grid_data->GetSelectArea()->start_pt.y = point.y;
	m_grid_data->GetSelectArea()->start_pt.x = point.x;
}

void CGridCtrl::PreDragSelected(int mode)
{
	if(!(m_gridStyle & GRS_MULTI_SELECT)) return;

	m_grid_data->GetSelectArea()->drag_flg = PRE_DRAG;
	m_grid_data->GetSelectArea()->select_mode = mode;
	if(HaveSelected() == FALSE) {
		CPoint pt = *m_grid_data->get_cur_cell();
		StartSelect(pt);
	}
	SetCapture();
	SetTimer(IdDragSelectTimer, 100, NULL);
}

void CGridCtrl::StartDragSelected()
{
	m_grid_data->GetSelectArea()->drag_flg = DO_DRAG;
}

void CGridCtrl::DoDragSelected(CPoint point)
{
	POINT		pt;

	HitCell(point, &pt, TRUE);
//	if(pt.x == -1 && pt.y == -1) return;

	if(m_grid_data->GetSelectArea()->select_mode == SELECT_MODE_ROW) {
		pt.x = m_grid_data->get_cur_col();
	} else if(m_grid_data->GetSelectArea()->select_mode == SELECT_MODE_COL) {
		pt.y = m_grid_data->get_cur_row();
	}

	SelChanged(&pt);
}

void CGridCtrl::EndDragSelected()
{
	KillTimer(IdDragSelectTimer);
	ReleaseCapture();
	m_grid_data->GetSelectArea()->drag_flg = NO_DRAG;
}

void CGridCtrl::SelectRow(int row)
{
	CPoint	sel_pt, pt1, pt2;

	sel_pt.y = row;
	sel_pt.x = GetScrollPos(SB_HORZ);
	if(IsFixColMode()) sel_pt.x = 0;
	pt1.y = row;
	pt1.x = 0;
	pt2.y = row;
	pt2.x = m_grid_data->Get_ColCnt() - 1;
	
	BOOL	shift = FALSE;
	if(GetAsyncKeyState(VK_SHIFT) < 0) shift = TRUE;
	if(shift == TRUE) {
		if(HaveSelected() == FALSE) {
			pt1.y = m_grid_data->get_cur_row();
		} else {
			pt1.y = m_grid_data->GetSelectArea()->start_pt.y;
		}
	}

	SelectArea(sel_pt, pt1, pt2);
}

void CGridCtrl::SelectCol(int col)
{
	CPoint	sel_pt, pt1, pt2;

	sel_pt.y = GetScrollPos(SB_VERT);
	sel_pt.x = col;
	pt1.y = 0;
	pt1.x = col;
	pt2.y = m_grid_data->Get_RowCnt() - 1;
	pt2.x = col;

	BOOL	shift = FALSE;
	if(GetAsyncKeyState(VK_SHIFT) < 0) shift = TRUE;
	if(shift == TRUE) {
		if(HaveSelected() == FALSE) {
			pt1.x = m_grid_data->get_cur_col();
		} else {
			pt1.x = m_grid_data->GetSelectArea()->start_pt.x;
		}
	}

	SelectArea(sel_pt, pt1, pt2);
}

void CGridCtrl::SelectAll()
{
	if(IsEnterEdit()) {
		m_edit_cell->SelectAll();
		return;
	}

	CPoint	sel_pt, pt1, pt2;

	sel_pt.y = m_grid_data->get_cur_row();
	sel_pt.x = m_grid_data->get_cur_col();
	pt1.y = 0;
	pt1.x = 0;
	pt2.y = m_grid_data->Get_RowCnt() - 1;
	pt2.x = m_grid_data->Get_ColCnt() - 1;

	SelectArea(sel_pt, pt1, pt2);
}

void CGridCtrl::SelectArea(CPoint sel_pt, CPoint pt1, CPoint pt2)
{
	if(!(m_gridStyle & GRS_MULTI_SELECT)) return;

	SelChanged(&sel_pt);
	ClearSelected(FALSE);
	StartSelect(pt1);
	SelectCell(pt2);
}

BOOL CGridCtrl::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;
}

UINT CGridCtrl::OnGetDlgCode() 
{
	return DLGC_WANTALLKEYS;

	//	return CWnd::OnGetDlgCode();
}

int CGridCtrl::InsertRows(int row_cnt)
{
	int row = m_grid_data->get_cur_row();
	if(m_gridStyle & GRS_SWAP_ROW_COL_MODE) row = m_grid_data->get_cur_col();

	LeaveEdit();
	m_grid_data->InsertRows(row, row_cnt);

	if(m_gridStyle & GRS_SWAP_ROW_COL_MODE) {
		SetCell(row + 1, 0);
	} else {
		SetCell(0, row + 1);
	}

	ValidateEditCell();
	Update_AllWnd();
	ShowCell(m_grid_data->get_cur_cell());

	NoticeChangeCellPos();
	NoticeUpdateGridData();

	return 0;
}

int CGridCtrl::DeleteRow()
{
	if(m_grid_data->is_selected_cell() == FALSE) return 0;

	BOOL b_have_selected = HaveSelected();

	int del_row = m_grid_data->get_cur_row();
	if(m_gridStyle & GRS_SWAP_ROW_COL_MODE) del_row = m_grid_data->get_cur_col();
	int row_cnt = m_grid_data->Get_RowCnt();

	LeaveEdit();
	if(b_have_selected) {
		if(m_gridStyle & GRS_SWAP_ROW_COL_MODE) {
			m_grid_data->DeleteRows(m_grid_data->GetSelectArea()->pos1.x, m_grid_data->GetSelectArea()->pos2.x);
		} else {
			m_grid_data->DeleteRows(m_grid_data->GetSelectArea()->pos1.y, m_grid_data->GetSelectArea()->pos2.y);
		}
	} else {
		m_grid_data->DeleteRows(del_row, del_row);
	}

	ValidateEditCell();
	if(row_cnt != m_grid_data->Get_RowCnt() || b_have_selected || (m_gridStyle & GRS_SWAP_ROW_COL_MODE)) {
		Update_AllWnd();
	} else {
		InvalidateRow_AllWnd(del_row);
	}
	ShowCell(m_grid_data->get_cur_cell());

	NoticeChangeCellPos();
	NoticeUpdateGridData();

	return 0;
}

int CGridCtrl::DeleteNullRows(BOOL all_flg)
{
	if(m_grid_data->IsEditable() == FALSE) return 0;

	LeaveEdit();

	int del_row_cnt = m_grid_data->DeleteNullRows(all_flg);
	if(del_row_cnt > 0) {
		ValidateEditCell();
		Update_AllWnd();
		ShowCell(m_grid_data->get_cur_cell());
	}

	NoticeChangeCellPos();
	NoticeUpdateGridData();

	return del_row_cnt;
}

void CGridCtrl::DeleteKey()
{
	if(!m_grid_data->IsEditable() || !IsValidCurPt()) return;

	if(HaveSelected()) {
		m_grid_data->UpdateCells(&(m_grid_data->GetSelectArea()->pos1), 
			&(m_grid_data->GetSelectArea()->pos2), _T(""), 0);
		InvalidateRange_AllWnd(&(m_grid_data->GetSelectArea()->pos1),
			&(m_grid_data->GetSelectArea()->pos2));
	} else {
		if(m_grid_data->IsEditableCell(m_grid_data->get_cur_cell()->y, m_grid_data->get_cur_cell()->x)) {
			m_grid_data->UpdateCell(m_grid_data->get_cur_cell()->y, m_grid_data->get_cur_cell()->x, _T(""), 0);
			InvalidateCell_AllWnd(m_grid_data->get_cur_cell());
			InvalidateRowHeader_AllWnd(m_grid_data->get_cur_cell()->y);
		}
	}
	NoticeUpdateGridData();
}

void CGridCtrl::SetCell(int x, int y)
{
	POINT	pt;
	pt.x = x;
	pt.y = y;

	if(::IsWindow(m_hWnd)) SelChanged(&pt);
}

void CGridCtrl::WindowMoved()
{
	if(IsEnterEdit()) {
		m_edit_cell->SetCaret(FALSE, 1);
	} else {
		SetEditCellPos();
	}
}

LRESULT CGridCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message) {
	case GC_WM_EDIT_KEY_DOWN:
		OnEditKeyDown((UINT)wParam, LOWORD(lParam), HIWORD(wParam));
		break;
	case EC_WM_KILLFOCUS:
		if((CWnd *)wParam == this || IsSplitterMode()) {
			m_edit_cell_focused = FALSE;
			LeaveEdit();
		} else {
			m_edit_cell_focused = TRUE;
		}
		break;
	case GC_WM_UPDATE:
		Update();
		break;
	case GC_WM_INVALIDATE:
		Invalidate();
		break;
	case GC_WM_INVALIDATE_CELL:
		{
			POINT	pt;
			pt.y = (LONG)wParam;
			pt.x = (LONG)lParam;
			InvalidateCell(&pt);
		}
		break;
	case GC_WM_INVALIDATE_RANGE:
		{
			POINT	*pt1 = (POINT *)wParam;
			POINT	*pt2 = (POINT *)lParam;
			InvalidateRange(pt1, pt2);
		}
	case GC_WM_INVALIDATE_ROW:
		InvalidateRow((int)wParam);
		break;
	case GC_WM_INVALIDATE_ROW_HEADER:
		InvalidateRowHeader((int)wParam);
		break;
	case GC_WM_CLEAR_ACTIVE_WND_FLG:
		m_last_active_wnd = FALSE;
		break;
	case GC_WM_INVALIDATE_TOP_LEFT:
		InvalidateTopLeft();
		break;
	}
	
	return CScrollWnd::WindowProc(message, wParam, lParam);
}

void CGridCtrl::OnEditKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch(nChar) {
	case VK_ESCAPE:
		LeaveEdit();
		ShowCell(m_grid_data->get_cur_cell());
		break;
	case VK_RETURN:
	case VK_TAB:
		if(LeaveEdit() != FALSE) {
			PostMessage(WM_KEYDOWN, nChar, MAKELPARAM(nRepCnt, nFlags));
		}
		break;
	}

	return;
}

void CGridCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// ドラッグ選択中は，文字の入力を受け付けない
	if(m_grid_data->GetSelectArea()->drag_flg == DO_DRAG) return;

	// コントロールキーが押されているときは、文字の入力を受け付けない
	if(GetKeyState(VK_CONTROL) < 0) {
		if(m_gridStyle & GRS_ON_DIALOG) {
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

	if(m_grid_data->IsEditable() != FALSE) {
		if(nChar != VK_RETURN && nChar != VK_TAB && nChar != VK_ESCAPE) {
			if(IsEnterEdit()) {
				m_edit_cell->PostMessage(WM_CHAR, nChar, MAKELPARAM(nRepCnt, nFlags));
			} else if(EnterEdit()) {
				if(m_edit_cell->HaveSelected() == FALSE) {
					m_edit_cell->SelectAll();
				}
				if(nChar == VK_BACK) {
					keybd_event(nChar, 0, NULL, NULL);
				} else {
					m_edit_cell->PostMessage(WM_CHAR, nChar, MAKELPARAM(nRepCnt, nFlags));
				}
			}
		}
	}

	CWnd::OnChar(nChar, nRepCnt, nFlags);
}

void CGridCtrl::ShowCell(POINT *pt)
{
	POINT	scr_pt, show;
	
	scr_pt.x = GetScrollPos(SB_HORZ);
	scr_pt.y = GetScrollPos(SB_VERT);

	show.x = GetShowCol();
	show.y = GetShowRow();

	if(pt->x >= scr_pt.x && pt->x < scr_pt.x + show.x && 
		pt->y >= scr_pt.y && pt->y < scr_pt.y + show.y) return;

	CSize scr_size(0, 0);

	if(pt->x != scr_pt.x) {
		if(pt->x < scr_pt.x) {
			scr_size.cx = pt->x - scr_pt.x;
		} else if(pt->x >= scr_pt.x + show.x) {
			int		x, col;
			RECT	rect;
			GetDispRect(&rect);

			x = GetScrollLeftMargin();
			col = pt->x;
			for(;;) {
				x = x + GetDispColWidth(col);
				if(x > (rect.right - rect.left) || col == 0) {
					if(col != pt->x) col++;
					break;
				}
				col--;
			}
			scr_size.cx = col - scr_pt.x;
		}

		if(IsFixColMode() && pt->x < m_fix_end_col) scr_size.cx = 0;
	}

	if(m_gridStyle & GRS_SHOW_CELL_ALWAYS_HALF) {
		scr_size.cy = pt->y - scr_pt.y - (show.y / 2) + 1;
	} else {
		if((m_gridStyle & GRS_SHOW_CELL_ALWAYS_TOP) || pt->y < scr_pt.y) {
			scr_size.cy = pt->y - scr_pt.y;
		} else if(pt->y >= scr_pt.y + show.y) {
			scr_size.cy = pt->y - show.y - scr_pt.y + 1;
		}
	}

	OnScrollBy(scr_size, TRUE);
}

void CGridCtrl::Undo()
{
	if(IsEnterEdit()) {
		m_edit_cell->Undo();
		return;
	}
	LeaveEdit();
	m_grid_data->Undo();
	ValidateEditCell();
	Update_AllWnd();
	ShowCell(m_grid_data->get_cur_cell());
	NoticeChangeCellPos();
	NoticeUpdateGridData();
}

void CGridCtrl::Redo()
{
	if(IsEnterEdit()) {
		m_edit_cell->Redo();
		return;
	}
	LeaveEdit();
	m_grid_data->Redo();
	ValidateEditCell();
	Update_AllWnd();
	ShowCell(m_grid_data->get_cur_cell());
	NoticeChangeCellPos();
	NoticeUpdateGridData();
}

void CGridCtrl::Paste()
{
	if(m_grid_data->IsEditable() == FALSE) return;

	if(IsEnterEdit()) {
		m_edit_cell->Paste();
		return;
	}

	COleDataObject	obj;

	if(obj.AttachClipboard() == FALSE) return;
	if(obj.IsDataAvailable(CF_UNICODETEXT) == FALSE) return;

	HGLOBAL hData = obj.GetGlobalData(CF_UNICODETEXT);
	if(hData == NULL) return;

	TCHAR *pstr = (TCHAR *)GlobalLock(hData);
	if(pstr == NULL) return;

	if(Paste(pstr) != 0) {
		CString msg = m_grid_data->GetLastErrorMessage();
		if(!msg.IsEmpty()) {
			m_edit_data->del_all();	// MessageBoxのタイミングで2回目が実行される？
			MessageBox(msg, _T("Error"), MB_OK | MB_ICONERROR);
		}
	}

	GlobalUnlock(hData);

	NoticeUpdateGridData();
}

int CGridCtrl::Paste(const TCHAR *pstr)
{
	if(m_grid_data->is_selected_cell() == FALSE) return 0;
	//if(*pstr == '\0') return 0;

	int		ret_v;

	if(HaveSelected()) {
		POINT cur_cell = *m_grid_data->get_cur_cell();
		m_grid_data->set_cur_cell(m_grid_data->GetSelectArea()->pos1.y,
			m_grid_data->GetSelectArea()->pos1.x);
		ret_v = m_grid_data->Paste(pstr);
		m_grid_data->set_cur_cell(cur_cell.y, cur_cell.x);
	} else {
		ret_v = m_grid_data->Paste(pstr);
	}

	Update_AllWnd();

	NoticeUpdateGridData();

	return ret_v;
}

int CGridCtrl::UpdateCell(int row, int col, const TCHAR *data, int len)
{
	if(m_grid_data->IsEditable() == FALSE) return 0;

	int		ret_v;

	ret_v = m_grid_data->UpdateCell(row, col, data, len);

	POINT		pt;
	pt.y = row;
	pt.x = col;
	InvalidateCell_AllWnd(&pt);
	InvalidateRowHeader_AllWnd(pt.y);

	NoticeUpdateGridData();

	return ret_v;
}

void CGridCtrl::Cut()
{
	if(IsEnterEdit()) {
		m_edit_cell->Cut();
		return;
	}

	Copy();
	DeleteKey();
	NoticeUpdateGridData();
}

BOOL CGridCtrl::EnterEdit(BOOL b_cursor_first /* = FALSE */, BOOL b_focus /* = FALSE */)
{
	if(IsEnterEdit()) return FALSE;

	POINT pt = *m_grid_data->get_cur_cell();
	if(pt.x < 0 || pt.y < 0) return FALSE;
	if(!m_grid_data->IsEditable() || !m_grid_data->IsEditableCell(pt.y, pt.x)) return FALSE;

	ASSERT(m_edit_cell);

	PreEnterEdit();

	m_edit_cell->SetColor(BG_COLOR, GetCellBkColor(pt.y, pt.x));
	m_edit_cell->SetColor(TEXT_COLOR, GetColor(GRID_TEXT_COLOR));
	m_edit_cell->SetColor(PEN_COLOR, GetColor(GRID_MARK_COLOR));
	m_edit_cell->SetColor(SELECTED_COLOR, GetColor(GRID_SELECT_COLOR));
	m_edit_cell->SetColor(SEARCH_COLOR, GetColor(GRID_SEARCH_COLOR));

	int data_row = (m_gridStyle & GRS_SWAP_ROW_COL_MODE) ? pt.x : pt.y;
	if(!m_grid_data->IsUpdateCell(pt.y, pt.x) && !m_grid_data->IsDeleteRow(data_row) && !m_grid_data->IsInsertRow(data_row)) {
		m_edit_cell->SetModifiedBkColor(GetColor(GRID_UPDATE_COLOR));
		m_edit_cell->OnExStyle(ECS_MODIFIED_BK_COLOR);
	} else {
		m_edit_cell->OffExStyle(ECS_MODIFIED_BK_COLOR);
	}

	m_edit_cell->ResetDispInfo();

	if(m_search_data.GetDispSearch()) {
		m_edit_cell->SaveSearchData(m_search_data.GetSearchText(),
			m_search_data.GetDistinctLwrUpr(), m_search_data.GetRegExp());
	} else {
		m_edit_cell->ClearSearchText();
	}

	// FIXME: singleline, 右寄せ/左寄せの設定を可能にする
	m_edit_data->del_all();
	m_edit_data->set_limit_text(m_grid_data->Get_ColLimit(pt.y, pt.x));
	m_edit_data->paste(GetEnterEditData(pt.y, pt.x));
	m_edit_data->reset_undo();
	if(b_cursor_first) m_edit_data->set_cur(0, 0);
	ClearSelected();

	ShowCell(&pt);
	m_edit_cell->NoticeUpdate();
	m_edit_cell->Redraw();
	m_edit_cell->ShowWindow(SW_SHOW);
	m_edit_cell->SetCaret(TRUE, 1);
	if(b_focus) m_edit_cell->SetFocus();

	SetEditCellPos();

	return TRUE;
}

void CGridCtrl::ClearIME()
{
	if(m_edit_cell) {
		HIMC hIMC = ::ImmGetContext(m_edit_cell->GetSafeHwnd());
		if(::ImmGetOpenStatus(hIMC)) {
			::ImmSetOpenStatus(hIMC, FALSE); // OFF
			::ImmSetOpenStatus(hIMC, TRUE);	 // ON
		}
		::ImmReleaseContext(m_edit_cell->GetSafeHwnd(), hIMC);
	}
}

BOOL CGridCtrl::LeaveEdit()
{
	if(!IsEnterEdit()) return FALSE;

	ClearIME();

	POINT pt = *m_grid_data->get_cur_cell();
	pt.x = m_grid_data->get_cur_col();
	if(pt.x < 0 || pt.y < 0) return FALSE;
	if(!m_grid_data->IsEditable() || !m_grid_data->IsEditableCell(pt.y, pt.x)) return FALSE;

	int edit_row = pt.y;
	if(m_gridStyle & GRS_SWAP_ROW_COL_MODE) edit_row = pt.x;

	BOOL b_update_row = m_grid_data->IsUpdateRow(edit_row);
	BOOL b_result = TRUE;

	// 編集結果をセット
	if(m_edit_data->is_edit_data()) {
		CString str = GetLeaveEditData();

		if(m_grid_data->UpdateCell(pt.y, pt.x, str.GetBuffer(0), -1) != 0) {
			// 編集結果の保存に失敗
			CString msg = m_grid_data->GetLastErrorMessage();
			if(!msg.IsEmpty()) {
				m_edit_data->del_all();	// MessageBoxのタイミングで2回目が実行される？
				MessageBox(msg, _T("Error"), MB_OK | MB_ICONERROR);
			}
			b_result = FALSE;
		}

		if(IsSplitterMode()) {
			InvalidateCell_AllWnd(&pt);
			InvalidateRowHeader_AllWnd(pt.y);
		} else if(!b_update_row && m_grid_data->IsUpdateRow(edit_row)) {
			InvalidateRowHeader_AllWnd(pt.y);
		}
	}

	m_edit_cell->ClearAll();
	m_edit_cell->ShowWindow(SW_HIDE);

	NoticeUpdateGridData();

	return b_result;
}

void CGridCtrl::InputEnter()
{
	POINT pt = *m_grid_data->get_cur_cell();
	if(pt.x < 0 || pt.y < 0) return;
	if(!m_grid_data->IsEditable() || !m_grid_data->IsEditableCell(pt.y, pt.x)) return;

	if(IsEnterEdit()) {
		m_edit_cell->Paste(_T("\n"));
		return;
	}
}

BOOL CGridCtrl::CanUndo()
{
	if(m_grid_data->IsEditable() == FALSE) return FALSE;
	if(!IsActiveSplitter()) return FALSE;
	
	if(IsEnterEdit()) {
		return m_edit_data->can_undo();
	}

	return m_grid_data->CanUndo();
}

BOOL CGridCtrl::CanRedo()
{
	if(m_grid_data->IsEditable() == FALSE) return FALSE;
	if(!IsActiveSplitter()) return FALSE;
	
	if(IsEnterEdit()) {
		return m_edit_data->can_redo();
	}

	return m_grid_data->CanRedo();
}

BOOL CGridCtrl::CanCut()
{
	if(m_grid_data->IsEditable() == FALSE) return FALSE;

	if(IsEnterEdit()) {
		return m_edit_cell->HaveSelected();
	}

	return m_grid_data->is_selected_cell();
}

BOOL CGridCtrl::CanCopy()
{
	if(IsEnterEdit()) {
		return m_edit_cell->CanCopy();
	}

	return m_grid_data->is_selected_cell();
}

BOOL CGridCtrl::CanCopyColumnName()
{
	if(CanCopy()) return TRUE;

	if(GetSelectedCol() >= 0) return TRUE;

	return FALSE;
}

BOOL CGridCtrl::CanPaste()
{
	if(m_grid_data->IsEditable() == FALSE) return FALSE;

	if(IsEnterEdit()) {
		return m_edit_cell->CanPaste();
	}

	COleDataObject	obj;

	if(obj.AttachClipboard() == FALSE) return FALSE;
	if(obj.IsDataAvailable(CF_UNICODETEXT) == FALSE) return FALSE;

	if(m_grid_data->is_selected_cell() == FALSE) return FALSE;

//	if(!HaveSelected() && m_grid_data->IsEditableCell(m_grid_data->get_cur_row(),	
//		m_grid_data->get_cur_col()) == FALSE) return FALSE;

	return TRUE;
}

void CGridCtrl::ValidateEditCell()
{
	SelChanged(m_grid_data->get_cur_cell());
}

void CGridCtrl::SetConversionWindow(int x, int y)
{
	if(GetFocus() != this) return;

	HIMC	hIMC;
	COMPOSITIONFORM cf;

	hIMC = ImmGetContext(this->GetSafeHwnd());
	if (x>=0) {
		cf.dwStyle = CFS_POINT;
		cf.ptCurrentPos.x = x;
		cf.ptCurrentPos.y = y;
	} else {
		cf.dwStyle = CFS_DEFAULT;
	}
	ImmSetCompositionWindow(hIMC,&cf);

	ImmReleaseContext(this->GetSafeHwnd(),hIMC);
}

int CGridCtrl::GetScrollLeftMargin()
{
	if(!IsFixColMode()) return m_row_header_width;

	int		result = m_row_header_width;
	for(int x = m_fix_start_col; x < m_fix_end_col; x++) {
		result += GetDispColWidth(x);
	}
	return result;
}

int CGridCtrl::GetScrollTopMargin()
{
	return m_col_header_height;
}

int CGridCtrl::GetHScrollSizeLimit()
{
	int	colwidth = m_row_header_width;
	for(int i = 0; i < m_grid_data->Get_ColCnt(); i++) colwidth += GetDispColWidth(i);
	return colwidth;
}

int CGridCtrl::GetVScrollSizeLimit()
{
	return m_row_height * (m_grid_data->Get_RowCnt()) + m_col_header_height;
}

int CGridCtrl::GetHScrollSize(int xOrig, int x)
{
	return GetScrSize(xOrig, x);
}

int CGridCtrl::GetVScrollSize(int yOrig, int y)
{
	return (yOrig - y) * m_row_height;
}

int CGridCtrl::GetVScrollMin()
{
	return 0;
}

int CGridCtrl::GetVScrollMax()
{
	if(m_grid_data->Get_RowCnt() == 0) return 0;
	return m_grid_data->Get_RowCnt() - 1;
}

int CGridCtrl::GetVScrollPage()
{
	CRect		winrect;
	GetDispRect(winrect);
	return ((winrect.Height() - m_col_header_height) / m_row_height);
}

int CGridCtrl::GetHScrollMin()
{
	int min;
	for(min = m_fix_end_col; min < m_grid_data->Get_ColCnt() && GetDispColWidth(min) == 0; min++) ;
	return min;
}

int CGridCtrl::GetHScrollMax()
{
	if(m_grid_data->Get_ColCnt() == 0) return 0;
	return m_grid_data->Get_ColCnt() - 1;
}

int CGridCtrl::GetHScrollPage()
{
	CRect		winrect;
	GetDispRect(winrect);
	int page, width;
	int max_width = winrect.Width() - GetScrollLeftMargin();
	page = 0;
	width = 0;
	for(int i = m_grid_data->Get_ColCnt() - 1; i >= 0; i--) {
		width = width + GetDispColWidth(i);
		if(width >= max_width) break;
		page++;
	}
	return page;
}

void CGridCtrl::Scrolled(CSize sizeScroll, BOOL bThumbTrack)
{
	if(!bThumbTrack) Invalidate_AllWnd(FALSE);
	InvalidateCell(m_grid_data->get_cur_cell());
	SetEditCellPos();
}

BOOL CGridCtrl::HitSelectedArea(CPoint cell_point)
{
	if(HaveSelected() == FALSE) return FALSE;
	if(cell_point.x == -1 || cell_point.y == -1) return FALSE;

	if(cell_point.y < m_grid_data->GetSelectArea()->pos1.y || 
		cell_point.y > m_grid_data->GetSelectArea()->pos2.y) return FALSE;

	if(m_grid_data->GetSelectArea()->pos1.x < m_grid_data->GetSelectArea()->pos2.x) {
		if(cell_point.x < m_grid_data->GetSelectArea()->pos1.x || 
			cell_point.x > m_grid_data->GetSelectArea()->pos2.x) return FALSE;
	} else {
		if(cell_point.x > m_grid_data->GetSelectArea()->pos1.x || 
			cell_point.x < m_grid_data->GetSelectArea()->pos2.x) return FALSE;
	}

	return TRUE;
}

BOOL CGridCtrl::HaveSelectedCol(int col)
{
	if(!HaveSelected()) return FALSE;

	int row1 = min(m_grid_data->GetSelectArea()->pos1.y, m_grid_data->GetSelectArea()->pos2.y);
	int row2 = max(m_grid_data->GetSelectArea()->pos1.y, m_grid_data->GetSelectArea()->pos2.y);
	int col1 = min(m_grid_data->GetSelectArea()->pos1.x, m_grid_data->GetSelectArea()->pos2.x);
	int col2 = max(m_grid_data->GetSelectArea()->pos1.x, m_grid_data->GetSelectArea()->pos2.x);

	if(col >= col1 && col <= col2 && row1 == 0 && row2 == m_grid_data->Get_RowCnt() - 1) return TRUE;
	return FALSE;
}

BOOL CGridCtrl::HaveSelectedRow(int row)
{
	if(!HaveSelected()) return FALSE;

	int row1 = min(m_grid_data->GetSelectArea()->pos1.y, m_grid_data->GetSelectArea()->pos2.y);
	int row2 = max(m_grid_data->GetSelectArea()->pos1.y, m_grid_data->GetSelectArea()->pos2.y);
	int col1 = min(m_grid_data->GetSelectArea()->pos1.x, m_grid_data->GetSelectArea()->pos2.x);
	int col2 = max(m_grid_data->GetSelectArea()->pos1.x, m_grid_data->GetSelectArea()->pos2.x);

	if(row >= row1 && row <= row2 && col1 == 0 && col2 == m_grid_data->Get_ColCnt() - 1) return TRUE;
	return FALSE;
}

void CGridCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// 行ヘッダ
	int row = HitRowHeader(point);
	if(row != -1) {
		if(!HaveSelectedRow(row)) SelectRow(row);
		CScrollWnd::OnRButtonDown(nFlags, point);
		return;
	}

	// 列ヘッダ
	int col = HitColHeader(point);
	if(col != -1) {
		if(!HaveSelectedCol(col)) SelectCol(col);
		CScrollWnd::OnRButtonDown(nFlags, point);
		return;
	}

	// 全選択
	if(HitAllSelectArea(point)) {
		SelectAll();
		CScrollWnd::OnRButtonDown(nFlags, point);
		return;
	}

	// セル選択
	POINT	pt;
	HitCell(point, &pt);
	if(pt.x == -1 || pt.y == -1) {
		CScrollWnd::OnRButtonDown(nFlags, point);
		return;
	}

	// 選択範囲内
	if(HitSelectedArea(pt) == TRUE) {
		CScrollWnd::OnRButtonDown(nFlags, point);
		return;
	}

	m_grid_data->GetSelectArea()->select_mode = SELECT_MODE_NORMAL;
	SelChanged(&pt);
	
	CScrollWnd::OnRButtonDown(nFlags, point);
}

int CGridCtrl::ReplaceTextAll(const TCHAR *search_text, const TCHAR *replace_text, 
	BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp, BOOL b_selected_area, int *preplace_cnt)
{
	if(!m_grid_data->IsEditable()) return 1;

	CRegData reg_data;

	if(!reg_data.Compile2(search_text, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp)) {
		MessageBox(_T("不正な正規表現です"), _T("メッセージ"), MB_ICONINFORMATION | MB_OK);
		return 1;
	}

	POINT	cur_pt;
	POINT	searched_pt;

	if(b_selected_area) {
		cur_pt.y = min(m_grid_data->GetSelectArea()->pos1.y, m_grid_data->GetSelectArea()->pos2.y);
		cur_pt.x = min(m_grid_data->GetSelectArea()->pos1.x, m_grid_data->GetSelectArea()->pos2.x);
	} else {
		cur_pt.y = 0;
		cur_pt.x = 0;
	}

	m_grid_data->StartUndoSet();

	int replace_cnt = 0;

	BOOL	b_cur_cell = TRUE;	// 最初の１回目の検索のときは，現在のセルも調べる

	for(;;) {
		if(m_grid_data->SearchDataRegexp(cur_pt, &searched_pt, 1,  
			FALSE, FALSE, b_cur_cell, b_selected_area, reg_data.GetRegData()) == 0) break;

		b_cur_cell = FALSE;
		cur_pt = searched_pt;

		if(!m_grid_data->IsEditableCell(searched_pt.y, searched_pt.x)) continue;

		m_edit_data->del_all();
		m_edit_data->set_limit_text(m_grid_data->Get_ColLimit(searched_pt.y, searched_pt.x));
		m_edit_data->paste(GetEditData(searched_pt.y, searched_pt.x));
		m_edit_data->reset_undo();
		m_edit_data->set_cur(0, 0);

		m_edit_data->replace_str_all_regexp(replace_text, b_regexp, NULL, NULL, NULL, reg_data.GetRegData());
		if(m_edit_data->is_edit_data()) {
			CString str = GetLeaveEditData();
			if(m_grid_data->UpdateCell(searched_pt.y, searched_pt.x, str.GetBuffer(0), -1) == 0) replace_cnt++;
		}
	}

	m_grid_data->EndUndoSet();

	if(preplace_cnt != NULL) *preplace_cnt = replace_cnt;

	Invalidate_AllWnd();

	return 0;
}

int CGridCtrl::ReplaceText2(const TCHAR *search_text, const TCHAR *replace_text, int dir,
	BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp, BOOL b_all, BOOL b_selected_area,
	int *preplace_cnt, BOOL *b_looped)
{
	int result;

	if(m_grid_data->IsEditable() == FALSE) return 0;

	if(b_looped != NULL) *b_looped = FALSE;

	if(b_all) {
		if(b_selected_area && IsEnterEdit() && m_edit_cell->HaveSelected()) {
			result = m_edit_cell->ReplaceText2(search_text, replace_text, dir,
				b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp, b_all, b_selected_area, NULL, NULL);
		} else {
			result = ReplaceTextAll(search_text, replace_text, b_distinct_lwr_upr, b_distinct_width_ascii,
				b_regexp, b_selected_area, preplace_cnt);
		}
		NoticeUpdateGridData();
		return result;
	}

	if(IsEnterEdit() && m_edit_cell->HaveSelected() == FALSE) {
		ClearSelected();
		m_edit_data->set_cur(0, 0);
	}

	POINT cur_pt = *m_grid_data->get_cur_cell();
	if(SearchText2(search_text, dir, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp, TRUE, b_looped, TRUE) != 0) {
		return 0;
	}
	if(cur_pt.y != m_grid_data->get_cur_row() || cur_pt.x != m_grid_data->get_cur_col()) return 0;

	if(EnterEdit(TRUE, FALSE)) {
		m_edit_cell->ReplaceText2(search_text, replace_text, 1,
			b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp, TRUE, FALSE, NULL, NULL);
		LeaveEdit();
	}

	return SearchText2(search_text, dir, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp, TRUE, b_looped, FALSE);
}

int CGridCtrl::ReplaceText(const TCHAR *search_text, const TCHAR *replace_text, int dir,
	BOOL b_distinct_lwr_upr, BOOL b_regexp, BOOL b_all, BOOL b_selected_area, 
	int *preplace_cnt, BOOL *b_looped)
{
	return ReplaceText2(search_text, replace_text, dir, b_distinct_lwr_upr, TRUE, b_regexp, b_all,
		b_selected_area, preplace_cnt, b_looped);
}

void CGridCtrl::UnSelect()
{
	POINT old_pt = *(m_grid_data->get_cur_cell());
	m_grid_data->clear_selected_cell();
	InvalidateCell_AllWnd(&old_pt);
}

void CGridCtrl::SetColor(int type, COLORREF color)
{
	m_grid_data->GetDispData()->SetColor(type, color);

	switch(type) {
	case GRID_TEXT_COLOR:
		m_grid_data->SetDefaultTextColor(color);
		m_grid_data->GetDispData()->GetToolTip()->SetTextColor(color);

		// 互換性のための設定 (現在行のテキストを同じ色にする)
		SetColor(GRID_CUR_ROW_TEXT_COLOR, color);
		break;
	case GRID_BG_COLOR:
		m_grid_data->GetDispData()->GetToolTip()->SetBkColor(make_yellow_color(color, 0.9));
		break;
	case GRID_CUR_ROW_TEXT_COLOR:
		if(color == GetColor(GRID_TEXT_COLOR)) {
			m_change_active_row_text_color = FALSE;
		} else {
			m_change_active_row_text_color = TRUE;
		}
		break;
	}
}

COLORREF CGridCtrl::GetColor(int type)
{
	return m_grid_data->GetDispData()->GetColor(type);
}

BOOL CGridCtrl::IsActiveSplitter()
{
	// 分割モードのとき，最後にアクティブになったウィンドウか調べる
	if(IsSplitterMode() && !m_last_active_wnd) return FALSE;
	return TRUE;
}

void CGridCtrl::SetFixColMode(int start_col, int end_col)
{
	if(start_col < 0 || start_col >= m_grid_data->Get_ColCnt() ||
		end_col < 0 || end_col >= m_grid_data->Get_ColCnt()) {
		ClearFixColMode();
		return;
	}
	m_fix_start_col = start_col;
	m_fix_end_col = end_col;
	CheckScrollBar();
	Invalidate();
}

void CGridCtrl::ClearFixColMode()
{
	m_fix_start_col = 0;
	m_fix_end_col = 0;
	if(::IsWindow(m_hWnd)) {
		CheckScrollBar();
		SetScrollPos(SB_HORZ, 0);
		Invalidate();
	}
}

void CGridCtrl::GetFixColModeData(int &start_col, int &end_col)
{
	start_col = m_fix_start_col;
	end_col = m_fix_end_col;
}

BOOL CGridCtrl::MakeFixColData(int &start_col, int &end_col)
{
	start_col = GetScrollPos(SB_HORZ);

	CRect	rect;
	GetDispRect(rect);
	int		w;
	end_col = start_col;

	for(w = m_row_header_width; end_col < m_grid_data->Get_ColCnt() && w < rect.Width() - 10; end_col++) {
		w = w + GetDispColWidth(end_col);
	}

	if(end_col == m_grid_data->Get_ColCnt()) return FALSE;

	return TRUE;
}

void CGridCtrl::SetDispFlg_SelectedCol(BOOL flg)
{
	if(!HaveSelected()) return;

	int col1 = min(m_grid_data->GetSelectArea()->pos1.x, m_grid_data->GetSelectArea()->pos2.x);
	int col2 = max(m_grid_data->GetSelectArea()->pos1.x, m_grid_data->GetSelectArea()->pos2.x);

	for(int col = col1; col <= col2; col++) {
		m_grid_data->SetDispFlg(col, flg);
	}

	Update_AllWnd();

	if(col2 == m_grid_data->Get_ColCnt() - 1) {
		MoveCell(-1, 0);
	} else {
		MoveCell(1, 0);
	}
}

void CGridCtrl::SetDispAllCol()
{
	for(int col = 0; col <= m_grid_data->Get_ColCnt(); col++) {
		m_grid_data->SetDispFlg(col, TRUE);
	}

	Update_AllWnd();
}

void CGridCtrl::SaveSearchData2(const TCHAR *search_text, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp)
{
	m_search_data.SetDispSearch(TRUE);
	m_search_data.MakeRegData2(search_text, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp);
	Invalidate_AllWnd();
}

void CGridCtrl::SaveSearchData(const TCHAR *search_text, BOOL b_distinct_lwr_upr, BOOL b_regexp)
{
	SaveSearchData2(search_text, b_distinct_lwr_upr, TRUE, b_regexp);
}

void CGridCtrl::ClearSearchText()
{
	if(m_search_data.GetDispSearch() == FALSE) return;
	
	m_search_data.SetDispSearch(FALSE);
	Invalidate_AllWnd();
}

int CGridCtrl::SearchColumn2(const TCHAR *search_text, int dir, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp,
	BOOL b_loop, BOOL *b_looped)
{
	if(b_looped != NULL) *b_looped = FALSE;
	if(_tcscmp(search_text, _T("")) == 0) return 1;

	CRegData	reg_data;
	if(!reg_data.Compile2(search_text, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp)) {
		MessageBox(_T("不正な正規表現です"), _T("メッセージ"), MB_ICONINFORMATION | MB_OK);
		return 1;
	}

	int		result_col;

	if(!(m_gridStyle & GRS_SWAP_ROW_COL_MODE)) {
		if(m_grid_data->SearchColumnRegexp(m_grid_data->get_cur_col(), &result_col, dir,
			b_loop, b_looped, FALSE, reg_data.GetRegData()) == 0) {
			// 見つからない
			return 1;
		}
		SelectCol(result_col);
	} else {
		if(m_grid_data->SearchColumnRegexp(m_grid_data->get_cur_row(), &result_col, dir,
			b_loop, b_looped, FALSE, reg_data.GetRegData()) == 0) {
			// 見つからない
			return 1;
		}
		SelectRow(result_col);
	}

	return 0;
}

int CGridCtrl::SearchColumn(const TCHAR *search_text, int dir, BOOL b_distinct_lwr_upr, BOOL b_regexp, 
	BOOL b_loop, BOOL *b_looped)
{
	return SearchColumn2(search_text, dir, b_distinct_lwr_upr, TRUE, b_regexp, b_loop, b_looped);
}

int CGridCtrl::ToUpper()
{
	int replace_cnt = m_grid_data->ToUpper();
	Invalidate_AllWnd();
	return replace_cnt;
}

int CGridCtrl::ToLower()
{
	int replace_cnt = m_grid_data->ToLower();
	Invalidate_AllWnd();
	return replace_cnt;
}

int CGridCtrl::ToHankaku(BOOL b_alpha, BOOL b_kata)
{
	int replace_cnt = m_grid_data->ToHankaku(b_alpha, b_kata);
	Invalidate_AllWnd();
	return replace_cnt;
}

int CGridCtrl::ToZenkaku(BOOL b_alpha, BOOL b_kata)
{
	int replace_cnt = m_grid_data->ToZenkaku(b_alpha, b_kata);
	Invalidate_AllWnd();
	return replace_cnt;
}

void CGridCtrl::InputSequence(__int64 start_num, __int64 incremental)
{
	m_grid_data->InputSequence(start_num, incremental);
	Invalidate_AllWnd();
}

void CGridCtrl::SortData(int *col_no, int *order, int sort_col_cnt)
{
	CWaitCursor wait_cursor;

	m_grid_data->SortData(col_no, order, sort_col_cnt);
	Update_AllWnd();
}

void CGridCtrl::SetNullText(const TCHAR *null_text)
{
	m_null_text = null_text;
	m_null_text_len = m_null_text.GetLength();
}

void CGridCtrl::SetTopLeftCellColor(COLORREF color)
{
	m_grid_data->GetDispData()->SetTopLeftCellColor(color);
	Invalidate_AllWnd();
}

void CGridCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	HScroll(nSBCode);
}

void CGridCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	VScroll(nSBCode);
}
