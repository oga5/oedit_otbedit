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
#include "Indenter.h"

CIndenter::CIndenter()
{
	m_edit_data = NULL;

	int		i;
	for(i = 0; i < 256; i++) {
		indent_char[i] = 0;
	}
	indent_char['\r'] = 1;
	indent_char['{'] = 1;
	indent_char['}'] = 1;
	indent_char[':'] = 1;
}

unsigned int CIndenter::get_last_char(int row)
{
	ASSERT(row >= 0 && row < m_edit_data->get_row_cnt());

	unsigned int ch = '\0';
	TCHAR *p = m_edit_data->get_row_buf(row);
	CStrToken *str_token = m_edit_data->get_str_token();

	if(m_edit_data->get_row_data_flg(row, ROW_DATA_COMMENT)) {
		p = _tcsstr(p, str_token->GetCloseComment());
		if(p == NULL) return ch;
		p += str_token->GetCloseCommentLen();
	}

	for(; *p != '\0'; p += get_char_len(p)) {
		if(str_token->isRowComment(p)) break;

		if(str_token->isOpenComment(p)) {
			p = _tcsstr(p, str_token->GetCloseComment());
			if(p == NULL) break;
			p += str_token->GetCloseCommentLen();
			break;
		}

		if(*p != ' ' && *p != '\t') ch = get_char(p);
	}

	return ch;
}

unsigned int CIndenter::get_first_char(int row)
{
	ASSERT(row >= 0 && row < m_edit_data->get_row_cnt());

	unsigned int ch = '\0';
	TCHAR *p = m_edit_data->get_row_buf(row);

	for(; *p != '\0'; p += get_char_len(p)) {
		if(*p != ' ' && *p != '\t') {
			ch = get_char(p);
			break;
		}
	}

	return ch;
}

int CIndenter::get_first_char_pos(int row)
{
	ASSERT(row >= 0 && row < m_edit_data->get_row_cnt());

	int pos = 0;
	TCHAR *p = m_edit_data->get_row_buf(row);

	for(; *p != '\0'; p++, pos++) {
		if(*p != ' ' && *p != '\t') {
			return pos;
		}
	}

	return -1;
}

void CIndenter::ltrim(int row)
{
	POINT	pt1, pt2;
	pt1.y = row;
	pt1.x = 0;
	pt2.y = row;
	pt2.x = 0;

	int y = m_edit_data->get_cur_row();
	int x = m_edit_data->get_cur_col();

	TCHAR	*p = m_edit_data->get_row_buf(pt1.y);
	for(; *p == '\t' || *p == ' '; p++) {
		pt2.x++;
		x--;
	}
	if(x < 0) x = 0;

	if(pt2.x != 0) {
		m_edit_data->del(&pt1, &pt2, TRUE);
		m_edit_data->set_cur(y, x);	
	}
}

void CIndenter::set_cur_col(int x)
{
	m_edit_data->set_cur(m_edit_data->get_cur_row(), x);
}

BOOL CIndenter::is_space_only(int row)
{
	TCHAR	*p = m_edit_data->get_row_buf(row);

	for(; *p != '\0'; p++) {
		if(*p != ' ' && *p != '\t') return FALSE;
	}

	return TRUE;
}

BOOL CIndenter::is_null_line(int row)
{
	return (get_last_char(row) == '\0');
}

int CIndenter::search_prev_row(int row)
{
	row--;

	for(; row >= 0; row--) {
		if(is_null_line(row)) continue;
		if(m_edit_data->check_char_type(row, m_edit_data->get_row_len(row)) != CHAR_TYPE_NORMAL) continue;
		break;
	}

	return row;
}

int CIndenter::search_row_top(int row)
{
	if(row == 0) return 0;

	int		result = row;
	unsigned int ch;

	row--;
	for(; row >= 0; row--) {
		if(is_null_line(row)) break;
		ch = get_last_char(row);
		if(ch == ';' || ch == '{' || ch == '}' || ch == ':') break;

		result = row;
	}

	return result;
}

__inline BOOL CIndenter::is_space_char(unsigned int ch)
{
	return (ch == '\t' || ch == ' ' || ch == L'　');
}

void CIndenter::indent(int sample_row, INDENT_MODE indent_mode, int add, BOOL b_ltrim)
{
	// インデント不要かチェックする
	{
		TCHAR	*p1;
		TCHAR	*p2;

		int x = 0;
		p1 = m_edit_data->get_row_buf(m_edit_data->get_cur_row());
		for(;;) {
			if(!is_space_char(get_char(p1))) break;
			x += get_char_len(p1);
			p1 += get_char_len(p1);
		}

		int x2 = 0;
		p2 = m_edit_data->get_row_buf(sample_row);
		for(;;) {
			if(!is_space_char(get_char(p2))) break;
			x2 += get_char_len(p2);
			p2 += get_char_len(p2);
		}
		if(add == 1) x2++;
		if(x == x2 && b_ltrim) {
			p1 = m_edit_data->get_row_buf(m_edit_data->get_cur_row());
			p2 = m_edit_data->get_row_buf(sample_row);
			if(_tcsncmp(p1, p2, x) == 0) {
				if(indent_mode == INDENT_MODE_AUTO) set_cur_col(x);
				return;
			}
		}
	}

	TCHAR	*p;
	unsigned int ch;

	if(b_ltrim) ltrim(m_edit_data->get_cur_row());

	int x = m_edit_data->get_cur_col();
	set_cur_col(0);
	p = m_edit_data->get_row_buf(sample_row);
	for(;;) {
		ch = get_char(p);
		if(!is_space_char(ch)) break;
		m_edit_data->input_char(ch);
		if(ch == '\t' && m_edit_data->get_tab_as_space()) {
			x += m_edit_data->get_tabstop();
		} else {
			x += get_char_len(p);
		}
		p += get_char_len(p);
	}
	if(add == 1) {
		m_edit_data->input_char('\t');
		if(m_edit_data->get_tab_as_space()) {
			x += m_edit_data->get_tabstop();
		} else {
			x++;
		}
	}
	set_cur_col(x);
}

void CIndenter::DoIndent(CEditData *edit_data, unsigned char ch)
{
	if(is_indent_char(ch) == FALSE) return;

	m_edit_data = edit_data;

	{
		if(ch == '\r') {
			int row = m_edit_data->get_cur_row() - 1;

			// ブロックコメント中のときは，前の行を引き継ぐ
			if(m_edit_data->check_char_type(row, m_edit_data->get_row_len(row)) == CHAR_TYPE_IN_COMMENT) {
				// FIXME: コメント開始位置にあわせる
				indent(row, INDENT_MODE_SMART);
				return;
			}

			if(is_null_line(row)) {
				if(is_space_only(row)) ltrim(row);
				row = search_prev_row(row);
				if(row < 0) return;
			}
			
			if(get_first_char(row) == '#') {
				indent(row, INDENT_MODE_SMART);
				return;
			}
			if(get_first_char(m_edit_data->get_cur_row()) == '}') {
				POINT	pt;
				if(m_edit_data->search_brackets('{', '}', 1, &pt) == TRUE) {
					indent(search_row_top(pt.y), INDENT_MODE_SMART);
				}
				return;
			}

			unsigned int prev_ch = get_last_char(row);

			if(prev_ch == ';') {
				indent(search_row_top(row), INDENT_MODE_SMART);
			} else if(prev_ch == ':') {
				indent(row, INDENT_MODE_SMART, 1);
			} else if(prev_ch == '{') {
				indent(row, INDENT_MODE_SMART, 1);
			} else if(prev_ch == '}') {
				indent(row, INDENT_MODE_SMART);
			} else {
				indent(search_row_top(row), INDENT_MODE_SMART, 1);
			}  

			return;
		}

		int char_type = m_edit_data->check_char_type();
		if(char_type == CHAR_TYPE_IN_COMMENT || char_type == CHAR_TYPE_IN_ROWCOMMENT || 
			char_type == CHAR_TYPE_IN_QUOTE) {
			return;
		}

		m_edit_data->save_cursor_pos();

		if(ch == ':') {
			// 3項演算子のとき，インデントしない
			if(_tcschr(m_edit_data->get_row_buf(m_edit_data->get_cur_row()), '?') != NULL) return;

			if(m_edit_data->get_cur_col() >= 2 &&
				m_edit_data->get_char(m_edit_data->get_cur_row(), m_edit_data->get_cur_col() - 2) == ':') {
				int row = m_edit_data->get_cur_row() - 1;
				if(row >= 0 && is_null_line(row)) row = search_prev_row(row);
				if(row >= 0) row = search_row_top(row);
				if(row >= 0) indent(row, INDENT_MODE_SMART);
			} else {
				// 高速化
				int break_row;
				for(break_row = m_edit_data->get_cur_row() - 1; break_row >= 0; break_row--) {
					TCHAR *p = m_edit_data->get_row_buf(break_row);
					if(inline_isalpha(*p)) break;
				}

				POINT	pt;
				POINT	pt2 = { 0, break_row };
				if(m_edit_data->search_brackets_ex('{', '}', 1, &pt2, '\0', -1, &pt) == TRUE) {
					indent(search_row_top(pt.y), INDENT_MODE_SMART);
				} else {
					ltrim(m_edit_data->get_cur_row());
				}
			}
		}
		if(ch == '{') {
			POINT	pt_open = {-1, -1};
			POINT	pt_close = {-1, -1};

			m_edit_data->search_brackets('{', '}', 0, &pt_close);

			m_edit_data->move_cur_col(-1);
			m_edit_data->search_brackets_ex('{', '}', 1, &pt_close, '\0', -1, &pt_open);
			m_edit_data->move_cur_col(1);

			if(pt_close.y > pt_open.y) {
				indent(search_row_top(pt_close.y), INDENT_MODE_SMART);
			} else if(pt_open.y >= 0) {
				indent(search_row_top(pt_open.y), INDENT_MODE_SMART, 1);
			} else {
				ltrim(m_edit_data->get_cur_row());
			}
		}
		if(ch == '}' && get_first_char_pos(m_edit_data->get_cur_row()) + 1 == m_edit_data->get_cur_col()) {
			POINT	pt;
			if(m_edit_data->search_brackets('{', '}', 0, &pt) == TRUE) {
				indent(search_row_top(pt.y), INDENT_MODE_SMART);
			}
		}
	}
}

void CIndenter::AutoIndent(CEditData *edit_data)
{
	m_edit_data = edit_data;

	if(m_edit_data == NULL) return;
	if(m_edit_data->get_cur_row() == 0) return;

	if(is_null_line(m_edit_data->get_cur_row() - 1)) {
		indent(m_edit_data->get_cur_row() - 1, INDENT_MODE_AUTO, 0, FALSE);
	} else {
		indent(m_edit_data->get_cur_row() - 1, INDENT_MODE_AUTO);
	}
}

