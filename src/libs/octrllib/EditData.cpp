/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "mbutil.h"

#include "EditData.h"

#include "indenter.h"
#include "ostrutil.h"
#include "hankaku.h"

#include "octrl_util.h"
#include "octrl_inline.h"
#include "octrl_msg.h"

#include <deque>

// static変数の定義
CStrToken CEditData::m_def_str_token;

//
// oregexp用の関数
//
static const TCHAR *__str_return = _T("\n");

__inline static const TCHAR *__get_char_buf_main(const void *src, INT_PTR row, INT_PTR col)
{
	const TCHAR *pbuf = ((CEditData *)src)->get_row_buf((int)row) + col;
	if(*pbuf == '\0' && row != (INT_PTR)((CEditData *)src)->get_row_cnt() - 1) return __str_return;
	return pbuf;
}

static const TCHAR *get_char_buf_edit_data(const void *src, OREG_POINT *pt)
{
	return __get_char_buf_main(src, pt->row, pt->col);
}

static const TCHAR *get_row_buf_len_edit_data(const void *src, INT_PTR row, size_t *len)
{
	if(row >= ((CEditData *)src)->get_row_cnt()) return NULL;

	*len = ((CEditData *)src)->get_row_len((int)row);
	return __get_char_buf_main(src, row, 0);
}

static const TCHAR *next_char_edit_data(const void *src, OREG_POINT *pt)
{
	TCHAR *p = ((CEditData *)src)->get_row_buf((int)(pt->row)) + (pt->col);
	if(*p != '\0') {
		pt->col += get_char_len(p);
	} else {
		ASSERT(pt->row < (INT_PTR)((CEditData *)src)->get_row_cnt() - 1);
		pt->row++;
		pt->col = 0;
	}
	return get_char_buf_edit_data(src, pt);
}

static const TCHAR *prev_char_edit_data(const void *src, OREG_POINT *pt)
{
	CEditData *data = (CEditData *)src;

	if(pt->col == 0) {
		if(pt->row > 0) {
			pt->row--;
			pt->col = data->get_row_len((int)(pt->row));
		} else {
			static const TCHAR *null_str = _T("");
			return null_str;
		}
	} else {
		TCHAR *p = data->get_row_buf((int)(pt->row));
		if(pt->col > 1 && is_lead_byte(p[pt->col - 2])) {
			pt->col -= 2;
		} else {
			pt->col--;
		}
	}
	return get_char_buf_edit_data(src, pt);
}

static const TCHAR *next_row_edit_data(const void *src, OREG_POINT *pt)
{
	CEditData *data = (CEditData *)src;

	if(pt->row == (INT_PTR)data->get_row_cnt() - 1) return NULL;
	pt->col = 0;
	(pt->row)++;
	return get_char_buf_edit_data(src, pt);
}

static INT_PTR get_len_edit_data(const void *src, OREG_POINT *start_pt, OREG_POINT *end_pt)
{
	if(start_pt->row == end_pt->row) {
		return end_pt->col - start_pt->col;
	}

	CEditData *data = (CEditData *)src;
	POINT pt1, pt2;

	pt1.y = (LONG)(start_pt->row);
	pt1.x = (LONG)(start_pt->col);
	pt2.y = (LONG)(end_pt->row);
	pt2.x = (LONG)(end_pt->col);

	int result = data->get_row_len(pt1.y) - pt1.x;

	for(int i = pt1.y + 1; i < pt2.y; i++) {
		result = result + 1; // 改行
		result = result + data->get_row_len(i);
	}

	result = result + 1; // 改行
	result = result + pt2.x;

	return result;
}

static int is_row_first_edit_data(const void *src, OREG_POINT *cur_pt)
{
	if(cur_pt->col == 0) return 1;
	return 0;
}

static int is_selected_first_edit_data(const void *src, OREG_POINT *cur_pt)
{
	CEditData *data = (CEditData *)src;

	if(!data->get_disp_data()->HaveSelected()) return 0;
	if(data->get_disp_data()->GetSelectArea()->pos1.y != cur_pt->row) return 0;

	if(data->get_disp_data()->GetSelectArea()->box_select) {
		int		start_x, end_x;
		data->get_box_x2((int)(cur_pt->row), &start_x, &end_x);
		if((int)(cur_pt->col) == start_x) return 1;
	} else {
		if(data->get_disp_data()->GetSelectArea()->pos1.x == cur_pt->col) return 1;
	}

	return 0;
}

static int is_selected_last_edit_data(const void *src, OREG_POINT *cur_pt)
{
	CEditData *data = (CEditData *)src;

	if(!data->get_disp_data()->HaveSelected()) return 0;
	if(data->get_disp_data()->GetSelectArea()->pos2.y != cur_pt->row) return 0;

	if(data->get_disp_data()->GetSelectArea()->box_select) {
		int		start_x, end_x;
		data->get_box_x2((int)(cur_pt->row), &start_x, &end_x);
		if(cur_pt->col == end_x) return 1;
	} else {
		if(data->get_disp_data()->GetSelectArea()->pos2.x == cur_pt->col) return 1;
	}

	return 0;
}

static int is_selected_row_first_edit_data(const void *src, OREG_POINT *cur_pt)
{
	CEditData *data = (CEditData *)src;

	if(!data->get_disp_data()->HaveSelected()) return 0;

	// 行が選択範囲内か？
	if(data->get_disp_data()->GetSelectArea()->pos1.y > cur_pt->row ||
		data->get_disp_data()->GetSelectArea()->pos2.y < cur_pt->row) return 0;

	if(data->get_disp_data()->GetSelectArea()->box_select) {
		int		start_x, end_x;
		data->get_box_x2((int)(cur_pt->row), &start_x, &end_x);
		if(cur_pt->col == start_x) return 1;
	} else {
		if(data->get_disp_data()->GetSelectArea()->pos1.y == cur_pt->row) {
			if(data->get_disp_data()->GetSelectArea()->pos1.x == cur_pt->col) return 1;
		} else {
			if(cur_pt->col == 0) return 1;
		}
	}

	return 0;
}

static int is_selected_row_last_edit_data(const void *src, OREG_POINT *cur_pt)
{
	CEditData *data = (CEditData *)src;

	if(!data->get_disp_data()->HaveSelected()) return 0;

	// 行が選択範囲内か？
	if(data->get_disp_data()->GetSelectArea()->pos1.y > cur_pt->row ||
		data->get_disp_data()->GetSelectArea()->pos2.y < cur_pt->row) return 0;

	if(data->get_disp_data()->GetSelectArea()->box_select) {
		int		start_x, end_x;
		data->get_box_x2((int)(cur_pt->row), &start_x, &end_x);
		if(cur_pt->col == end_x) return 1;
	} else {
		if(data->get_disp_data()->GetSelectArea()->pos2.y == cur_pt->row) {
			if(data->get_disp_data()->GetSelectArea()->pos2.x == cur_pt->col) return 1;
		} else {
			if(cur_pt->col == data->get_row_len((int)(cur_pt->row))) return 1;
		}
	}

	return 0;
}

static void make_oreg_datasrc(CEditData *src, HREG_DATASRC data_src)
{
	memset(data_src, 0, sizeof(OREG_DATASRC));
	data_src->src = src;
	data_src->get_char_buf = get_char_buf_edit_data;
	data_src->get_row_buf_len = get_row_buf_len_edit_data;
	data_src->next_char = next_char_edit_data;
	data_src->prev_char = prev_char_edit_data;
	data_src->next_row = next_row_edit_data;
	data_src->get_len = get_len_edit_data;

	data_src->is_row_first = is_row_first_edit_data;
	data_src->is_selected_first = is_selected_first_edit_data;
	data_src->is_selected_last = is_selected_last_edit_data;
	data_src->is_selected_row_first = is_selected_row_first_edit_data;
	data_src->is_selected_row_last = is_selected_row_last_edit_data;
}
//
//
//

CEditData::CEditData()
{
	m_undo = NULL;
	
	m_tab_as_space = FALSE;
	m_indent_mode = INDENT_MODE_NONE;

	m_last_confinement = NULL;
	m_first_confinement = NULL;
	m_word_wrap = FALSE;
	m_confinement_flg = FALSE;

	m_str_token = &m_def_str_token;

	m_undo_redo_mode = 0;

	m_prev_input_char = '\0';

	init_editdata();
	init_undo();

	make_oreg_datasrc(this, &m_data_src);

	m_disp_data.GetDispColorDataPool()->SetEditData(this);
}

CEditData::~CEditData()
{
	free_editdata();
	free_undo();

	if(m_last_confinement != NULL) free(m_last_confinement);
	m_last_confinement = NULL;
	if(m_first_confinement != NULL) free(m_first_confinement);
	m_first_confinement = NULL;
}

int CEditData::init_editdata()
{
	free_editdata();

	m_char_cnt = 0;
	m_limit_char_cnt = -1;

	m_cur_pt.y = 0;
	m_cur_pt.x = 0;

	m_row_buffer.init_data();

	m_max_disp_width = 0;
	m_max_disp_width_row = m_row_buffer.get_edit_row_data(0);

	m_disp_col = 0;

	set_copy_line_type(LINE_TYPE_CR_LF);

	m_edit_seq = 0;

	return 0;
}

void CEditData::set_limit_text(int limit)
{
	if(limit == INT_MAX) limit = -1;
	m_limit_char_cnt = limit;
}

void CEditData::set_copy_line_type(int line_type)
{
	switch(line_type) {
	case LINE_TYPE_LF:
		_tcscpy(m_copy_line_type_str, _T("\n"));
		m_copy_line_type = LINE_TYPE_LF;
		break;
	case LINE_TYPE_CR:
		_tcscpy(m_copy_line_type_str, _T("\r"));
		m_copy_line_type = LINE_TYPE_CR;
		break;
	case LINE_TYPE_CR_LF:
	default:
		_tcscpy(m_copy_line_type_str, _T("\r\n"));
		m_copy_line_type = LINE_TYPE_CR_LF;
		break;
	}
	m_copy_line_type_len = (int)_tcslen(m_copy_line_type_str);
}

void CEditData::check_confinement_flg()
{
	m_confinement_flg = FALSE;
	if(m_word_wrap || m_last_confinement || m_first_confinement) m_confinement_flg = TRUE;
}

void CEditData::set_word_wrap(BOOL b_wrap)
{
	m_word_wrap = b_wrap;
	check_confinement_flg();
}

void CEditData::set_last_confinement(const TCHAR *str)
{
	if(m_last_confinement != NULL) {
		free(m_last_confinement);
		m_last_confinement = NULL;
	}
	if(str == NULL || _tcslen(str) == 0) {
		check_confinement_flg();
		return;
	}
	m_last_confinement = _tcsdup(str);
	check_confinement_flg();
}

void CEditData::set_first_confinement(const TCHAR *str)
{
	if(m_first_confinement != NULL) {
		free(m_first_confinement);
		m_first_confinement = NULL;
	}
	if(str == NULL || _tcslen(str) == 0) {
		check_confinement_flg();
		return;
	}
	m_first_confinement = _tcsdup(str);
	check_confinement_flg();
}

void CEditData::free_editdata()
{
	m_row_buffer.free_data();
}

int CEditData::init_undo()
{
	m_undo = new CUndo();
	m_saved_undo_sequence = get_undo_sequence();
	return 0;
}

void CEditData::reset_undo()
{
	m_undo->reset();
	set_all_edit_undo_sequence();
}

void CEditData::free_undo()
{
	if(m_undo != NULL) delete m_undo;
	m_undo = NULL;
}

void CEditData::set_undo_cnt(int cnt)
{
	free_undo();
	m_undo = new CUndo(cnt);
}

int CEditData::input_tab(CFontWidthHandler *dchandler)
{
	if(!m_tab_as_space) {
		return input_char('\t');
	}

	int tab_width = m_disp_data.GetTabWidth();

	int sp_w = m_disp_data.GetFontWidth(NULL, ' ', dchandler);
	sp_w += m_disp_data.GetCharSpace(sp_w);
	int cur_w = get_disp_width_pt(get_cur_pt(), dchandler) + sp_w;

	int w = 0;
	for(; cur_w > w; w += tab_width) ;

	fill_space(w, dchandler);

	clear_disp_col();

	return 0;
}

int CEditData::add_char(TCHAR data)
{
	if(data == '\n') return 0;

	// データ長がリミットを越えないようにする
	if(m_limit_char_cnt > 0) {
		int data_size = get_data_size();
		if(data == '\r') {
			if(data_size + 2 > m_limit_char_cnt) {
				MessageBeep(MB_ICONEXCLAMATION);
				return 1;
			}
		}
		if(data_size + 1 > m_limit_char_cnt) {
			// マルチバイト文字の先頭バイトが残らないようにする
			if(get_cur_col() > 0 && get_edit_row_data(m_cur_pt.y)->is_lead_byte(m_cur_pt.x - 1)) {
				delete_char();
			}
			MessageBeep(MB_ICONEXCLAMATION);
			return 1;
		}
	}

	if(data == '\r') return new_line();

	if(get_edit_row_data(m_cur_pt.y)->add_char(m_cur_pt.x, data) != 0) return 1;

	m_cur_pt.x++;

	if(m_undo->get_cur_operation() != 0) {
		m_undo->add_undo_char(data);
	}
	set_edit_undo_sequence(m_cur_pt.y);

	m_char_cnt++;
	return 0;
}

int CEditData::new_line()
{
	m_cur_pt.y++;
	m_row_buffer.new_line(m_cur_pt.y);

	if(m_cur_pt.x != get_edit_row_data(m_cur_pt.y - 1)->get_char_cnt() - 1) {
		// 行の途中
		get_edit_row_data(m_cur_pt.y)->add_chars(get_edit_row_data(m_cur_pt.y)->get_char_cnt() - 1,
			get_edit_row_data(m_cur_pt.y - 1)->get_buffer() + m_cur_pt.x,
			get_edit_row_data(m_cur_pt.y - 1)->get_char_cnt() - 1 - m_cur_pt.x);
		get_edit_row_data(m_cur_pt.y - 1)->delete_chars(m_cur_pt.x,
			get_edit_row_data(m_cur_pt.y - 1)->get_char_cnt() - 1 - m_cur_pt.x);

		get_edit_row_data(m_cur_pt.y)->set_data(get_edit_row_data(m_cur_pt.y - 1)->get_data());
		get_edit_row_data(m_cur_pt.y - 1)->set_data(0);
	}

	get_edit_row_data(m_cur_pt.y - 1)->set_edit_undo_sequence(get_undo_sequence());
	get_edit_row_data(m_cur_pt.y)->set_edit_undo_sequence(get_undo_sequence());
	
	set_cur_col(0);

	if(m_undo->get_cur_operation() != 0) {
		m_undo->add_undo_char('\r');
	}

	return 0;
}

void CEditData::move_cur_col(int cnt)
{
	if(cnt == 0) return;

	clear_cur_operation();

	move_col(cnt);

	clear_disp_col();
}

void CEditData::move_line_end()
{
	int col = get_row_len(get_cur_row()) - get_cur_col();
	move_cur_col(col);
}

void CEditData::move_document_end()
{
	move_cur_row(get_row_cnt() - 1 - get_cur_row());
	int col = get_row_len(get_cur_row()) - get_cur_col();
	move_cur_col(col);
}

void CEditData::move_cur_row(int cnt)
{
	if(cnt == 0) return;

	clear_cur_operation();

	move_row(cnt);

	set_cur(get_cur_row(), 0);
}

int CEditData::back_space()
{
	int		ret_v;

	m_edit_seq++;
	if(m_undo->get_cur_operation() != OPE_BACKSPACE) {
		m_undo->next_undo(OPE_BACKSPACE);
	}

	int	row_cnt = get_row_cnt();

	ret_v = back();

	clear_disp_col();

	m_undo->get_cur_undo_data()->set_row(m_cur_pt.y);
	m_undo->get_cur_undo_data()->set_col(m_cur_pt.x);

	notice_update();

	return ret_v;
}

void CEditData::set_cur_row(int row) 
{
	ASSERT(row >= 0 && row < get_row_cnt());
	m_cur_pt.y = row;
}

void CEditData::set_cur_col(int col) 
{
	ASSERT(col >= 0 && col < get_edit_row_data(m_cur_pt.y)->get_char_cnt());
	m_cur_pt.x = col;
}

void CEditData::set_cur(int row, int col)
{
	ASSERT(row >= 0 && row < get_row_cnt());

	clear_cur_operation();

	POINT pt = {col, row};
	set_valid_point(&pt);

	set_cur_row(pt.y);
	set_cur_col(pt.x);

	clear_disp_col();
}

void CEditData::get_point(int disp_row, int disp_col, POINT *pt)
{
	int		col;

	if(disp_row < 0) disp_row = 0;
	if(disp_row >= get_row_cnt()) disp_row = get_row_cnt() - 1;
	
	col = get_edit_row_data(disp_row)->get_col_from_disp_pos(disp_col, get_tabstop());

	if(get_edit_row_data(disp_row)->is_lead_byte(col - 1)) col++;

	if(col < 0) col = 0;
	if(col >= get_edit_row_data(disp_row)->get_char_cnt()) {
		col = get_edit_row_data(disp_row)->get_char_cnt() - 1;
	}

	pt->y = disp_row;
	pt->x = col;
}

void CEditData::set_max_disp_width_row(int row)
{
	m_max_disp_width = get_edit_row_data(row)->get_disp_width();
	m_max_disp_width_row = get_edit_row_data(row);
}

void CEditData::calc_disp_width(int row, CFontWidthHandler *dchandler)
{
	if(m_max_disp_width < get_edit_row_data(row)->calc_disp_width(&m_disp_data, dchandler, get_tabstop())) {
		set_max_disp_width_row(row);
	}
}

int CEditData::get_disp_width_pt(POINT pt, CFontWidthHandler *dchandler)
{
	return get_edit_row_data(pt.y)->get_disp_width(&m_disp_data, dchandler, get_tabstop(), pt.x);
}

int CEditData::get_x_from_width_pt(int row, int width, CFontWidthHandler *dchandler)
{
	return get_edit_row_data(row)->get_x_from_width(&m_disp_data, dchandler, get_tabstop(), width);
}

void CEditData::recalc_max_disp_width()
{
	int		i;

	m_max_disp_width = 0;
	m_max_disp_width_row = m_row_buffer.get_edit_row_data(0);

	for(i = 0; i < get_row_cnt(); i++) {
		if(m_max_disp_width < get_edit_row_data(i)->get_disp_width()) {
			set_max_disp_width_row(i);
		}
	}
}

int CEditData::delete_char()
{
	if(m_cur_pt.x == 0) {
		if(m_cur_pt.y == 0) return 0;

		if(m_undo->get_cur_operation() != 0) {
			m_undo->add_undo_char('\r');
		}

		set_cur_row(get_cur_row() - 1);
		set_cur_col(get_edit_row_data(m_cur_pt.y)->get_char_cnt() - 1);

		get_edit_row_data(m_cur_pt.y)->add_chars(get_edit_row_data(m_cur_pt.y)->get_char_cnt() - 1,
			get_edit_row_data(m_cur_pt.y + 1)->get_buffer(),
			get_edit_row_data(m_cur_pt.y + 1)->get_char_cnt() - 1);

// VC++互換にする
//		if(get_edit_row_data(m_cur_pt.y + 1)->get_char_cnt() - 1 > 0) {
//			get_edit_row_data(m_cur_pt.y)->set_data(get_edit_row_data(m_cur_pt.y + 1)->get_data());
//		}
		get_edit_row_data(m_cur_pt.y)->set_data(get_edit_row_data(m_cur_pt.y + 1)->get_data());

		m_row_buffer.delete_line(m_cur_pt.y + 1);
	} else {
		if(m_undo->get_cur_operation() != 0) {
			m_undo->add_undo_char(get_edit_row_data(m_cur_pt.y)->get_buffer()[m_cur_pt.x - 1]);
		}

		if(get_edit_row_data(m_cur_pt.y)->delete_char(m_cur_pt.x) != 0) return 1;

		m_cur_pt.x--;
		m_char_cnt--;
	}
	set_edit_undo_sequence(m_cur_pt.y);

	return 0;
}

int CEditData::replace_all(const TCHAR *pstr)
{
	int		ret_v;
	POINT	pt1, pt2;

	pt1.y = 0;
	pt1.x = 0;
	pt2 = get_end_of_text_pt();

	ret_v = replace_str(&pt1, &pt2, pstr);

	set_cur(0, 0);

	return ret_v;
}

CString CEditData::make_back_ref_str(POINT *pt1, int len, const TCHAR *replace_str,
	HREG_DATA reg_data)
{
	CString result;
	int result_size;
	POINT	last_pos;

	last_pos = *pt1;
	calc_find_last_pos(&last_pos, len);

	OREG_POINT	start_pt, end_pt;
	start_pt.row = pt1->y;
	start_pt.col = pt1->x;
	end_pt.row = last_pos.y;
	end_pt.col = last_pos.x;

	result_size = (int)oreg_make_replace_str(&m_data_src, &start_pt, &end_pt,
		replace_str, reg_data, NULL);

	oreg_make_replace_str(&m_data_src, &start_pt, &end_pt,
		replace_str, reg_data, result.GetBuffer(result_size));
	result.ReleaseBuffer();

	return result;
}

void CEditData::calc_find_last_pos(POINT *pt, int len)
{
	for(; len > 0; len--) {
		pt->x++;
		if(pt->x > get_row_len(pt->y)) {
			if(pt->y >= get_row_cnt() - 1) return;
			pt->y++;
			pt->x = 0;
		}
	}
}

int CEditData::get_search_text_regexp(POINT *pt, int dir, HREG_DATA reg_data, 
	const TCHAR *replace_str, CString *str)
{
	int len = search_text_regexp(pt, dir, FALSE, NULL, NULL, reg_data, NULL);
	if(len < 0) return len;

	// 後方参照を処理する
	*str = make_back_ref_str(pt, len, replace_str, reg_data);
	return len;
}

static int get_str_last_row_len(const TCHAR *p)
{
	int last_row_len = 0;
	for(; *p != '\0'; p++) {
		if(*p == '\n') {
			last_row_len = 0;
			continue;
		}
		last_row_len++;
	}
	return last_row_len;
}

int CEditData::replace_str_all_regexp(const TCHAR *reg_str, const TCHAR *replace_str,
	BOOL b_distinct_lwr_upr, int *preplace_cnt)
{
	CRegData reg_data;
	if(!reg_data.Compile(reg_str, b_distinct_lwr_upr, TRUE)) return 1;

	int ret_v = replace_str_all_regexp(replace_str, 
		TRUE, NULL, NULL, preplace_cnt, reg_data.GetRegData());

	return ret_v;
}

int CEditData::replace_str_all_regexp(const TCHAR *replace_str, 
	BOOL b_regexp, POINT *start_pt, POINT *end_pt, 
	int *preplace_cnt, HREG_DATA reg_data)
{
	return replace_str_all_regexp_main(replace_str, b_regexp, start_pt, end_pt,
		preplace_cnt, reg_data, FALSE, NULL);
}

int CEditData::replace_str_all_regexp_box(CFontWidthHandler *dchandler,
	const TCHAR *replace_str,
	BOOL b_regexp, int *preplace_cnt, HREG_DATA reg_data)
{
	return replace_str_all_regexp_main(replace_str, b_regexp, 
		&m_disp_data.GetSelectArea()->pos1,
		&m_disp_data.GetSelectArea()->pos2,
		preplace_cnt, reg_data, TRUE, dchandler);
}

int CEditData::replace_str_all_regexp_main(const TCHAR *replace_str,
	BOOL b_regexp, POINT *start_pt, POINT *end_pt, 
	int *preplace_cnt, HREG_DATA reg_data,
	BOOL box, CFontWidthHandler *dchandler)
{
	int		ret_v;
	POINT	pt1, pt2;
	POINT	cur_pt;
	POINT	prev_pos = {-1, -1};
	int		len;
	int		replace_cnt = 0;
	POINT	next_pos = {-1, -1};
	int		next_len = -1;
	CString next_paste_str;

	int		end_pt_x_offset = 0;

	BOOL	b_need_make_replace_str = (b_regexp && have_replace_str(replace_str));
	
	CString paste_str = replace_str;

	// 現在のカーソル位置を保存
	cur_pt.y = get_cur_row();
	cur_pt.x = get_cur_col();

	// 選択範囲の末尾と行の末尾の間隔を計算する
	if(start_pt != NULL && end_pt != NULL) {
		end_pt_x_offset = get_row_len(end_pt->y) - end_pt->x;
	}

	// 先頭から末尾に向かって検索する
	if(start_pt != NULL && end_pt != NULL) {
		set_cur_row(start_pt->y);
		if(box) {
			int		box_start_x, box_end_x;
			get_box_x2(start_pt->y, &box_start_x, &box_end_x);
			set_cur_col(box_start_x);
		} else {
			set_cur_col(start_pt->x);
		}
	} else {
		set_cur_row(0);
		set_cur_col(0);
	}

	CUndoSetMode undo_obj(m_undo, FALSE);

	int		rep_start_x, rep_end_x, rep_end_x_offset;
	int		prev_box_y = -1;

	for(;;) {
		if(b_regexp && next_len >= 0) {
			len = next_len;
			pt1 = next_pos;
			next_len = -1;
			if(b_need_make_replace_str) paste_str = next_paste_str;
		} else {
			len = search_text_regexp(&pt1, 1, FALSE, NULL, NULL, reg_data, NULL);
			if(len == -1) break;
	
			// 後方参照を処理する
			if(b_need_make_replace_str) {
				paste_str = make_back_ref_str(&pt1, len, replace_str, reg_data);
			}
		}

		// 無限ループを検出
		if(prev_pos.y == pt1.y && prev_pos.x == pt1.x && len == 0) {
			if(is_end_of_text()) break;
			move_right();
			continue;
		}
		prev_pos.y = pt1.y;
		prev_pos.x = pt1.x;

		// 検索結果の終点を計算
		pt2 = pt1;
		calc_find_last_pos(&pt2, len);

		// 検索結果が選択範囲内にあるか調べる
		if(start_pt != NULL && end_pt != NULL) {
			if(end_pt->y < pt2.y) break;
			if(!box && end_pt->y == pt2.y && end_pt->x < pt2.x) break;

			if(box) {
				if(pt1.y != pt2.y) { // 検索結果が複数行のときは置換できない
					move_right();
					continue;
				}

				// 置換範囲を調整
				if(prev_box_y != pt1.y) {
					get_box_x2(pt1.y, &rep_start_x, &rep_end_x);
					rep_end_x_offset = get_row_len(pt2.y) - rep_end_x;
				}
				prev_box_y = pt1.y;

				if(pt1.x < rep_start_x) {
					move_right();
					continue;
				}
				if(get_row_len(pt2.y) - pt2.x < rep_end_x_offset) {
					if(end_pt->y == pt2.y) break;
					set_cur_row(get_cur_row() + 1);
					set_cur_col(0);
					continue;
				}
			}
		}

		// 正規表現のときは，文字列を置換する前に，次の検索位置を探す
		if(b_regexp) {
			set_cur(pt2.y, pt2.x);

			if(!is_end_of_text() || len > 0) {
				if(len == 0) move_right();
				next_len = search_text_regexp(&next_pos, 1, FALSE, NULL, NULL, reg_data, NULL);
				// 後方参照を処理する
				if(b_need_make_replace_str && next_len >= 0) {
					next_paste_str = make_back_ref_str(&next_pos, next_len, replace_str, reg_data);
				}
			}
		}

		if(!undo_obj.IsSetMode()) undo_obj.Start();	// UNDOスタート
		int old_row_cnt = get_row_cnt();			// 置換前の行数を覚えておく

		// 置換実行
		ret_v = del_main(&pt1, &pt2, FALSE);
		if(ret_v != 0) return ret_v;
		ret_v = paste(paste_str, FALSE);
		if(ret_v != 0) return ret_v;
		replace_cnt++;

		// 次の検索位置を調整する(置換でずれてしまうため)
		if(b_regexp && next_len >= 0) {
			if(pt2.y == next_pos.y) {
				next_pos.x += get_str_last_row_len(paste_str) - pt2.x;
				if(paste_str.Find('\n') == -1) next_pos.x += pt1.x;
			}
			next_pos.y += (get_row_cnt() - old_row_cnt);

			ASSERT(is_valid_point(next_pos));
			if(!is_valid_point(next_pos)) break;
		}

		// 置換の結果、行の数が変わったとき、選択範囲を調整する
		if(end_pt != NULL && old_row_cnt != get_row_cnt()) {
			end_pt->y += get_row_cnt() - old_row_cnt;
		}

		// 最終行の置換のとき，選択範囲を調整する
		if(end_pt != NULL && pt1.y == end_pt->y) {
			end_pt->x = get_row_len(end_pt->y) - end_pt_x_offset;
		}

		// 正規表現で，次の検索結果がないときは終了
		if(b_regexp && next_len < 0) break;
	}

	clear_disp_col();

	// 範囲指定の置換のとき，カーソル位置を調整する
	if(start_pt != NULL && end_pt != NULL) {
		if(start_pt->y != cur_pt.y || start_pt->x != cur_pt.x) {
			cur_pt = *end_pt;
		}
	}
	// カーソルを元に戻す
	if(cur_pt.y >= get_row_cnt()) cur_pt.y = get_row_cnt() - 1;
	if(cur_pt.x > get_row_len(cur_pt.y)) cur_pt.x = get_row_len(cur_pt.y);
	set_cur(cur_pt.y, cur_pt.x);

	if(preplace_cnt != NULL) *preplace_cnt = replace_cnt;

	return 0;
}

int CEditData::filter_str(int start_row, int end_row, int *pdel_row_cnt, 
	HREG_DATA reg_data, BOOL b_del_flg)
{
	if(start_row < 0) start_row = 0;
	if(end_row < 0) end_row = get_row_cnt() - 1;

	int		del_row_cnt = 0;
	set_cur(start_row, 0);

	CUndoSetMode undo_obj(m_undo, FALSE);

	for(;;) {
		POINT	pt1, pt2;
		int cur_row = get_cur_row();

		int len = search_text_regexp(&pt1, 1, FALSE, NULL, NULL, reg_data, NULL);
		if(b_del_flg && (len == -1 || pt1.y > end_row)) break;
		
		if(!undo_obj.IsSetMode()) undo_obj.Start();	// UNDOスタート

		pt2 = pt1;
		calc_find_last_pos(&pt2, len);

		int del_row1, del_row2;
		if(b_del_flg) {
			del_row1 = pt1.y;
			del_row2 = pt2.y;
		} else {
			del_row1 = cur_row;
			del_row2 = pt1.y - 1;
			if(len == -1 || del_row2 > end_row) del_row2 = end_row;
		}

		if(del_row1 <= del_row2 && !is_empty()) {
			del_rows(del_row1, del_row2);
			end_row -= (del_row2 - del_row1 + 1);
			pt2.y -= (del_row2 - del_row1 + 1);
			del_row_cnt += (del_row2 - del_row1 + 1);
		}

		if(len == -1 || pt2.y >= end_row) break;
		set_cur(pt2.y + 1, 0);
	}

	clear_disp_col();

	if(pdel_row_cnt != NULL) *pdel_row_cnt = del_row_cnt;

	return 0;
}

int CEditData::replace_str(POINT *pt1, POINT *pt2, const TCHAR *pstr)
{
	return replace_str_main(pt1, pt2, pstr);
}

int CEditData::replace_str_main(POINT *pt1, POINT *pt2, const TCHAR *pstr)
{
	int		ret_v;

	CUndoSetMode undo_obj(m_undo);
	save_cursor_pos();

	ret_v = del_main(pt1, pt2, FALSE);
	if(ret_v != 0) return ret_v;

	ret_v = paste(pstr, TRUE);
	if(ret_v != 0) return ret_v;

	return 0;
}

void CEditData::fill_space(int w, CFontWidthHandler *dchandler)
{
	int disp_w = get_disp_width_pt(get_cur_pt(), dchandler);
	int sp_w = m_disp_data.GetFontWidth(NULL, ' ', dchandler);
	sp_w += m_disp_data.GetCharSpace(sp_w);

	if(w - disp_w > sp_w / 2) {
		int cnt = (w - disp_w) / sp_w;
		for(int i = 0; i < cnt; i++) input_char(' ');
	}
}

int CEditData::replace_str_box(const TCHAR *pstr)
{
	int start_w;
	POINT pt1, pt2;
	BOOL b_delete = FALSE;

	if(m_disp_data.GetSelectArea()->box_start_w != m_disp_data.GetSelectArea()->box_end_w) {
		b_delete = TRUE;
	}

	if(m_disp_data.GetSelectArea()->box_start_w < m_disp_data.GetSelectArea()->box_end_w) {
		start_w = m_disp_data.GetSelectArea()->box_start_w;
	} else {
		start_w = m_disp_data.GetSelectArea()->box_end_w;
	}

	pt1 = m_disp_data.GetSelectArea()->pos1;
	pt2 = m_disp_data.GetSelectArea()->pos2;
	CFontWidthHandler *dchandler = get_disp_data()->GetSelectArea()->dchandler;

	TCHAR *pdup;
	pdup = _tcsdup(pstr);
	if(pdup == NULL) return 1;

	CUndoSetMode undo_obj(m_undo);

	int		row;
	int		ret_v;
	TCHAR *p1, *p2;
	p1 = pdup;

	for(row = pt1.y; row <= pt2.y; row++) {
		if(p1 != NULL) {
			p2 = my_mbschr(p1, '\n');
			if(p2 != NULL) {
				if(*(p2 - 1) == '\r') *(p2 - 1) = '\0';
				*p2 = '\0';
				p2++;
			}
		}

		if(b_delete) {
			int start_x, end_x;
			get_box_x2(row, &start_x, &end_x);

			POINT del_pt1, del_pt2;
			del_pt1.y = row;
			del_pt1.x = start_x;
			del_pt2.y = row;
			del_pt2.x = end_x;
		
			ret_v = del_main(&del_pt1, &del_pt2, FALSE);
			if(ret_v != 0) goto ERR1;

			fill_space(start_w, dchandler);
		}

		if(p1 != NULL) {
			ret_v = paste(p1, FALSE);
			if(ret_v != 0) goto ERR1;
		}

		p1 = p2;
	}

	// データが余った場合，普通にペーストする
	for(; p1 != NULL && p1[0] != '\0';) {
		p2 = my_mbschr(p1, '\n');
		if(p2 != NULL) {
			if(*(p2 - 1) == '\r') *(p2 - 1) = '\0';
			*p2 = '\0';
			p2++;
		}

		// 次の行に移動
		if(get_cur_row() + 1 < get_row_cnt()) {
			int x = get_edit_row_data(get_cur_row() + 1)->get_x_from_width(&m_disp_data, dchandler, get_tabstop(), start_w);
			set_cur(get_cur_row() + 1, x);
		} else {
			int col = get_row_len(get_cur_row()) - get_cur_col();
			move_cur_col(col);
			paste(_T("\n"), FALSE);
		}

		fill_space(start_w, dchandler);

		ret_v = paste(p1, FALSE);
		if(ret_v != 0) goto ERR1;

		p1 = p2;
	}

	clear_disp_col();

	free(pdup);
	
	return 0;

ERR1:
	free(pdup);
	return ret_v;
}

int CEditData::paste_main(const TCHAR *pstr)
{
	const TCHAR	*p;

	for(p = pstr; *p != '\0'; p++) {
		if(*p == '\n') {
			// '\r'なしで'\n'がきたときの処理
			if(p == pstr || *(p-1) != '\r') {
				if(add_char('\r') != 0) return 1;
			}
		} else {
			if(add_char(*p) != 0) return 1;
		}
	}
	return 0;
}

int CEditData::paste_no_undo(const TCHAR * pstr)
{
	// 高速化
	clear_cur_operation();
	return fast_paste_main(pstr);
}

int CEditData::fast_new_line(CEditRowData **&new_rows, int &new_rows_alloc_cnt,
	int &new_row_cnt)
{
	if(new_rows_alloc_cnt == new_row_cnt) {
		int alloc_cnt = new_row_cnt * 2;
		if(alloc_cnt == 0) alloc_cnt = 256;

		new_rows = (CEditRowData **)realloc(new_rows, sizeof(CEditRowData *) * alloc_cnt);
		if(new_rows == NULL) return 1;

		new_rows_alloc_cnt = alloc_cnt;
	}

	new_rows[new_row_cnt] = new CEditRowData();

	m_cur_pt.y++;
	m_cur_pt.x = 0;

	new_row_cnt++;

	if(m_undo->get_cur_operation() != 0) {
		m_undo->add_undo_char('\r');
	}

	return 0;
}

/*----------------------------------------------------------------------
  strchrのinline版(探索する文字が2つ)
----------------------------------------------------------------------*/
__inline static const TCHAR *str_strchr2(const TCHAR *s, unsigned int c1, unsigned int c2)
{
/*
	// サロゲートペアが分割されてくると、下記のコードでは動作しない
	// get_char, get_char_lenを使わないようにする
	unsigned int ch;

	for( ; *s != '\0'; s += get_char_len(s) ) {
		ch = get_char(s);
		if( ch == c1 || ch == c2 ) {
			return s;
		}
	}
	return s;
*/
	for( ; *s != '\0'; s ++ ) {
		if( *s == c1 || *s == c2 ) {
			return s;
		}
	}
	return s;
}

int CEditData::fast_paste_main(const TCHAR * pstr)
{
	int				len;
	const TCHAR	*p1, *p2;

	CEditRowData	*row_data = m_row_buffer.get_edit_row_data(m_cur_pt.y);

	CEditRowData	**new_rows = NULL;
	int				new_rows_alloc_cnt = 0;
	int				new_row_cnt = 0;
	POINT			new_row_pt;

	for(p1 = pstr;;) {
		p2 = str_strchr2(p1, '\r', '\n');
		len = (int)(p2 - p1);

		if(row_data->add_chars(m_cur_pt.x, p1, len) != 0) return 1;
		m_char_cnt += len;

		if(m_undo->get_cur_operation() != 0) {
			if(m_undo->add_undo_chars(p1, len) != 0) return 1;
		}
		m_cur_pt.x += len;

		if(*p2 == '\0') break;

		if(new_rows == NULL) {
			new_row_pt.x = m_cur_pt.x;
			new_row_pt.y = m_cur_pt.y;
		}

		if(fast_new_line(new_rows, new_rows_alloc_cnt, new_row_cnt) != 0) return 1;
		row_data = new_rows[new_row_cnt - 1];

		// 改行コードが"\r\n"のときは，'\n'をスキップする
		if(*p2 == '\r' && *(p2 + 1) == '\n') p2++;

		p2++;
		p1 = p2;
	}

	// 複数行のとき，一時データを行バッファに追加する
	if(new_rows != NULL) {
		m_row_buffer.add_new_rows(new_rows, m_cur_pt.y, m_cur_pt.x, new_row_cnt, new_row_pt);

		for(int r = m_cur_pt.y - new_row_cnt; r <= m_cur_pt.y; r++) {
			get_edit_row_data(r)->set_edit_undo_sequence(get_undo_sequence());
		}

		free(new_rows);
	} else {
		get_edit_row_data(m_cur_pt.y)->set_edit_undo_sequence(get_undo_sequence());
	}

	return 0;
}

int CEditData::paste(const TCHAR * pstr, BOOL recalc)
{
	int row = m_cur_pt.y;

	m_edit_seq++;

	if(pstr != NULL && pstr[0] != '\0') {
		m_undo->next_undo(OPE_INPUT);
		m_undo->get_cur_undo_data()->set_row(m_cur_pt.y);
		m_undo->get_cur_undo_data()->set_col(m_cur_pt.x);

		if(m_limit_char_cnt > 0) {
			if(paste_main(pstr) != 0) goto ERR1;
		} else {
			if(fast_paste_main(pstr) != 0) goto ERR1;
		}
		clear_cur_operation();
	}

	if(recalc == TRUE) clear_disp_col();

	notice_update();

	return 0;

ERR1:
	clear_cur_operation();
	clear_disp_col();
	return 1;
}

int CEditData::del_all()
{
	m_disp_data.ClearSelected();
	init_editdata();
	reset_undo();

	return 0;
}

int CEditData::del(POINT *pt1, POINT *pt2, BOOL recalc)
{
	return del_main(pt1, pt2, recalc);
}

int CEditData::del_box(BOOL recalc)
{
	CUndoSetMode undo_obj(m_undo);

	int		row;
	int		ret_v;
	int		start_x, end_x;
	POINT	del_pt1, del_pt2;

	for(row = m_disp_data.GetSelectArea()->pos2.y; row >= m_disp_data.GetSelectArea()->pos1.y; row--) {
		get_box_x2(row, &start_x, &end_x);

		del_pt1.y = row;
		del_pt1.x = start_x;
		del_pt2.y = row;
		del_pt2.x = end_x;

		ret_v = del_main(&del_pt1, &del_pt2, FALSE);
		if(ret_v != 0) return ret_v;
	}

	clear_disp_col();

	return 0;
}

int CEditData::del_main(POINT *pt1, POINT *pt2, BOOL recalc, BOOL make_undo)
{
	int	r;

	m_edit_seq++;

	// UNDOデータを作成
	if(make_undo) {
		m_undo->next_undo(OPE_DELETE);
		m_undo->get_cur_undo_data()->set_row(pt1->y);
		m_undo->get_cur_undo_data()->set_col(pt1->x);

		TCHAR *p;

		if(pt1->y == pt2->y) {
			p = get_row_buf(pt1->y) + pt1->x;
			m_undo->add_undo_chars(p, pt2->x - pt1->x);
		} else {
			p = get_row_buf(pt1->y) + pt1->x;
			m_undo->add_undo_chars(p, get_edit_row_data(pt1->y)->get_char_cnt() - 1 - pt1->x);
			m_undo->add_undo_char('\r');
			for(r = pt1->y + 1; r < pt2->y; r++) {
				p = get_row_buf(r);
				m_undo->add_undo_chars(p, get_edit_row_data(r)->get_char_cnt() - 1);
				m_undo->add_undo_char('\r');
			}
			p = get_row_buf(pt2->y);
			m_undo->add_undo_chars(p, pt2->x);
		}

		clear_cur_operation();
	}

	// データを削除
	if(pt1->y == pt2->y) {
		get_edit_row_data(pt1->y)->delete_chars(pt1->x, pt2->x - pt1->x);
		m_char_cnt -= (pt2->x - pt1->x);
	} else {
		get_edit_row_data(pt1->y)->delete_chars(pt1->x, get_edit_row_data(pt1->y)->get_char_cnt() - 1 - pt1->x);
		m_char_cnt -= (get_edit_row_data(pt1->y)->get_char_cnt() - 1 - pt1->x);

		for(r = pt1->y + 1; r < pt2->y; r++) {
			get_edit_row_data(r)->delete_chars(0, get_edit_row_data(r)->get_char_cnt() - 1);
			m_char_cnt -= get_edit_row_data(r)->get_char_cnt() - 1;
		}

		get_edit_row_data(pt2->y)->delete_chars(0, pt2->x);
		m_char_cnt -= pt2->x;

		set_cur_row(pt2->y);
		set_cur_col(0);
		for(r = pt1->y; r < pt2->y; r++) {
			delete_char();
		}
	}
	set_cur_row(pt1->y);
	set_cur_col(pt1->x);

	if(make_undo) {
		get_edit_row_data(pt1->y)->set_edit_undo_sequence(get_undo_sequence());
	} else {
		set_edit_undo_sequence(pt1->y);
	}

	clear_disp_col();

	notice_update();

	return 0;
}

void CEditData::get_box_x2(int row, int *start_x, int *end_x)
{
	int start_w = m_disp_data.GetSelectArea()->box_start_w;
	int end_w = m_disp_data.GetSelectArea()->box_end_w;

	if(start_w > end_w) {
		int t = start_w;
		start_w = end_w;
		end_w = t;
	}

	CFontWidthHandler *dchandler = get_disp_data()->GetSelectArea()->dchandler;

	*start_x = get_edit_row_data(row)->get_x_from_width(&m_disp_data, dchandler, get_tabstop(), start_w);
	*end_x = get_edit_row_data(row)->get_x_from_width(&m_disp_data, dchandler, get_tabstop(), end_w);
}

int CEditData::calc_char_cnt_box()
{
	int		row;
	int		result = 0;

	for(row = m_disp_data.GetSelectArea()->pos2.y; row >= m_disp_data.GetSelectArea()->pos1.y; row--) {
		int start_x, end_x;
		get_box_x2(row, &start_x, &end_x);

		TCHAR *p = get_row_buf(row);
		result = result + m_copy_line_type_len; // 改行
		result = result + ostr_str_char_cnt(p + start_x, p + end_x);
	}

	return result;
}

int CEditData::calc_char_cnt(POINT *pt1, POINT *pt2)
{
	int		result;
	int		i;
	TCHAR *p;

	if(pt1->y == pt2->y) {
		int end_x = pt2->x;
		if(pt2->x > get_row_len(pt1->y)) end_x = get_row_len(pt1->y);
		p = get_row_buf(pt1->y);
		result = ostr_str_char_cnt(p + pt1->x, p + end_x);
		return result;
	}

	p = get_row_buf(pt1->y);
	result = ostr_str_char_cnt(p + pt1->x, NULL);

	for(i = pt1->y + 1; i < pt2->y; i++) {
		result = result + m_copy_line_type_len; // 改行
		result = result + ostr_str_char_cnt(get_row_buf(i), NULL);
	}

	result = result + m_copy_line_type_len; // 改行
	p = get_row_buf(pt2->y);
	result = result + ostr_str_char_cnt(p, p + pt2->x);

	return result;
}

int CEditData::calc_buf_size_box()
{
	int		result = 0;

	for(int row = m_disp_data.GetSelectArea()->pos1.y; row <= m_disp_data.GetSelectArea()->pos2.y; row++) {
		int start_x, end_x;
		get_box_x2(row, &start_x, &end_x);

		result += (end_x - start_x) + m_copy_line_type_len;
	}
	result++; // '\0';

	return result * sizeof(TCHAR);
}

int CEditData::calc_buf_size(POINT * pt1, POINT * pt2)
{
	int		result = 0;

	if(pt1->y == pt2->y) {
		int end_x = pt2->x;
		if(pt2->x > get_row_len(pt1->y)) end_x = get_row_len(pt1->y);
		result = end_x - pt1->x + 1;	// +1 for '\0'
	} else {
		result = get_edit_row_data(pt1->y)->get_char_cnt() - pt1->x - 1;

		for(int i = pt1->y + 1; i < pt2->y; i++) {
			result = result + m_copy_line_type_len; // 改行
			result = result + get_edit_row_data(i)->get_char_cnt() - 1;
		}

		result = result + m_copy_line_type_len; // 改行
		result = result + pt2->x;

		result = result + 1; // '\0'
	}

	return result * sizeof(TCHAR);
}

CString CEditData::get_all_text()
{
	POINT	pt1, pt2;

	pt1.y = 0;
	pt1.x = 0;
	pt2.y = get_row_cnt() - 1;
	pt2.x = get_row_len(pt2.y);

	int		buf_size = 	calc_buf_size(&pt1, &pt2);

	CString	str;

	copy_text_data(&pt1, &pt2, (TCHAR *)str.GetBuffer(buf_size), buf_size);
	
	str.ReleaseBuffer();

	return str;
}

int CEditData::copy_text_data(POINT *pt1, POINT *pt2, TCHAR *buf, int bufsize)
{
	// FIXME: bufsizeを超えないようにする
	int		i;
	TCHAR	*p;

	p = buf;

	if(pt1->y == pt2->y) {
		_tcsncpy(p, get_edit_row_data(pt1->y)->get_buffer() + pt1->x, (INT_PTR)pt2->x - pt1->x);
		p = p + ((INT_PTR)pt2->x - pt1->x);
		*p = '\0';
		return 0;
	}

	_tcsncpy(p, get_edit_row_data(pt1->y)->get_buffer() + pt1->x, 
		(INT_PTR)get_edit_row_data(pt1->y)->get_char_cnt() - pt1->x - 1);
	p = p + get_edit_row_data(pt1->y)->get_char_cnt() - pt1->x - 1;

	for(i = pt1->y + 1; i < pt2->y; i++) {
		_tcsncpy(p, m_copy_line_type_str, m_copy_line_type_len);
		p = p + m_copy_line_type_len;
		_tcsncpy(p, get_edit_row_data(i)->get_buffer(), (INT_PTR)get_edit_row_data(i)->get_char_cnt() - 1);
		p = p + get_edit_row_data(i)->get_char_cnt() - 1;
	}

	_tcsncpy(p, m_copy_line_type_str, m_copy_line_type_len);
	p = p + m_copy_line_type_len;
	_tcsncpy(p, get_edit_row_data(pt2->y)->get_buffer(), pt2->x);
	p = p + pt2->x;
	*p = '\0';

	return 0;
}

int CEditData::copy_text_data_box(TCHAR *buf, int bufsize)
{
	TCHAR	*p = buf;

	for(int row = m_disp_data.GetSelectArea()->pos1.y; row <= m_disp_data.GetSelectArea()->pos2.y; row++) {
		int start_x, end_x;
		get_box_x2(row, &start_x, &end_x);

		_tcsncpy(p, get_edit_row_data(row)->get_buffer() + start_x, (INT_PTR)end_x - start_x);
		p = p + end_x - start_x;

		_tcsncpy(p, _T("\n"), 1);
		p++;
	}
	*p = '\0';

	return 0;
}

int CEditData::input_char(unsigned int data)
{
	int		ret_v;

	CUndoSetMode undo_obj(m_undo, FALSE);

	m_edit_seq++;

	// 区切り文字の前後は，別のUndoにする
	if(m_undo->get_cur_operation() != OPE_INPUT || 
		(m_prev_input_char != data &&
			(m_str_token->isBreakChar(data) || m_str_token->isBreakChar(m_prev_input_char)))) {

		if((m_indent_mode == INDENT_MODE_SMART || m_indent_mode == INDENT_MODE_AUTO) && 
			data == '\r') {
			undo_obj.Start();
		}

		m_undo->next_undo(OPE_INPUT);
		m_undo->get_cur_undo_data()->set_row(m_cur_pt.y);
		m_undo->get_cur_undo_data()->set_col(m_cur_pt.x);
	}
	m_prev_input_char = data;

	int row_cnt = get_row_cnt();

	ret_v = add_char(data);

	static CIndenter code_indenter;

	if(m_indent_mode == INDENT_MODE_SMART && code_indenter.is_indent_char(data)) {
		if(data != '\r') undo_obj.Start();
		code_indenter.DoIndent(this, data);
	} else if(m_indent_mode == INDENT_MODE_AUTO && data == '\r') {
		// 自動インデント
		code_indenter.AutoIndent(this);
	}

	clear_disp_col();

	notice_update();

	return ret_v;
}

POINT CEditData::get_end_of_text_pt()
{
	POINT pt;

	pt.y = get_row_cnt() - 1;
	pt.x = get_row_len(pt.y);

	return pt;
}

BOOL CEditData::search_brackets_ex(unsigned int open_char, unsigned int close_char, 
	int start_cnt, POINT *p_end_pt, unsigned int end_char, int arrow, POINT *pt)
{
	ASSERT(arrow == 1 || arrow == -1);
	ASSERT(open_char <= 0xff && close_char <= 0xff && end_char <= 0xff);

	BOOL	find = FALSE;
	int		cnt = start_cnt;
	POINT	cur_pt = get_cur_pt();

	struct check_char_type_cache cache;

	// check_char_type_cacheを初期化
	cache.row = -1;
	cache.char_type_arr = cache.ini_buf;
	cache.char_type_arr_cnt = CHECK_CHAR_TYPE_CACHE_BUF_SIZE;

	// 検索終了位置を設定する
	POINT	end_pt;
	if(!p_end_pt || p_end_pt->y < 0 || p_end_pt->y >= get_row_cnt()) {
		if(arrow > 0) {
			end_pt = get_end_of_text_pt();
		} else {
			end_pt.y = 0;
			end_pt.x = 0;
		}
	} else {
		end_pt = *p_end_pt;
	}

	// 検索用のテーブルを作成
	char ch_tbl[256];
	memset(ch_tbl, 0, sizeof(ch_tbl));
	ch_tbl[open_char] = 1;
	ch_tbl[close_char] = 1;
	if(end_char != '\0') ch_tbl[end_char] = 1;

	// テキストを直接参照して高速化する
	if(arrow > 0) {
		TCHAR *p = get_row_buf(cur_pt.y) + cur_pt.x;
		TCHAR *p_end = get_row_buf(cur_pt.y) + get_row_len(cur_pt.y);
		
		for(;;) {
			// 終了位置の判定
			if(inline_pt_cmp(&cur_pt, &end_pt) >= 0) break;

			// 移動する
			if(p == p_end) {
				if(cur_pt.y == get_row_cnt() - 1) break;
				cur_pt.y++;
				cur_pt.x = 0;
				p = get_row_buf(cur_pt.y);
				p_end = get_row_buf(cur_pt.y) + get_row_len(cur_pt.y);
			} else {
				int ch_len = get_char_len(p);
				cur_pt.x += ch_len;
				p += ch_len;

				if(p > p_end) break;
			}

			unsigned int ch = ::get_char(p);
			if(ch < 256 && ch_tbl[ch] == 0) continue;
			if(ch == close_char && check_char_type(cur_pt.y, cur_pt.x, &cache) == CHAR_TYPE_NORMAL) cnt++;
			else if(ch == open_char && check_char_type(cur_pt.y, cur_pt.x, &cache) == CHAR_TYPE_NORMAL) cnt--;
			else if(ch == end_char && end_char != '\0' && check_char_type(cur_pt.y, cur_pt.x, &cache) == CHAR_TYPE_NORMAL) break;

			if(cnt == 0) {
				*pt = cur_pt;
				find = TRUE;
				break;
			}
		}
	} else {
		TCHAR *p = get_row_buf(cur_pt.y) + cur_pt.x;
		TCHAR *p_start = get_row_buf(cur_pt.y);
		
		for(;;) {
			// 終了位置の判定
			if(inline_pt_cmp(&cur_pt, &end_pt) <= 0) break;

			// 移動する
			if(cur_pt.x == 0) {
				if(cur_pt.y == 0) break;
				cur_pt.y--;
				cur_pt.x = get_row_len(cur_pt.y);
				p = get_row_buf(cur_pt.y) + get_row_len(cur_pt.y);
				p_start = get_row_buf(cur_pt.y);
			} else {
				cur_pt.x--;
				p--;
			}

			// ２バイト文字の間に位置しないようにする
			if(is_low_surrogate(*p) && cur_pt.x > 0) {
				cur_pt.x--;
				p--;
			}

			unsigned int ch = ::get_char(p);
			if(ch < 256 && ch_tbl[ch] == 0) continue;

			if(ch == close_char && check_char_type(cur_pt.y, cur_pt.x, &cache) == CHAR_TYPE_NORMAL) cnt++;
			else if(ch == open_char && check_char_type(cur_pt.y, cur_pt.x, &cache) == CHAR_TYPE_NORMAL) cnt--;
			else if(ch == end_char && end_char != '\0' && check_char_type(cur_pt.y, cur_pt.x, &cache) == CHAR_TYPE_NORMAL) break;

			if(cnt == 0) {
				*pt = cur_pt;
				find = TRUE;
				break;
			}
		}
	}

	if(cache.char_type_arr != NULL && cache.char_type_arr != cache.ini_buf) {
		free(cache.char_type_arr);
	}

	return find;
}

BOOL CEditData::search_brackets(unsigned int open_char, unsigned int close_char, 
	int start_cnt, POINT *pt)
{
	return search_brackets_ex(open_char, close_char, start_cnt, NULL, '\0', -1, pt);
}

int CEditData::undo()
{
	m_edit_seq++;

	CUndoData	*undo_data;

	clear_cur_operation();

	if(m_undo->get_cur_undo_data() == NULL) return 1;

	m_undo_redo_mode = MODE_UNDO;

	for(undo_data = m_undo->get_cur_undo_data()->get_last(); undo_data != NULL; undo_data = undo_data->get_prev()) {
		switch(undo_data->get_operation()) {
		case 0:		// dummy
			break;
		case OPE_INPUT:
			{
				if(undo_data->get_char_cnt() == 0) break;
				set_cur_row(undo_data->get_row());
				set_cur_col(undo_data->get_col());
				move_col((undo_data->get_char_cnt()));

				POINT pt1, pt2;
				pt1.y = undo_data->get_row();
				pt1.x = undo_data->get_col();
				pt2.y = get_cur_row();
				pt2.x = get_cur_col();

				del_main(&pt1, &pt2, FALSE, FALSE);
			}
			break;
		case OPE_BACKSPACE:
			{
				if(undo_data->get_char_cnt() == 0) break;
				set_cur_row(undo_data->get_row());
				set_cur_col(undo_data->get_col());

				for(int i = undo_data->get_char_cnt() - 1; i >= 0; i--) {
					add_char(undo_data->get_char(i));
				}
			}
			break;
		case OPE_DELETE:
			{
				if(undo_data->get_char_cnt() == 0) break;
				set_cur_row(undo_data->get_row());
				set_cur_col(undo_data->get_col());

				undo_data->add_null_char();
				fast_paste_main(undo_data->get_char_buf());

				set_cur_row(undo_data->get_row());
				set_cur_col(undo_data->get_col());
			}
			break;
		case OPE_SAVE_CURSOR_POS:
			set_cur_row(undo_data->get_row());
			set_cur_col(undo_data->get_col());
			break;
		case OPE_SWAP_ROW:
			swap_row(undo_data->get_row(), undo_data->get_col(), FALSE);
			break;
		}
	}
	clear_disp_col();

	m_undo->decr_undo_cnt();

	if(get_undo_sequence() == m_saved_undo_sequence) set_all_edit_undo_sequence();

	m_undo_redo_mode = 0;

	notice_update();

	return 0;
}

int CEditData::back()
{
	if(delete_char() != 0) return 1;

	if(get_edit_row_data(m_cur_pt.y)->is_lead_byte(m_cur_pt.x - 1)) {
		if(delete_char() != 0) return 1;
	}

	return 0;
}

int CEditData::delete_key()
{
	m_edit_seq++;

	if(m_undo->get_cur_operation() != OPE_DELETE) {
		m_undo->next_undo(OPE_DELETE);
	}

	int row_cnt = get_row_cnt();

	if(get_edit_row_data(m_cur_pt.y)->is_lead_byte(m_cur_pt.x)) {
		m_cur_pt.x++;
		if(delete_char() != 0) return 1;
		m_cur_pt.x++;
		if(delete_char() != 0) return 1;
	} else {
		move_right();
		if(delete_char() != 0) return 1;
	}

	clear_disp_col();

	m_undo->get_cur_undo_data()->set_row(m_cur_pt.y);
	m_undo->get_cur_undo_data()->set_col(m_cur_pt.x);

	notice_update();

	return 0;
}

void CEditData::move_right(POINT *pt)
{
	move_col(pt, 1);
}

void CEditData::move_left(POINT *pt)
{
	move_col(pt, -1);
}

void CEditData::move_col(POINT *pt, int cnt)
{
	int		i;

	if(cnt > 0) {
		for(i = 0; i < cnt; i++) {
			if(pt->x == get_row_len(pt->y)) {
				if(pt->y != get_row_cnt() - 1) {
					pt->y++;
					pt->x = 0;
				}
			} else {
				pt->x++;
			}
		}
	} else {
		for(i = 0; i > cnt; i--) {
			if(pt->x == 0) {
				if(pt->y != 0) {
					pt->y--;
					pt->x = get_row_len(pt->y);
				}
			} else {
				pt->x--;
			}
		}
	}

	// ２バイト文字の間に位置しないようにする
	if(pt->x != 0 && get_edit_row_data(pt->y)->is_lead_byte(pt->x - 1)) {
		if(cnt > 0) {
			pt->x++;
			if(pt->x >= get_edit_row_data(pt->y)->get_char_cnt()) {
				pt->x = get_edit_row_data(pt->y)->get_char_cnt() - 1;
			}
		}
		if(cnt < 0) pt->x--;
	}
}

void CEditData::move_row(POINT *pt, int cnt)
{
	pt->y += cnt;

	if(pt->y < 0) pt->y = 0;
	if(pt->y >= get_row_cnt()) pt->y = get_row_cnt() - 1;

	if(pt->x >= get_edit_row_data(pt->y)->get_char_cnt()) pt->x = get_row_len(pt->y);
	if(get_edit_row_data(pt->y)->is_lead_byte(pt->x - 1)) pt->x--;
}

int CEditData::redo()
{
	m_edit_seq++;

	CUndoData	*undo_data;

	clear_cur_operation();

	m_undo->incr_undo_cnt();
	if(m_undo->get_cur_undo_data() == NULL) {
		m_undo->decr_undo_cnt();
		return 1;
	}

	m_undo_redo_mode = MODE_REDO;

	for(undo_data = m_undo->get_cur_undo_data()->get_first(); undo_data != NULL; 
		undo_data = undo_data->get_next()) {
		switch(undo_data->get_operation()) {
		case 0:		// dummy
			break;
		case OPE_INPUT:
			{
				if(undo_data->get_char_cnt() == 0) break;
				set_cur_row(undo_data->get_row());
				set_cur_col(undo_data->get_col());

				undo_data->add_null_char();
				fast_paste_main(undo_data->get_char_buf());
			}
			break;
		case OPE_BACKSPACE:
		case OPE_DELETE:
			{
				if(undo_data->get_char_cnt() == 0) break;
				set_cur_row(undo_data->get_row());
				set_cur_col(undo_data->get_col());
				move_col((undo_data->get_char_cnt()));

				POINT pt1, pt2;
				pt1.y = undo_data->get_row();
				pt1.x = undo_data->get_col();
				pt2.y = get_cur_row();
				pt2.x = get_cur_col();

				del_main(&pt1, &pt2, FALSE, FALSE);
			}
			break;
		case OPE_SAVE_CURSOR_POS:
			set_cur_row(undo_data->get_row());
			set_cur_col(undo_data->get_col());
			break;
		case OPE_SWAP_ROW:
			swap_row(undo_data->get_row(), undo_data->get_col(), FALSE);
			break;
		}
	}
	clear_disp_col();

	if(get_undo_sequence() == m_saved_undo_sequence) set_all_edit_undo_sequence();

	m_undo_redo_mode = 0;

	notice_update();

	return 0;
}

unsigned int CEditData::get_pt_char(const POINT *pt)
{
	TCHAR *p = get_edit_row_data(pt->y)->get_buffer();

	int x = pt->x;
	if(x > 0 && ::is_lead_byte(p[x - 1])) x--;

	return ::get_char(p + x);
}

unsigned int CEditData::get_prev_char()
{
	if(m_cur_pt.y == 0 && m_cur_pt.x == 0) return '\0';

	unsigned int ch;

	move_left();
	ch = get_cur_char();
	move_right();

	return ch;
}

unsigned int CEditData::get_char(int row, int col)
{
	ASSERT(row >= 0 && row < get_row_cnt());
	ASSERT(col >= 0 && col < get_edit_row_data(row)->get_char_cnt());
	if(row < 0 || row >= get_row_cnt()) return '\0';	
	if(col < 0 || col >= get_edit_row_data(row)->get_char_cnt()) return '\0';

	TCHAR *p = get_edit_row_data(row)->get_buffer() + col;

	return ::get_char(p);
}

const TCHAR *CEditData::get_disp_row_text(int row)
{
	ASSERT(row >= 0 && row < get_row_cnt());
	if(row < 0 || row >= get_row_cnt()) return _T("");
	return get_edit_row_data(row)->get_buffer();
}

void CEditData::calc_del_rows_pos(int row1, int row2, POINT *pt1, POINT *pt2)
{
	pt1->x = 0;
	pt1->y = row1;
	pt2->x = 0;
	pt2->y = row2 + 1;

	// 最終行の処理
	if(pt2->y == get_row_cnt()) {
		if(row1 > 0) {
			pt1->y = row1 - 1;
			pt1->x = get_row_len(pt1->y);
		}
		pt2->y = row2;
		pt2->x = get_row_len(pt2->y);
	}
}

int CEditData::del_rows(int row1, int row2)
{
	POINT	pt1, pt2;
	calc_del_rows_pos(row1, row2, &pt1, &pt2);
	return del_main(&pt1, &pt2, TRUE);
}

int CEditData::strstr_dir_regexp(int row, int col, int dir, int *hit_len, HREG_DATA reg_data)
{
    OREG_POINT  search_start, search_end, result_start, result_end;

	search_start.row = row;
	search_start.col = col;
	search_end.row = row;
	search_end.col = get_row_len(row);

	if(dir == 1) {
		if(oreg_exec(reg_data, &m_data_src, &search_start, &search_end,
			&result_start, &result_end, 0) != OREGEXP_FOUND) {
			return -1;
		}

		*hit_len = (int)get_len_edit_data(this, &result_start, &result_end);
		return (int)result_start.col;
	}

	int			len;
	int			result_col;

	// 先頭から検索する
	search_start.col = 0;

	if(oreg_exec(reg_data, &m_data_src, &search_start, &search_end,
		&result_start, &result_end, 0) != OREGEXP_FOUND) {
		return -1;
	}
	len = (int)get_len_edit_data(this, &result_start, &result_end);
	result_col = (int)result_start.col;

	// 現在位置の場合
	if(col > 0 && len == 0) return -1;

	// 検索範囲を超えた場合
	if(col > 0 && (result_start.row != result_end.row || result_end.col > col)) {
		return -1;
	}

	// 最後か判断する
	int			tmp_len;
	for(;;) {
		search_start.row = row;
		search_start.col = result_col;
		search_end.row = row;
		search_end.col = get_row_len(row);

		if(search_start.col >= get_row_len((int)(search_start.row))) {
			if(search_start.row >= (INT_PTR)get_row_cnt() - 1) break;
		}

		m_data_src.next_char(m_data_src.src, &search_start);
		if(search_start.row != row) break;

		if(oreg_exec(reg_data, &m_data_src, &search_start, &search_end,
			&result_start, &result_end, 0) != OREGEXP_FOUND) break;

		tmp_len = (int)get_len_edit_data(this, &result_start, &result_end);
		if(col > 0 && (result_start.row != result_end.row || result_end.col > col)) break;

		result_col = (int)result_start.col;
		len = tmp_len;
	}

	*hit_len = len;
	return result_col;
}

//
//
int CEditData::search_text_loop_regexp(POINT *pt, int dir, int row, int loop_cnt, HREG_DATA reg_data)
{
	int		i;
	int		col;
	int		len;

	for(i = 0; i < loop_cnt; i++, row = row + dir) {
		col = strstr_dir_regexp(row, 0, dir, &len, reg_data);
		if(col >= 0) {
			pt->y = row;
			pt->x = col;
			return len;
		}
	}

	return -1;
}

int CEditData::search_text_regexp(const TCHAR *reg_str, POINT *pt,
	BOOL b_distinct_lwr_upr, POINT *start_pt)
{
	CRegData reg_data;
	if(!reg_data.Compile(reg_str, b_distinct_lwr_upr, TRUE)) return -1;

	int ret_v = search_text_regexp_end_pt(pt, 1, 
		FALSE, start_pt, NULL, NULL, reg_data.GetRegData(), NULL);

	return ret_v;
}

//
// 文字を探す
// text		検索文字列
// pt		見つかった位置
// dir		-1 上方向に検索，1 下方向に検索
//
int CEditData::search_text_regexp(POINT *pt, int dir, BOOL loop, POINT *start_pt, 
	HWND loop_msg_wnd, HREG_DATA reg_data, BOOL *b_looped)
{
	return search_text_regexp_end_pt(pt, dir, loop,
		start_pt, NULL, loop_msg_wnd, reg_data, b_looped);
}

// end_ptも指定できるようにしたバージョン
int CEditData::search_text_regexp_end_pt(POINT *pt, int dir,
	BOOL loop, POINT *start_pt, POINT *end_pt, 
	HWND loop_msg_wnd, HREG_DATA reg_data, BOOL *b_looped)
{
	int		row, loop_cnt;
	int		len = 0;
	int		start_row, start_col;

	if(b_looped != NULL) *b_looped = FALSE;

	if(start_pt == NULL) {
		start_row = get_cur_row();
		start_col = get_cur_col();
	} else {
		start_row = start_pt->y;
		start_col = start_pt->x;
	}

	// 不正なdirectionのときは，実行しない
	if(dir != 1 && dir != -1) return -1;

	// 現在のカーソル位置から検索
	if(dir == 1 || start_col != 0) {
		int col = strstr_dir_regexp(start_row, start_col, dir, &len, reg_data);
		if(col >= 0) {
			pt->y = start_row;
			pt->x = col;

			// end_ptが指定されたとき、検索範囲を超えているかチェックする
			if(end_pt) {
				if(dir == 1) {
					if(inline_pt_cmp(pt, end_pt) > 0) return -1;
				} else {
					if(inline_pt_cmp(pt, end_pt) < 0) return -1;
				}
			}

			return len;
		}
	}

	if(dir == 1) {
		// 現在の行から，下を検索
		row = start_row + 1;
		loop_cnt = get_row_cnt() - start_row - 1;

		if(end_pt) {
			loop_cnt = end_pt->y - start_row - 1;
		}
	} else {
		// 現在の行から，上を検索
		row = start_row - 1;
		loop_cnt = start_row;

		if(end_pt) {
			loop_cnt = start_row - end_pt->y;
		}
	}

	len = search_text_loop_regexp(pt, dir, row, loop_cnt, reg_data);
	if(len != -1) {
		// end_ptが指定されたとき、検索範囲を超えているかチェックする
		if(end_pt) {
			if(dir == 1) {
				if(inline_pt_cmp(pt, end_pt) > 0) return -1;
			} else {
				if(inline_pt_cmp(pt, end_pt) < 0) return -1;
			}
		}
		return len;
	}

	if(loop == TRUE) {
		if(loop_msg_wnd != NULL) {
			if(dir == 1) {
				if(MessageBox(loop_msg_wnd,
					_T("ファイルの終わりまで検索しました。\n")
					_T("ファイルの先頭から検索しますか？"),
					_T("Message"), MB_ICONQUESTION | MB_OKCANCEL) != IDOK) return -2;
			} else {
				if(MessageBox(loop_msg_wnd,
					_T("ファイルの先頭まで検索しました。\n")
					_T("ファイルの終わりから検索しますか？"),
					_T("Message"), MB_ICONQUESTION | MB_OKCANCEL) != IDOK) return -2;
			}
		}
		if(dir == 1) {
			// 先頭に戻って，現在の行まで検索
			row = 0;
			loop_cnt = start_row + 1;
		} else {
			// 末尾に戻って，現在の行まで検索
			row = get_row_cnt() - 1;
			loop_cnt = get_row_cnt() - start_row;
		}
		if(b_looped != NULL) *b_looped = TRUE;

		len = search_text_loop_regexp(pt, dir, row, loop_cnt, reg_data);
		if(len != -1) return len;
	}

	return -1;
}

void CEditData::set_tabstop(int tabstop)
{
	if(tabstop == 0) tabstop = 4;
	m_disp_data.GetFontData()->SetTabStop(tabstop);
	clear_disp_col();
}

BOOL CEditData::can_undo()
{
	return m_undo->can_undo();
}

BOOL CEditData::can_redo()
{
	return m_undo->can_redo();
}

int CEditData::swap_row(int row1, int row2, BOOL b_make_undo)
{
	m_edit_seq++;

	if(b_make_undo) {
		m_undo->next_undo(OPE_SWAP_ROW);
		m_undo->get_cur_undo_data()->set_row(row1);
		m_undo->get_cur_undo_data()->set_col(row2);
	}

	m_row_buffer.swap_row(row1, row2);

	// 最大幅の行を入れ替える
	if(is_max_disp_width_row(get_edit_row_data(row1))) set_max_disp_width_row(row2);
	if(is_max_disp_width_row(get_edit_row_data(row2))) set_max_disp_width_row(row1);

	notice_update();

	return 0;
}

int CEditData::reverse_rows(int start_row, int end_row)
{
	int		ret_v = 0;
	int		row1, row2;

	// 変更開始
	CUndoSetMode undo_obj(m_undo);
	save_cursor_pos();

	for(row1 = start_row, row2 = end_row; row1 < row2; row1++, row2--) {
		ret_v = swap_row(row1, row2);
		if(ret_v != 0) return ret_v;
	}

	return 0;
}

int CEditData::sort_rows_asc(int left, int right)
{
	int		i, last;
	int		ret_v;

	if(left >= right) return 0;

	ret_v = swap_row(left, (left + right) / 2);
	if(ret_v != 0) return ret_v;

	last = left;
	for(i = left + 1; i <= right; i++) {
		if(_tcscmp(get_row_buf(i), get_row_buf(left)) < 0) {
			ret_v = swap_row(++last, i);
			if(ret_v != 0) return ret_v;
		}
	}
	ret_v = swap_row(left, last);
	if(ret_v != 0) return ret_v;

	ret_v = sort_rows_asc(left, last - 1);
	if(ret_v != 0) return ret_v;
	ret_v = sort_rows_asc(last + 1, right);
	if(ret_v != 0) return ret_v;

	return 0;
}

int CEditData::sort_rows_desc(int left, int right)
{
	int		i, last;
	int		ret_v;

	if(left >= right) return 0;

	ret_v = swap_row(left, (left + right) / 2);
	if(ret_v != 0) return ret_v;

	last = left;
	for(i = left + 1; i <= right; i++) {
		if(_tcscmp(get_row_buf(i), get_row_buf(left)) > 0) {
			ret_v = swap_row(++last, i);
			if(ret_v != 0) return ret_v;
		}
	}
	ret_v = swap_row(left, last);
	if(ret_v != 0) return ret_v;

	ret_v = sort_rows_desc(left, last - 1);
	if(ret_v != 0) return ret_v;
	ret_v = sort_rows_desc(last + 1, right);
	if(ret_v != 0) return ret_v;

	return 0;
}

int CEditData::sort_rows(int start_row, int end_row, BOOL desc)
{
	int		ret_v;

	// 変更開始
	CUndoSetMode undo_obj(m_undo);
	save_cursor_pos();

	if(desc == FALSE) {
		ret_v = sort_rows_asc(start_row, end_row);
	} else {
		ret_v = sort_rows_desc(start_row, end_row);
	}

	return ret_v;
}

#define CHAR_TYPE_NULL		0
#define CHAR_TYPE_BREAK		1
#define CHAR_TYPE_SPACE		2
#define CHAR_TYPE_ASCII		3
#define CHAR_TYPE_M_MARK	10
#define CHAR_TYPE_M_ALPHA	11
#define CHAR_TYPE_M_HIRA	12
#define CHAR_TYPE_M_KATA	13
#define CHAR_TYPE_M_HALF_KATA	14
#define CHAR_TYPE_M_KANJI	15
#define CHAR_TYPE_M_HIRA_KATA	16

int CEditData::get_char_type(unsigned int ch)
{
	if(ch == '\0') return CHAR_TYPE_NULL;

	if(ch == ' ' || ch == '\t') return CHAR_TYPE_SPACE;

	// 区切り文字
	if(m_str_token->isBreakChar(ch)) return CHAR_TYPE_BREAK;

	// その他ASCII文字
	if(ch <= 0xff) return CHAR_TYPE_ASCII;

	// マルチバイト
	// 'ー'は特殊処理
	if(ch == 0x30fc) return CHAR_TYPE_M_HIRA_KATA;
	// 数字，アルファベット
	if(ch >= 0xff10 && ch <= 0xff19) return CHAR_TYPE_M_ALPHA;
	if(ch >= 0xff21 && ch <= 0xff3a) return CHAR_TYPE_M_ALPHA;
	if(ch >= 0xff41 && ch <= 0xff5a) return CHAR_TYPE_M_ALPHA;
	// 全角記号 (全角数字、アルファベット以外)
	if(ch >= 0xff01 && ch <= 0xff60) return CHAR_TYPE_M_MARK;
	// ひらがな
	if(ch >= 0x3040 && ch <= 0x309f) return CHAR_TYPE_M_HIRA;
	// カタカナ
	if(ch >= 0x30a0 && ch <= 0x30ff) return CHAR_TYPE_M_KATA;
	if(ch >= 0x31f0 && ch <= 0x31ff) return CHAR_TYPE_M_KATA;
	// 半角カタカナ
	if(ch >= 0xff61 && ch <= 0xff9f) return CHAR_TYPE_M_HALF_KATA;

	// 漢字
	return CHAR_TYPE_M_KANJI;
}

BOOL CEditData::same_char_type(unsigned int ch1, unsigned int ch2)
{
	int		type1 = get_char_type(ch1);
	int		type2 = get_char_type(ch2);

	if(type1 == type2) return TRUE;

	// 'ー'は，カタカナと平仮名と同じ種類と判断する
	if(type1 == CHAR_TYPE_M_HIRA_KATA) {
		if(type2 == CHAR_TYPE_M_HIRA || type2 == CHAR_TYPE_M_KATA) return TRUE;
	}
	if(type2 == CHAR_TYPE_M_HIRA_KATA) {
		if(type1 == CHAR_TYPE_M_HIRA || type1 == CHAR_TYPE_M_KATA) return TRUE;
	}

	return FALSE;
}

void CEditData::move_break_char(int dir)
{
	clear_cur_operation();

	if(dir > 0) {
		for(;;) {
			move_right();
			if(m_str_token->isBreakChar(get_cur_char())) break;
		}
	} else {
		for(; get_cur_col() > 0;) {
			move_left();
			if(m_str_token->isBreakChar(get_cur_char())) {
				move_right();
				break;
			}
		}
	}

	clear_disp_col();
}

static bool ch_arr_match(unsigned int ch, unsigned int *ch_arr, int ch_cnt)
{
	int i;
	for(i = 0; i < ch_cnt; i++) {
		if(ch == ch_arr[i]) return true;
	}
	return false;
}

void CEditData::move_chars(int dir, unsigned int *ch_arr, int ch_cnt)
{
	unsigned int	ch;

	clear_cur_operation();

	if(dir > 0) {
		if(is_end_of_line()) {
			move_right();
		} else {
			ch = get_cur_char();
			bool match_flg = ch_arr_match(ch, ch_arr, ch_cnt);
			for(;;) {
				if(is_end_of_line()) break;
				move_right();
				ch = get_cur_char();
				if(ch_arr_match(ch, ch_arr, ch_cnt) != match_flg) break;
			}
		}
	} else {
		if(is_first_of_line()) {
			move_left();
		} else {
			move_left();
			ch = get_cur_char();
			bool match_flg = ch_arr_match(ch, ch_arr, ch_cnt);
			for(;;) {
				if(is_first_of_line()) {
					break;
				}
				move_left();
				ch = get_cur_char();
				if(ch_arr_match(ch, ch_arr, ch_cnt) != match_flg) {
					move_right();
					break;
				}
			}
		}
	}

	clear_disp_col();
}

void CEditData::move_word(int dir)
{
	unsigned int	ch;

	clear_cur_operation();

	if(dir > 0) {
		ch = get_cur_char();
		for(;;) {
			move_right();
			if(get_cur_char() == '\0' || same_char_type(ch, get_cur_char()) == FALSE) break;
			if(get_char_type(ch) == CHAR_TYPE_M_HIRA_KATA) {
				ch = get_cur_char();
			}
		}
	} else {
		if(m_cur_pt.y == 0 && m_cur_pt.x == 0) return;

		move_left();
		ch = get_cur_char();
		for(;;) {
			if(m_cur_pt.y == 0 && m_cur_pt.x == 0) break;
			move_left();
			if(get_cur_char() == '\0' || same_char_type(ch, get_cur_char()) == FALSE) {
				move_right();
				break;
			}
			if(get_char_type(ch) == CHAR_TYPE_M_HIRA_KATA) {
				ch = get_cur_char();
			}
		}
	}

	clear_disp_col();
}

void CEditData::skip_space(int dir)
{
	unsigned int	ch;

	if(dir > 0) {
		for(;;) {
			ch = get_cur_char();
			if(ch == '\0' || get_char_type(ch) != CHAR_TYPE_SPACE) break;
			move_right();
		}
	} else {
		if(m_cur_pt.y == 0 && m_cur_pt.x == 0) return;

		move_left();
		for(;;) {
			if(m_cur_pt.y == 0 && m_cur_pt.x == 0) break;
			ch = get_cur_char();
			if(ch == '\0' || get_char_type(ch) != CHAR_TYPE_SPACE) {
				move_right();
				break;
			}
			move_left();
		}
	}
	clear_disp_col();
}

__inline void CEditData::set_edit_undo_sequence(int row)
{
	if(m_undo->get_cur_operation() == 0 && m_undo_redo_mode == 0) return;

	if(m_undo->get_cur_operation() != 0) {
		if(!is_edit_row(row)) {
			get_edit_row_data(row)->set_edit_undo_sequence(get_undo_sequence());
		}
	} else {
		if(m_undo_redo_mode == MODE_UNDO) {
			if(m_saved_undo_sequence < get_undo_sequence()) {
				if(get_edit_row_data(row)->get_edit_undo_sequence() == get_undo_sequence()) {
					get_edit_row_data(row)->set_edit_undo_sequence(-1);
				}
			} else {
				if(!is_edit_row(row)) {
					get_edit_row_data(row)->set_edit_undo_sequence(get_undo_sequence());
				}
			}
		} else {
			if(m_saved_undo_sequence < get_undo_sequence()) {
				if(!is_edit_row(row)) {
					get_edit_row_data(row)->set_edit_undo_sequence(get_undo_sequence());
				}
			} else {
				if(get_edit_row_data(row)->get_edit_undo_sequence() == get_undo_sequence()) {
					get_edit_row_data(row)->set_edit_undo_sequence(-1);
				}
			}
		}
	}
}

void CEditData::set_all_edit_undo_sequence()
{
	int		i;
	for(i = 0; i < get_row_cnt(); i++) {
		get_edit_row_data(i)->set_edit_undo_sequence(-1);
	}

	m_saved_undo_sequence = get_undo_sequence();
}

int CEditData::load_file(const TCHAR *file_name, TCHAR *msg_buf)
{
	CFile	file;
	CFileException	e;

	// 他プロセスが使用中のファイルでも共有違反にならないようにする
	if(!file.Open(file_name, CFile::modeRead|CFile::shareDenyNone, &e)) {
		if(msg_buf) {
			_tcscpy(msg_buf, _T(""));
			e.GetErrorMessage(msg_buf, 1000);
		}
		return 1;
	}

	CArchive ar(&file, CArchive::load);

	int		kanji_code = UnknownKanjiCode;
	int		line_type = LINE_TYPE_CR_LF;
	int		ret_v;

	ret_v = load_file(ar, kanji_code, line_type);

	if(ret_v != 0) {
		if(msg_buf) {
			_stprintf(msg_buf, _T("file load error"));
		}
	}

	ar.Close();
	file.Close();

	return ret_v;
}

int CEditData::load_file(CArchive &ar, int &kanji_code, int &line_type)
{
	if(kanji_code == UnknownKanjiCode) {
		kanji_code = CUnicodeArchive::CheckKanjiCode(&ar);
	}
	CUnicodeArchive uni_ar(&ar, kanji_code);
	line_type = uni_ar.CheckLineType();

	TCHAR	*p;

	del_all();
	for(;;) {
		p = uni_ar.Read();
		if(p == NULL) break;
		paste_no_undo(p);
	}
	clear_disp_col();
	set_cur(0, 0);
	reset_undo();

	return 0;
}

int CEditData::append_file(CArchive &ar, int &kanji_code, int &line_type)
{
	if(kanji_code == UnknownKanjiCode) {
		kanji_code = CUnicodeArchive::CheckKanjiCode(&ar);
	}
	CUnicodeArchive uni_ar(&ar, kanji_code);
	line_type = uni_ar.CheckLineType();

	POINT pt = { get_cur_col(), get_cur_row() };

	CUndoSetMode undo_obj(m_undo);

	TCHAR	*p;
	for(;;) {
		p = uni_ar.Read();
		if(p == NULL) break;
		paste(p, FALSE);
	}
	clear_disp_col();

	set_cur(pt.y, pt.x);
	save_cursor_pos();

	return 0;
}

int CEditData::save_file(const TCHAR *file_name, int kanji_code, int line_type)
{
	CFile		file(file_name, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone);
	CArchive	ar(&file, CArchive::store);
	return save_file(ar, kanji_code, line_type);
}

int CEditData::save_file_main(CUnicodeArchive &uni_ar)
{
	int		i;
	int		loop_cnt;

	loop_cnt = get_row_cnt() - 1;
	for(i = 0; i < loop_cnt; i++) {
		uni_ar.WriteString(get_row_buf(i));
		uni_ar.WriteNextLine();
	}
	// 最後の行は，改行をいれない
	if(get_row_cnt() != 0) {
		uni_ar.WriteString(get_row_buf(i));
	}

	uni_ar.Flush();

	return 0;
}

int CEditData::save_file(CArchive &ar, int kanji_code, int line_type)
{
	CUnicodeArchive uni_ar(&ar, kanji_code, line_type);
	save_file_main(uni_ar);
	set_all_edit_undo_sequence();
	return 0;
}

char CEditData::check_char_type(int row, int col, struct check_char_type_cache *cache)
{
	// FIXME: HTMLなどタグ言語とき，タグの外のquotは無視する処理が必要
	int				len;
	TCHAR	*p;
	int				cur_x;
	char			cur_type;

	if(cache && cache->row == row && cache->cur_x != 0) {
		if(col < cache->cur_x) return cache->char_type_arr[col];
		p = cache->p;
		cur_x = cache->cur_x;
		cur_type = cache->cur_type;
	} else {
		p = get_row_buf(row);
		cur_x = 0;
		cur_type = CHAR_TYPE_NORMAL;
		if(get_row_data_flg(row, ROW_DATA_COMMENT)) cur_type = CHAR_TYPE_IN_COMMENT;

		if(cache) {
			cache->row = row;
		}
	}

	for(; cur_x <= col;) {
		len = m_str_token->get_word_len(p);
		if(len == 0) break;

		if(cur_type == CHAR_TYPE_IN_QUOTE) cur_type = CHAR_TYPE_NORMAL;

		if(m_str_token->isQuoteStart(p)) {
			if(cur_type == CHAR_TYPE_IN_COMMENT) {
				len = 1;
			} else {
				cur_type = CHAR_TYPE_IN_QUOTE;
			}
		}

		if(cur_type == CHAR_TYPE_NORMAL) {
			if(m_str_token->isRowComment(p) == TRUE) {
				cur_type = CHAR_TYPE_IN_ROWCOMMENT;
				if(cache) {
					len = get_row_len(row) - cur_x;
				} else {
					break;
				}
			}
			if(m_str_token->isOpenComment(p) == TRUE) {
				len = m_str_token->GetOpenCommentLen();
				cur_type = CHAR_TYPE_IN_COMMENT;
			}
		} else if(cur_type == CHAR_TYPE_IN_COMMENT) {
			if(m_str_token->isCloseComment(p) == TRUE) {
				len = m_str_token->GetCloseCommentLen();
				if(cur_x + m_str_token->GetCloseCommentLen() > col) break;
				cur_type = CHAR_TYPE_NORMAL;
			}
		}

		if(cache) {
			if(cache->char_type_arr_cnt < cur_x + len) {
				int arr_size = cache->char_type_arr_cnt;
				if(arr_size == 0) arr_size = 1024;
				for(; arr_size < cur_x + len;) arr_size *= 2;

				if(arr_size < CHECK_CHAR_TYPE_CACHE_BUF_SIZE) arr_size = CHECK_CHAR_TYPE_CACHE_BUF_SIZE;

				if(cache->char_type_arr == cache->ini_buf) {
					cache->char_type_arr = (char *)malloc(arr_size);
					if(cache->char_type_arr == NULL) {
						return cur_type;
					}
					memcpy(cache->char_type_arr, cache->ini_buf, CHECK_CHAR_TYPE_CACHE_BUF_SIZE);
				} else {
					cache->char_type_arr = (char *)realloc(cache->char_type_arr, arr_size);
					if(cache->char_type_arr == NULL) {
						return cur_type;
					}
				}
				cache->char_type_arr_cnt = arr_size;
			}
			memset(cache->char_type_arr + cur_x, cur_type, len);
		}

		p = p + len;
		cur_x = cur_x + len;
	}

	if(cache) {
		cache->p = p;
		cache->cur_x = cur_x;
		cache->cur_type = cur_type;
		if(cur_type == CHAR_TYPE_IN_QUOTE) cache->cur_type = CHAR_TYPE_NORMAL;
	}

	return cur_type;
}

int CEditData::tab_to_space_all()
{
	// 全部のタブをスペースに変換する
	int		row;

	int		buf_size = 1024;
	TCHAR	*buf = (TCHAR *)malloc(buf_size * sizeof(TCHAR));

	for(row = 0; row < get_row_cnt(); row++) {
		TCHAR *pstr = get_row_buf(row);
		int need_size = (int)(ostr_calc_tabbed_text_size(pstr, get_tabstop()) + 1) * sizeof(TCHAR);
		if(buf_size < need_size) {
			for(; buf_size < need_size;) buf_size *= 2;
			buf = (TCHAR *)realloc(buf, buf_size);
		}

		ostr_tabbed_text_format(pstr, buf, get_tabstop(), 0);

		POINT pt1 = {0, row};
		POINT pt2 = {get_row_len(row), row};
		replace_str(&pt1, &pt2, buf);
	}

	free(buf);

	return 0;
}

int CEditData::tab_to_space(const POINT *start_pt, const POINT *end_pt)
{
	TCHAR	*p1, *p2;
	int				row;
	int				tab_cnt;
	POINT			pt1, pt2;
	int				ret_v;
	TCHAR			buf[1024];

	CUndoSetMode undo_obj(m_undo);

	for(row = start_pt->y; row <= end_pt->y; row++) {
		p1 = get_row_buf(row);
		p2 = p1;
		for(; *p2 == '\t'; p2++) ;
		tab_cnt = (int)(p2 - p1);
		if(tab_cnt == 0) continue;

		pt1.y = row;
		pt1.x = 0;
		pt2.y = row;
		pt2.x = tab_cnt;
		ret_v = del_main(&pt1, &pt2, FALSE);
		if(ret_v != 0) return ret_v;

		int space_cnt = tab_cnt * get_tabstop();
		if(space_cnt >= 1024) space_cnt = 1023;
		for(int i = 0; i < space_cnt; i++) buf[i] = ' ';
		buf[space_cnt] = '\0';
		ret_v = paste(buf, FALSE);
		if(ret_v != 0) return ret_v;
	}

	clear_disp_col();

	return 0;
}

int CEditData::space_to_tab(const POINT *start_pt, const POINT *end_pt)
{
	TCHAR	*p1, *p2;
	int				row;
	int				tab_cnt;
	POINT			pt1, pt2;
	int				ret_v;
	TCHAR			buf[1024];

	CUndoSetMode undo_obj(m_undo);

	for(row = start_pt->y; row <= end_pt->y; row++) {
		p1 = get_row_buf(row);
		p2 = p1;
		for(; *p2 == ' '; p2++) ;
		tab_cnt = (int)(p2 - p1) / get_tabstop();
		if(tab_cnt == 0) continue;

		pt1.y = row;
		pt1.x = 0;
		pt2.y = row;
		pt2.x = tab_cnt * get_tabstop();
		ret_v = del_main(&pt1, &pt2, FALSE);
		if(ret_v != 0) return ret_v;

		if(tab_cnt >= 1024) tab_cnt = 1023;
		for(int i = 0; i < tab_cnt; i++) buf[i] = '\t';
		buf[tab_cnt] = '\0';
		ret_v = paste(buf, FALSE);
		if(ret_v != 0) return ret_v;
	}

	clear_disp_col();

	return 0;
}

void CEditData::save_cursor_pos()
{
	m_undo->next_undo(OPE_SAVE_CURSOR_POS, TRUE);
	m_undo->get_cur_undo_data()->set_row(m_cur_pt.y);
	m_undo->get_cur_undo_data()->set_col(m_cur_pt.x);
}

void CEditData::move_last_edit_pos()
{
	clear_cur_operation();

	if(m_undo->get_cur_undo_data() == NULL) return;

	CUndoData *undo_data;

	for(undo_data = m_undo->get_cur_undo_data()->get_last(); undo_data != NULL; undo_data = undo_data->get_prev()) {
		switch(undo_data->get_operation()) {
		case 0:		// dummy
			break;
		case OPE_INPUT:
			set_cur_row(undo_data->get_row());
			set_cur_col(undo_data->get_col());
			move_col((undo_data->get_char_cnt()));
			return;
		case OPE_BACKSPACE:
		case OPE_DELETE:
			if(undo_data->get_char_cnt() == 0) break;
			set_cur_row(undo_data->get_row());
			set_cur_col(undo_data->get_col());
			return;
		}
	}
}

CString CEditData::get_selected_text_box()
{
	CString	buf = _T("");

	if(!get_disp_data()->HaveSelected()) return buf;

	int bufsize = calc_buf_size_box();

	copy_text_data_box(buf.GetBuffer(bufsize), bufsize);
	buf.ReleaseBuffer();

	return buf;
}

CString CEditData::get_selected_text()
{
	CString	buf = _T("");

	if(!get_disp_data()->HaveSelected()) return buf;

	return get_point_text(get_disp_data()->GetSelectArea()->pos1, get_disp_data()->GetSelectArea()->pos2);
}

CString CEditData::get_point_text(POINT pt1, POINT pt2)
{
	CString	buf = _T("");

	int bufsize = calc_buf_size(&pt1, &pt2);

	copy_text_data(&pt1, &pt2, buf.GetBuffer(bufsize), bufsize);
	buf.ReleaseBuffer();

	return buf;
}

CString CEditData::get_prev_word()
{
	POINT	cur_pt;
	cur_pt.y = get_cur_row();
	cur_pt.x = get_cur_col();

	move_break_char(-1);
	move_word(-1);
	CString word = get_word(GET_WORD_CURRENT_PT, GET_WORD_MOVE_METHOD_NORMAL);
	set_cur(cur_pt.y, cur_pt.x);

	return word;
}

CString CEditData::get_word(GET_WORD_OPT opt, GET_WORD_MOVE_METHOD_OPT move_method,
	UINT ex_opt)
{
	typedef void (CEditData::*MoveMethodFunc)(int);
	MoveMethodFunc	move_method_func;

	switch(move_method) {
	case GET_WORD_MOVE_METHOD_NORMAL:
		move_method_func = &CEditData::move_word;
		break;
	case GET_WORD_MOVE_METHOD_BREAK_CHAR:
		move_method_func = &CEditData::move_break_char;
		break;
	default:
		move_method_func = &CEditData::move_break_char;
		break;
	}

	POINT	cur_pt, pt1, pt2;

	// 現在位置を保存
	cur_pt.y = get_cur_row();
	cur_pt.x = get_cur_col();

	if(ex_opt & GET_WORD_EX_OPT_CONSIDER_QUOTE_WORD && 
		m_str_token->isQuoteStart(get_cur_buf())) {
		pt1.y = get_cur_row();
		pt1.x = get_cur_col();

		unsigned int quote_start_ch = get_cur_char();
		move_right();

		int skip_cnt;
		for(;;) {
			if(is_end_of_text()) break;

			if(m_str_token->isQuoteEnd(get_cur_buf(), quote_start_ch, &skip_cnt)) {
				move_right();
				break;
			} else {
				move_right();
				if(skip_cnt == 2) move_right();
			}
		}

		pt2.y = get_cur_row();
		pt2.x = get_cur_col();
	} else {
		// 単語の末尾位置を取得
		if(opt == GET_WORD_NORMAL) (this->*move_method_func)(1);
		pt2.y = get_cur_row();
		pt2.x = get_cur_col();

		// 単語の先頭位置を取得
		(this->*move_method_func)(-1);
		pt1.y = get_cur_row();
		pt1.x = get_cur_col();
	}

	if(ex_opt & GET_WORD_EX_OPT_MOVE_CURSOR) {
		// カーソル位置を単語の終了位置に移動
		set_cur(pt2.y, pt2.x);
	} else {
		// カーソル位置を戻す
		set_cur(cur_pt.y, cur_pt.x);
	}

	if(ex_opt & GET_WORD_EX_OPT_CONSIDER_QUOTE_WORD) {
		// クォートの単語を取得する場合、複数行のテキストも取得できるようにする
		if(pt1.y == pt2.y && pt1.x == pt2.x) return _T("");
	} else {
		if((pt1.y != pt2.y) || (pt1.y == pt2.y && pt1.x == pt2.x)) return _T("");
	}

	return get_point_text(pt1, pt2);
}

void CEditData::skip_comment_and_space()
{
	POINT pt1 = skip_comment_and_space(get_cur_pt(), FALSE, FALSE);
	set_cur(pt1.y, pt1.x);
}

void CEditData::skip_comment_and_space(BOOL b_skip_2byte_space)
{
	POINT pt1 = skip_comment_and_space(get_cur_pt(), b_skip_2byte_space, FALSE);
	set_cur(pt1.y, pt1.x);
}

void CEditData::skip_comment()
{
	POINT pt1 = skip_comment_and_space(get_cur_pt(), FALSE, TRUE);
	set_cur(pt1.y, pt1.x);
}

POINT CEditData::skip_comment_and_space(POINT pt, BOOL b_skip_2byte_space, BOOL b_comment_only)
{
	BOOL comment_flg = FALSE;
	TCHAR *p = get_row_buf(pt.y) + pt.x;
	int	ch_len;

	for(;;) {
		if(comment_flg == FALSE && get_str_token()->isOpenComment(p)) {
			comment_flg = TRUE;
			pt.x += get_str_token()->GetOpenCommentLen();
			p += get_str_token()->GetOpenCommentLen();
			continue;
		}
		if(comment_flg == TRUE && get_str_token()->isCloseComment(p)) {
			comment_flg = FALSE;
			pt.x += get_str_token()->GetCloseCommentLen();
			p += get_str_token()->GetCloseCommentLen();
			continue;
		}

		if(*p == '\0' || (!comment_flg && get_str_token()->isRowComment(p))) {
			if(pt.y >= get_row_cnt() - 1) {
				pt.x = get_row_len(pt.y);
				break;
			}
			pt.y++;
			pt.x = 0;
			p = get_row_buf(pt.y);
		} else if(comment_flg == TRUE || (!b_comment_only && (*p == ' ' || *p == '\t'))) {
			ch_len = get_char_len(p);
			pt.x += ch_len;
			p += ch_len;
		} else if(b_skip_2byte_space && ::get_char(p) == L'　') {
			ch_len = get_char_len(p);
			pt.x += ch_len;
			p += ch_len;
		} else {
			break;
		}
	}
	return pt;
}

POINT CEditData::move_pt(POINT pt, int cnt)
{
	move_col(&pt, cnt);
	return pt;
}


BOOL CEditData::is_valid_point(const POINT pt)
{
	POINT tmp_pt = pt;
	set_valid_point(&tmp_pt);
	
	return (tmp_pt.y == pt.y && tmp_pt.x == pt.x);
}

void CEditData::set_valid_point(POINT *pt)
{
	if(pt->y < 0) pt->y = 0;
	if(pt->y >= get_row_cnt()) pt->y = get_row_cnt() - 1;
	if(pt->x < 0) pt->x = 0;
	if(pt->x > get_row_len(pt->y)) pt->x = get_row_len(pt->y);

	// 2byte文字の間にならないようにする
	if(pt->x != 0 && get_edit_row_data(pt->y)->is_lead_byte(pt->x - 1)) {
		(pt->x)--;
	}
}

int CEditData::hankaku_zenkaku_convert(POINT *start_pt, POINT *end_pt, BOOL b_alpha, BOOL b_kata,
	BOOL b_hankaku_to_zenkaku)
{
	ASSERT(start_pt != NULL && end_pt != NULL);

	typedef int (* CONV_FUNC)(const TCHAR *str, TCHAR *buf,
		int b_alpha, int b_kata);
	CONV_FUNC	p_func;

	if(b_hankaku_to_zenkaku) {
		p_func = HankakuToZenkaku2;
	} else {
		p_func = ZenkakuToHankaku2;
	}

	POINT	cur_pt;

	// 現在のカーソル位置を保存
	cur_pt.y = get_cur_row();
	cur_pt.x = get_cur_col();

	// 先頭から末尾に向かって検索する
	set_cur_row(start_pt->y);
	set_cur_col(start_pt->x);

	TCHAR *p_str = NULL;
	TCHAR *p_last_row_buf = NULL;
	TCHAR *p_buf = NULL;

	int p_buf_size = 1024;
	p_buf = (TCHAR *)malloc(p_buf_size);
	if(p_buf == NULL) return 1;

	CUndoSetMode undo_obj(m_undo, FALSE);

	int		row;
	int		ret_v;
	POINT	pt1, pt2;

	for(row = start_pt->y; row <= end_pt->y; row++) {
		p_str = get_row_buf(row);
		if(row == end_pt->y) {
			p_last_row_buf = (TCHAR *)malloc(((INT_PTR)end_pt->x + 1) * sizeof(TCHAR));
			_tcsncpy(p_last_row_buf, p_str, end_pt->x);
			p_last_row_buf[end_pt->x] = '\0';
			p_str = p_last_row_buf;
		}
		if(row == start_pt->y) p_str += start_pt->x;

		int need_size = (get_row_len(row) + 1) * sizeof(TCHAR);
		if(p_buf_size < need_size) {
			for(; p_buf_size < need_size;) p_buf_size *= 2;
			p_buf = (TCHAR *)realloc(p_buf, p_buf_size);
			if(p_buf == NULL) {
				ret_v = 1;
				goto ERR1;
			}
		}

		if(p_func(p_str, p_buf, b_alpha, b_kata)) {
			if(!undo_obj.IsSetMode()) undo_obj.Start();

			pt1.y = row;
			pt1.x = 0;
			if(row == start_pt->y) pt1.x = start_pt->x;
			pt2.y = row;
			pt2.x = get_row_len(row);
			if(row == end_pt->y) pt2.x = end_pt->x;

			ret_v = del_main(&pt1, &pt2, FALSE, TRUE);
			if(ret_v != 0) goto ERR1;
			ret_v = paste(p_buf, FALSE);
			if(ret_v != 0) goto ERR1;

			// 選択範囲を調節
			if(row == end_pt->y) end_pt->x = get_cur_col();
		}
	}

	clear_disp_col();

	// カーソルを元に戻す
	if(start_pt->y != cur_pt.y || start_pt->x != cur_pt.x) {
		cur_pt = *end_pt;
	}
	set_cur(cur_pt.y, cur_pt.x);

	if(p_buf) free(p_buf);
	if(p_last_row_buf) free(p_last_row_buf);
	return 0;

ERR1:
	if(p_buf) free(p_buf);
	if(p_last_row_buf) free(p_last_row_buf);
	return ret_v;

}

int CEditData::hankaku_to_zenkaku(POINT *start_pt, POINT *end_pt, BOOL b_alpha, BOOL b_kata)
{
	return hankaku_zenkaku_convert(start_pt, end_pt, b_alpha, b_kata, TRUE);
}

int CEditData::zenkaku_to_hankaku(POINT *start_pt, POINT *end_pt, BOOL b_alpha, BOOL b_kata)
{
	return hankaku_zenkaku_convert(start_pt, end_pt, b_alpha, b_kata, FALSE);
}

int CEditData::without_comment_and_literal_convert(POINT* start_pt, POINT* end_pt, BOOL to_lower, BOOL to_upper)
{
	BOOL	find = FALSE;
	POINT	cur_pt = *start_pt;
	TCHAR* buf = NULL;
	int buf_size = calc_buf_size(start_pt, end_pt);

	buf = (TCHAR*)malloc(buf_size);
	if(buf == NULL) return 1;
	TCHAR* buf_p = buf;

	struct check_char_type_cache cache;

	// check_char_type_cacheを初期化
	cache.row = -1;
	cache.char_type_arr = cache.ini_buf;
	cache.char_type_arr_cnt = CHECK_CHAR_TYPE_CACHE_BUF_SIZE;

	TCHAR* p = get_row_buf(cur_pt.y) + cur_pt.x;
	TCHAR* p_end = get_row_buf(cur_pt.y) + get_row_len(cur_pt.y);

	for(;;) {
		// 終了位置の判定
		if(inline_pt_cmp(&cur_pt, end_pt) >= 0) break;

		// 移動する
		if(p == p_end) {
			if(cur_pt.y == get_row_cnt() - 1) break;
			cur_pt.y++;
			cur_pt.x = 0;
			p = get_row_buf(cur_pt.y);
			p_end = get_row_buf(cur_pt.y) + get_row_len(cur_pt.y);

			::put_char(buf_p, '\n');
			buf_p++;
		} else {
			unsigned int ch = ::get_char(p);
			int ch_len = get_char_len(p);

			if(ch < 255 && check_char_type(cur_pt.y, cur_pt.x, &cache) == CHAR_TYPE_NORMAL) {
				if(to_lower) {
					ch = ::inline_tolower(ch);
				} else if(to_upper) {
					ch = ::inline_toupper(ch);
				}
			}
			::put_char(buf_p, ch);
			buf_p += ch_len;

			cur_pt.x += ch_len;
			p += ch_len;

			if(p > p_end) break;
		}
	}

	*buf_p = '\0';

	// Undoデータの記録を開始
	CUndoSetMode undo_obj(m_undo);
	del(start_pt, end_pt, FALSE);
	paste(buf);

	if(cache.char_type_arr != NULL && cache.char_type_arr != cache.ini_buf) {
		free(cache.char_type_arr);
	}
	if(buf != NULL) {
		free(buf);
	}

	return 0;
}

int CEditData::to_lower_without_comment_and_literal(POINT* start_pt, POINT* end_pt)
{
	return without_comment_and_literal_convert(start_pt, end_pt, TRUE, FALSE);
}

int CEditData::to_upper_without_comment_and_literal(POINT* start_pt, POINT* end_pt)
{
	return without_comment_and_literal_convert(start_pt, end_pt, FALSE, TRUE);
}

BOOL CEditData::is_last_confinement_char(unsigned int ch)
{
	if(m_last_confinement == NULL) return FALSE;
	return (inline_strchr(m_last_confinement, ch) != NULL);
}

BOOL CEditData::is_first_confinement_char(unsigned int ch)
{
	if(m_first_confinement == NULL) return FALSE;
	return (inline_strchr(m_first_confinement, ch) != NULL);
}

int CEditData::check_word_wrap(const TCHAR *p_start, int offset, int prev_offset)
{
	if(!m_word_wrap) return offset;

	const TCHAR *p = p_start + offset;

	if(get_char_type(::get_char(p)) == CHAR_TYPE_ASCII) {
		const TCHAR *ret_p = p_start + prev_offset;
		const TCHAR *tmp_p = ret_p;
		int sepa_char_width = 0;

		for(; tmp_p < p; tmp_p += get_char_len(tmp_p)) {
			//if(m_str_token->isBreakChar(::get_char(tmp_p)) || get_char_len(tmp_p) != 1) {
			if(m_str_token->isBreakChar(::get_char(tmp_p)) || get_char_type(::get_char(tmp_p)) != CHAR_TYPE_ASCII) {
				ret_p = tmp_p;
				sepa_char_width = get_char_len(tmp_p);
			}
		}

		if(p - ret_p < 40) {
			int new_offset = offset - (int)(p - ret_p - sepa_char_width);
			if(new_offset > prev_offset) offset = new_offset;
		}
	}

	return offset;
}

int CEditData::check_confinement(const TCHAR *p_start, int offset)
{
	if(!m_confinement_flg) return offset;
	if(offset <= 2) return offset;

	const TCHAR *p = p_start + offset;
	const TCHAR *prev_p = get_prev_str(p);

	// 禁則処理
	if(m_first_confinement && is_first_confinement_char(::get_char(p))) {
		if(is_first_confinement_char(::get_char(p + get_char_len(p)))) {
			offset -= get_char_len(prev_p);
		} else {
			offset += get_char_len(p);
		}
	} else if(m_last_confinement && 
		is_last_confinement_char(::get_char(prev_p))) {
		offset -= get_char_len(prev_p);
	}

	return offset;
}

int CEditData::get_next_line_len(const TCHAR *pstr, int width, 
	CDC *pdc, CFontWidthData *font_width_data)
{
	const TCHAR *p_start = pstr;
	int	offset = 0;
	int disp_width = 0;

	int	tab_width = font_width_data->GetTabWidth();

	for(; *pstr != '\0';) {
		int ch_width;
		int add_width;

		if(*pstr == '\t') {
			ch_width = 2;
			add_width = tab_width - (disp_width % tab_width);
		} else {
			unsigned int ch = ::get_char(pstr);
			ch_width = font_width_data->GetFontWidth(pdc, ch, NULL);
			ch_width += font_width_data->GetCharSpace(ch_width);
			add_width = ch_width;
		}

		if(disp_width + ch_width > width) {
			// 禁則処理
			offset = check_confinement(p_start, offset);
			// ワードラップに対応する
			offset = check_word_wrap(p_start, offset, 0);
			break;
		} else {
			disp_width += add_width;
			offset += get_char_len(pstr);
			pstr += get_char_len(pstr);
		}
	}

	return offset;
}

int CEditData::get_caret_col(int row, int col, CFontWidthHandler *dchandler)
{
	if(row < 0 || row >= get_row_cnt()) return 0;

	POINT pt = {col, row};
	set_valid_point(&pt);
	row = pt.y;
	col = pt.x;

	const TCHAR *pstr = get_row_buf(row);
	const TCHAR *p_end = pstr + col;
	int caret_col = 0;

	for(; pstr < p_end;) {
		if(*pstr == '\t') {
			caret_col += get_tabstop() - (caret_col % get_tabstop());
		} else {
			if(m_disp_data.IsFullWidthChar(NULL, ::get_char(pstr), dchandler)) {
				caret_col += 2;
			} else {
				caret_col += 1;
			}
		}

		pstr += get_char_len(pstr);
	}
	
	ASSERT(col == get_col_from_caret_col(row, caret_col, dchandler));

	return caret_col;
}

int CEditData::get_col_from_caret_col(int row, int disp_col, CFontWidthHandler *dchandler)
{
	if(row < 0 || row >= get_row_cnt()) return 0;

	const TCHAR *pstr = get_row_buf(row);
	const TCHAR *p_start = pstr;
	int caret_col = 0;

	for(; caret_col < disp_col && *pstr != '\0';) {
		if(*pstr == '\t') {
			caret_col += get_tabstop() - (caret_col % get_tabstop());
		} else {
			if(m_disp_data.IsFullWidthChar(NULL, ::get_char(pstr), dchandler)) {
				caret_col += 2;
			} else {
				caret_col += 1;
			}
		}

		pstr += get_char_len(pstr);
	}

	return (int)(pstr - p_start);
}

int CEditData::calc_disp_row_split_cnt(int row, CFontWidthHandler *dchandler)
{
	if(get_disp_data()->GetLineWidth() == INT_MAX) return 1;

	int width = m_disp_data.GetLineWidth();
	CEditRowData *row_data = get_edit_row_data(row);

	int		split = 1;
	TCHAR	*pbuf = row_data->get_buffer();
	int		offset = 0;
	CDC		*pdc = dchandler->GetDC();
	CFontWidthData *font_width_data = m_disp_data.GetFontData();

	for(;;) {
		int line_len = get_next_line_len(pbuf, width, pdc, font_width_data);
		offset += line_len;
		pbuf += line_len;
		if(*pbuf == '\0') break;

		row_data->set_split_pos(split, offset);
		split++;
	}

	return split;

/*
	TCHAR	*pbuf = row_data->get_buffer();
	TCHAR	*pstr = pbuf;
	int split = 1;
	int offset = 0;
	int prev_offset = 0;
	int	disp_width = 0;
	int ch_width;
	int add_width;

	int tab_width = m_disp_data.GetTabWidth();

	for(; *pstr != '\0';) {
		if(*pstr == '\t') {
			ch_width = 2;
			add_width = tab_width - (disp_width % tab_width);
		} else {
			unsigned int ch = ::get_char(pstr);
			ch_width = m_disp_data.GetFontWidth(NULL, ch, dchandler);
			ch_width += m_disp_data.GetCharSpace(ch_width);
			add_width = ch_width;
		}

		if(disp_width + ch_width > width) {
			disp_width = 0;

			// 禁則処理
			offset = check_confinement(pbuf, offset);

			// ワードラップに対応する
			offset = check_word_wrap(pbuf, offset, prev_offset);
			prev_offset = offset;

			row_data->set_split_pos(split, offset);

			pstr = pbuf + offset;
			if(*pstr == '\0') break;

			split++;
		} else {
			disp_width += add_width;
			offset += get_char_len(pstr);
			pstr += get_char_len(pstr);
		}
	}

	// for debug
	int		nnn_split = 1;
	{
		TCHAR	*nnn_pbuf = row_data->get_buffer();
		int		nnn_offset = 0;
		CDC		*pdc = dchandler->GetDC();
		CFontWidthData *font_width_data = m_disp_data.GetFontData();

		for(;;) {
			int line_len = get_next_line_len(nnn_pbuf, width, pdc, font_width_data);
			nnn_offset += line_len;
			nnn_pbuf += line_len;
			if(*nnn_pbuf == '\0') break;

			if(row_data->get_split_pos(nnn_split) != nnn_offset) {
				TRACE(_T("!!!%d:%d,%d\n"), line_len, row_data->get_split_pos(nnn_split), nnn_offset);
			}
			row_data->set_split_pos(nnn_split, nnn_offset);
			nnn_split++;
		}

		if(split != nnn_split) TRACE(_T("!!!%d:%d\n"), split, nnn_split);
	}

	return split;
*/
}

int CEditData::split_rows(POINT *start_pt, POINT *end_pt)
{
	typedef std::deque<POINT>			PointContainer;
	typedef PointContainer::iterator	IPointIterator;
	PointContainer	m_pt_list;

	// 改行を挿入する位置のリストを作成する
	for(int row = start_pt->y; row <= end_pt->y; row++) {
		int idx = get_scroll_row(row);
		int cur_row = row;
		for(int cnt = 1; cnt < get_split_cnt(row); cnt++) {
			POINT	pt;
			pt.y = row;
			pt.x = get_disp_offset(idx + cnt);

			if(pt.y == start_pt->y && pt.x < start_pt->x) continue;
			if(pt.y == end_pt->y && pt.x > end_pt->x) continue;

			m_pt_list.push_front(pt);
		}
	}

	// 編集箇所が無い場合，終了
	if(m_pt_list.empty()) return 0;

	// 選択範囲の終了位置を調節する
	POINT last_pt = m_pt_list.back();
	if(last_pt.y == end_pt->y) end_pt->x -= last_pt.x;
	end_pt->y += (int)m_pt_list.size();

	// Undoデータの記録を開始
	CUndoSetMode undo_obj(m_undo);

	// 改行の挿入を実行する
	for(IPointIterator it = m_pt_list.begin(); it != m_pt_list.end(); it++) {
		set_cur_row((*it).y);
		set_cur_col((*it).x);
		if(paste(_T("\n"), FALSE) != 0) return 1;
	}

	clear_disp_col();

	return 0;
}

BOOL CEditData::is_blank_row(int row)
{
	TCHAR *p = get_row_buf(row);
	
	for(; *p != '\0'; p++) {
		if(*p != ' ' && *p != '\t') return FALSE;
	}

	return TRUE;
}

BOOL CEditData::is_empty()
{
	return (get_row_cnt() == 1 && get_row_len(0) == 0);
}

int CEditData::get_disp_offset(int idx, int *psplit_idx)
{
	CIntArray *disp_row_arr = get_disp_data()->GetDispRowArr();

	int row = disp_row_arr->GetData(idx);

	// 行の変わり目を探す
	idx--;
	int split_idx = 0;
	for(; idx >= 0; idx--, split_idx++) {
		if(disp_row_arr->GetData(idx) != row) break;
	}

	if(psplit_idx) *psplit_idx = split_idx;

	return get_edit_row_data(row)->get_split_pos(split_idx);
}

int CEditData::get_cur_split()
{
	int row = get_cur_row();
	int col = get_cur_col();

	int split_cnt = get_split_cnt(row);
	if(split_cnt == 1) return 0;

	int idx = get_scroll_row(row);

	int split;
	for(split = 0; split < split_cnt - 1; split++) {
		if(col < get_disp_offset(idx + split + 1)) break;
	}

	return split;
}

void CEditData::check_comment_row(int start_row, int end_row, QWORD ex_style, BOOL *p_invalidate)
{
	if(ex_style & ECS_NO_COMMENT_CHECK) return;

	/* この関数は実行に時間がかかるため，頻繁に呼び出さないこと */

	register TCHAR	*p;
	int		row;
	int		len;
	BOOL	comment = FALSE;
	BOOL	in_quote = FALSE;
	BOOL	in_tag = FALSE;
	BOOL	invalidate = FALSE;

	int		bracket_cnt = 0;

	if(p_invalidate) *p_invalidate = FALSE;

	if(start_row > 0) {
		// 1行前からチェックする
		// 文字の削除で行が連結されるとき，下の行のデータでフラグが上書きされてしまうため
		start_row--;
		if(get_row_data_flg(start_row, ROW_DATA_COMMENT)) comment = TRUE;
		if(get_row_data_flg(start_row, ROW_DATA_IN_TAG)) in_tag = TRUE;

		bracket_cnt = get_row_bracket_cnt(start_row);
	}

	if(start_row < 0) start_row = 0;
	if(end_row < 0) end_row = get_row_cnt() - 1;

	// 高速化
	CStrToken	*str_token = get_str_token();
	TCHAR	*close_comment = str_token->GetCloseComment();

	for(row = start_row; row <= end_row; row++) {
		p = get_row_buf(row);

		if(comment == TRUE) {
			if(!(get_row_data_flg(row, ROW_DATA_COMMENT))) {
				invalidate = TRUE;
				set_row_data_flg(row, ROW_DATA_COMMENT);
			}
		} else {
			if(get_row_data_flg(row, ROW_DATA_COMMENT)) {
				invalidate = TRUE;
				unset_row_data_flg(row, ROW_DATA_COMMENT);
			}
		}

		if((ex_style & ECS_HTML_MODE)) {
			if(in_tag == TRUE) {
				if(!(get_row_data_flg(row, ROW_DATA_IN_TAG))) {
					invalidate = TRUE;
					set_row_data_flg(row, ROW_DATA_IN_TAG);
				}
			} else {
				if(get_row_data_flg(row, ROW_DATA_IN_TAG)) {
					invalidate = TRUE;
					unset_row_data_flg(row, ROW_DATA_IN_TAG);
				}
			}
		}

		if((ex_style & ECS_BRACKET_MULTI_COLOR) && get_row_bracket_cnt(row) != bracket_cnt) {
			invalidate = TRUE;
			TRACE(_T("set_row_bracket_cnt: %d, %d\n"), row, bracket_cnt);
			set_row_bracket_cnt(row, bracket_cnt);
		}

/*
//		クォートが閉じられていないとき，以降の行に影響しないようにした
		if(in_quote == TRUE) {
			if(!(get_row_data(row) & ROW_DATA_IN_QUOTE)) {
				InvalidateRow(row);
				set_row_data(row, get_row_data(row) | ROW_DATA_IN_QUOTE);
			}
		} else {
			if(get_row_data(row) & ROW_DATA_IN_QUOTE) {
				InvalidateRow(row);
				set_row_data(row, get_row_data(row) & (~ROW_DATA_IN_QUOTE));
			}
		}
*/
		for(;;) {
			for(; *p != '\0'; p++) { // このループに負荷がかかる
				if(is_lead_byte(*p)) {
					p++;
					if(*p == '\0') break;
					continue;
				}
				if(str_token->isCommentChar(*p)) break;
				if((ex_style & ECS_BRACKET_MULTI_COLOR) && (*p == '(' || *p == ')')) {
					// NOTE: 現在は改行をまたがったin_quoteは考慮しないので、ここではin_quoteはチェックしない
					if(comment == FALSE && in_tag == FALSE) {
						if(*p == '(') {
							bracket_cnt++;
						} else if(*p == ')' && bracket_cnt > 0) {
							bracket_cnt--;
						}
					}
				}
			}
			if(*p == '\0') break;

			if(comment == TRUE) {
				p = _tcsstr(p, close_comment);
				if(p == NULL) break;
			}
			len = str_token->get_word_len(p);
			if(len == 0) break;

			if((ex_style & ECS_HTML_MODE) && comment == FALSE) {
				if(p[0] == '<' && str_token->isOpenComment(p) == FALSE && in_tag == FALSE) {
					in_tag = TRUE;
					p = p + len;
					continue;
				}
				if(p[0] == '>' && in_tag == TRUE) {
					in_tag = FALSE;
					p = p + len;
					continue;
				}
			}

			in_quote = FALSE;
			if(str_token->isQuoteStart(p)) {
				if(comment == TRUE || ((ex_style & ECS_HTML_MODE) && in_tag == FALSE)) {
					len = 1;
				} else {
					in_quote = TRUE;
				}
				p = p + len;
				continue;
			}

			if(comment == FALSE && in_quote == FALSE &&
				str_token->isRowComment(p) == TRUE) {
				break;
			}

			if(comment == FALSE && in_quote == FALSE &&
				str_token->isOpenComment(p) == TRUE) {
				len = str_token->GetOpenCommentLen();
				comment = TRUE;
			}
			if(comment == TRUE && str_token->isCloseComment(p) == TRUE) {
				len = str_token->GetCloseCommentLen();
				comment = FALSE;
			}

			p = p + len;
		}
	}

	if(invalidate) {
		get_disp_data()->GetDispColorDataPool()->ClearAllData();
		if(p_invalidate) *p_invalidate = TRUE;
	}

	if(row < get_row_cnt()) { // row = end_row + 1
		if((comment && !(get_row_data_flg(row, ROW_DATA_COMMENT))) ||
			(!comment && (get_row_data_flg(row, ROW_DATA_COMMENT)))) {
			check_comment_row(row, -1, ex_style, p_invalidate);
			return;
		} else if((in_tag && !(get_row_data_flg(row, ROW_DATA_IN_TAG))) ||
			(!in_tag && (get_row_data_flg(row, ROW_DATA_IN_TAG)))) {
			check_comment_row(row, -1, ex_style, p_invalidate);
			return;
		} else if((ex_style & ECS_BRACKET_MULTI_COLOR) && get_row_bracket_cnt(row) != bracket_cnt) {
			check_comment_row(row, -1, ex_style, p_invalidate);
			return;
		}
	}
}

void CEditData::notice_update()
{
	get_disp_data()->SendNotifyMessage(NULL, EC_WM_NOTICE_UPDATE, 0, 0);
}
