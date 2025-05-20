/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef _STRING_BUFFER_H_INCLUDE
#define _STRING_BUFFER_H_INCLUDE

#include <tchar.h>
#include <stdlib.h>

#define CSTRING_BUFFER_INITIAL_BUF_SIZE	(1024 * 32)

class CStringBuffer {
private:
	TCHAR	m_initial_buf[CSTRING_BUFFER_INITIAL_BUF_SIZE];
	TCHAR	*m_large_buf;
	TCHAR	*m_buf;

	size_t	m_buf_size;
	size_t	m_str_len;
	int		m_realloc_cnt;

	void init_buffer() {
		m_buf = m_initial_buf;
		m_buf_size = CSTRING_BUFFER_INITIAL_BUF_SIZE;
		m_large_buf = NULL;
		m_realloc_cnt = 0;
	}
	void clear_buffer() {
		m_buf[0] = '\0';
		m_str_len = 0;
	}
	void free_buffer() {
		if(m_large_buf != NULL) {
			free(m_large_buf);
			init_buffer();
		}
	}

public:
	CStringBuffer() {
		init_buffer();
		clear_buffer();
	}
	~CStringBuffer() {
		free_buffer();
	}

	void reset_buffer() {
		// 4MB以上のときは、メモリを解放する
		if(m_buf_size >= 1024 * 1024 * 4) {
			free_buffer();
		}
		clear_buffer();
	}
	bool append(const TCHAR *str) {
		TCHAR *last_p;
		size_t len = _tcslen(str);
		size_t need_len = (m_str_len + len + 1) * sizeof(TCHAR);
		
		if(need_len >= m_buf_size) {
			size_t new_size = m_buf_size * 2;
			for(; need_len >= new_size;) {
				new_size *= 2;
			}
			m_large_buf = (TCHAR *)realloc(m_large_buf, new_size);
			if(m_large_buf == NULL) {
				return false;
			}

			if(m_buf == m_initial_buf) {
				_tcscpy(m_large_buf, m_initial_buf);
			}
			m_buf = m_large_buf;
			m_buf_size = new_size;

			m_realloc_cnt++;
		}

		last_p = m_buf + m_str_len;
		_tcscat(last_p, str);
		m_str_len += len;

		return true;
	}

	const TCHAR *get_str() const { return m_buf; }
	size_t get_str_len() const { return m_str_len; }

	size_t get_buf_size() const { return m_buf_size; }
	int get_realloc_cnt() const { return m_realloc_cnt; }

	bool is_empty() const { return (m_str_len == 0); }
};

#endif _STRGRID_DATA_H_INCLUDE
