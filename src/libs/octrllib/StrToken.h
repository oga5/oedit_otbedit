/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#if !defined(_STR_TOKEN_INCLUDED_)
#define _STR_TOKEN_INCLUDED_

#include "mbutil.h"
#include "get_char.h"
#include "CodeAssistData.h"

struct _key_word_st {
	const TCHAR	*str;
	const TCHAR	*org_str;
	// NOTE: lwr_strはdatasetのデータを小文字に更新するので、constはつけない
	TCHAR	*lwr_str;
	unsigned int	len;
	int				type;
	const TCHAR	*type_name;
	const TCHAR	*comment;
	// 候補の抽出を高速化するための値 (a-zの各文字を1bitとしてフラグを立てる)
	unsigned int ch_map;
};

#define CLOSE_BRACKETS_FLG	1
#define OPEN_BRACKETS_FLG	2

#define MAX_ROW_COMMENT_CNT	4

/* KEYWORD_IDX_SIZE: ASCII + 1(ASCII以外) */
#define KEYWORD_IDX_SIZE	(0x80 + 1)

class CStrToken
{
public:
	CStrToken();
	virtual ~CStrToken();

	BOOL isBreakChar(unsigned int ch) {
		if(ch > 0xff) {
			// 全角の空白と句読点(，、。．)は区切り文字にする
			return (ch == L'　' || ch == L'，' || ch == L'、'|| ch == L'。' || ch == L'．');
		}
		return m_break_char[ch];
	}
	virtual BOOL isQuoteStart(const TCHAR *p) {
		if(*p > 0xff) return FALSE;
		return m_quote_char[*p];
	}
	virtual BOOL isQuoteEnd(const TCHAR *p, unsigned int quote_start_ch, int *skip_cnt) {
		*skip_cnt = 1;
		if(*p > 0xff) return FALSE;
		if(m_double_bracket_is_escape_char && *p == quote_start_ch && *(p + 1) == quote_start_ch) {
			*skip_cnt = 2;
			return FALSE;
		}
		if(isEscapeChar(*p) && *(p + 1) == quote_start_ch) {
			*skip_cnt = 2;
			return FALSE;
		}
		if(*p != quote_start_ch) return FALSE;
		return TRUE;
	}
	BOOL isCommentChar(unsigned int ch) {
		if(ch > 0xff) return FALSE;
		return m_comment_char[ch];
	}
	BOOL isCloseBrackets(unsigned int ch) {
		if(ch > 0xff) return FALSE;
		return (m_bracket_char[ch] == CLOSE_BRACKETS_FLG);
	}
	BOOL isOpenBrackets(unsigned int ch) {
		if(ch > 0xff) return FALSE;
		return (m_bracket_char[ch] == OPEN_BRACKETS_FLG);
	}
	BOOL isEscapeChar(unsigned int ch) {
		if(ch == m_escape_char) return TRUE;
		return FALSE;
	}
	BOOL isOperatorChar(unsigned int ch) {
		if(ch > 0xff) return FALSE;
		return m_operator_char[ch];
	}

	TCHAR getOpenBrackets(unsigned int ch);
	TCHAR getCloseBrackets(unsigned int ch);

	void SetKeywordLowerUpper(BOOL distinct_lower_upper) { m_keyword_lower_upper = distinct_lower_upper; }
	int GetKeywordType(const TCHAR *p, unsigned int *len, unsigned int first_word_len);

	void SetEscapeChar(TCHAR ch);
	void SetBreakChars(const TCHAR *p);
	void SetQuoteChars(const TCHAR *p);
	void SetBracketChars(const TCHAR *p);
	void SetOperatorChars(const TCHAR *p);
	void SetLangKeywordChars(const TCHAR *p);
	void SetRowComment(const TCHAR *comment);
	void SetOpenComment(const TCHAR *comment);
	void SetCloseComment(const TCHAR *comment);
	void SetTagMode(BOOL tag_mode) { m_tag_mode = tag_mode; }
	void SetDoubleBracketIsEscapeChar(TCHAR ch) { m_double_bracket_is_escape_char = ch; }

	void SetAssistMatchType(int t) { m_assist_match_type = t; }

	const _TCHAR *GetBracketStr() { return m_bracket_str; }

	BOOL isRowComment(const TCHAR *p);
	BOOL isOpenComment(const TCHAR *p);
	BOOL isCloseComment(const TCHAR *p);
	TCHAR *GetOpenComment();
	TCHAR *GetCloseComment();
	TCHAR *GetRowComment();

	BOOL HaveRowComment();
	BOOL HaveComment();
	BOOL GetTagMode() { return m_tag_mode; }

	const TCHAR *get_word(const TCHAR *p, TCHAR *buf, size_t buf_size, BOOL skip_comment = TRUE);
	const TCHAR *get_word2(const TCHAR *p, const TCHAR **p_buf, BOOL skip_comment = TRUE);

	int get_word_len(const TCHAR *p, BOOL single_line = TRUE) {
		if(isQuoteStart(p)) return get_word_len_quote(p, single_line);

		int		len = 0;
		int		ch_len;

		for(;;) {
			ch_len = get_char_len(p);
			if(isBreakChar(get_char(p))) {
				if(len == 0 && *p != '\0') len = ch_len;
				break;
			}
			p += ch_len;
			len += ch_len;
		}
		return len;
	}

	static TCHAR *skipSpace(const TCHAR *p);
	static TCHAR *skipSpaceMultibyte(const TCHAR *p);
	static TCHAR *skipBracket(const TCHAR *p);

	void Init();
	int initDefaultKeyword(const TCHAR *file_name, TCHAR *msg_buf);

	int GetOpenCommentLen() { return m_open_comment_len; }
	int GetCloseCommentLen() { return m_close_comment_len; }

	virtual int KeywordCompletion(const TCHAR *str, int cnt, TCHAR *buf, 
		unsigned int bufsize, BOOL reverse);
	virtual const TCHAR *skipCommentAndSpace(const TCHAR *p) { return p; }
	virtual BOOL GetCompletionList(CCodeAssistData &grid_data, const TCHAR *str);

private:
	TCHAR *m_key_word_buf;
	unsigned int m_key_word_buf_len;

	TCHAR m_min_keyword_ch;
	TCHAR m_max_keyword_ch;

	TCHAR *m_word_buf;
	int   m_word_buf_len;

	int		m_assist_match_type;


	struct {
		short	start_pos;
		short	end_pos;
		int		break_char_flg;
	} m_key_word_idx[KEYWORD_IDX_SIZE];

	int initKeywordBuf(TCHAR *msg_buf);
	void freeKeywordBuf();

	int post_binary_search_key_word(const TCHAR *p, 
		const TCHAR *first_word, unsigned int first_word_len, int row);
	int binary_search_key_word(const TCHAR *p, unsigned int first_word_len);

	BOOL have_break_char(const TCHAR *p);
	void init_key_word_idx();

	bool realloc_keywords(int needs, TCHAR *msg_buf);
	bool set_keyword(int idx, int type, TCHAR *keyword, TCHAR *msg_buf);

	static unsigned int calc_ch_map(const TCHAR* p);
	static void key_words_qsort(struct _key_word_st* key_words, int left, int right);

protected:
	unsigned int m_max_key_word_len;
	struct _key_word_st	*m_key_words;
	int m_key_word_cnt;

	struct _key_word_st	* volatile m_comp_key_words;
	volatile int m_comp_key_word_cnt;

	TCHAR m_break_char[256];
	TCHAR m_quote_char[256];
	TCHAR m_comment_char[256];
	TCHAR m_bracket_char[256];
	TCHAR m_operator_char[256];
	TCHAR m_lang_keyword_char[256];

	CString m_bracket_str;

	BOOL m_keyword_lower_upper;
	TCHAR *m_open_comment;
	TCHAR *m_close_comment;
	int m_open_comment_len;
	int m_close_comment_len;

	TCHAR *m_row_comment[MAX_ROW_COMMENT_CNT];
	int m_row_comment_len[MAX_ROW_COMMENT_CNT];
	int m_row_comment_cnt;

	unsigned int m_escape_char;
	BOOL m_double_bracket_is_escape_char;

	BOOL m_tag_mode;

	void freeCommentChar();
	void setCommentChar(const TCHAR *comment);
	void freeKeyword();

	int KeywordCompletion(const TCHAR *str, int cnt, TCHAR *buf, 
		unsigned int bufsize, BOOL reverse,	struct _key_word_st	*word_list, int word_cnt);
	virtual int get_word_len_quote(const TCHAR *p, BOOL single_line);

	int addKeyword(const TCHAR *file_name, TCHAR *msg_buf);
	void ClearCharTables();

	static void postAddKeyword(struct _key_word_st* key_words, int key_word_cnt);

	BOOL isLangKeywordChar(unsigned int ch) {
		if(ch > 0xff) return FALSE;
		return m_lang_keyword_char[ch];
	}

	virtual BOOL getDispCommentFlg() { return FALSE; }
};

#include "str_inline.h"

__inline BOOL CStrToken::isRowComment(const TCHAR *p)
{
	int		i;
	for(i = 0; i < m_row_comment_cnt; i++) {
		if(inline_strncmp(p, m_row_comment[i], m_row_comment_len[i]) == 0) return TRUE;
	}

	return FALSE;
}

__inline BOOL CStrToken::isOpenComment(const TCHAR *p) 
{
	if(m_open_comment == NULL) return FALSE;
	if(inline_strncmp(p, m_open_comment, m_open_comment_len) == 0) return TRUE;

	return FALSE;
}

__inline BOOL CStrToken::isCloseComment(const TCHAR *p)
{
	if(m_close_comment == NULL) return FALSE;
	if(inline_strncmp(p, m_close_comment, m_close_comment_len) == 0) return TRUE;

	return FALSE;
}

#endif _STR_TOKEN_INCLUDED_
