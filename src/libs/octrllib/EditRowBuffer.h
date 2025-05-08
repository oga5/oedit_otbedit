/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef _EDIT_ROW_BUFFER_H_INCLUDE
#define _EDIT_ROW_BUFFER_H_INCLUDE

#include "EditRowData.h"

class CEditRowBuffer
{
private:
	static CEditRowData	m_null_row_data;
	CEditRowData	**m_row_data;
	int				m_row_cnt;
	int				m_allocated_buf_cnt;
	int				m_alloc_buf_cnt;

	int				m_gap_start;
	int				m_gap_end;

	int calc_next_row_cnt(int cnt);
	int realloc_data_buf(int cnt);

	void set_edit_row_data(int row, CEditRowData *row_data) {
		m_row_data[get_row_idx(row)] = row_data;
	}

	int get_row_idx(int row) {
		if(row < m_gap_start) return row;
		return row + (m_gap_end - m_gap_start);
	}

	void make_gap(int row);
	void dump_buffer();

public:
	CEditRowBuffer();
	~CEditRowBuffer();

	void init_data();
	void free_data();

	int get_row_cnt() { return m_row_cnt; }
	int new_line(int row);
	int delete_line(int row);

	void swap_row(int row1, int row2);

	CEditRowData *get_edit_row_data(int row) {
		if(row < 0 || row >= m_row_cnt) {
			ASSERT(0);
			return &m_null_row_data;
		}
		return m_row_data[get_row_idx(row)];
	}

	int add_new_rows(CEditRowData **new_rows, int cur_row, int cur_col,
		int new_row_cnt, POINT &new_row_pt);
};

#endif _EDIT_ROW_BUFFER_H_INCLUDE
