/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"

#include "editdata.h"
#include "Renderer.h"
#include "FontWidth.h"
#include <windows.h>
#include <winspool.h>
#include <math.h>

#include "mbutil.h"

#define BYTEBUF_SIZE	1024 * 10

class PRINT_INFO {
public:
	PRINT_INFO() {
		printer_name = NULL;
		msg_buf = NULL;
		doc_name = NULL;
		print_page = FALSE;
		print_date = FALSE;
		_tcscpy(now_date, _T(""));
		hdc = 0;
		old_font = 0;
		wx = wy = logicx = logicy = row_height = char_width = chars_par_line = line_width = rows_par_page = 0;
		space = NULL;
		space_pixel.top = space_pixel.bottom = space_pixel.left = space_pixel.right = 0;
		row_num_digit = 0;
		font_width_data = NULL;
		renderer = NULL;
		num_width = 0;
	}

	TCHAR* printer_name;	// プリンタ名
	TCHAR* msg_buf;		// エラー情報
	TCHAR* doc_name;		// タイトル
	BOOL	print_page;
	BOOL	print_date;
	TCHAR	now_date[100];
	HDC		hdc;
	CFont	font;
	HFONT	old_font;
	int		wx;				// ページ全体のピクセル数
	int		wy;				// ページ全体のピクセル数
	int		logicx;			// インチあたりのピクセル数
	int		logicy;			// インチあたりのピクセル数
	int		row_height;		// 文字の高さのピクセル数
	int		char_width;
	int		chars_par_line;	// １行に出力可能な文字数
	int		line_width;
	int		rows_par_page;	// １ページに出力可能な行数
	RECT* space;			// 余白
	RECT	space_pixel;	// 余白(ピクセル)
	int		row_num_digit;	// 行番号の桁数(0のとき出力しない)
	CFontWidthData* font_width_data;
	CRenderer* renderer;
	int				num_width;
};

static void init_print_info(PRINT_INFO *pi)
{
}

static void print_edit_data_init(PRINT_INFO *pi, HDC hdc, TCHAR *doc_name, RECT *space,
	BOOL print_page, BOOL print_date, int row_num_digit)
{
	pi->hdc = hdc;
	pi->doc_name = doc_name;
	pi->space = space;
	pi->print_page = print_page;
	pi->print_date = print_date;
	pi->row_num_digit = row_num_digit;

	if(print_date) {
		CTime	now = CTime::GetCurrentTime();
		_tcscpy(pi->now_date, now.Format(_T("%Y-%m-%d %H:%M")).GetBuffer(0));
	}
}

static int calc_row_left(PRINT_INFO *pi)
{
	if(pi->row_num_digit > 0) {
		return pi->space_pixel.left + pi->num_width * (pi->row_num_digit + 2);
	}
	return pi->space_pixel.left;
}

static int print_edit_data_init_page(PRINT_INFO *pi, TCHAR *font_face_name, 
	int font_point, int line_len, CEditData *edit_data)
{
	SetMapMode(pi->hdc, MM_TEXT);

	// ページの情報を取得・設定
	pi->wx = GetDeviceCaps(pi->hdc, HORZRES);
	pi->wy = GetDeviceCaps(pi->hdc, VERTRES);
//	pi->wx = GetDeviceCaps(pi->hdc, PHYSICALWIDTH);
//	pi->wy = GetDeviceCaps(pi->hdc, PHYSICALHEIGHT);
	pi->logicx = GetDeviceCaps(pi->hdc, LOGPIXELSX);
	pi->logicy = GetDeviceCaps(pi->hdc, LOGPIXELSY);

	// フォント作成
	pi->font.CreatePointFont(font_point, font_face_name, CDC::FromHandle(pi->hdc));
	pi->old_font = (HFONT)SelectObject(pi->hdc, pi->font.GetSafeHandle());

	// 余白の設定
	pi->space_pixel.top = pi->space->top * (int)ceil(pi->logicy / 25.4) - 
		GetDeviceCaps(pi->hdc, PHYSICALOFFSETY);
	pi->space_pixel.bottom = pi->space->bottom * (int)ceil(pi->logicy / 25.4);

	pi->space_pixel.left = pi->space->left * (int)ceil(pi->logicx / 25.4) - 
		GetDeviceCaps(pi->hdc, PHYSICALOFFSETX);
	pi->space_pixel.right = pi->space->right * (int)ceil(pi->logicx / 25.4);

	// フォントの高さ，幅を取得する
	TEXTMETRIC	tm;
    GetTextMetrics(pi->hdc, &tm);
	pi->row_height = tm.tmHeight + tm.tmExternalLeading;
	pi->char_width = tm.tmAveCharWidth;
	if(line_len == -1) {
		pi->chars_par_line = (pi->wx - calc_row_left(pi) - pi->space_pixel.right) / pi->char_width;
		pi->line_width = (pi->wx - calc_row_left(pi) - pi->space_pixel.right);
	} else {
		pi->chars_par_line = line_len;
		pi->line_width = line_len * pi->char_width;
	}

	pi->rows_par_page = (pi->wy - pi->space_pixel.top - pi->space_pixel.bottom) / (pi->row_height);

	CDC dc;
	dc.Attach(pi->hdc);
	pi->font_width_data = new CFontWidthData();
	pi->font_width_data->InitFontWidth(&dc);
	if(edit_data) {
		pi->font_width_data->SetTabStop(edit_data->get_tabstop());
		pi->num_width = pi->font_width_data->GetFontWidth(&dc, '0', NULL);
	}

	dc.Detach();

	pi->renderer = new CRenderer();

	return 0;
}

static void start_doc(PRINT_INFO *pi)
{
	DOCINFO			docinfo;

	// 初期化
	memset(&docinfo, 0, sizeof(docinfo));

	docinfo.cbSize = sizeof(DOCINFO);
	docinfo.lpszDocName = pi->doc_name;

	// 印刷開始
	StartDoc(pi->hdc, &docinfo);
}

static void print_header_doc_name(PRINT_INFO *pi)
{
	if(pi->doc_name == NULL || _tcslen(pi->doc_name) == 0) return;

	int		dx, dy;

	dx = pi->space_pixel.left + (pi->chars_par_line - (int)_tcslen(pi->doc_name)) * pi->char_width / 2;
	dy = pi->space_pixel.top - (pi->row_height * 2);

	TextOut(pi->hdc, dx, dy, pi->doc_name, (int)_tcslen(pi->doc_name));
}

static void print_header_date(PRINT_INFO *pi)
{
	if(pi->print_date == FALSE) return;
	
	int		dx, dy;

	dx = pi->space_pixel.left + (pi->chars_par_line - (int)_tcslen(pi->now_date) - 1) * pi->char_width;
	dy = pi->space_pixel.top - (pi->row_height * 2);

	TextOut(pi->hdc, dx, dy, pi->now_date, (int)_tcslen(pi->now_date));
}

static void print_header(PRINT_INFO *pi)
{
	print_header_doc_name(pi);
	print_header_date(pi);
}

static void print_footer(PRINT_INFO *pi, int page)
{
	if(pi->print_page == FALSE) return;

	int		dx, dy;
	CString	page_str;
	page_str.Format(_T("%d"), page);

	dx = pi->space_pixel.left + (pi->chars_par_line - (int)_tcslen(page_str.GetBuffer(0))) * pi->char_width / 2;
	dy = pi->wy - pi->space_pixel.bottom + pi->row_height * 2;

	TextOut(pi->hdc, dx, dy, page_str.GetBuffer(0), (int)_tcslen(page_str.GetBuffer(0)));
}

static void next_row(PRINT_INFO *pi, int &dy, int &dx, int &page_row, int &page)
{
	dy = dy + pi->row_height;
	page_row++;
	if(page_row == pi->rows_par_page) {
		page_row = 0;
		dx = calc_row_left(pi);
		dy = pi->space_pixel.top;
		print_footer(pi, page);
		EndPage(pi->hdc);
		StartPage(pi->hdc);
		print_header(pi);
		page++;
	}
}

static void line_row_num(PRINT_INFO *pi, int dy)
{
	int line_x = calc_row_left(pi) - pi->char_width;
	MoveToEx(pi->hdc, line_x, dy, NULL);
	LineTo(pi->hdc, line_x, dy + pi->row_height);
}

static int print_edit_data_main(PRINT_INFO *pi, CEditData *edit_data)
{
	int		dx, dy;
	int		row;
	int		page;
	int		row_num;
	TCHAR	row_num_str[20];

	page = 1;
	dx = calc_row_left(pi);
	dy = pi->space_pixel.top;

	// ページ開始
	StartPage(pi->hdc);
	print_header(pi);

	int		page_row, row_len;
	page_row = 0;
	row_num = 0;

	CDC dc;
	dc.Attach(pi->hdc);

	for(row = 0; row < edit_data->get_row_cnt(); row++, row_num++) {
		if(pi->row_num_digit != 0) {
			_stprintf(row_num_str, _T("%*d"), pi->row_num_digit, row + 1);
			TextOut(pi->hdc, pi->space_pixel.left, dy, row_num_str, (int)_tcslen(row_num_str));
			line_row_num(pi, dy);
		}

		const TCHAR *p = edit_data->get_disp_row_text(row);

		row_len = (int)_tcslen(p);
		if(row_len == 0) {
			next_row(pi, dy, dx, page_row, page);
		} else {
			for(;;) {
				int tmp_len = edit_data->get_next_line_len(p, pi->line_width, &dc, pi->font_width_data);

				RECT rect;
				rect.top = dy;
				rect.bottom = rect.top + pi->row_height;
				rect.left = dx;
				rect.right = rect.left + pi->line_width;
				pi->renderer->TextOut2(&dc, &dc, p, tmp_len, rect, pi->font_width_data, dx);
				//TextOut(pi->hdc, dx, dy, p, tmp_len);

				next_row(pi, dy, dx, page_row, page);
				if(pi->row_num_digit != 0) {
					line_row_num(pi, dy);
				}

				p += tmp_len;
				row_len -= tmp_len;
				if(row_len == 0) break;
			}
		}
	}

	print_footer(pi, page);

	// ページ終了
	dc.Detach();
	EndPage(pi->hdc);

	return 0;
}

static void clear_print_info(PRINT_INFO *pi)
{
	delete pi->font_width_data;
	delete pi->renderer;
}

int print_edit_data(HDC hdc, TCHAR *doc_name, CEditData *edit_data,
	TCHAR *font_face_name, int font_point, RECT *space, BOOL print_page, 
	BOOL print_date, int line_len, int row_num_digit, TCHAR *msg_buf)
{
	PRINT_INFO		pi;

	init_print_info(&pi);

	print_edit_data_init(&pi, hdc, doc_name, space, print_page, print_date, row_num_digit);

	print_edit_data_init_page(&pi, font_face_name, font_point, line_len, edit_data);

	start_doc(&pi);

	print_edit_data_main(&pi, edit_data);

	SelectObject(pi.hdc, pi.old_font);

	EndDoc(pi.hdc);

	clear_print_info(&pi);

	return 0;
}

int get_print_info(HDC hdc, TCHAR *font_face_name, int font_point, RECT *space, 
	int line_len, int row_num_digit, int *rows_par_page, int *chars_par_page)
{
	PRINT_INFO		pi;

	init_print_info(&pi);

	print_edit_data_init(&pi, hdc, _T(""), space, FALSE, FALSE, row_num_digit);

	print_edit_data_init_page(&pi, font_face_name, font_point, line_len, NULL);

	*rows_par_page = pi.rows_par_page;
	*chars_par_page = pi.chars_par_line;

	clear_print_info(&pi);

	return 0;
}
