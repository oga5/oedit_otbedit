/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef _UNDO_H_INCLUDE
#define _UNDO_H_INCLUDE

#include "MemoryPool.h"

class CUndoData {
public:
	static void *operator new(size_t size);
	static void operator delete(void *p, size_t size);

	CUndoData(int seq);
	CUndoData(int seq, CUndoData *prev);
	~CUndoData();

	int add_undo_char(TCHAR ch) { return add_undo_chars(&ch, 1); }
	int add_undo_chars(const TCHAR *data, int cnt);
	int add_null_char();
	int get_operation() { return m_ope; }
	void set_operation(int arg);

	int get_char_cnt() { return (int)m_char_cnt; }

	int get_row() { return m_row; }
	void set_row(int arg) { m_row = arg; }
	int get_col() { return m_col; }
	void set_col(int arg) { m_col = arg; }
	TCHAR get_char(int i) { return m_buf[i]; }
	TCHAR *get_char_buf() { return m_buf; }

	void set_buf_data(void *data, size_t size);
	void *get_buf_data();

	void set_data(INT_PTR data) { 
		// NOTE: m_char_cnt(m_dataとunion)は，文字列を設定したときは文字列の長さ，データを設定したときはその値を保持する
		//       そのため，文字列とデータは片方しか設定できない
		ASSERT(m_buf == NULL);	// m_buf != NULLのとき，先に文字列が登録されている
		m_data = data;
	}
	INT_PTR get_data() { return m_data; }

	int get_seq() { return m_seq; }

	CUndoData *get_next() { return m_next; }
	CUndoData *get_prev() { return m_prev; }
	CUndoData *get_first(); 
	CUndoData *get_last();

private:
	friend class CUndo;

	int realloc_undo_data_buf(size_t size);
	int init(int seq);

private:
	static const int m_default_row_size;
	static CFixedMemoryPool m_mem_pool;
	static CMemoryPool2 m_buf_mem_pool;

	int		m_seq;
	int		m_ope;
	int		m_row;
	int		m_col;
	union {
		INT_PTR		m_char_cnt;		// set_data()/get_data()でも使う
		INT_PTR		m_data;
	};
	size_t  m_buf_size;
	TCHAR	*m_buf;
	CUndoData	*m_next;
	CUndoData	*m_prev;
};

class CUndo
{
private:
	static const int m_undo_alloc_size;

	int		m_undo_cnt;
	int		m_undo_sequence;
	int		m_max_undo_cnt;
	int		m_undo_alloc_cnt;
	int		m_cur_ope;
	int		m_set_mode_cnt;
	CUndoData	*m_cur_undo_data;
	CUndoData	**m_undo_data;

	int alloc_undo_data(int cnt);
	void free_undo_data(CUndoData *undo_data);
	void free_undo_data(int cnt);

	int realloc_undo_data_arr(int cnt);
	void free_undo_data_arr();

	void set_cur_operation(int arg);

	void init(int max_undo_cnt);

	void clear_next_undo();

public:
	CUndo();
	CUndo(int max_undo_cnt);
	~CUndo();

	void reset();

	int get_cur_operation() { return m_cur_ope; }
	void clear_cur_operation() { m_cur_ope = 0; }

	CUndoData *get_cur_undo_data() { return m_cur_undo_data; }

	void incr_undo_cnt();
	void decr_undo_cnt();
	void cancel_cur_undo();

	int next_undo(int operation, BOOL set_mode = FALSE);
	int add_undo_char(TCHAR ch) { return get_cur_undo_data()->add_undo_char(ch); }
	int add_undo_chars(const TCHAR *data, int cnt) {
		return get_cur_undo_data()->add_undo_chars(data, cnt); 
	}

	void start_undo_set(BOOL save_redo_data = FALSE);
	void end_undo_set();
	int get_undo_set() { return m_set_mode_cnt; }

	BOOL can_undo();
	BOOL can_redo();
	int get_undo_sequence() {
		if(m_cur_undo_data == NULL) return 0;
		return m_cur_undo_data->get_seq();
	}
};

__inline void *CUndoData::operator new(size_t size)
{
	return m_mem_pool.alloc();
}

__inline void CUndoData::operator delete(void *p, size_t size)
{
	m_mem_pool.free(p);
}

class CUndoSetMode
{
public:
	CUndoSetMode();
	CUndoSetMode(CUndo *undo, BOOL set_now = TRUE);
	~CUndoSetMode();

	void Start(CUndo *undo, BOOL save_redo_data);
	void Start();
	void End();
	BOOL IsSetMode() { return m_b_set; }

private:
	CUndo	*m_undo;
	BOOL	m_b_set;
	BOOL	m_save_redo_data;
};


#endif _UNDO_H_INCLUDE
