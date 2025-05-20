/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

/*----------------------------------------------------------------------
  strutil.c
  文字列処理
----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "ostrutil.h"
#include "mbutil.h"
#include "get_char.h"
#include "str_inline.h"

/*----------------------------------------------------------------------
  行末の改行コードとsepaで指定した文字を削除する
----------------------------------------------------------------------*/
void ostr_chomp(TCHAR *buf, TCHAR sepa)
{
	TCHAR    *p1, *p2;

	if(buf == NULL || buf[0] == '\0') return;

	p2 = NULL;
	for(p1 = buf; *p1 != '\0'; p1 += get_char_len(p1)) {
		if(*p1 == '\n' || *p1 == '\r' || *p1 == sepa) {
			if(p2 == NULL) p2 = p1;
		} else {
			p2 = NULL;
		}
	}
	if(p2 != NULL) *p2 = '\0';
}

/*----------------------------------------------------------------------
  両端のスペースを削除
----------------------------------------------------------------------*/
void ostr_trim(TCHAR *buf)
{
	int		i;
	TCHAR	*p1, *p2;

	if(buf == NULL || _tcslen(buf) == 0) return;

	/* 前方のスペースを削除 */
	p1 = buf;
	for(i = 0; *p1 == ' '; i++, p1 += get_char_len(p1)) {
		;
	}
	memmove(buf, buf + i, (_tcslen(buf) + 1 - i) * sizeof(TCHAR));

	if(_tcslen(buf) == 0) return;

	/* 後方のスペースを削除 */
	p2 = NULL;
	for(p1 = buf; *p1 != '\0'; p1 += get_char_len(p1)) {
		if(*p1 == ' ') {
			if(p2 == NULL) p2 = p1;
		} else {
			p2 = NULL;
		}
	}
	if(p2 != NULL) *p2 = '\0';
}

/*----------------------------------------------------------------------
  左端のスペースを削除
----------------------------------------------------------------------*/
void ostr_trimleft(TCHAR *buf)
{
	int		i;
	TCHAR	*p1;

	if(buf == NULL || _tcslen(buf) == 0) return;

	/* 前方のスペースを削除 */
	p1 = buf;
	for(i = 0; *p1 == ' '; i++, p1 += get_char_len(p1)) {
		;
	}
	memmove(buf, buf + i, _tcslen(buf) + 1 - i);
}

/*----------------------------------------------------------------------
  文字列をsepaで分割する
----------------------------------------------------------------------*/
const TCHAR *ostr_split(const TCHAR *buf, TCHAR *buf2, TCHAR sepa)
{
	if(buf == NULL) {
		*buf2 = '\0';
		return NULL;
	}

	if(*buf == '\"') {
		buf++;
		for(;; buf++, buf2++) {
			if(*buf == '\0') break;
			if(*buf == '\"') {
				buf++;
				if(*buf != '\"') break;
			}
			*buf2 = *buf;
		}
		for(;; buf++) {
			if(*buf == '\0') break;
			if(*buf == sepa) break;
		}
	} else {
		for(;; buf++, buf2++) {
			if(*buf == '\0') break;
			if(*buf == sepa) break;
			*buf2 = *buf;
		}
	}
	*buf2 = '\0';

	if(*buf == '\0') return NULL;
	return buf + 1;
}

/*----------------------------------------------------------------------
  文字の置換(c1 を c2に置換)
----------------------------------------------------------------------*/
void ostr_char_replace(TCHAR *s, const TCHAR c1, const TCHAR c2)
{
	for( ; *s; s += get_char_len(s) ) {
		if(*s == c1) *s = c2;
	}
}

/*----------------------------------------------------------------------
  文字数を数える
----------------------------------------------------------------------*/
int ostr_str_char_cnt(const TCHAR *p, const TCHAR *end)
{
	int		cnt = 0;
	for(; *p != '\0' && (end == NULL || p < end); p += get_char_len(p)) {
		cnt++;
	}
	return cnt;
}

/*----------------------------------------------------------------------
  文字を数える
----------------------------------------------------------------------*/
int ostr_str_cnt(const TCHAR *p, unsigned int ch)
{
	int		cnt = 0;

	for(; *p != '\0'; p += get_char_len(p)) {
		if(*p == (TCHAR)ch) cnt++;
	}

	return cnt;
}

/*----------------------------------------------------------------------
  文字を数える
----------------------------------------------------------------------*/
int ostr_str_cnt2(const TCHAR *p, const TCHAR *end, unsigned int ch)
{
	int		cnt = 0;

	for(; *p != '\0' && p < end; p += get_char_len(p)) {
		if(*p == (TCHAR)ch) cnt++;
	}

	return cnt;
}

/*----------------------------------------------------------------------
  strncmp
----------------------------------------------------------------------*/
int ostr_strcmp_nocase(const TCHAR *p1, const TCHAR *p2)
{
	int		i;
	unsigned int	ch1, ch2;

	for(i = 0;; i++) {
		ch1 = get_char_nocase(p1);
		ch2 = get_char_nocase(p2);
		if(ch1 != ch2) return ch1 - ch2;
		if(ch1 == '\0') break;

		p1 += get_char_len(p1);
		p2 += get_char_len(p2);
	}
	return 0;
}

int ostr_strncmp_nocase(const TCHAR *p1, const TCHAR *p2, int len)
{
	int		i;
	unsigned int	ch1, ch2;

	for(i = 0; i < len; i++) {
		ch1 = get_char_nocase(p1);
		ch2 = get_char_nocase(p2);
		if(ch1 != ch2) return ch1 - ch2;

		p1 += get_char_len(p1);
		p2 += get_char_len(p2);
	}
	return 0;
}

const TCHAR *ostr_strstr_nocase(const TCHAR* p1, const TCHAR* p2, int p2_len)
{
	const TCHAR* p = p1;

	for(; *p != '\0'; p += get_char_len(p)) {
		if(ostr_strncmp_nocase(p, p2, p2_len) == 0) return p;
	}

	return NULL;
}

int ostr_get_cmplen_nocase(const TCHAR *p1, const TCHAR *p2, int len)
{
	int		i;
	unsigned int	ch1, ch2;
	int		char_len;

	for(i = 0; i < len;) {
		ch1 = get_char_nocase(p1);
		ch2 = get_char_nocase(p2);
		if(ch1 != ch2) return i;

		char_len = get_char_len(p1);
		p1 += char_len;
		p2 += char_len;
		i += char_len;
	}

	return i;
}

//
// 小文字が含まれるかチェックする
//
int ostr_is_contain_lower(const TCHAR *str)
{
	if(str == NULL) return 0;

	for(; *str != '\0'; str++) {
		if(is_lead_byte(*str)) {
			str++;
			if(*str == '\0') break;
			continue;
		}
		if(inline_islower(*str)) return 1;
	}

	return 0;
}

//
// 大文字が含まれるかチェックする
//
int ostr_is_contain_upper(const TCHAR *str)
{
	if(str == NULL) return 0;

	for(; *str != '\0'; str++) {
		if(is_lead_byte(*str)) {
			str++;
			if(*str == '\0') break;
			continue;
		}
		if(inline_isupper(*str)) return 1;
	}

	return 0;
}


//
// 文字列が数字かチェックする
//
int ostr_is_digit_only(const TCHAR *str)
{
	if(str == NULL) return 0;

	for(; *str != '\0'; str++) {
		if(is_lead_byte(*str)) return 0;
		if(!inline_isdigit(*str)) return 0;
	}

	return 1;
}

//
// 文字列がASCIIかチェックする
//
int ostr_is_ascii_only(const TCHAR *str)
{
	if(str == NULL) return 0;

	for(; *str != '\0'; str++) {
		if(*str > 0x80) return 0;
	}

	return 1;
}

//
// 文字列の表示幅を求める (末尾のスペースは含めない)
//
int ostr_first_line_len_no_last_space(const TCHAR *str)
{
	int		len = 0;
	int		last_space_pos = -1;

	for(; *str != '\0'; str += get_char_len(str)) {
		if(*str == ' ') {
			if(last_space_pos == -1) last_space_pos = len;
		} else {
			last_space_pos = -1;
		}
		if(*str == '\n') {
			len++;
			return len;
		}
		len += get_char_len(str);
	}
	if(last_space_pos != -1) return last_space_pos;
	return len;
}

//
// テキストデータを分割
//
static TCHAR *ostr_get_sepa_data(const TCHAR *p, TCHAR *buf, 
	int buf_size, unsigned int sepa)
{
	unsigned int	ch;
	int		char_len;

	int		mode = 0;
	if(*p == '"') {
		p++;
		mode = 1;
	}

	for(;;) {
		if(*p == '\0') break;

		ch = get_char(p);
		if(mode == 0 && (ch == sepa || ch == '\r' || ch == '\n')) break;

		if(mode == 1 && ch == '"') {
			p += get_char_len2(ch);
			ch = get_char(p);
			if(ch != '"') break;
		}

		char_len = get_char_len2(ch);
		p += char_len;

		if(buf != NULL && buf_size > char_len) {
			buf = put_char(buf, ch);
			buf_size -= char_len;
		}
	}

	if(*p == '\r' && *(p + 1) == '\n') p++;

	if(buf != NULL) *buf = '\0';
	return (TCHAR *)p;
}

//
// ExcelからのPasteデータを分割する
//
TCHAR *ostr_get_tsv_data(const TCHAR *p, TCHAR *buf, 
	int buf_size)
{
	return ostr_get_sepa_data(p, buf, buf_size, '\t');
}

//
// csvデータを分割する
//
TCHAR *ostr_get_csv_data(const TCHAR *p, TCHAR *buf, 
	int buf_size)
{
	return ostr_get_sepa_data(p, buf, buf_size, ',');
}

//
// ExcelからのPasteデータのセルの数を数える
//
int ostr_get_tsv_col_cnt(const TCHAR *p)
{
	int		cnt = 0;

	for(;;) {
		cnt++;
		p = ostr_get_tsv_data(p, NULL, 0);
		if(*p != '\t') break;
		p++;
	}

	return cnt;
}

//
// ExcelからのPasteデータのセルが1行のみか確認
//
int ostr_is_tsv_single_row(const TCHAR* p)
{
	for (;;) {
		p = ostr_get_tsv_data(p, NULL, 0);
		if (*p != '\t') break;
		p++;
	}

	if (*p == '\0') return 1;

	return 0;
}

//
// ExcelからのPasteデータのセルが1セルのみか確認
//
int ostr_is_tsv_single_col(const TCHAR* p)
{
	p = ostr_get_tsv_data(p, NULL, 0);
	if (*p == '\0' || *p == '\r' || *p == '\n') return 1;

	return 0;
}

//
// 文字列が数値かチェックする
//
int ostr_str_isnum(const TCHAR *str)
{
	char first_char_flg = 0;
	char dot_flg = 0;

	for(; *str != '\0';) {
		if(is_lead_byte(*str)) return 0;

		switch(*str) {
		case ' ':
			break;
		case '-':
			if(first_char_flg) return 0;
			first_char_flg = 1;
			break;
		case '.':
			if(dot_flg) return 0;
			dot_flg = 1;
			break;
		default:
			if(!inline_isdigit(*str)) return 0;
			break;
		}

		str++;
	}

	return 1;
}

//
// tab textをformatする
//
#pragma intrinsic(memcpy)
void ostr_tabbed_text_format(const TCHAR *p1, TCHAR *buf, 
	int tabstop, int tab_flg)
{
	const TCHAR	*p2 = p1;
	INT_PTR		x, tab;

	for(x = 0;;) {
		for(; *p2; p2++) if(*p2 == '\t') break;
		if(!(*p2)) {
			// '\0'までコピーする
			memcpy((buf + x), p1, (p2 - p1 + 1) * sizeof(TCHAR));
			break;
		}

		if(p2 > p1) {
			memcpy((buf + x), p1, (p2 - p1) * sizeof(TCHAR));
			x = x + (p2 - p1);
		}

		tab = tabstop - (x % tabstop);

		if(tab_flg) {
			buf[x] = '\t';
			x++;
			tab--;
		}
		for(; tab; tab--, x++) {
			buf[x] = ' ';
		}

		p2++;
		p1 = p2;
	}
}
#pragma function(memcpy)

//
// tab textのsizeを計算する
//
INT_PTR ostr_calc_tabbed_text_size(const TCHAR *pstr, int tabstop)
{
	return ostr_calc_tabbed_text_size_n(pstr, tabstop, -1);
}

INT_PTR ostr_calc_tabbed_text_size_n(const TCHAR *pstr, int tabstop, int x)
{
	const TCHAR	*pstr2 = pstr;
	INT_PTR		disp_size = 0;
	int		i;

	if(x < 0) x = INT_MAX;

	for(i = 0; i < x ; i++) {
		if(!(*pstr)) break;

		if(*pstr == '\t') {
			disp_size += pstr - pstr2;
			disp_size = disp_size + tabstop - (disp_size % tabstop);

			pstr++;
			pstr2 = pstr;
		} else {
			pstr++;
		}
	}
	disp_size += pstr - pstr2;

	return disp_size;
}

//
// CSVデータを出力
//
int ostr_csv_fputs(FILE *stream, TCHAR *string, TCHAR sepa)
{
	if(string == NULL) return EOF;

	if(putc('\"', stream) == EOF) return EOF;
	for(;*string != '\0'; string++) {
		if(putc(*string, stream) == EOF) return EOF;
		if(*string == '\"') {
			if(putc('\"', stream) == EOF) return EOF;
		}
	}
	if(putc('\"', stream) == EOF) return EOF;

	putc(sepa, stream);

	return 1;
}

// atofのwcharバージョン
double _ttofbak(const TCHAR *p)
{
#ifdef _UNICODE
	char buf[1024];
	wcstombs(buf, p, sizeof(buf));
	return atof(buf);
#else
	return atof(p);
#endif
}

int get_scale(const TCHAR *value)
{
	const TCHAR *p = value;

	for(; *p != '\0'; p++) {
		if(*p == '.') {
			return (int)_tcslen(p + 1);
		}
	}
	return 0;
}


//
// Oracleのオブジェクト名で、ダブルクォートが必要かチェックする
//
int ostr_need_object_name_quote_for_oracle(const TCHAR *object_name)
{
	// 大文字英数字, _, $, #, マルチバイト文字以外はquote対象にする
	const TCHAR *p = object_name;

	for(; *p != '\0'; p += get_char_len(p)) {
		unsigned int ch = get_char(p);

		// 全角英数字は半角にする
		if(ch >= L'！' && ch <= L'｝') {
			ch -= L'！' - L'!';
		}

		// 大文字, 数字, ASCII
		if(inline_isupper(ch) || inline_isdigit(ch) || ch >= 0x80) continue;

		// [_$#.]
		if(ch == '_' || ch == '#' || ch == '$' || ch == '.') continue;

		// その他はquote
		return 1;
	}

	return 0;
}

//
const TCHAR *ostr_skip_space(const TCHAR *p)
{
	for(; *p != '\0'; p += get_char_len(p)) {
		if(*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') continue;
		break;
	}

	return p;
}


__inline int check_char(unsigned int ch, const TCHAR *trim_chars)
{
	const TCHAR *t = trim_chars;

	for(; *t != '\0'; t += get_char_len(t)) {
		if(ch == get_char(t)) {
			return 1;
		}
	}
	return 0;
}

void ostr_trim_chars(TCHAR *buf, const TCHAR *trim_chars)
{
	TCHAR *p = buf;
	TCHAR *p_start = buf;
	TCHAR *p_end = buf;

	for(; *p != '\0'; p += get_char_len(p)) {
		if(!check_char(get_char(p), trim_chars)) break;
	}
	p_start = p;

	for(; *p != '\0'; p += get_char_len(p)) {
		if(check_char(get_char(p), trim_chars)) break;
	}
	p_end = p;

	memmove(buf, p_start, (p_end - p_start) * sizeof(TCHAR));
	buf[p_end - p_start] = '\0';
}

void ostr_strcpy_replace(const TCHAR* src, TCHAR* dest, const TCHAR* pattern_str, const TCHAR* replace_str)
{
	const TCHAR* s = src;
	const TCHAR* r;
	TCHAR* d = dest;
	size_t pat_len = _tcslen(pattern_str);

	for(; *s != '\0'; s++) {
		if(inline_strncmp(s, pattern_str, pat_len) == 0) {
			for(r = replace_str; *r != '\0'; r++) {
				*d = *r;
				d++;
			}
			s += pat_len - 1;
		} else {
			*d = *s;
			d++;
		}
	}
	*d = '\0';
}

