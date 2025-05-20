/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef _EDIT_ROW_DATA_H_INCLUDE
#define _EDIT_ROW_DATA_H_INCLUDE

#define ROW_DATA_COMMENT		(0x01 << 0)
//#define ROW_DATA_IN_QUOTE		(0x01 << 1)
#define ROW_DATA_IN_TAG			(0x01 << 2)
#define ROW_DATA_UPDATE_FLG		(0x01 << 3)		// CDispColorDataで使う
#define ROW_DATA_BOOK_MARK		(0x01 << 4)
#define ROW_DATA_BREAK_POINT	(0x01 << 5)
#define ROW_DATA_USER_DEF1		(0x01 << 6)
#define ROW_DATA_USER_DEF2		(0x01 << 7)

#define SPLIT_POS_ARR_DEFULT_SIZE	4

#include "EditDispData.h"
#include "MemoryPool.h"

class CEditRowData
{
public:
	static void *operator new(size_t size);
	static void operator delete(void *p, size_t size);

	static int get_disp_width_str(const TCHAR *p, CEditDispData *disp_data, 
		CFontWidthHandler *dchandler, int tabstop, int col);
	static int get_x_from_disp_width_str(const TCHAR *p, CEditDispData *disp_data, 
		CFontWidthHandler *dchandler, int tabstop, int width);
	
	CEditRowData();
	~CEditRowData() { 
		free_buf();
		free_split_pos();
	}

	void set_data(int arg) { m_data = arg; }
	int get_data() { return m_data; }

	int get_data_flg(int flg) { return m_data & flg; }
	void set_data_flg(int flg) { m_data |= flg; }
	void unset_data_flg(int flg) { m_data &= (~flg); }
	void toggle_data_flg(int flg) { m_data ^= flg; }

	int get_bracket_cnt() { return m_bracket_cnt; }
	void set_bracket_cnt(int cnt) { m_bracket_cnt = cnt; }

	void set_edit_undo_sequence(int seq) { m_edit_undo_sequence = seq; }
	int get_edit_undo_sequence() { return m_edit_undo_sequence; }

	int get_char_cnt() { return m_char_cnt; }
	TCHAR *get_buffer() { return m_buf; }

	int get_disp_width() { return m_disp_width; }
	int calc_disp_width(CEditDispData *disp_data, CFontWidthHandler *dchandler, int tabstop);

	int get_disp_width(CEditDispData *disp_data, CFontWidthHandler *dchandler, int tabstop, int col);
	int get_x_from_width(CEditDispData *disp_data, CFontWidthHandler *dchandler, int tabstop, int width);

	int add_char(int col, TCHAR data) { return add_chars(col, &data, 1); }
	int add_chars(int col, const TCHAR *data, int cnt);

	int delete_char(int col) { 	return delete_chars(col - 1, 1); }
	int delete_chars(int col, int cnt);

	BOOL is_lead_byte(int col);

	int get_col_from_disp_pos(int disp_col, int tabstop);

	int get_scroll_row() { return m_scroll_row; }
	void set_scroll_row(int r) { m_scroll_row = r; }
	int get_split_cnt() { return m_split_cnt; }
	void set_split_cnt(int s) { m_split_cnt = s; }

	int get_split_pos(int idx) { return (idx == 0) ? 0 : m_split_pos[idx - 1]; }
	void set_split_pos(int idx, int pos) {
		ASSERT(idx > 0);
		idx--;
		if(idx >= m_split_pos_arr_cnt) {
			if(m_split_pos_arr_cnt == 0) {
				m_split_pos_arr_cnt = SPLIT_POS_ARR_DEFULT_SIZE;
				m_split_pos  = (int *)m_mem_pool2.alloc(sizeof(int) * m_split_pos_arr_cnt);
			} else {
				int new_arr_cnt = m_split_pos_arr_cnt * 2;
				m_split_pos = (int *)m_mem_pool2.realloc(m_split_pos, sizeof(int) * new_arr_cnt);
				m_split_pos_arr_cnt = new_arr_cnt;
			}
		}
		m_split_pos[idx] = pos;
	}

private:
	int realloc_row_buf(int size);

	void free_buf();
	void free_split_pos();

private:
	static const int m_default_row_size;
	static CFixedMemoryPool m_mem_pool;
	static CMemoryPool2		m_mem_pool2;
	static TCHAR *m_dummy_buf;

	TCHAR	*m_buf;
	size_t	m_buf_size;
	int		m_char_cnt;

	int		m_disp_width;

	int		m_data;
	int		m_bracket_cnt;
	int		m_edit_undo_sequence;

	int		m_scroll_row;
	int		m_split_cnt;

	int		*m_split_pos;
	int		m_split_pos_arr_cnt;
};

inline void *CEditRowData::operator new(size_t size)
{
	return m_mem_pool.alloc();
}

inline void CEditRowData::operator delete(void *p, size_t size)
{
	m_mem_pool.free(p);
}

inline CEditRowData::CEditRowData() : m_buf(m_dummy_buf), m_buf_size(0), m_char_cnt(1),
	m_disp_width(0), m_data(0), m_edit_undo_sequence(-1), 
	m_scroll_row(0), m_split_cnt(1), m_split_pos(NULL), m_split_pos_arr_cnt(0),
	m_bracket_cnt(0)
{
}

#endif _EDIT_ROW_DATA_H_INCLUDE
