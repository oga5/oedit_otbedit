/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef _OSTRUTIL_STR_INLINE_H_INCLUDED_
#define _OSTRUTIL_STR_INLINE_H_INCLUDED_

#ifdef WIN32
#include <basetsd.h>
#endif
#include "get_char.h"

#ifdef  __cplusplus
extern "C" {
#endif

__inline static int inline_isupper(unsigned int ch) {
	return (ch >= 'A' && ch <= 'Z');
}

__inline static int inline_islower(unsigned int ch) {
	return (ch >= 'a' && ch <= 'z');
}

__inline static int inline_isdigit(unsigned int ch) {
	return (ch >= '0' && ch <= '9');
}

__inline static int inline_isalpha(unsigned int ch) {
	return (inline_islower(ch) || inline_isupper(ch));
}

__inline static int inline_isalnum(unsigned int ch) {
	return (inline_islower(ch) || inline_isupper(ch) || inline_isdigit(ch));
}

__inline static int inline_isascii(unsigned int ch) {
	return (ch < 0x80);
}

__inline static int inline_is_sjis_hankana(unsigned int ch) {
	return (ch >= 0xA1 && ch <= 0xDF);
}

__inline static TCHAR *inline_strchr(const TCHAR *p, unsigned int ch)
{
	for(; *p != '\0'; p += get_char_len(p)) {
		if(get_char(p) == ch) return (TCHAR *)p;
	}
	return NULL;
}

__inline static TCHAR *inline_strchr2(const TCHAR *p, unsigned int ch, unsigned int ch2)
{
	unsigned int cur_ch;
	for(; *p != '\0'; p += get_char_len(p)) {
		cur_ch = get_char(p);
		if(cur_ch == ch || cur_ch == ch2) return (TCHAR *)p;
	}
	return NULL;
}

__inline static int inline_strcmp(const TCHAR *p1, const TCHAR *p2)
{
	for(;; p1++, p2++) {
		if(*p1 != *p2) return *p1 - *p2;
		if(*p1 == '\0') break;
	}
	return 0;
}

__inline static int inline_strncmp(const TCHAR *p1, const TCHAR *p2, size_t len)
{
	size_t	i;
	for(i = 0; i < len; i++, p1++, p2++) {
		if(*p1 != *p2) return *p1 - *p2;
		if(*p1 == '\0') break;
	}
	return 0;
}

__inline static void inline_strncpy(TCHAR *p1, const TCHAR *p2, size_t len)
{
	size_t		i;
	for(i = 0; i < len; i++, p1++, p2++) {
		*p1 = *p2;
		if(*p2 == '\0') break;
	}
}

__inline static int inline_ostr_strncmp_nocase(const TCHAR *p1, const TCHAR *p2, size_t len)
{
	size_t		i;
	unsigned int	ch1, ch2;

	for(i = 0; i < len;) {
		ch1 = get_char_nocase(p1);
		ch2 = get_char_nocase(p2);
		if(ch1 != ch2) return ch1 - ch2;
		if(ch1 == '\0') break;

		i += get_char_len(p1);
		p1 += get_char_len(p1);
		p2 += get_char_len(p2);
	}
	return 0;
}

__inline static INT_PTR inline_strlen_first_line(const TCHAR *p, int line_feed_width)
{
	const TCHAR *p_start = p;

	for(; *p > '\r' || (*p != '\0' && *p != '\n' && *p != '\r');) {
		p += get_char_len(p);
	}
	if(*p != '\0') return p - p_start + line_feed_width;
	return p - p_start;
}

__inline static const TCHAR *skip_chars(const TCHAR *p, INT_PTR cnt)
{
	INT_PTR		i;

	for(i = 0; i < cnt; i++) {
		if(get_char(p) == '\0') { // error
			return p;
		}
		p += get_char_len(p);
	}

	return p;
}

__inline static int get_chars_len(const TCHAR *p, int cnt)
{
	int		i;
	int		result = 0;

	for(i = 0; i < cnt; i++) {
		if(get_char(p) == '\0') { // error
			return result;
		}

		result += get_char_len(p);
		p += get_char_len(p);
	}

	return result;
}

#ifdef  __cplusplus
}
#endif

#endif /* _OSTRUTIL_STR_INLINE_H_INCLUDED_ */

