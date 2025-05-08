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

#ifndef OHASH_H_INCLUDED
#define OHASH_H_INCLUDED

typedef void *(*_hash_alloc_func)(void *alloc_arg, size_t size);
typedef void (*_hash_free_func)(void *free_arg, void *p);
typedef unsigned int (*_hash_value_func)(const void *key, size_t keysize);
typedef int (*_hash_match_func)(
    void *match_arg, const void *key1, const void *key2, size_t keysize);

unsigned int ohash_default_hash(const void *key, size_t keysize);
unsigned int ohash_string_hash(const void *key, size_t keysize);

typedef struct _ohash_st *ohash;
typedef struct _ohash_iter_st *ohash_iter;

ohash ohash_create(int nelem, int key_size, int entry_size,
	_hash_value_func hash_func,
    _hash_alloc_func alloc_func, _hash_free_func free_func, void *alloc_arg,
	_hash_match_func match_func, void *match_arg);
void ohash_delete(ohash h);

void *ohash_search(ohash h, const void *key);
void *ohash_remove(ohash h, const void *key);
void *ohash_enter(ohash h, const void *key);

unsigned int ohash_num_entries(ohash h);

ohash_iter ohash_iter_init(ohash h);
void ohash_iter_delete(ohash_iter iter);
void *ohash_iter_next(ohash_iter iter);

#endif /* OHASH_H_INCLUDED */
