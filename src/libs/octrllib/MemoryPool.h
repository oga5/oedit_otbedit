/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef _MEMORY_POOL_H_INCLUDE
#define _MEMORY_POOL_H_INCLUDE

#include "ommgr.h"

struct memory_pool_free_list_st {
	struct memory_pool_free_list_st *next;
};

struct memory_pool_block_list_st {
	struct memory_pool_free_list_st *pool;
	struct memory_pool_block_list_st *next;
};

class CMemoryPool
{
public:
	CMemoryPool(size_t alloc_size, int alloc_cnt);
	~CMemoryPool();

	void *alloc(size_t n);
	void free(void *p, size_t n);

	size_t get_alloc_size() const { return m_alloc_size; }

protected:
	int alloc_block();

	size_t		m_alloc_size;
	int			m_alloc_cnt;

	struct memory_pool_block_list_st *m_block_list;
	struct memory_pool_free_list_st *m_free_list;

private:
	CMemoryPool() { 
		m_alloc_size = 0; 
		m_alloc_cnt = 0;
		m_block_list = NULL;
		m_free_list = NULL;
	};
};

class CFixedMemoryPool : public CMemoryPool
{
public:
	CFixedMemoryPool(size_t alloc_size, int alloc_cnt);

	void *alloc() { return alloc(m_alloc_size); }
	void free(void *p) { free(p, m_alloc_size);	}

	void *calloc();

private:
	CFixedMemoryPool();

	void *alloc(size_t n);
	void free(void *p, size_t n);
};

__inline void *CFixedMemoryPool::alloc(size_t n)
{
	if(m_free_list == NULL) alloc_block();

	void *p = m_free_list;
	m_free_list = m_free_list->next;

	return p;
}

__inline void *CFixedMemoryPool::calloc()
{
	void *p = alloc();
	if(p == NULL) return NULL;

	memset(p, 0, m_alloc_size);
	return p;
}

__inline void CFixedMemoryPool::free(void *p, size_t n)
{
	if(p == NULL) return;

	struct memory_pool_free_list_st *free_list = 
		static_cast<struct memory_pool_free_list_st *>(p);
	free_list->next = m_free_list;
	m_free_list = free_list;
}


class CMemoryPool2
{
private:
	alloc_set	m_aset;

public:
	CMemoryPool2(size_t init_block_size, size_t max_block_size)
	{
		m_aset = alloc_set_create(init_block_size, max_block_size);
	}
	~CMemoryPool2()
	{
		alloc_set_delete(m_aset);
	}

	void *alloc(size_t n)
	{
		return alloc_set_alloc(m_aset, n);
	}
	void free(void *p)
	{
		alloc_set_free(p);
	}
	void *realloc(void *p, size_t n)
	{
		return alloc_set_realloc(m_aset, p, n);
	}
	size_t get_chunk_size(void *p)
	{
		return alloc_set_get_chunk_size(p);
	}
};

#endif _MEMORY_POOL_H_INCLUDE
