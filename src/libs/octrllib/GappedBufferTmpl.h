/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef _GAPPED_BUFFER_TMPL_H_INCLUDE
#define _GAPPED_BUFFER_TMPL_H_INCLUDE

// CEditableGridDataのデータ管理用

template <class T>
class GappedBufferTmpl
{
private:
	T				m_null_data{};
	T				*m_buffer;
	int				m_row_cnt;

	int				m_allocated_buf_cnt;
	int				m_gap_start;
	int				m_gap_end;

	int				m_min_gap_size;
	int				m_max_gap_size;

	int calc_next_row_cnt(int cnt);
	int realloc_buffer(int cnt);

	int get_row_idx(int row) {
		if(row < m_gap_start) return row;
		return row + (m_gap_end - m_gap_start);
	}
	T &get_ref(int row) {
		if(row < 0 || row >= m_row_cnt) {
			ASSERT(0);
			return m_null_data;
		}
		return m_buffer[get_row_idx(row)];
	}

	void make_gap(int row);

public:
	GappedBufferTmpl();
	~GappedBufferTmpl();

	int init_buffer(int row_cnt);
	void free_data();

	int get_row_cnt() { return m_row_cnt; }
	int insert_row(int row);
	int delete_row(int row);

	void swap_row(int row1, int row2);

	T& operator [](int row) {
		return get_ref(row);
	}
};

template <class T>
GappedBufferTmpl<T>::GappedBufferTmpl()
{
	m_min_gap_size = 2;
	m_max_gap_size = 100;

	m_buffer = NULL;
	m_row_cnt = 0;
	m_allocated_buf_cnt = 0;
	m_gap_start = 1;
	m_gap_end = 0;
}

template <class T>
GappedBufferTmpl<T>::~GappedBufferTmpl()
{
	free_data();
}

template <class T>
void GappedBufferTmpl<T>::free_data()
{
	if(m_buffer != NULL) {
		free(m_buffer);
		m_buffer = NULL;
	}

	m_row_cnt = 0;
	m_allocated_buf_cnt = 0;
	m_gap_start = 1;
	m_gap_end = 0;
}

template <class T>
int GappedBufferTmpl<T>::init_buffer(int row_cnt)
{
	free_data();
	if(realloc_buffer(row_cnt + m_max_gap_size) != 0) return 1;

	m_row_cnt = row_cnt;
	m_gap_start = row_cnt;

	return 0;
}

template <class T>
int GappedBufferTmpl<T>::calc_next_row_cnt(int cnt)
{
	if(cnt < (1024 * 1024)) return cnt * 2;
	return cnt + (1024 * 1024);
}

template <class T>
void GappedBufferTmpl<T>::make_gap(int row)
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
	memmove(m_buffer + move_to, m_buffer + move_start, sizeof(T) * (move_rows));
}

template <class T>
int GappedBufferTmpl<T>::realloc_buffer(int cnt)
{
	if(cnt == 0) cnt = 256;

	if(m_row_cnt != 0) make_gap(m_row_cnt);

	m_buffer = (T *)realloc(m_buffer, sizeof(T) * cnt);
	if(m_buffer == 0) return 1;

	m_allocated_buf_cnt = cnt;
	m_gap_end = m_allocated_buf_cnt;

	return 0;
}

template <class T>
int GappedBufferTmpl<T>::insert_row(int row)
{
	if(m_row_cnt + m_min_gap_size >= m_allocated_buf_cnt) {
		if(realloc_buffer(calc_next_row_cnt(m_allocated_buf_cnt)) != 0) return 1;
	}

	m_row_cnt++;

	if(row != m_gap_start) make_gap(row);
	m_gap_start++;

	return 0;
}

template <class T>
int GappedBufferTmpl<T>::delete_row(int row)
{
	if(row == m_gap_start) {
		m_gap_end++;
		m_row_cnt--;
	} else {
		if((row + 1)!= m_gap_start) make_gap(row + 1);
		m_gap_start--;
		m_row_cnt--;
	}

	if(m_row_cnt + m_max_gap_size > (m_allocated_buf_cnt * 4)) {
		int alloc_size = m_allocated_buf_cnt / 2;
		if(realloc_buffer(alloc_size) != 0) return 1;
	}

	return 0;
}

template <class T>
void GappedBufferTmpl<T>::swap_row(int row1, int row2)
{
	T tmp = get_ref(row1);
	get_ref(row1) = get_ref(row2);
	get_ref(row2) = tmp;
}

#endif _GAPPED_BUFFER_TMPL_H_INCLUDE
