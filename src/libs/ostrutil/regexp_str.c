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
#include <ctype.h>
#include <assert.h>
#include "local.h"

#ifdef WIN32
#include "mbutil.h"
#endif

static const TCHAR *get_char_buf_str(const void *src, OREG_POINT *pt)
{
	static const TCHAR *null_str = _T("");
	if(pt->row > 0) return null_str;
	return ((const TCHAR *)src + pt->col);
}

static const TCHAR *get_row_buf_str_len(const void *src, INT_PTR row, size_t *len)
{
	if(row > 0) return NULL;
	*len = _tcslen(src);
	return (const TCHAR *)src;
}

static const TCHAR *next_char_str(const void *src, OREG_POINT *pt)
{
	const TCHAR *p = (const TCHAR *)src;
	pt->col += get_char_len(p + pt->col);
	if(*(p + pt->col) == '\r' && *(p + pt->col + 1) == '\n') {
		pt->col++;
	}
	return p + pt->col;
}

static const TCHAR *prev_char_str(const void *src, OREG_POINT *pt)
{
	const TCHAR *p = (const TCHAR *)src;
	if(pt->row == 0 && pt->col == 0) return NULL;
	if(pt->row == 0 && pt->col == 1) {
		pt->col--;
	} else {
		pt->col -= is_lead_byte2(p, pt->col - 2) ? 2 : 1;
	}

	if(*(p + pt->col) == '\r' && *(p + pt->col + 1) == '\n' &&
		pt->col > 0) pt->col--;

	return p + pt->col;
}

static const TCHAR *next_row_str(const void *src, OREG_POINT *pt)
{
	const TCHAR *str = (const TCHAR *)src;
	const TCHAR *p = _tcschr(str + pt->col, '\n');
	if(p == NULL) return NULL;

	p++;
	pt->col = p - str;
	return p;
}

static INT_PTR get_len_str(const void *src, OREG_POINT *start_pt, OREG_POINT *end_pt)
{
	return end_pt->col - start_pt->col;
}

static int is_row_first_str(const void *src, OREG_POINT *pt)
{
	OREG_POINT tmp_pt = *pt;
	if(pt->col == 0) return 1;
	if(prev_char_str(src, &tmp_pt)[0] == '\n') return 1;
	return 0;
}

void oreg_make_str_datasrc(HREG_DATASRC data_src, const TCHAR *str)
{
	memset(data_src, 0, sizeof(OREG_DATASRC));
	data_src->src = str;
	data_src->get_char_buf = get_char_buf_str;
	data_src->get_row_buf_len = get_row_buf_str_len;
	data_src->next_char = next_char_str;
	data_src->prev_char = prev_char_str;
	data_src->next_row = next_row_str;
	data_src->get_len = get_len_str;
	data_src->is_row_first = is_row_first_str;
}

int oreg_exec_str(HREG_DATA reg_data, const TCHAR *str, INT_PTR search_start_col, INT_PTR *result_start_col, INT_PTR *len)
{
	int ret_v;
	OREG_DATASRC data_src;
	OREG_POINT  search_start = {0, 0};
	OREG_POINT  search_end = {-1, -1};
	OREG_POINT  result_start = {0, 0};
	OREG_POINT  result_end = {0, 0};

	search_start.col = search_start_col;

	*len = 0;
	if(reg_data == NULL) return OREGEXP_COMP_ERR;

	oreg_make_str_datasrc(&data_src, str);

	ret_v = oreg_exec(reg_data, &data_src, &search_start, &search_end,
		&result_start, &result_end, 0);
	if(ret_v == 1) {
		*result_start_col = result_start.col;
		*len = result_end.col - result_start.col;
	}
	return ret_v;
}

int oreg_exec_str2(HREG_DATA reg_data, const TCHAR *str)
{
	OREG_DATASRC    data_src;
 	oreg_make_str_datasrc(&data_src, str);
	if(oreg_exec2(reg_data, &data_src) == OREGEXP_FOUND) {
		return 1;
	}
	return 0;
}

static TCHAR *_alloc_result_buf(TCHAR *result_buf,
	TCHAR *org_result_buf, INT_PTR *presult_buf_size,
	INT_PTR result_pos, INT_PTR need_len, INT_PTR org_str_len)
{
	INT_PTR		need_size = (result_pos + need_len) * sizeof(TCHAR);
	if(need_size >= *presult_buf_size) {
		if(*presult_buf_size == 0) *presult_buf_size = org_str_len * 2 * sizeof(TCHAR);
		for(; need_size >= *presult_buf_size;) {
			*presult_buf_size = *presult_buf_size * 2;
		}
		if(result_buf == org_result_buf) {
			result_buf = (TCHAR *)malloc(*presult_buf_size);
			memcpy(result_buf, org_result_buf, result_pos * sizeof(TCHAR));
		} else {
			result_buf =
				(TCHAR *)realloc(result_buf, *presult_buf_size);
		}
		if(result_buf == NULL) return NULL;
	}
	return result_buf;
}

static TCHAR *_put_result(TCHAR *result_buf,
	TCHAR *org_result_buf, INT_PTR *presult_buf_size,
	INT_PTR *presult_pos, const TCHAR *str, INT_PTR copy_len, INT_PTR org_str_len)
{
	if(copy_len == 0) return result_buf;

	result_buf = _alloc_result_buf(result_buf, org_result_buf,
		presult_buf_size, *presult_pos, copy_len, org_str_len);
	if(result_buf == NULL) return result_buf;

	memcpy(result_buf + *presult_pos, str, copy_len * sizeof(TCHAR));
	*presult_pos = *presult_pos + copy_len;
	return result_buf;
}

static TCHAR *_oreg_replace_str_main(
	HREG_DATA reg_data, const TCHAR *str,
	const TCHAR *replace_str, int all_flg,
	TCHAR *result_buf, INT_PTR result_buf_size,
	oreg_replace_callback_func callback_func, void *callback_arg)
{
	size_t	len;
	INT_PTR result_pos = 0;
	TCHAR *org_result_buf = result_buf;
	size_t replace_len;
	OREG_POINT  search_start = {0, 0};
	OREG_POINT  search_end = {-1, -1};
	OREG_POINT  result_start = {0, 0};
	OREG_POINT  result_end = {0, 0};
	OREG_POINT  prev_result = {-1, -1};
	OREG_DATASRC data_src;
	int hit_flg = 0;
	int make_replace_str = have_replace_str(replace_str);

	oreg_make_str_datasrc(&data_src, str);

	if(org_result_buf == NULL) result_buf_size = 0;

	len = _tcslen(str);
	if(!make_replace_str) replace_len = _tcslen(replace_str);

	for(;;) {
		if(oreg_exec(reg_data, &data_src, &search_start, &search_end,
			&result_start, &result_end, 0) != OREGEXP_FOUND) break;

		hit_flg = 1;

		if(result_start.col == result_end.col &&
			result_start.col == prev_result.col &&
			result_start.row == prev_result.row) {

			int ch_len = get_char_len(str + result_start.col);
			result_buf = _put_result(result_buf, org_result_buf,
				&result_buf_size, &result_pos, str + result_start.col,
				ch_len, len);
			if(result_buf == NULL) goto ERR1;

			search_start.col += ch_len;
			if(len == (size_t)result_end.col) break;
			continue;
		}
		prev_result.col = result_start.col;
		prev_result.row = result_start.row;

		if(result_start.col != search_start.col) {
			result_buf = _put_result(result_buf, org_result_buf,
				&result_buf_size, &result_pos, str + search_start.col,
				result_start.col - search_start.col, len);
			if(result_buf == NULL) goto ERR1;
		}

		if(callback_func) {
			TCHAR *rep = callback_func(callback_arg, reg_data);
			if(rep == NULL) goto ERR1;

			result_buf = _put_result(result_buf, org_result_buf,
				&result_buf_size,
				&result_pos, rep, _tcslen(rep), len);
			if(result_buf == NULL) goto ERR1;
		} else if(make_replace_str) {
			replace_len = oreg_make_replace_str(&data_src,
				&result_start, &result_end, replace_str, reg_data, NULL) - 1;
			if(replace_len > 0) {
				result_buf = _alloc_result_buf(result_buf, org_result_buf,
					&result_buf_size, result_pos, replace_len, len);
				if(result_buf == NULL) goto ERR1;

				oreg_make_replace_str(&data_src,
					&result_start, &result_end, replace_str, reg_data,
					result_buf + result_pos);
				result_pos += replace_len;
			}
		} else {
			if(replace_len > 0) {
				result_buf = _put_result(result_buf, org_result_buf,
					&result_buf_size,
					&result_pos, replace_str, replace_len, len);
				if(result_buf == NULL) goto ERR1;
			}
		}

		search_start.col = result_end.col;
		if(!all_flg) break;
	}
	if(!hit_flg) return (TCHAR *)str;

	result_buf = _put_result(result_buf, org_result_buf,
		&result_buf_size, &result_pos,
		str + search_start.col, len - search_start.col + 1, len);
	if(result_buf == NULL) goto ERR1;

	return result_buf;

ERR1:
	if(org_result_buf != result_buf && result_buf != NULL) free(result_buf);
	return NULL;
}

TCHAR *oreg_replace_str(HREG_DATA reg_data, const TCHAR *str,
	const TCHAR *replace_str, int all_flg,
	TCHAR *result_buf, int result_buf_size)
{
	return _oreg_replace_str_main(reg_data, str, replace_str, all_flg,
		result_buf, result_buf_size, NULL, NULL);
}

TCHAR *oreg_replace_str_cb(
	HREG_DATA reg_data, const TCHAR *str,
	oreg_replace_callback_func callback_func, void *callback_arg, int all_flg,
	TCHAR *result_buf, int result_buf_size)
{
	return _oreg_replace_str_main(reg_data, str, _T(""), all_flg,
		result_buf, result_buf_size, callback_func, callback_arg);
}

TCHAR  *oreg_replace_simple(const TCHAR *pattern, const TCHAR *str, const TCHAR *replace_str,
	int all_flg, int regexp_opt)
{
	TCHAR		*result;
	OREG_DATA   *reg_data;

	reg_data = oreg_comp2(pattern, regexp_opt);
	if(reg_data == NULL) return NULL;

	result = _oreg_replace_str_main(reg_data, str, replace_str, all_flg,
		NULL, 0, NULL, NULL);

	oreg_free(reg_data);

	return result;
}

int oregexp_lwr_main2(const TCHAR *pattern,
	const TCHAR *str, INT_PTR search_start_col,
	INT_PTR *start, INT_PTR *len, int regexp_opt, int simple_str_flg)
{
    int         ret_v;
	INT_PTR		tmp_start, tmp_len;
    OREG_DATA   *reg_data;

	if(simple_str_flg) {
    	reg_data = oreg_comp_str2(pattern, regexp_opt);
	} else {
    	reg_data = oreg_comp2(pattern, regexp_opt);
	}
    if(reg_data == NULL) return OREGEXP_COMP_ERR;

	tmp_start = 0;
	tmp_len = 0;
    ret_v = oreg_exec_str(reg_data, str, search_start_col, &tmp_start, &tmp_len);

    oreg_free(reg_data);

	if(start != NULL) *start = tmp_start;
	if(len != NULL) *len = tmp_len;

    return ret_v;
}

int oregexp_lwr(const TCHAR *pattern, const TCHAR *str, INT_PTR *start, INT_PTR *len, int i_flg)
{
	int		regexp_opt = 0;
	if(i_flg) regexp_opt |= OREGEXP_OPT_IGNORE_CASE;
    return oregexp_lwr_main2(pattern, str, 0, start, len, regexp_opt, 0);
}

int oregexp_lwr2(const TCHAR *pattern, const TCHAR *str, INT_PTR *start, INT_PTR *len, int regexp_opt)
{
    return oregexp_lwr_main2(pattern, str, 0, start, len, regexp_opt, 0);
}

int oregexp_str_lwr(const TCHAR *pattern, const TCHAR *str, INT_PTR *start, INT_PTR *len, int i_flg)
{
	int		regexp_opt = 0;
	if(i_flg) regexp_opt |= OREGEXP_OPT_IGNORE_CASE;
    return oregexp_lwr_main2(pattern, str, 0, start, len, regexp_opt, 1);
}

int oregexp_str_lwr2(const TCHAR *pattern, const TCHAR *str, INT_PTR *start, INT_PTR *len, int regexp_opt)
{
    return oregexp_lwr_main2(pattern, str, 0, start, len, regexp_opt, 1);
}

int oregexp(const TCHAR *pattern, const TCHAR *str, INT_PTR *start, INT_PTR *len)
{
    return oregexp_lwr_main2(pattern, str, 0, start, len, 0, 0);
}

