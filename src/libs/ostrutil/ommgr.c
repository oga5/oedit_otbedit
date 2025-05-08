/*
 * memory manager
 *
 * Copyright (c) 2025 Atsushi Ogawa
 *
 * Based on PostgreSQL.
 * Refer to LICENSE_POSTGRESQL for respective license details.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include <stdlib.h>
#include <string.h>
#include "ommgr.h"

typedef struct _alloc_block_st *alloc_block;
typedef struct _alloc_chunk_st *alloc_chunk;

#ifdef WIN32
#define INLINE _inline
#elif __GNUC__
#define INLINE inline
#else
#define INLINE
#endif

#define TYPEALIGN(ALIGNVAL,LEN)  \
	(((long) (LEN) + ((ALIGNVAL) - 1)) & ~((long) ((ALIGNVAL) - 1)))
#define MAXALIGN(LEN)		TYPEALIGN(8, (LEN))

#define ALLOC_BLOCKHDRSZ	MAXALIGN(sizeof(struct _alloc_block_st))
#define ALLOC_CHUNKHDRSZ	MAXALIGN(sizeof(struct _alloc_chunk_st))

#define ALLOC_MINBITS       3   /* smallest chunk size is 8 bytes */
#define ALLOCSET_NUM_FREELISTS  11
#define ALLOC_CHUNK_LIMIT   (1 << (ALLOCSET_NUM_FREELISTS-1+ALLOC_MINBITS))

#define AllocPointerGetChunk(ptr) \
	((alloc_chunk)(((char *)(ptr)) - ALLOC_CHUNKHDRSZ))
#define AllocChunkGetPointer(chk) \
	((void *)(((char *)(chk)) + ALLOC_CHUNKHDRSZ))

struct _alloc_block_st {
	alloc_set		aset;
	alloc_block		next;
	char			*free_ptr;
	char			*end_ptr;
};

struct _alloc_chunk_st {
	union {
		alloc_set	aset;
		alloc_chunk	next;
	} _p;
	size_t			size;
};

struct _alloc_set_st {
	alloc_block		blocks;
	alloc_chunk		freelist[ALLOCSET_NUM_FREELISTS];
	size_t			init_block_size;
	size_t			next_block_size;
	size_t			max_block_size;
	size_t			alloc_chunk_limit;
};

static INLINE int alloc_set_free_index(size_t size)
{
	int         idx = 0;

	if (size > 0) {
		size = (size - 1) >> ALLOC_MINBITS;
		while (size != 0) {
			idx++;
			size >>= 1;
		}
	}

	return idx;
}

static alloc_block alloc_set_alloc_block(alloc_set aset, size_t size)
{
	alloc_block block;

	block = (alloc_block)malloc(size);
	if(block == NULL) return NULL;

	block->aset = aset;
	block->free_ptr = ((char *)block) + ALLOC_BLOCKHDRSZ;
	block->end_ptr = ((char *)block) + size;

	return block;
}

alloc_set alloc_set_create(size_t init_block_size, size_t max_block_size)
{
	alloc_set	aset = NULL;
	int			i;
	size_t		max_chunk_size;

	aset = (alloc_set)malloc(sizeof(struct _alloc_set_st));
	if(aset == NULL) goto ERR1;

	aset->blocks = NULL;
	for(i = 0; i < ALLOCSET_NUM_FREELISTS; i++) aset->freelist[i] = NULL;

	init_block_size = MAXALIGN(init_block_size);
	if(init_block_size < 1024) init_block_size = 1024;

	max_block_size = MAXALIGN(max_block_size);
	if(max_block_size < init_block_size) max_block_size = init_block_size;

	aset->init_block_size = init_block_size;
	aset->max_block_size = max_block_size;
	aset->next_block_size = init_block_size;

	max_chunk_size = max_block_size - ALLOC_BLOCKHDRSZ - ALLOC_CHUNKHDRSZ;
	aset->alloc_chunk_limit = ALLOC_CHUNK_LIMIT;
	while (aset->alloc_chunk_limit > max_chunk_size) {
		aset->alloc_chunk_limit >>= 1;
	}

	return aset;

ERR1:
	if(aset != NULL) free(aset);
	return NULL;
}

void alloc_set_delete(alloc_set aset)
{
	alloc_block block;

	if(aset == NULL) return;

	block = aset->blocks;
	while(block != NULL) {
		alloc_block next = block->next;
		free(block);
		block = next;
	}

	free(aset);
}

static void alloc_set_availspace_to_freelist(alloc_set aset, alloc_block block)
{
	size_t		availspace = block->end_ptr - block->free_ptr;
	alloc_chunk	chunk;

	while(availspace >= ((1 << ALLOC_MINBITS) + ALLOC_CHUNKHDRSZ)) {
		size_t	availchunk = availspace - ALLOC_CHUNKHDRSZ;
		INT_PTR		fidx = alloc_set_free_index(availchunk);

		if(availchunk != (size_t)((size_t)1 << (fidx + ALLOC_MINBITS))) {
			fidx--;
			availchunk = ((INT_PTR)1 << (fidx + ALLOC_MINBITS));
		}

		chunk = (alloc_chunk)(block->free_ptr);
		block->free_ptr += (availchunk + ALLOC_CHUNKHDRSZ);
		availspace -= (availchunk + ALLOC_CHUNKHDRSZ);

		chunk->size = availchunk;
		chunk->_p.next = aset->freelist[fidx];
		aset->freelist[fidx] = chunk;
	}
}

void *alloc_set_alloc(alloc_set aset, size_t size)
{
	alloc_block		block;
	alloc_chunk		chunk;
	int				fidx;
	size_t			chunk_size;
	size_t			block_size;

	if(size > aset->alloc_chunk_limit) {
		chunk_size = MAXALIGN(size);
		if(chunk_size % 1024 != 0) {
			chunk_size = chunk_size + 1024 - (chunk_size % 1024);
		}

		block_size = chunk_size + ALLOC_BLOCKHDRSZ + ALLOC_CHUNKHDRSZ;

		block = alloc_set_alloc_block(aset, block_size);
		if(block == NULL) return NULL;

		if(aset->blocks != NULL) {
			block->next = aset->blocks->next;
			aset->blocks->next = block;
		} else {
			block->next = NULL;
			aset->blocks = block;
		}

		block->free_ptr = block->end_ptr;
		chunk = (alloc_chunk)(((char *)block) + ALLOC_BLOCKHDRSZ);
		chunk->_p.aset = aset;
		chunk->size = chunk_size;

		return AllocChunkGetPointer(chunk);
	}

	fidx = alloc_set_free_index(size);
	chunk = aset->freelist[fidx];
	if(chunk != NULL) {
		aset->freelist[fidx] = chunk->_p.next;
		chunk->_p.aset = aset;
		return AllocChunkGetPointer(chunk);
	}

	chunk_size = ((size_t)1 << ALLOC_MINBITS) << fidx;
	block = aset->blocks;
	if(block != NULL) {
		size_t	availspace = block->end_ptr - block->free_ptr;
		if(availspace < (chunk_size + ALLOC_CHUNKHDRSZ)) {
			alloc_set_availspace_to_freelist(aset, block);
			block = NULL;
		}
	}

	if(block == NULL) {
		size_t required_size;

		block_size = aset->next_block_size;
		aset->next_block_size <<= 1;
		if(aset->next_block_size > aset->max_block_size) {
			aset->next_block_size = aset->max_block_size;
		}

		required_size = chunk_size + ALLOC_BLOCKHDRSZ + ALLOC_CHUNKHDRSZ;
		while(block_size < required_size) block_size <<= 1;

		block = alloc_set_alloc_block(aset, block_size);
		if(block == NULL) return NULL;

		block->next = aset->blocks;
		aset->blocks = block;
	}

	chunk = (alloc_chunk)(block->free_ptr);
	block->free_ptr += (chunk_size + ALLOC_CHUNKHDRSZ);

	chunk->_p.aset = aset;
	chunk->size = chunk_size;

	return AllocChunkGetPointer(chunk);
}

void alloc_set_free(void *p)
{
	alloc_chunk	chunk = AllocPointerGetChunk(p);
	alloc_set	aset = chunk->_p.aset;

	if(chunk->size > aset->alloc_chunk_limit) {
		alloc_block	block = aset->blocks;
		alloc_block	prev_block = NULL;

		while(block != NULL) {
			if(chunk == (alloc_chunk)(((char *)block) + ALLOC_BLOCKHDRSZ)) {
				break;
			}
			prev_block = block;
			block = block->next;
		}

		if(block != NULL) {
			if(prev_block == NULL) {
				aset->blocks = block->next;
			} else {
				prev_block->next = block->next;
			}
			free(block);
		}
	} else {
		int		fidx = alloc_set_free_index(chunk->size);
		chunk->_p.next = aset->freelist[fidx];
		aset->freelist[fidx] = chunk;
	}
}

void *alloc_set_realloc(alloc_set aset, void *p, size_t size)
{
	alloc_chunk	old_chunk;
	void		*new_p;
	int			fidx;
	size_t		new_chunk_size;

	if(p == NULL) return alloc_set_alloc(aset, size);

	fidx = alloc_set_free_index(size);
	new_chunk_size = ((size_t)1 << ALLOC_MINBITS) << fidx;

	old_chunk = AllocPointerGetChunk(p);
	if(new_chunk_size == old_chunk->size) return p;

	new_p = alloc_set_alloc(aset, size);
	if(new_p == NULL) {
		alloc_set_free(p);
		return NULL;
	}

	memcpy(new_p, p, min(size, old_chunk->size));
	alloc_set_free(p);

	return new_p;
}

size_t alloc_set_get_chunk_size(void *p)
{
	alloc_chunk	chunk;
	
	if(p == NULL) return 0;

	chunk = AllocPointerGetChunk(p);

	return chunk->size;
}

