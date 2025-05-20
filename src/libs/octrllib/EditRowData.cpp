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
#include "ostrutil.h"

#include "EditRowData.h"

//
// CEditRowData impl.
//
const int CEditRowData::m_default_row_size = 32;
CFixedMemoryPool CEditRowData::m_mem_pool(sizeof(CEditRowData), 256);
CMemoryPool2 CEditRowData::m_mem_pool2(1024*256, 1024*1024*8);	// 256KB to 8MB

TCHAR *CEditRowData::m_dummy_buf = _T("");

BOOL CEditRowData::is_lead_byte(int col)
{
	if(col < 0 || col >= m_char_cnt) return 0;
	return ::is_lead_byte(m_buf[col]);
}

void CEditRowData::free_buf()
{
	if(m_buf != m_dummy_buf) {
		m_mem_pool2.free(m_buf);
		m_buf = m_dummy_buf;
	}
}

void CEditRowData::free_split_pos()
{
	if(m_split_pos != NULL) {
		m_mem_pool2.free(m_split_pos);
		m_split_pos = NULL;
	}
}

#pragma intrinsic(memcpy)
int CEditRowData::realloc_row_buf(int size)
{
	if(m_buf == m_dummy_buf) {
		m_buf = NULL;
	}
	m_buf = (TCHAR *)m_mem_pool2.realloc(m_buf, size);
	m_buf_size = m_mem_pool2.get_chunk_size(m_buf);
	return 0;
}
#pragma function(memcpy)

int CEditRowData::delete_chars(int col, int cnt)
{
	if(cnt == 0) return 0;

	set_data_flg(ROW_DATA_UPDATE_FLG);

	memmove(m_buf + col, m_buf + col + cnt, ((INT_PTR)m_char_cnt - col - cnt) * sizeof(TCHAR));
	m_char_cnt -= cnt;

	if(m_buf_size - (m_char_cnt * sizeof(TCHAR)) > m_default_row_size * sizeof(TCHAR) * 2) {
		int buf_size = (m_char_cnt + 1) * sizeof(TCHAR);
		if(realloc_row_buf(buf_size) != 0) return 1;
	}

	return 0;
}

int CEditRowData::calc_disp_width(CEditDispData *disp_data, CFontWidthHandler *dchandler, 
	int tabstop)
{
	m_disp_width = get_disp_width(disp_data, dchandler, tabstop, -1);
	return m_disp_width;
}

int CEditRowData::get_disp_width_str(const TCHAR *p, CEditDispData *disp_data, 
	CFontWidthHandler *dchandler, int tabstop, int col)
{
	int		w = 0;

	if(col < 0) col = INT_MAX;

	int tab_width = disp_data->GetTabWidth();

	for(int i = 0; i < col;) {
		if(*p == '\0') break;
		if(*p == '\t') {
			w += tab_width - (w % tab_width);
		} else {
			int ch_w = disp_data->GetFontWidth(NULL, get_char(p), dchandler);
			w += ch_w + disp_data->GetCharSpace(ch_w);
		}

		int ch_len = get_char_len(p);
		p += ch_len;
		i += ch_len;
	}

	return w;
}

int CEditRowData::get_disp_width(CEditDispData *disp_data, CFontWidthHandler *dchandler, 
	int tabstop, int col)
{
	TCHAR	*p = get_buffer();
	return get_disp_width_str(p, disp_data, dchandler, tabstop, col);
}

int CEditRowData::get_x_from_disp_width_str(const TCHAR *p, CEditDispData *disp_data, 
	CFontWidthHandler *dchandler, int tabstop, int width)
{
	int x = 0;
	int w = 0;
	int font_width;
	int tab_width = disp_data->GetTabWidth();

	for(;;) {
		if(*p == '\0') break;
		if(*p == '\t') {
			font_width = tab_width - (w % tab_width);
		} else {
			font_width = disp_data->GetFontWidth(NULL, get_char(p), dchandler);
			font_width += disp_data->GetCharSpace(font_width);
		}

		if(w + font_width > width) {
			if(w + font_width / 2 <= width) x += get_char_len(p);
			break;
		}
		w += font_width;
		x += get_char_len(p);
		p += get_char_len(p);
	}

	return x;
}

int CEditRowData::get_x_from_width(CEditDispData *disp_data, CFontWidthHandler *dchandler, int tabstop, int width)
{
	TCHAR	*p = get_buffer();
	return get_x_from_disp_width_str(p, disp_data, dchandler, tabstop, width);
}

int CEditRowData::get_col_from_disp_pos(int disp_col, int tabstop)
{
	// FIXME: 高速化する

	TCHAR	*pstr;
	int		x;
	int		col;

	pstr = get_buffer();
	for(x = 0, col = 0; pstr != NULL && *pstr != '\0'; pstr++, col++) {
		x++;
		if(x > disp_col) break;
	}

	return col;
}

#pragma intrinsic(memcpy)
int CEditRowData::add_chars(int col, const TCHAR *data, int cnt)
{
	if(cnt == 0) return 0;

	set_data_flg(ROW_DATA_UPDATE_FLG);

	if(m_buf_size <= ((INT_PTR)m_char_cnt + cnt) * sizeof(TCHAR)) {
		int buf_size = (m_char_cnt + cnt + 1) * sizeof(TCHAR);
		if(realloc_row_buf(buf_size) != 0) return 1;
	}

	// 行の末尾以外
	if(col != get_char_cnt() - 1) {
		memmove(m_buf + col + cnt, m_buf + col, ((INT_PTR)get_char_cnt() - 1 - col) * sizeof(TCHAR));
	}

	memcpy(m_buf + col, data, cnt * sizeof(TCHAR));
	m_char_cnt += cnt;
	m_buf[m_char_cnt - 1] = '\0';

	return 0;
}
#pragma function(memcpy)

