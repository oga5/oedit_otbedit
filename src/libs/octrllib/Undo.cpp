/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"

#include "Undo.h"
#include <malloc.h>

const int CUndo::m_undo_alloc_size = 64;
const int CUndoData::m_default_row_size = 32;

CFixedMemoryPool CUndoData::m_mem_pool(sizeof(CUndoData), 128);
CMemoryPool2 CUndoData::m_buf_mem_pool(sizeof(TCHAR) * CUndoData::m_default_row_size, 64);

int CUndoData::realloc_undo_data_buf(size_t size)
{
	if(size == 0) size = m_default_row_size;
	if(m_buf == NULL) {
		m_buf = (TCHAR *)m_buf_mem_pool.alloc(size);
		if(m_buf == NULL) return 1;
	} else {
		TCHAR *tmp = (TCHAR *)m_buf_mem_pool.alloc(size);
		if(tmp == NULL) return 1;
		memcpy(tmp, m_buf, min(m_buf_size, size));
		m_buf_mem_pool.free(m_buf);
		m_buf = tmp;
	}

	m_buf_size = size;

	return 0;
}

int CUndoData::init(int seq)
{
	m_ope = 0;
	m_seq = seq;

	m_char_cnt = 0;
	m_data = 0;

	m_buf_size = 0;
	m_row = 0;
	m_col = 0;
	m_buf = NULL;
	m_next = NULL;
	m_prev = NULL;

	return 0;
}

CUndoData::CUndoData(int seq)
{
	init(seq);
}

CUndoData::CUndoData(int seq, CUndoData *prev)
{
	init(seq);

	if(prev != NULL) {
		m_prev = prev;
		prev->m_next = this;
	}
}

CUndoData::~CUndoData()
{
	if(m_buf != NULL) m_buf_mem_pool.free(m_buf);
}

#pragma intrinsic(memcpy)
int CUndoData::add_undo_chars(const TCHAR *data, int cnt)
{
	if(cnt == -1) cnt = (int)_tcslen(data);

	if(cnt == 0) return 0;

	size_t mem_size = (m_char_cnt + cnt) * sizeof(TCHAR);
	if(m_buf_size <= mem_size) {
		size_t buf_size = m_buf_size;
		if(buf_size == 0) {
			ASSERT(m_char_cnt == 0);	// m_char_cnt != 0のとき，先にset_data()で使われている
			buf_size = m_default_row_size * sizeof(TCHAR);
		}
		for(; buf_size <= mem_size;) {
			buf_size *= 2;
		}

		if(realloc_undo_data_buf(buf_size) != 0) return 1;
	}

	memcpy(m_buf + m_char_cnt, data, cnt * sizeof(TCHAR));
	m_char_cnt += cnt;

	return 0;
}
#pragma function(memcpy)

int CUndoData::add_null_char()
{
	if(m_buf_size <= (m_char_cnt * sizeof(TCHAR))) {
		if(realloc_undo_data_buf(m_buf_size * 2) != 0) return 1;
	}
	m_buf[m_char_cnt] = '\0';

	return 0;
}

void CUndoData::set_operation(int arg)
{
	m_ope = arg;
	m_char_cnt = 0;
}

CUndoData *CUndoData::get_first()
{ 
	CUndoData	*data;
	for(data = this; data->m_prev != NULL; data = data->m_prev) ;
	return data;
}

CUndoData *CUndoData::get_last()
{ 
	CUndoData	*data;
	for(data = this; data->m_next != NULL; data = data->m_next) ;
	return data;
}

void CUndoData::set_buf_data(void *data, size_t size)
{
	if(realloc_undo_data_buf(size) != 0) return;

	memcpy(m_buf, data, size);
}

void *CUndoData::get_buf_data()
{
	return (void *)m_buf;
}

///////////////////////////////////////////////////////////////////////////////////////////////
int CUndo::alloc_undo_data(int cnt)
{
	if(m_undo_data[cnt] != NULL) free_undo_data(cnt);

	m_undo_data[cnt] = new CUndoData(m_undo_sequence);
	if(m_undo_data[cnt] == NULL) {
		MessageBox(NULL, _T("メモリが確保できません"), _T("メッセージ"), MB_ICONINFORMATION | MB_OK);
		return 1;
	}

	return 0;
}

void CUndo::free_undo_data(int cnt)
{
	if(m_undo_data[cnt] != NULL) {
		free_undo_data(m_undo_data[cnt]->get_first());
		m_undo_data[cnt] = NULL;
	}
}

void CUndo::free_undo_data(CUndoData *undo_data)
{
	if(undo_data == NULL) return;

	CUndoData *data, *next_data;
	
	for(data = undo_data; data != NULL;) {
		next_data = data->get_next();
		delete data;
		data = next_data;
	}
}

void CUndo::free_undo_data_arr()
{
	int		i;

	if(m_undo_data != NULL) {
		for(i = 0; i < m_undo_alloc_cnt; i++) {
			free_undo_data(i);
		}
		free(m_undo_data);
		m_undo_data = NULL;
	}
}

int CUndo::realloc_undo_data_arr(int cnt)
{
	int		i;

	m_undo_data = (CUndoData **)realloc(m_undo_data, sizeof(CUndoData *) * cnt);
	if(m_undo_data == NULL) {
		MessageBox(NULL, _T("メモリが確保できません"), _T("メッセージ"), MB_ICONINFORMATION | MB_OK);
		return 1;
	}

	for(i = m_undo_alloc_cnt; i < cnt; i++) {
		m_undo_data[i] = NULL;
	}

	m_undo_alloc_cnt = cnt;

	return 0;
}

void CUndo::set_cur_operation(int arg)
{
	m_cur_ope = arg;
	get_cur_undo_data()->set_operation(arg);
};

void CUndo::init(int max_undo_cnt)
{
	m_undo_data = NULL;
	m_undo_alloc_cnt = 0;
	m_cur_ope = 0;
	m_undo_sequence = 0;
	m_undo_cnt = 0;
	m_max_undo_cnt = max_undo_cnt;
	m_cur_undo_data = NULL;

	m_set_mode_cnt = 0;

	realloc_undo_data_arr(min(m_max_undo_cnt, m_undo_alloc_size));
}

CUndo::CUndo()
{
	init(2);	// 1回分のUndoデータ
}

CUndo::CUndo(int max_undo_cnt)
{
	init(max_undo_cnt);
}

CUndo::~CUndo()
{
	free_undo_data_arr();
}

void CUndo::reset()
{
	free_undo_data_arr();
	init(m_max_undo_cnt);
}

void CUndo::incr_undo_cnt()
{
	m_undo_cnt++;
	if(m_undo_cnt == m_max_undo_cnt) m_undo_cnt = 0;
	m_cur_undo_data = m_undo_data[m_undo_cnt];
}

void CUndo::decr_undo_cnt()
{
	m_undo_cnt--;
	if(m_undo_cnt == -1) m_undo_cnt = m_max_undo_cnt - 1;
	m_cur_undo_data = m_undo_data[m_undo_cnt];
}

void CUndo::clear_next_undo()
{
	// 1つ先のundo情報を空にしておく
	if(m_undo_cnt + 1 == m_max_undo_cnt) {
		free_undo_data(0);
	} else {
		if(m_undo_cnt + 1 == m_undo_alloc_cnt) {
			realloc_undo_data_arr(m_undo_alloc_cnt + m_undo_alloc_size);
		}
		free_undo_data(m_undo_cnt + 1);
	}
}

void CUndo::cancel_cur_undo()
{
	if(!can_undo()) return;
	decr_undo_cnt();
	clear_next_undo();
}

int CUndo::next_undo(int operation, BOOL set_mode)
{
	if(get_undo_set() == 0 && set_mode == FALSE) {
		m_undo_sequence++;
		incr_undo_cnt();

		clear_next_undo();

		if(alloc_undo_data(m_undo_cnt) != 0) return 1;

		m_cur_undo_data = m_undo_data[m_undo_cnt];
		set_cur_operation(operation);
	} else {
		new CUndoData(m_undo_sequence, get_cur_undo_data());

		m_cur_undo_data = get_cur_undo_data()->get_next();
		set_cur_operation(operation);
	}

	return 0;
}

void CUndo::start_undo_set(BOOL save_redo_data)
{
	if(save_redo_data && m_set_mode_cnt == 0) {
		// set modeの開始時にredo可能だった場合、現在のredo情報を破壊しないようにする
		// set mode中にデータが更新されなかったら、redo情報を復元する
		// この時点ではUndoバッファに登録せずに、UndoDataを作成する
		clear_cur_operation();
		m_undo_sequence++;
		m_cur_undo_data = new CUndoData(m_undo_sequence, NULL);
		m_cur_undo_data->set_data(1);
	} else {
		// リストの先頭に，空のデータを作る
		next_undo(0);
	}
	m_set_mode_cnt++;
}

void CUndo::end_undo_set()
{
	ASSERT(m_set_mode_cnt > 0);

	m_set_mode_cnt--;
	if(m_set_mode_cnt > 0) return;

	if(get_cur_undo_data() == NULL) return;

	CUndoData *first_undo = get_cur_undo_data()->get_first();

	// 空のUNDO DATAか調べる
	for(CUndoData *undo_data = first_undo; undo_data != NULL; undo_data = undo_data->get_next()) {
		if(undo_data->get_operation() != 0) {
			if(first_undo->get_data() != 0) {
				// redo情報を破壊しないモードのとき、Undoバッファに登録する
				next_undo(0);
				m_cur_undo_data->m_next = first_undo;
			}
			return;
		}
	}

	// 空だった場合、元に戻す
	if(first_undo->get_data() == 0) {
		decr_undo_cnt();
		clear_next_undo();
	} else {
		m_cur_undo_data = m_undo_data[m_undo_cnt];
		free_undo_data(first_undo);
	}
}

BOOL CUndo::can_undo()
{
	if(m_undo_data[m_undo_cnt] == NULL) return FALSE;

	return TRUE;
}

BOOL CUndo::can_redo()
{
	int undo_cnt = m_undo_cnt + 1;
	if(undo_cnt == m_max_undo_cnt) undo_cnt = 0;

	if(m_undo_data[undo_cnt] == NULL) return FALSE;
	
	return TRUE;
}

CUndoSetMode::CUndoSetMode()
{
	m_undo = NULL;
	m_b_set = FALSE;
	m_save_redo_data = FALSE;
}

CUndoSetMode::CUndoSetMode(CUndo *undo, BOOL set_now)
{
	m_undo = undo;
	m_b_set = FALSE;
	m_save_redo_data = FALSE;
	if(set_now) Start();
}

CUndoSetMode::~CUndoSetMode()
{
	End();
}

void CUndoSetMode::Start(CUndo *undo, BOOL save_redo_data)
{
	if(m_b_set) return;
	m_undo = undo;
	m_save_redo_data = save_redo_data;
	Start();
}

void CUndoSetMode::Start()
{
	if(m_b_set || m_undo == NULL) return;

	m_undo->start_undo_set(m_save_redo_data);
	m_b_set = TRUE;
}

void CUndoSetMode::End()
{
	if(!m_b_set || m_undo == NULL) return;

	m_undo->end_undo_set();
	m_b_set = FALSE;
}
