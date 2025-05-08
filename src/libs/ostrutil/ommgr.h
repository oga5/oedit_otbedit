/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * Based on PostgreSQL.
 * Refer to LICENSE_POSTGRESQL for respective license details.
 * 
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef OMMGR_INCLUDED
#define OMMGR_INCLUDED

#ifdef  __cplusplus
extern "C" {
#endif

#include <basetsd.h>

typedef struct _alloc_set_st *alloc_set;

alloc_set alloc_set_create(size_t init_block_size, size_t max_block_size);
void alloc_set_delete(alloc_set aset);

void *alloc_set_alloc(alloc_set aset, size_t size);
void *alloc_set_realloc(alloc_set aset, void *p, size_t size);
void alloc_set_free(void *p);

size_t alloc_set_get_chunk_size(void *p);

#ifdef  __cplusplus
}
#endif

#endif /* OMMGR_INCLUDED */

