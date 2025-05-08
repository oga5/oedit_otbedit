/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bm_search.h"
#include "bm_local.h"

#include "get_char.h"

void bm_free(HBM_DATA data)
{
	if(data == NULL) return;

	if(data->pattern != NULL) free(data->pattern);
	if(data->lwr_buf != NULL) free(data->lwr_buf);
	free(data);
}

static void lower_str(TCHAR *p)
{
	for(; *p;) p = put_char(p, get_char_nocase(p));
}

static int is_valid_pt(const TCHAR *p_start, const TCHAR *p)
{
	if(p_start == p) return 1;
	if(is_lead_byte2(p_start, p - p_start - 1) == 0) return 1;

	return 0;
}

static TCHAR *bm_search_nolwr(HBM_DATA bm_data,
	const TCHAR *p_text, size_t len)
{
	const TCHAR *p_start = p_text;
	size_t *skip_table = bm_data->skip_table;
	TCHAR *p = bm_data->pattern;
	size_t		p_len = bm_data->pattern_len;
	size_t		skip;
	size_t		i;
	unsigned int ch;

	if(p_len > len || p_len == 0) return NULL;

	for(; p_len <= len;) {
		for(i = p_len - 1; p[i] == p_text[i]; i--) {
			if(i == 0) {
				if(is_valid_pt(p_start, p_text)) return (TCHAR *)p_text;
				break;
			}
		}

		ch = p_text[p_len];
		skip = skip_table[ch];
		p_text += skip;
		len -= skip;
	}

	return NULL;
}

__inline void make_lwr_buf(const TCHAR *p, size_t len, TCHAR *lwr_buf)
{
	int		ch_len;

	for(; len > 0;) {
		ch_len = get_char_len(p);
		lwr_buf = put_char(lwr_buf, get_char_nocase(p));

		p += ch_len;
		len -= ch_len;
	}
	*lwr_buf = '\0';
}

static TCHAR *bm_search_lwr(HBM_DATA bm_data, const TCHAR *p_text, size_t len)
{
	const TCHAR *p_start = p_text;
	TCHAR *p_lwr;

	size_t *skip_table = bm_data->skip_table;
	TCHAR *p = bm_data->pattern;
	size_t	p_len = bm_data->pattern_len;
	size_t	skip;
	size_t	i;
	unsigned int ch;

	if(p_len > len || p_len == 0) return NULL;

	if(len >= bm_data->lwr_buf_len) {
		size_t alloc_len = len + 1;
		if(alloc_len < p_len + 1) alloc_len = p_len + 1;
		alloc_len += (1024 - (alloc_len % 1024));
		bm_data->lwr_buf = (TCHAR *)realloc(bm_data->lwr_buf,
			alloc_len * sizeof(TCHAR));
		if(bm_data->lwr_buf == NULL) return NULL;
		bm_data->lwr_buf_len = alloc_len;
	}
	p_lwr = bm_data->lwr_buf;

	make_lwr_buf(p_text, len, p_lwr);

	for(; p_len <= len;) {
		for(i = p_len - 1; p[i] == p_lwr[i]; i--) {
			if(i == 0) {
				if(is_valid_pt(p_start, p_text)) return (TCHAR *)p_text;
				break;
			}
		}

		ch = p_lwr[p_len];
		skip = skip_table[ch];

		p_text += skip;
		p_lwr += skip;
		len -= skip;
	}

	return NULL;
}

HBM_DATA bm_create_data(const TCHAR *pattern, int i_flg)
{
	HBM_DATA	data;
	UINT_PTR	i;
	size_t		p_len;

	data = (HBM_DATA)malloc(sizeof(BM_DATA));
	if(data == NULL) return NULL;

	data->lwr_buf = NULL;
	data->lwr_buf_len = 0;

	data->pattern = (TCHAR *)_tcsdup(pattern);
	if(data->pattern == NULL) {
		bm_free(data);
		return NULL;
	}

	if(i_flg) {
		lower_str(data->pattern);
		data->func = &bm_search_lwr;
	} else {
		data->func = &bm_search_nolwr;
	}

	p_len = _tcslen(pattern);
	data->pattern_len = p_len;

	for(i = 0; i < BM_SKIP_TABLE_SIZE; i++) {
		data->skip_table[i] = p_len;
	}
	for(i = 0; i < p_len; i++) {
		data->skip_table[(data->pattern[i])] = p_len - i;
	}
	data->skip_table[0] = 1;

	return data;
}

TCHAR *bm_search(HBM_DATA bm_data, const TCHAR *p_buf, INT_PTR len)
{
	return bm_data->func(bm_data, p_buf, len);
}



