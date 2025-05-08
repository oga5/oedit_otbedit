/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include <stdio.h>
#include <string.h>

#include "get_char.h"
#include "mbutil.h"

void my_mbslwr(TCHAR *mbstr)
{
	unsigned int ch;

	for(; *mbstr != '\0'; mbstr++) {
		if(!is_lead_byte(*mbstr)) {
			*mbstr = inline_tolower(*mbstr);
		} else {
			ch = get_char(mbstr);
			if(ch >= LARGE_A && ch <= LARGE_Z) {
				put_char(mbstr, ch + (SMALL_A - LARGE_A));
			}
			mbstr++;
			if(*mbstr == '\0') break;
		}
	}
}

void my_mbslwr_1byte(TCHAR *mbstr)
{
	unsigned int ch;

	for(; *mbstr != '\0'; mbstr++) {
		if(!is_lead_byte(*mbstr)) {
			*mbstr = inline_tolower(*mbstr);
		} else {
			ch = get_char(mbstr);
			mbstr++;
			if(*mbstr == '\0') break;
		}
	}
}

void my_mbsupr(TCHAR *mbstr)
{
	unsigned int ch;

	for(; *mbstr != '\0'; mbstr++) {
		if(!is_lead_byte(*mbstr)) {
			*mbstr = inline_toupper(*mbstr);
		} else {
			ch = get_char(mbstr);
			if(ch >= SMALL_A && ch <= SMALL_Z) {
				put_char(mbstr, ch - (SMALL_A - LARGE_A));
			}
			mbstr++;
			if(*mbstr == '\0') break;
		}
	}
}

void my_mbsupr_1byte(TCHAR *mbstr)
{
	unsigned int ch;

	for(; *mbstr != '\0'; mbstr++) {
		if(!is_lead_byte(*mbstr)) {
			*mbstr = inline_toupper(*mbstr);
		} else {
			ch = get_char(mbstr);
			mbstr++;
			if(*mbstr == '\0') break;
		}
	}
}

TCHAR *my_mbschr(const TCHAR *mbstr, unsigned int ch)
{
	const TCHAR *p = mbstr;
	for(; *p; p += get_char_len(p)) {
		if(*p == ch) return (TCHAR *)p;
	}
	return NULL;
}

TCHAR  *my_mbsstr(const TCHAR *string1, const TCHAR *string2)
{
	const TCHAR *p = string1;
	for(;;) {
		p = _tcsstr(p, string2);
		if(p == NULL) return NULL;
		if(p == string1) break;
		if(is_lead_byte2(string1, p - string1 - 1) == 0) break;

		p++;
	}

	return (TCHAR *)p;
}

