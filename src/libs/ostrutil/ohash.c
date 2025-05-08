/*
 * hash manager
 *
 * Copyright (c) 2025 Atsushi Ogawa
 *
 * Based on PostgreSQL.
 * Refer to LICENSE_POSTGRESQL for respective license details.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "get_char.h"
#include "ohash.h"

typedef unsigned int  uint;

static void *_ohash_default_alloc(void *alloc_arg, size_t size)
{
	return malloc(size);
}

static void _ohash_default_free(void *alloc_arg, void *p)
{
	free(p);
}

static int _ohash_default_match(void *match_arg,
	const void *key1, const void *key2, size_t size)
{
	return memcmp(key1, key2, size);
}

#define FNV_32_PRIME ((uint)0x01000193)

uint ohash_default_hash(const void *key, size_t keysize)
{
    unsigned char *bp = (unsigned char *)key;	/* start of buffer */
    unsigned char *be = bp + keysize;			/* beyond end of buffer */
	uint	hval = 0;

    /* FNV-1 hash each octet in the buffer */
	while (bp < be) {
		/* multiply by the 32 bit FNV magic prime mod 2^32 */
		hval *= FNV_32_PRIME;

		/* xor the bottom with the current octet */
		hval ^= (uint)*bp++;
    }

    /* return our new hash value */
    return hval;
}

uint ohash_string_hash(const void *key, size_t keysize)
{
    TCHAR *s = *(TCHAR **)key;	/* start of buffer */
	uint	hval = 0;

    /* FNV-1 hash each octet in the buffer */
	while (*s) {
		/* multiply by the 32 bit FNV magic prime mod 2^32 */
		hval *= FNV_32_PRIME;

		/* xor the bottom with the current octet */
		hval ^= (uint)*s++;
    }

    /* return our new hash value */
    return hval;
}

typedef enum {
	OHASH_FIND,
	OHASH_ENTER,
	OHASH_REMOVE,
} OHASH_ACTION;

#define DEF_HASH_SEGSIZE            256
#define DEF_HASH_SEGSIZE_SHIFT      8    /* must be log2(DEF_SEGSIZE) */

#define TYPEALIGN(ALIGNVAL,LEN)  \
	(((long) (LEN) + ((ALIGNVAL) - 1)) & ~((long) ((ALIGNVAL) - 1)))
#define MAXALIGN(LEN)           TYPEALIGN(8, (LEN))

typedef struct _hash_element_st {
	struct _hash_element_st *link;
	uint					hash_value;
} _hash_element;

typedef _hash_element *_hash_bucket;
typedef _hash_bucket *_hash_segment;

typedef struct _hash_element_block_st {
	struct _hash_element_block_st *link;
} _hash_element_block;
	
typedef struct _ohash_st {
	_hash_segment		*dir;
	_hash_element		*free_list;
	_hash_element_block	*element_block_list;
	uint				nentries;

	uint				dir_size;
	uint				nsegs;
	uint				max_bucket;
	uint				high_mask;
	uint				low_mask;

	uint				key_size;
	uint				entry_size;
	uint				seg_size;
	uint				seg_shift;
	uint				fill_factor;
	uint				nelem_alloc;

	_hash_alloc_func	alloc;
	void				*alloc_arg;
	_hash_free_func		free;
	_hash_value_func	hash;
	_hash_match_func	match;
	void				*match_arg;
} _ohash_table;

struct _ohash_iter_st {
	ohash			h;
	uint			cur_bucket;
	_hash_element	*cur_element;
};

#define OHASH_ENTRY(el) (((char *)(el)) + MAXALIGN(sizeof(_hash_element)))

/* fast mod */
#define OHASH_MOD(x,y)               ((x) & ((y)-1))

/* calculate ceil(log base 2) of num */
static int _ohash_log2(long num)
{
    int         i;
    long        limit;

    for (i = 0, limit = 1; limit < num; i++, limit <<= 1)
        ;
    return i;
}

void ohash_delete(ohash h)
{
	unsigned int		i;
	_hash_free_func	free_func;
	void	*alloc_arg;

	if(h == NULL || h->free == NULL) return;

	free_func = h->free;
	alloc_arg = h->alloc_arg;

	if(h->dir) {
		for(i = 0; i < h->nsegs; i++) free_func(alloc_arg, h->dir[i]);
		free_func(alloc_arg, h->dir);
	}

	if(h->element_block_list) {
		_hash_element_block *_next_block = h->element_block_list;
		_hash_element_block *_free_block;

		for(; _next_block != NULL;) {
			_free_block = _next_block;
			_next_block = _next_block->link;
			free_func(alloc_arg, _free_block);
		}
	}

	free_func(alloc_arg, h);
}

static _hash_segment _ohash_seg_alloc(ohash h)
{
	_hash_segment segp;
	uint alloc_size = sizeof(_hash_bucket) * h->seg_size;
	
	segp = (_hash_segment)h->alloc(h->alloc_arg, alloc_size);
	memset(segp, 0, alloc_size);

	return segp;
}

static int _ohash_choose_nelem_alloc(ohash h, int nelem)
{
	int nelem_alloc = 128;
	int	alloc_size = h->entry_size * nelem_alloc;

	for(; h->entry_size * nelem_alloc > (1024 * 256); ) {
		nelem_alloc = nelem_alloc >> 1;
	}
	if(nelem_alloc > nelem) nelem_alloc = nelem;
	if(nelem_alloc < 8) nelem_alloc = 8;
	
	return nelem_alloc;
}

static int _ohash_init(ohash h, int nelem)
{
	uint	lnbuckets;
	uint	nbuckets;
	uint	nsegs;

	lnbuckets = (nelem - 1) / h->fill_factor + 1;
	nbuckets = 1 << _ohash_log2(lnbuckets);

	h->max_bucket = h->low_mask = nbuckets - 1;
	h->high_mask = (nbuckets << 1) - 1;

	nsegs = (nbuckets - 1) / h->seg_size + 1;
	nsegs = 1 << _ohash_log2(nsegs);

	h->dir_size = nsegs;

	h->dir = (_hash_segment *)h->alloc(h->alloc_arg,
		h->dir_size * sizeof(_hash_segment));
	if(h->dir == NULL) return 1;

	for(; h->nsegs < nsegs; h->nsegs++) {
		h->dir[h->nsegs] = _ohash_seg_alloc(h);
		if(h->dir[h->nsegs] == NULL) return 1;
	}

	h->nelem_alloc = _ohash_choose_nelem_alloc(h, nelem);

#ifdef _HASH_DEBUG
	printf(
		"lnbuckets %d\n"
		"nbuckets %d\n"
		"h->max_bucket %d\n"
		"h->low_mask %x\n"
		"h->high_mask %x\n"
		"h->nsegs %d\n"
		"h->nelem_alloc %d\n",
		lnbuckets, nbuckets,
		h->max_bucket, h->low_mask, h->high_mask,
		h->nsegs, h->nelem_alloc);
#endif

	return 0;
}

ohash ohash_create(int nelem, int key_size, int entry_size,
	_hash_value_func hash_func,
	_hash_alloc_func alloc_func, _hash_free_func free_func, void *alloc_arg,
	_hash_match_func match_func, void *match_arg)
{
	ohash	h;

	if(alloc_func == NULL) {
		alloc_func = _ohash_default_alloc;
		free_func = _ohash_default_free;
	}
	if(match_func == NULL) match_func = _ohash_default_match;
	if(hash_func == NULL) hash_func = ohash_default_hash;

	h = (ohash)alloc_func(alloc_arg, sizeof(_ohash_table));
	if(h == NULL) return NULL;

	h->hash = hash_func;
	h->alloc = alloc_func;
	h->alloc_arg = alloc_arg;
	h->free = free_func;
	h->match = match_func;
	h->match_arg = match_arg;

	h->dir = NULL;
	h->free_list = NULL;
	h->element_block_list = NULL;
	h->nentries = 0;

	h->dir_size = 0;
	h->nsegs = 0;
	h->max_bucket = 0;
	h->high_mask = 0;
	h->low_mask = 0;

	h->key_size = key_size;
	h->entry_size = MAXALIGN(entry_size);
	h->seg_size = DEF_HASH_SEGSIZE;
	h->seg_shift = DEF_HASH_SEGSIZE_SHIFT;
	h->fill_factor = 1;
	h->nelem_alloc = 0;

	if(_ohash_init(h, nelem) != 0) {
		ohash_delete(h);
		return NULL;
	}

	return h;
}

static uint _ohash_calc_bucket(ohash h, uint hash_value)
{
	uint		bucket;
	bucket = hash_value & h->high_mask;
	if(bucket > h->max_bucket) bucket = bucket & h->low_mask;
	return bucket;
}

static int _ohash_element_alloc(ohash h, int nelem)
{
	_hash_element_block	*block;
	uint			element_size;
	_hash_element	*first_element;
	_hash_element	*prev_element;
	_hash_element	*tmp_element;
	uint			block_size;
	int				i;

	element_size = MAXALIGN(sizeof(_hash_element)) + MAXALIGN(h->entry_size);

	block_size = MAXALIGN(sizeof(_hash_element_block)) + (nelem * element_size);
	block = (_hash_element_block*)h->alloc(h->alloc_arg, block_size);
	if(block == NULL) return 1;

	block->link = h->element_block_list;
	h->element_block_list = block;

	first_element = (_hash_element *)(((char *)block) +
		MAXALIGN(sizeof(_hash_element_block)));

	prev_element = NULL;
	tmp_element = first_element;
	for(i = 0; i < nelem; i++) {
		tmp_element->link = prev_element;
		prev_element = tmp_element;
		tmp_element = (_hash_element *)(((char *)tmp_element) + element_size);
	}

	first_element->link = h->free_list;
	h->free_list = prev_element;

	return 0;
}

static _hash_bucket _ohash_get_entry(ohash h)
{
	_hash_bucket	new_entry; 

	if(h->free_list == NULL) {
		if(_ohash_element_alloc(h, h->nelem_alloc) != 0) return NULL;
		if(h->free_list == NULL) return NULL;
	}

	new_entry = h->free_list;
	h->free_list = new_entry->link;
	h->nentries++;

	return new_entry;
}

static void *_ohash_action_with_hash_value(ohash h, uint hash_value,
	const void *key, OHASH_ACTION action)
{
	uint				bucket;
	uint				seg_num;
	uint				seg_idx;
	_hash_bucket		cur_bucket;
	_hash_bucket		*prev_bucket_ptr;
	_hash_match_func	match;
	void				*match_arg;
	uint				key_size;

	bucket = _ohash_calc_bucket(h, hash_value);
	seg_num = bucket >> h->seg_shift;
	seg_idx = OHASH_MOD(bucket, h->seg_size);

	if(h->dir[seg_num] == NULL) {
		fprintf(stderr, "hash corrupted!!\n");
		exit(1);
	}

	prev_bucket_ptr = &(h->dir[seg_num][seg_idx]);
	cur_bucket = *prev_bucket_ptr;

	match = h->match;
	match_arg = h->match_arg;
	key_size = h->key_size;

	while(cur_bucket) {
		if(cur_bucket->hash_value == hash_value &&
			match(match_arg, OHASH_ENTRY(cur_bucket), key, key_size) == 0) {
			break;
		}

		prev_bucket_ptr = &(cur_bucket->link);
		cur_bucket = *prev_bucket_ptr;
	}

	if(action == OHASH_FIND) {
		if(cur_bucket != NULL) return OHASH_ENTRY(cur_bucket);
		return NULL;
	}
	if(action == OHASH_ENTER) {
		if(cur_bucket != NULL) return OHASH_ENTRY(cur_bucket);

		cur_bucket = _ohash_get_entry(h);
		if(cur_bucket == NULL) return NULL;

		*prev_bucket_ptr = cur_bucket;
		cur_bucket->link = NULL;

		cur_bucket->hash_value = hash_value;
		memcpy(OHASH_ENTRY(cur_bucket), key, key_size);

		return OHASH_ENTRY(cur_bucket);
	}
	if(action == OHASH_REMOVE) {
		if(cur_bucket == NULL) return NULL;

		h->nentries--;
		*prev_bucket_ptr = cur_bucket->link;
		cur_bucket->link = h->free_list;
		h->free_list = cur_bucket;

		return cur_bucket;
	}

	return NULL;
}

unsigned int ohash_num_entries(ohash h)
{
	return h->nentries;
}

void *ohash_search(ohash h, const void *key)
{
	int		hash_value = h->hash(key, h->key_size);
	return _ohash_action_with_hash_value(h, hash_value, key, OHASH_FIND);
}

void *ohash_remove(ohash h, const void *key)
{
	int		hash_value = h->hash(key, h->key_size);
	return _ohash_action_with_hash_value(h, hash_value, key, OHASH_REMOVE);
}

void *ohash_enter(ohash h, const void *key)
{
	int		hash_value = h->hash(key, h->key_size);
	return _ohash_action_with_hash_value(h, hash_value, key, OHASH_ENTER);
}

ohash_iter ohash_iter_init(ohash h)
{
	ohash_iter iter;
	if(h == NULL) return NULL;

	iter = (ohash_iter)h->alloc(h->alloc_arg, sizeof(struct _ohash_iter_st));
	iter->h = h;
	iter->cur_bucket = 0;
	iter->cur_element = NULL;

	return iter;
}

void ohash_iter_delete(ohash_iter iter)
{
	ohash	h;

	if(iter == NULL || iter->h == NULL) return;
	h = iter->h;
	if(h->free) h->free(h->alloc_arg, iter);
}

void *ohash_iter_next(ohash_iter iter)
{
	ohash	h;
	uint	cur_bucket;
	uint	max_bucket;
	uint	seg_size;
	uint	seg_num;
	uint	seg_idx;
	_hash_segment segp;
	_hash_element *cur_element;

	cur_element = iter->cur_element;
	if(cur_element != NULL) {
		iter->cur_element = cur_element->link;
		if(iter->cur_element == NULL) iter->cur_bucket++;
		return (void *)OHASH_ENTRY(cur_element);
	}

	h = iter->h;
	max_bucket = h->max_bucket;
	cur_bucket = iter->cur_bucket;

	if(cur_bucket > max_bucket) return NULL;

	seg_size = h->seg_size;
	seg_num = cur_bucket >> h->seg_shift;
	seg_idx = OHASH_MOD(cur_bucket, seg_size);
	segp = h->dir[seg_num];

	for(;;) {
		cur_element = segp[seg_idx];
		if(cur_element != NULL) break;

		cur_bucket++;
		if(cur_bucket > max_bucket) {
			iter->cur_bucket = cur_bucket;
			return NULL;
		}

		seg_idx++;
		if(seg_idx >= seg_size) {
			seg_num++;
			seg_idx = 0;
			segp = h->dir[seg_num];
		}
	}

	iter->cur_element = cur_element->link;
	if(iter->cur_element == NULL) cur_bucket++;
	iter->cur_bucket = cur_bucket;
	return (void *)OHASH_ENTRY(cur_element);
}

