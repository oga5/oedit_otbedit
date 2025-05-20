/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */


#include "stdafx.h"

#include "MemoryPool.h"

CMemoryPool::CMemoryPool(size_t alloc_size, int alloc_cnt) : 
	m_alloc_size(alloc_size), m_alloc_cnt(alloc_cnt), m_block_list(NULL), m_free_list(NULL)
{
}

CMemoryPool::~CMemoryPool()
{
	struct memory_pool_block_list_st *p = NULL;

	for(; m_block_list != NULL;) {
		p = m_block_list;
		m_block_list = m_block_list->next;
		delete p->pool;
		delete p;
	}
}

int CMemoryPool::alloc_block()
{
	size_t offset_size = m_alloc_size;
	size_t st_size = sizeof(struct memory_pool_free_list_st);
	if(offset_size % st_size != 0) {
		offset_size += (st_size - (offset_size % st_size));
	}
	ASSERT(offset_size >= st_size && (offset_size % st_size) == 0);

	struct memory_pool_block_list_st *new_block = static_cast<struct memory_pool_block_list_st *>
		(::operator new(sizeof(struct memory_pool_block_list_st)));
	if(new_block == NULL) return 1;

	size_t alloc_size = m_alloc_cnt * offset_size;
	new_block->pool = static_cast<struct memory_pool_free_list_st *>
		(::operator new(alloc_size));
	if(new_block->pool == NULL) return 1;

	if(m_block_list == NULL) {
		m_block_list = new_block;
		m_block_list->next = NULL;
	} else {
		new_block->next = m_block_list;
		m_block_list = new_block;
	}

	size_t offset_cnt = offset_size / sizeof(struct memory_pool_free_list_st);
	int i;
	size_t offset;
	struct memory_pool_free_list_st *pool = new_block->pool;

	for(i = 0, offset = 0; i < m_alloc_cnt; i++, offset += offset_cnt) {
		pool[offset].next = &pool[offset + offset_cnt];
	}
	pool[((INT_PTR)m_alloc_cnt - 1) * offset_cnt].next = NULL;

	m_free_list = &pool[0];

	return 0;
}

void *CMemoryPool::alloc(size_t n)
{
	void *p;

	if(n != m_alloc_size) {
		p = ::operator new(n);
		if(p == NULL) goto ERR1;
	} else {
		if(m_free_list == NULL) {
			if(alloc_block() != 0) goto ERR1;
		}
		p = m_free_list;
		m_free_list = m_free_list->next;
	}

	return p;

ERR1:
	MessageBox(NULL, _T("メモリが確保できません"), _T("メッセージ"), MB_ICONINFORMATION | MB_OK);
	return NULL;
}

void CMemoryPool::free(void *p, size_t n)
{
	if(p == NULL) return;

	if(n != m_alloc_size) {
		::operator delete(p);
		return;
	}

	struct memory_pool_free_list_st *free_list = 
		static_cast<struct memory_pool_free_list_st *>(p);
	free_list->next = m_free_list;
	m_free_list = free_list;
}

CFixedMemoryPool::CFixedMemoryPool(size_t alloc_size, int alloc_cnt) : CMemoryPool(alloc_size, alloc_cnt)
{
	if(m_alloc_size < sizeof(struct memory_pool_free_list_st)) {
		m_alloc_size = sizeof(struct memory_pool_free_list_st);
	}
}
