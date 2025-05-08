/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef __OREGEXP_H_INCLUDED__
#define __OREGEXP_H_INCLUDED__

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef WIN32
#include <basetsd.h>
#endif

#include "get_char.h"

#define OREGEXP_FOUND					 1
#define OREGEXP_NOT_FOUND				 0
#define OREGEXP_COMP_ERR				-1
#define OREGEXP_STACK_OVERFLOW			-2
#define OREGEXP_FATAL_ERROR				-10
#define OREGEXP_REPLACE_OK				 1
#define OREGEXP_REPLACE_BUF_SIZE_ERR	-4

#define OREGEXP_OPT_IGNORE_CASE			(0x01 << 0)
#define OREGEXP_OPT_IGNORE_WIDTH_ASCII	(0x01 << 1)
#define OREGEXP_OPT_NO_OPTIMIZE			(0x01 << 2)

typedef struct oreg_data_st *HREG_DATA;

typedef struct oreg_data_point_st OREG_POINT;
struct oreg_data_point_st {
	INT_PTR		row;
	INT_PTR		col;
};

typedef struct oreg_datasrc_st *HREG_DATASRC;
typedef struct oreg_datasrc_st OREG_DATASRC;
struct oreg_datasrc_st {
	const void *src;
	const TCHAR *(*get_char_buf)(const void *src, OREG_POINT *pt);
	const TCHAR *(*get_row_buf_len)(const void *src, INT_PTR row, size_t *len);
	const TCHAR *(*next_char)(const void *src, OREG_POINT *pt);
	const TCHAR *(*prev_char)(const void *src, OREG_POINT *pt);
	const TCHAR *(*next_row)(const void *src, OREG_POINT *pt);
	INT_PTR (*get_len)(const void *src, OREG_POINT *start_pt, OREG_POINT *end_pt);

	int (*is_row_first)(const void *src, OREG_POINT *pt);
	int (*is_selected_first)(const void *src, OREG_POINT *pt);
	int (*is_selected_last)(const void *src, OREG_POINT *pt);
	int (*is_selected_row_first)(const void *src, OREG_POINT *pt);
	int (*is_selected_row_last)(const void *src, OREG_POINT *pt);
};

HREG_DATA oreg_comp(const TCHAR *pattern, int i_flg);
HREG_DATA oreg_comp2(const TCHAR *pattern, int regexp_opt);

HREG_DATA oreg_comp_str(const TCHAR *pattern, int i_flg);
HREG_DATA oreg_comp_str2(const TCHAR *pattern, int regexp_opt);

void oreg_free(HREG_DATA reg_data);

void oreg_make_str_datasrc(HREG_DATASRC data_src, const TCHAR *str);
int oreg_exec_str(HREG_DATA reg_data, const TCHAR *str, INT_PTR search_start_col, INT_PTR *result_start_col, INT_PTR *len);
int oreg_exec_str2(HREG_DATA reg_data, const TCHAR *str);

int oreg_exec(HREG_DATA reg_data, HREG_DATASRC data_src,
    OREG_POINT *search_start, OREG_POINT *search_end,
    OREG_POINT *result_start, OREG_POINT *result_end, int loop_flg);
int oreg_exec2(HREG_DATA reg_data, HREG_DATASRC data_src);

void oreg_get_match_data(HREG_DATA reg_data,
	OREG_POINT *pstart_pt, OREG_POINT *pend_pt);
int oreg_get_backref_cnt(HREG_DATA reg_data);
void oreg_get_backref_data(HREG_DATA reg_data, int idx,
	OREG_POINT *pstart_pt, OREG_POINT *pend_pt);

INT_PTR oreg_check_backref_idx(HREG_DATA reg_data, int idx);
INT_PTR oreg_get_backref_str(HREG_DATASRC data_src, HREG_DATA reg_data, int idx, TCHAR *buf);
INT_PTR oreg_make_replace_str(HREG_DATASRC data_src,
    OREG_POINT *start_pt, OREG_POINT *end_pt,
    const TCHAR *rep_str, HREG_DATA reg_data, TCHAR *result);
int have_replace_str(const TCHAR *p);

int oregexp(const TCHAR *pattern, const TCHAR *str, INT_PTR *start, INT_PTR *len);

int oregexp_lwr(const TCHAR *pattern, const TCHAR *str, INT_PTR *start, INT_PTR *len, int i_flg);
int oregexp_lwr2(const TCHAR *pattern, const TCHAR *str, INT_PTR *start, INT_PTR *len, int regexp_opt);

int oregexp_str_lwr(const TCHAR *pattern, const TCHAR *str, INT_PTR *start, INT_PTR *len, int i_flg);
int oregexp_str_lwr2(const TCHAR *pattern, const TCHAR *str, INT_PTR *start, INT_PTR *len, int regexp_opt);

int oregexp_print_nfa(const TCHAR* pattern, int regexp_opt);

TCHAR *oreg_replace_str(HREG_DATA reg_data, const TCHAR *str,
    const TCHAR *replace_str, int all_flg,
    TCHAR *result_buf, int result_buf_size);

typedef TCHAR *(*oreg_replace_callback_func)(
	void *callback_arg, HREG_DATA reg_data);
TCHAR *oreg_replace_str_cb(
	HREG_DATA reg_data, const TCHAR *str,
    oreg_replace_callback_func callback_func, void *callback_arg, int all_flg,
    TCHAR *result_buf, int result_buf_size);
TCHAR  *oreg_replace_simple(const TCHAR *pattern, const TCHAR *str, const TCHAR *replace_str,
	int all_flg, int regexp_opt);

#ifdef  __cplusplus
}
#endif

#endif /* __OREGEXP_H_INCLUDED__ */


