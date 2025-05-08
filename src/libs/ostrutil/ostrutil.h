/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef OSTRUTIL_INCLUDE
#define OSTRUTIL_INCLUDE

#ifdef  __cplusplus
extern "C" {
#endif

#define ARRAY_SIZEOF(arr)   (sizeof(arr)/sizeof(arr[0]))

#include <basetsd.h>
#include <tchar.h>

void ostr_chomp(TCHAR *buf, TCHAR sepa);
void ostr_trim(TCHAR *buf);
void ostr_trimleft(TCHAR *buf);

void ostr_trim_chars(TCHAR *buf, const TCHAR *trim_chars);

const TCHAR *ostr_split(const TCHAR *buf, TCHAR *buf2, TCHAR sepa);
const TCHAR *ostr_skip_space(const TCHAR *p);

void ostr_char_replace(TCHAR *s, const TCHAR c1, const TCHAR c2);

int ostr_str_cnt(const TCHAR *p, unsigned int ch);
int ostr_str_cnt2(const TCHAR *p, const TCHAR *end, unsigned int ch);

int ostr_strcmp_nocase(const TCHAR *p1, const TCHAR *p2);
int ostr_strncmp_nocase(const TCHAR *p1, const TCHAR *p2, int len);
int ostr_get_cmplen_nocase(const TCHAR *p1, const TCHAR *p2, int len);
const TCHAR* ostr_strstr_nocase(const TCHAR* p1, const TCHAR* p2, int p2_len);

void ostr_strcpy_replace(const TCHAR* src, TCHAR* dest, const TCHAR* pattern_str, const TCHAR* replace_str);

int ostr_is_contain_lower(const TCHAR *str);
int ostr_is_contain_upper(const TCHAR *str);
int ostr_is_digit_only(const TCHAR *str);
int ostr_is_ascii_only(const TCHAR *str);
int ostr_str_isnum(const TCHAR *str);

int ostr_str_char_cnt(const TCHAR *p, const TCHAR *end);
int ostr_first_line_len_no_last_space(const TCHAR *str);

void ostr_tabbed_text_format(const TCHAR *p1, TCHAR *buf, 
	int tab_stop, int tab_flg);
INT_PTR ostr_calc_tabbed_text_size(const TCHAR *pstr, int tabstop);
INT_PTR ostr_calc_tabbed_text_size_n(const TCHAR *pstr, int tabstop, int x);

TCHAR *ostr_get_tsv_data(const TCHAR *p, TCHAR *buf, int buf_size);
TCHAR *ostr_get_csv_data(const TCHAR *p, TCHAR *buf, int buf_size);
int ostr_csv_fputs(FILE *stream, TCHAR *string, TCHAR sepa);
int ostr_get_tsv_col_cnt(const TCHAR *p);

int ostr_is_tsv_single_row(const TCHAR* p);
int ostr_is_tsv_single_col(const TCHAR* p);

//double _ttof(const TCHAR *p);
int get_scale(const TCHAR *value);

int ostr_need_object_name_quote_for_oracle(const TCHAR *object_name);

#ifdef  __cplusplus
}
#endif


#endif  OSTRUTIL_INCLUDE
