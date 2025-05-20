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

#include "EditRowBuffer.h"

//#define GAP_RESERVE_CNT	32
#define GAP_RESERVE_CNT	5

CEditRowData CEditRowBuffer::m_null_row_data;

CEditRowBuffer::CEditRowBuffer()
{
	m_row_data = NULL;
	m_row_cnt = 0;
	m_allocated_buf_cnt = 0;
//	m_alloc_buf_cnt = 256;
	m_alloc_buf_cnt = 10;

	m_gap_start = 1;
	m_gap_end = 0;
}

CEditRowBuffer::~CEditRowBuffer()
{
	free_data();
}

void CEditRowBuffer::init_data()
{
	free_data();

	realloc_data_buf(m_alloc_buf_cnt);
	set_edit_row_data(0, new CEditRowData());
	m_row_cnt = 1;
	m_gap_start = 1;
}

void CEditRowBuffer::free_data()
{
	int		i;

	if(m_row_data != NULL) {
		for(i = 0; i < m_row_cnt; i++) {
			if(get_edit_row_data(i) != NULL) delete get_edit_row_data(i);
		}
		free(m_row_data);
		m_row_data = NULL;
	}

	m_row_cnt = 0;
	m_allocated_buf_cnt = 0;
}

int CEditRowBuffer::realloc_data_buf(int cnt)
{
	if(cnt == 0) cnt = m_alloc_buf_cnt;

	if(m_row_cnt != 0) make_gap(m_row_cnt);

	m_row_data = (CEditRowData **)realloc(m_row_data, sizeof(CEditRowData *) * cnt);
	if(m_row_data == 0) return 1;

	if(cnt > m_allocated_buf_cnt) {
		int cnt_diff = cnt - m_allocated_buf_cnt;
		memset(m_row_data + m_allocated_buf_cnt, 0, 
			sizeof(CEditRowData *) * cnt_diff);
	}

	m_allocated_buf_cnt = cnt;
	m_gap_end = m_allocated_buf_cnt;

	return 0;
}

void CEditRowBuffer::dump_buffer()
{
	// for debug
	int idx;

	for(idx = 0; idx < m_allocated_buf_cnt; idx++) {
		CEditRowData *p = m_row_data[idx];
		if(p == NULL) {
			TRACE(_T("."));
		} else {
			if(p->get_buffer()[0] == '\0') {
				TRACE(_T(" "), p->get_buffer()[0]);
			} else {
				TRACE(_T("%c"), p->get_buffer()[0]);
			}
		}
	}
	TRACE(_T("\n"));
}

void CEditRowBuffer::make_gap(int row)
{
	if(row == m_gap_start) return;

	int move_rows;
	int move_start;
	int move_to;
	int fill_start;
	int fill_cnt;

	if(m_gap_start > row) {
		move_rows = m_gap_start - row;
		move_start = row;
		move_to = m_gap_end - move_rows;

		m_gap_start = row;
		m_gap_end = move_to;

		fill_start = move_start;
		fill_cnt = min(move_rows, m_gap_end - fill_start);
	} else {
		move_rows = row - m_gap_start;
		move_start = m_gap_end;
		move_to = m_gap_start;

		m_gap_start = row;
		m_gap_end = m_gap_end + move_rows;

		fill_start = max(move_start, move_to + move_rows);
		fill_cnt = m_gap_end - fill_start;
	}

	// データを移動
	memmove(m_row_data + move_to, m_row_data + move_start, sizeof(CEditRowData *) * (move_rows));

	// 空いた領域をNULLで埋める
	memset(m_row_data + fill_start, 0, sizeof(CEditRowData *) * fill_cnt);
/*
#ifdef _DEBUG
	{
		int i;
		for(i = 0; i < m_row_cnt; i++) ASSERT(get_edit_row_data(i) != NULL);
		for(i = m_gap_start; i < m_gap_end; i++) ASSERT(m_row_data[i] == NULL);
	}
#endif
*/
}

int CEditRowBuffer::new_line(int row)
{
	if(m_row_cnt + GAP_RESERVE_CNT >= m_allocated_buf_cnt) {
		if(realloc_data_buf(calc_next_row_cnt(m_allocated_buf_cnt)) != 0) return 1;
	}

	m_row_cnt++;

	if(row != m_gap_start) make_gap(row);

	m_gap_start++;
	set_edit_row_data(row, new CEditRowData());

	return 0;
}

int CEditRowBuffer::delete_line(int row)
{
	if(row == m_gap_start) {
		delete get_edit_row_data(row);
		set_edit_row_data(row, NULL);
		m_gap_end++;
		m_row_cnt--;
	} else {
		if((row + 1)!= m_gap_start) make_gap(row + 1);
		delete get_edit_row_data(row);
		set_edit_row_data(row, NULL);
		m_gap_start--;
		m_row_cnt--;
	}

	if(m_row_cnt + GAP_RESERVE_CNT > (m_allocated_buf_cnt * 4)) {
		int alloc_size = m_allocated_buf_cnt / 2;
		if(realloc_data_buf(alloc_size) != 0) return 1;
	}

	return 0;
}

int CEditRowBuffer::add_new_rows(CEditRowData **new_rows, int cur_row, int cur_col,
	int new_row_cnt, POINT &new_row_pt)
{
	int need_row_cnt = m_row_cnt + new_row_cnt + GAP_RESERVE_CNT;
	if(need_row_cnt >= m_allocated_buf_cnt) {
		int buf_size = m_allocated_buf_cnt;
		for(; buf_size <= need_row_cnt;) {
			buf_size = calc_next_row_cnt(buf_size);
		}
		if(realloc_data_buf(buf_size) != 0) return 1;
	}

	make_gap(new_row_pt.y + 1);
	memcpy(m_row_data + new_row_pt.y + 1, new_rows, sizeof(CEditRowData *) * new_row_cnt);

	m_row_cnt += new_row_cnt;
	m_gap_start += new_row_cnt;

	// カーソル位置より右のテキストを，末尾につける
	get_edit_row_data(cur_row)->add_chars(cur_col,
		get_edit_row_data(new_row_pt.y)->get_buffer() + new_row_pt.x,
		get_edit_row_data(new_row_pt.y)->get_char_cnt() - new_row_pt.x - 1);
	get_edit_row_data(new_row_pt.y)->delete_chars(new_row_pt.x,
		get_edit_row_data(new_row_pt.y)->get_char_cnt() - 1 - new_row_pt.x);

	return 0;
}

int CEditRowBuffer::calc_next_row_cnt(int cnt)
{
	if(cnt < (1024 * 1024)) return cnt * 2;
	return cnt + (1024 * 1024);
}

void CEditRowBuffer::swap_row(int row1, int row2)
{
	CEditRowData *pr = get_edit_row_data(row1);
	set_edit_row_data(row1, get_edit_row_data(row2));
	set_edit_row_data(row2, pr);
}
