/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include <string.h>
#include <assert.h>
#include "local.h"
#include "get_char.h"
#include "str_inline.h"

static int check_posix_char_class(const TCHAR *p)
{
	unsigned int ch;

	p += get_char_len(p);
	ch = get_char(p);
	if(ch != ':') return 0;

	p += get_char_len(p);
	ch = get_char(p);
	if(!((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))) return 0;

    return 1;
}

static int skip_bress_char_class(const TCHAR *p, INT_PTR p_len, LexWord *lex_word)
{
	unsigned int ch;
	unsigned int open_ch = '[';
	unsigned int close_ch = ']';
	int		cnt = 1;
	int		len = 0;

	lex_word->data = p;

	len += get_char_len(p);
	p += get_char_len(p);

	if(*p == '^') {
		len += get_char_len(p);
		p += get_char_len(p);
	}
	if(*p == ']') {
		len += get_char_len(p);
		p += get_char_len(p);
	}

	for(;;) {
		ch = get_char(p);
		if(ch == '\0') { // error
			return -1;
		} else if(ch == '\\') { // escape
			len += get_char_len(p);
			p += get_char_len(p);
			if(len == p_len) return -1;	// error
			if(get_char(p) == '\0') return -1; // error
		} else if(ch == open_ch) {
			if(check_posix_char_class(p)) {
				cnt++;
			}
		} else if(ch == close_ch) {
			cnt--;
		}

		len += get_char_len(p);
		p += get_char_len(p);

		if(cnt == 0) break;
		if(len == p_len) return -1;	// error
	}

	lex_word->len = len;

	return 0;
}

static int find_char(const TCHAR *p, unsigned int stop_ch)
{
	unsigned int ch;
	int len = 0;

	for(;;) {
		ch = get_char(p);
		if(ch == '\0') return -1;

		len += get_char_len(p);
		p += get_char_len(p);

		if(ch == stop_ch) break;
	}

	return len;
}

static int skip_bress(const TCHAR *p, INT_PTR p_len, LexWord *lex_word)
{
	unsigned int ch;
	unsigned int open_ch;
	unsigned int close_ch;
	int		cnt = 1;
	INT_PTR	len = 0;

	lex_word->data = p;

	open_ch = get_char(p);
	switch(open_ch) {
	case '(':
		close_ch = ')';
		break;
	case '{':
		close_ch = '}';
		break;
	default:
		return -1;	// error
		break;
	}
	len += get_char_len(p);
	p += get_char_len(p);

	for(;;) {
		ch = get_char(p);

		if(ch == '[') {
			LexWord tmp_lex_word;
			if(skip_bress_char_class(p, p_len - len, &tmp_lex_word) != 0) {
				return -1;
			}
			len += tmp_lex_word.len;
			p += tmp_lex_word.len;
			continue;
		}

		if(ch == '\0') { // error
			return -1;
		} else if(ch == '\\') { // escape
			len += get_char_len(p);
			p += get_char_len(p);
			if(len == p_len) return -1;	// error
			if(get_char(p) == '\0') return -1; // error
		} else if(ch == open_ch) {
			cnt++;
		} else if(ch == close_ch) {
			cnt--;
		}

		len += get_char_len(p);
		p += get_char_len(p);

		if(cnt == 0) break;
		if(len == p_len) return -1;	// error
	}

	lex_word->len = len;

	return 0;
}

static int group(const TCHAR *p, INT_PTR p_len, LexWord *lex_word)
{
	lex_word->type = LEX_GROUP;
	if(_tcsncmp(p, _T("(?#"), 3) == 0) {
		lex_word->type = LEX_COMMENT;
	}

	return skip_bress(p, p_len, lex_word);
}

static int char_class(const TCHAR *p, INT_PTR p_len, LexWord *lex_word)
{
	lex_word->type = LEX_CHAR_CLASS;

	return skip_bress_char_class(p, p_len, lex_word);
}

static int quantifier(const TCHAR *p, INT_PTR p_len, LexWord *lex_word)
{
	int		ret_v;
	unsigned int last_ch;

	lex_word->type = LEX_QUANTIFIER;

	if(get_char(p) == '{') {
		ret_v = skip_bress(p, p_len, lex_word);
		if(ret_v != 0) return ret_v;
	} else {
		lex_word->data = p;
		lex_word->len = 1;
	}

	last_ch = get_char(lex_word->data + lex_word->len);
	if(last_ch == '?' || last_ch == '+') {
		lex_word->len++;
	}

	return 0;
}

static int make_lex_type(const TCHAR *p, int p_len, RegWordType type,
	LexWord *lex_word)
{
	lex_word->type = type;
	lex_word->data = p;
	lex_word->len = p_len;
	return 0;
}

static int word(const TCHAR *p, LexWord *lex_word)
{
	lex_word->type = LEX_WORD;
	lex_word->data = p;
	lex_word->len = get_char_len(p);

	return 0;
}

static const TCHAR *skip_integer(const TCHAR *p, INT_PTR p_len, LexWord *lex_word)
{
	unsigned int ch;

	for(; lex_word->len < p_len;) {
		ch = get_char(p);
		if(ch < '0' || ch > '9') break;
		p += get_char_len(p);
		(lex_word->len)++;
	}

	return p;
}

static int is_hex_value(const TCHAR *p, INT_PTR hex_len)
{
	int				i;
	unsigned int	ch;

	if(hex_len <= 0) return 0;

	for(i = 0; i < hex_len; i++) {
		ch = get_char(p);
		p += get_char_len(p);

		if((ch >= '-' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')) {
			continue;
		} else {
			return 0;
		}
	}

	return 1;
}

static int make_escape(const TCHAR *p, INT_PTR p_len, LexWord *lex_word)
{
	unsigned int ch;

	lex_word->type = LEX_ESCAPE_CHAR;
	lex_word->data = p;
	lex_word->len = 1;		// '\'

	p += get_char_len(p);	// '\'をスキップ
	ch = get_char(p);

	if(ch == '=') {
		(lex_word->len)++;
		p += get_char_len(p);
		(lex_word->len) += get_char_len(p);
	} else if(ch >= '1' && ch <= '9') {
		(lex_word->len)++;
		p += get_char_len(p);
		p = skip_integer(p, p_len, lex_word);
	} else if(ch == 'k') {
		ch = get_char(p + get_char_len(p));
		if(ch == '<' || ch == '\'') {
			int		tmp_len;
			unsigned int stop_char = '>';
			if(ch == '\'') stop_char = '\'';

			p += get_char_len(p);		// skip k
			(lex_word->len)++;			// k
			p += get_char_len(p);		// skip '<' or '\''
			(lex_word->len)++;			// '<' or '\''

			tmp_len = find_char(p, stop_char);
			if(tmp_len == -1) return -1;

			(lex_word->len) += tmp_len;
		}
	} else if(ch == 'g') {
		int bracket_flg = 0;

		(lex_word->len)++;
		p += get_char_len(p);	// 'g'をスキップ
		ch = get_char(p);

		if(ch == '{') {
			bracket_flg = 1;
			(lex_word->len)++;
			p += get_char_len(p);
			ch = get_char(p);
		}
		if(ch == '-') {
			(lex_word->len)++;
			p += get_char_len(p);
			ch = get_char(p);
		}
		if(ch < '1' || ch > '9') return -1;		// error

		(lex_word->len)++;
		p += get_char_len(p);
		p = skip_integer(p, p_len, lex_word);

		if(bracket_flg) {
			if(get_char(p) != '}') return -1;	// error
			(lex_word->len)++;
			p += get_char_len(p);
		}
	} else if(ch == 'x') {
		// support \x7f, \x{264a}
		// skip 'x'
		p += get_char_len(p);
		(lex_word->len)++;

		if(get_char(p) == '{') {
			LexWord tmp_lex_word;

			// -2 for '\x'
			if(skip_bress(p, p_len - 2, &tmp_lex_word) != 0) {
				return -1;
			}

			p += get_char_len(p);
			if(!is_hex_value(p, tmp_lex_word.len - 2)) {
				return -1;
			}

			lex_word->len += tmp_lex_word.len;
		} else if(is_hex_value(p, 2)) {
			lex_word->len += get_chars_len(p, 2);
		} else {
			return -1;
		}
	} else if(ch == 'Q') {
		const TCHAR *p2 = _tcsstr(p, _T("\\E"));
		if(p2 == NULL) {
			lex_word->len = p_len;
		} else {
			ptrdiff_t pd = p2 - p;
			lex_word->len = pd + 3;		// +3 for 'Q' and '\E'
		}
	} else {
		(lex_word->len) += get_char_len(p);
	}

	return 0;
}

int is_quantifier(unsigned int ch)
{
	switch(ch) {
	case '{':
	case '*':
	case '+':
	case '?':
		return 1;
	}
	return 0;
}

int lexer(const TCHAR *p, INT_PTR p_len, LexWord *lex_word)
{
	unsigned int ch = get_char(p);

	if(is_quantifier(ch)) {
		return quantifier(p, p_len, lex_word);
	}

	switch(ch) {
	case '(':
		return group(p, p_len, lex_word);
	case '[':
		return char_class(p, p_len, lex_word);
	case '|':
		return make_lex_type(p, 1, LEX_OR_CHAR, lex_word);
	case '.':
		return make_lex_type(p, 1, LEX_ANY_CHAR, lex_word);
	case '^':
		return make_lex_type(p, 1, LEX_FIRST_MATCH, lex_word);
	case '$':
		return make_lex_type(p, 1, LEX_LAST_MATCH, lex_word);
	case '\\':
		return make_escape(p, p_len, lex_word);
	}

	return word(p, lex_word);
}

int skip_comment(const TCHAR **pp, INT_PTR *pp_len)
{
	LexWord lex_word;
	const TCHAR *p = *pp;
	INT_PTR p_len = *pp_len;

	for(; p_len > 0;) {
		if(get_char(p) != '(' || get_char(p + 1) != '?' ||
			get_char(p + 2) != '#') break;

		if(lexer(p, p_len, &lex_word) != 0) return 1;
		assert(lex_word.type == LEX_COMMENT);
		p += lex_word.len;
		p_len -= lex_word.len;
	}

	*pp = p;
	*pp_len = p_len;

	return 0;
}

