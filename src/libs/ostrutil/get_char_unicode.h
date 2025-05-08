/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef WIN32
	#ifndef __inline
	#define __inline
	#endif
#endif

#include <tchar.h>

#define LARGE_A			L'Ç`'
#define LARGE_Z			L'Çy'
#define SMALL_A			L'ÇÅ'
#define SMALL_Z			L'Çö'
#define ZENKAKU_SPACE	L'Å@'

__inline static unsigned int inline_ismblower(unsigned int ch)
{
	return (ch <= SMALL_Z && ch >= SMALL_A);
}

__inline static unsigned int inline_ismbupper(unsigned int ch)
{
	return (ch <= LARGE_Z && ch >= LARGE_A);
}

// impremented at hankaku.c
unsigned int get_ascii_char(unsigned int ch);

__inline static unsigned int inline_tolower(unsigned int ch)
{
	if(ch <= 'Z' && ch >= 'A') return ch + ('a' - 'A');
	return ch;
}

__inline static unsigned int inline_toupper(unsigned int ch)
{
	if(ch <= 'z' && ch >= 'a') return ch - ('a' - 'A');
	return ch;
}

__inline static unsigned int inline_tolower_mb(unsigned int ch)
{
	if(ch <= 0x80) {
		return inline_tolower(ch);
	} else if(inline_ismbupper(ch)) {
		return ch + (SMALL_A - LARGE_A);
	}
	return ch;
}

__inline static unsigned int inline_toupper_mb(unsigned int ch)
{
	if(ch <= 0x80) {
		return inline_toupper(ch);
	} else if(inline_ismblower(ch)) {
		return ch - (SMALL_A - LARGE_A);
	}
	return ch;
}

__inline static int is_high_surrogate_ch(unsigned char ch)
{
	return (ch >= 0xD8 && ch <= 0xDB);
}

__inline static int is_high_surrogate(TCHAR ch)
{
	return (ch >= 0xD800 && ch <= 0xDBFF);
	//return ((ch & 0xFC00) == 0xD800);
}

__inline static int is_low_surrogate(TCHAR ch)
{
	return (ch >= 0xDC00 && ch <= 0xDFFF);
}

__inline static int is_lead_byte(TCHAR ch)
{
	return is_high_surrogate(ch) ? 1 : 0;
}

__inline static int get_char_len(const TCHAR *p)
{
	return is_high_surrogate(*p) ? 2 : 1;
}

__inline static int is_lead_byte2(const TCHAR *mbstr, UINT_PTR count)
{
	return is_lead_byte(mbstr[count]);
}

__inline static int get_char_len2(unsigned int ch)
{
	return (ch >= 0x10000) ? 2 : 1;
}

__inline static unsigned int get_char(const TCHAR *p)
{
	if(is_high_surrogate(*p)) {
		return ((*p & 0x3FF) << 10) + (*(p + 1) & 0x3FF) + 0x10000;
	}
	return *p;
}

__inline static unsigned int get_char_nocase(const TCHAR *p)
{
	return inline_tolower_mb(get_char(p));
}

__inline static TCHAR *put_char(TCHAR *p, unsigned int ch)
{
	if(get_char_len2(ch) == 2) {
		ch -= 0x10000;
		*p = 0xD800 | ((ch & 0x000FFC00) >> 10);
		p++;
		*p = 0xDC00 | (ch & 0x000003FF);
		p++;
	} else {
		*p = ch;
		p++;
	}
	return p;
}

__inline static const TCHAR *get_prev_str(const TCHAR *p)
{
	return (is_low_surrogate(*(p - 1))) ? p - 2 : p - 1;
}

#ifdef  __cplusplus
}
#endif
