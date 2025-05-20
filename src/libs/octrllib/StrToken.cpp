/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "strtoken.h"

#include "UnicodeArchive.h"
#include "mbutil.h"
#include "str_inline.h"
#include "ostrutil.h"

#define NO_BREAK_CHAR_FLG			0
#define BREAK_CHAR_FLG_NORMAL		1
#define BREAK_CHAR_FLG_ONE_CHAR		2


static TCHAR KEYWORD_STR[] = _T("KEYWORD");
static TCHAR KEYWORD2_STR[] = _T("KEYWORD2");
static TCHAR UNKNOWN_STR[] = _T("UNKNOWN");
static TCHAR NULL_STR[] = _T("");

CStrToken::CStrToken()
{
	m_key_words = NULL;
	m_key_word_cnt = 0;
	m_key_word_buf = NULL;
	m_key_word_buf_len = 0;
	m_comp_key_words = 0;
	m_comp_key_word_cnt = NULL;

	m_max_key_word_len = 0;

	int i;
	m_row_comment_cnt = 0;
	for(i = 0; i < MAX_ROW_COMMENT_CNT; i++) {
		m_row_comment[i] = NULL;
		m_row_comment_len[i] = 0;
	}

	m_close_comment = NULL;
	m_open_comment = NULL;

	m_open_comment_len = 0;
	m_close_comment_len = 0;

	m_double_bracket_is_escape_char = '\0';

	m_word_buf = NULL;
	m_word_buf_len = 0;

	m_assist_match_type = ASSIST_FORWARD_MATCH;

	Init();
}

CStrToken::~CStrToken()
{
	freeCommentChar();
	freeKeyword();
	freeKeywordBuf();

	if(m_word_buf != NULL) {
		free(m_word_buf);
		m_word_buf = NULL;
	}
}

void CStrToken::Init()
{
	freeCommentChar();
	freeKeyword();
	freeKeywordBuf();

	m_keyword_lower_upper = TRUE;
	m_tag_mode = FALSE;

	ClearCharTables();
	init_key_word_idx();
}

void CStrToken::freeCommentChar()
{
	int i;
	for(i = 0; i < m_row_comment_cnt; i++) {
		if(m_row_comment[i] != NULL) free(m_row_comment[i]);
		m_row_comment[i] = NULL;
		m_row_comment_len[i] = 0;
	}
	m_row_comment_cnt = 0;

	if(m_close_comment != NULL) {
		free(m_close_comment);
		m_close_comment = NULL;
	}
	if(m_open_comment != NULL) {
		free(m_open_comment);
		m_open_comment = NULL;
	}
	m_open_comment_len = 0;
	m_close_comment_len = 0;
}

TCHAR *CStrToken::skipSpace(const TCHAR *p)
{
	if(p == NULL) return NULL;

	for(; *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'; ) p++;

	return (TCHAR *)p;
}

TCHAR *CStrToken::skipSpaceMultibyte(const TCHAR *p)
{
	for(; *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || get_char(p) == L'　'; ) {
		p += get_char_len(p);
	}

	return (TCHAR *)p;
}

TCHAR *CStrToken::skipBracket(const TCHAR *p)
{
	int		cnt = 0;

	for(; *p != '\0'; p++) {
		if(*p == '(') cnt++;
		if(*p == ')') cnt--;

		if(cnt == 0) break;
	}

	return (TCHAR *)p;
}

const TCHAR *CStrToken::get_word2(const TCHAR *p, const TCHAR **p_buf, BOOL skip_comment)
{
	int		len;

	if(skip_comment) {
		p = skipCommentAndSpace(p);
	}
	if(p == NULL || *p == '\0') goto NO_MORE_WORDS;

	len = get_word_len(p);
	if(len == 0) goto NO_MORE_WORDS;

	if(m_word_buf_len <= len) {
		int new_len = len + 1;
		if(new_len % 1024 != 0) {
			new_len += 1024 - (new_len % 1024);
		}

		m_word_buf = (TCHAR *)realloc(m_word_buf, new_len * sizeof(TCHAR));
		if(m_word_buf == NULL) return NULL;

		m_word_buf_len = new_len;
	}

	_tcsncpy(m_word_buf, p, len);
	m_word_buf[len] = '\0';
	*p_buf = m_word_buf;

	p = p + len;

	return (TCHAR *)p;

NO_MORE_WORDS:
	m_word_buf[0] = '\0';
	*p_buf = m_word_buf;
	return NULL;
}

const TCHAR *CStrToken::get_word(const TCHAR *p, TCHAR *buf, size_t buf_size, BOOL skip_comment)
{
	size_t		len;

	_tcscpy(buf, _T(""));

	if(skip_comment) {
		p = skipCommentAndSpace(p);
	}
	if(p == NULL || *p == '\0') return NULL;

	len = get_word_len(p);
	if(len == 0) return NULL;

	if(len * sizeof(TCHAR) >= buf_size) {
		len = (buf_size - 1) / sizeof(TCHAR);
	}
	
	_tcsncpy(buf, p, len);
	buf[len] = '\0';

	p = p + len;

	return p;
}

void CStrToken::freeKeyword()
{
	if(m_key_words != NULL) {
		for(int i = 0; i < m_key_word_cnt; i++) {
			if(m_key_words[i].lwr_str != NULL) free((void *)m_key_words[i].lwr_str);
			m_key_words[i].lwr_str = NULL;
			if(m_key_words[i].org_str != NULL) free((void*)m_key_words[i].org_str);
			m_key_words[i].org_str = NULL;
			
			m_key_words[i].str = NULL;
		}
		free(m_key_words);
		m_key_words = NULL;
	}

	m_key_word_cnt = 0;
	m_max_key_word_len = 0;

	m_comp_key_words = 0;
	m_comp_key_word_cnt = NULL;

	m_min_keyword_ch = 0xffff;
	m_max_keyword_ch = 0x0000;
}

/*----------------------------------------------------------------------
  キーワードをクイックソートする(行の入れ替え)
----------------------------------------------------------------------*/
__inline static void key_word_st_swap(struct _key_word_st *key_words, int r1, int r2) 
{
	struct _key_word_st tmp_key_word;

	tmp_key_word = key_words[r1];
	key_words[r1] = key_words[r2];
	key_words[r2] = tmp_key_word;
}

/*----------------------------------------------------------------------
  キーワードをクイックソートする(メインの再帰関数)
  昇順ソート
----------------------------------------------------------------------*/
void CStrToken::key_words_qsort(struct _key_word_st *key_words, int left, int right)
{
	int		i, last;
	int		cmp;

	if(left >= right) return;

	key_word_st_swap(key_words, left, (left + right) / 2);

	last = left;
	for(i = left + 1; i <= right; i++) {
		cmp = inline_strcmp(key_words[i].str, key_words[left].str);
		if(cmp < 0) {
			key_word_st_swap(key_words, ++last, i);
		} else if(cmp == 0 && key_words[i].type < key_words[left].type) {
			key_word_st_swap(key_words, ++last, i);
		}
	}
	key_word_st_swap(key_words, left, last);

	key_words_qsort(key_words, left, last - 1);
	key_words_qsort(key_words, last + 1, right);
}

/*----------------------------------------------------------------------
  キーワードを登録したあとの処理
  ソートとch_mapの計算
----------------------------------------------------------------------*/
void CStrToken::postAddKeyword(struct _key_word_st* key_words, int key_word_cnt)
{
	int		i;

	for(i = 0; i < key_word_cnt; i++) {
		key_words[i].ch_map = calc_ch_map(key_words[i].lwr_str);
	}

	// キーワードをソートする(バイナリサーチするため)
	// bounds checkerで，ダングリングメモリエラーになるので，自前のqsortを使う
	//qsort(key_words, key_word_cnt, sizeof(struct _key_word_st), compare_key_word);
	key_words_qsort(key_words, 0, key_word_cnt - 1);
}

/*----------------------------------------------------------------------
  行末の改行コードとsepaで指定した文字を削除する
----------------------------------------------------------------------*/
// FIXME: 文字列処理ライブラリを作成する
static void str_chomp(TCHAR *buf, TCHAR sepa)
{
	TCHAR    *c1;

	if(buf == NULL) return;
	if(buf[0] == '\0') return;

	c1 = buf + _tcslen(buf) - 1;
	for(;(*c1 == '\n' || *c1 == '\r' || *c1 == sepa); c1--) {
		*c1 = '\0';
		if(c1 == buf) break;
	}
}

void CStrToken::freeKeywordBuf()
{
	if(m_key_word_buf != NULL) {
		free(m_key_word_buf);
		m_key_word_buf = NULL;
		m_key_word_buf_len = 0;
	}
}

int CStrToken::initKeywordBuf(TCHAR *msg_buf)
{
	if(m_key_word_buf_len >= m_max_key_word_len + 2) return 0;

	freeKeywordBuf();

	m_key_word_buf_len = m_max_key_word_len + 2;
	if(m_key_word_buf_len < 128) m_key_word_buf_len = 128;

	// キーワードマッチ処理で1文字多くコピーする分と，'\0'のため，max_key_word_len + 2文字分確保する
	m_key_word_buf = (TCHAR *)malloc(m_key_word_buf_len * sizeof(TCHAR));
	if(m_key_word_buf == NULL) {
		_stprintf(msg_buf, _T("CStrToken::initDefaultKeyword(): memory allocate error"));
		return 1;
	}

	return 0;
}

static int gettype(TCHAR *buf)
{
	TCHAR	*p;
	p = _tcschr(buf, ',');
	if(p == NULL) return 1;

	int type = _ttoi(p + 1);
	if(type == 0) type = 1;

	*p = '\0';

	if(type < 0 || type > 2) type = 1;

	return type;
}

BOOL CStrToken::have_break_char(const TCHAR *p)
{
	for(; *p; p += get_char_len(p)) {
		if(isBreakChar(get_char(p))) return TRUE;
	}

	return FALSE;
}

static int get_key_word_idx(TCHAR ch)
{
	if(ch >= KEYWORD_IDX_SIZE) return KEYWORD_IDX_SIZE - 1;
	return ch;
}

void CStrToken::init_key_word_idx()
{
	int		i;

	for(i = 0; i < KEYWORD_IDX_SIZE; i++) {
		m_key_word_idx[i].start_pos = -1;
		m_key_word_idx[i].end_pos = -1;
		m_key_word_idx[i].break_char_flg = NO_BREAK_CHAR_FLG;
	}
	if(m_key_word_cnt == 0) return;

	int idx = get_key_word_idx(m_key_words[0].str[0]);
	m_key_word_idx[idx].start_pos = 0;

	for(i = 0; i < m_key_word_cnt; i++) {
		if(m_key_word_idx[idx].break_char_flg != BREAK_CHAR_FLG_ONE_CHAR && have_break_char(m_key_words[i].str)) {
			m_key_word_idx[idx].break_char_flg = BREAK_CHAR_FLG_NORMAL;
			if(isBreakChar(m_key_words[i].str[0]) ||
				(m_key_words[i].len > 1 && isBreakChar(m_key_words[i].str[1]))) {
				m_key_word_idx[idx].break_char_flg = BREAK_CHAR_FLG_ONE_CHAR;
			}
		}

		if(idx != get_key_word_idx(m_key_words[i].str[0])) {
			m_key_word_idx[idx].end_pos = i - 1;
			idx = get_key_word_idx(m_key_words[i].str[0]);
			m_key_word_idx[idx].start_pos = i;
		}
	}
	m_key_word_idx[idx].end_pos = i - 1;
}

bool CStrToken::realloc_keywords(int needs, TCHAR *msg_buf)
{
	int		i;

	ASSERT(needs > 0);

	m_key_words = (struct _key_word_st *)realloc(m_key_words, sizeof(struct _key_word_st) * needs);
	if(m_key_words == NULL) {
		_stprintf(msg_buf, _T("CStrToken::alloc_keywords(): memory allocate error"));
		return false;
	}
	for(i = m_key_word_cnt; i < needs; i++) {
		m_key_words[i].org_str = NULL;
		m_key_words[i].lwr_str = NULL;
		m_key_words[i].str = NULL;
	}

	return true;
}

unsigned int CStrToken::calc_ch_map(const TCHAR* p)
{
	unsigned int ch_map = 0;

	// キーワード中に使われている文字からAND演算用の数値を生成する
	// a:1, b:2...z:25としてbitを立てていく
	// a-z以外は26〜31を使う 26: $, 27: 0-9, 28〜31は未使用

	for(; *p != '\0'; p++) {
		if(*p >= 'a' && *p <= 'z') {
			ch_map |= (1 << (*p - 'a'));
		} else if(*p >= 'A' && *p <= 'Z') {
			ch_map |= (1 << (*p - 'A'));
		} else if(*p == '$') {
			ch_map |= (1 << 26);
		} else if(*p >= '0' && *p <= '9') {
			ch_map |= (1 << 27);
		}
	}

	return ch_map;
}

bool CStrToken::set_keyword(int idx, int type, TCHAR *keyword, TCHAR *msg_buf)
{
	m_key_words[idx].type = type;
	if(m_key_words[idx].type == 1) {
		m_key_words[idx].type_name = KEYWORD_STR;
	} else if(m_key_words[idx].type == 2){
		m_key_words[idx].type_name = KEYWORD2_STR;
	} else {
		m_key_words[idx].type_name = UNKNOWN_STR;
	}

	m_key_words[idx].comment = NULL_STR;

	m_key_words[idx].org_str = _tcsdup(keyword);
	m_key_words[idx].lwr_str = _tcsdup(keyword);
	if(m_key_words[idx].lwr_str == NULL || m_key_words[idx].org_str == NULL) {
		_stprintf(msg_buf, _T("CStrToken::set_keyword(): memory allocate error"));
		return false;
	}
	my_mbslwr(m_key_words[idx].lwr_str);
	m_key_words[idx].str = (m_keyword_lower_upper) ? m_key_words[idx].lwr_str : m_key_words[idx].org_str;

	m_key_words[idx].len = (unsigned int)_tcslen(m_key_words[idx].str);

	if(m_max_key_word_len < m_key_words[idx].len) m_max_key_word_len = m_key_words[idx].len;
	if(m_min_keyword_ch > m_key_words[idx].str[0]) m_min_keyword_ch = m_key_words[idx].str[0];
	if(m_max_keyword_ch < m_key_words[idx].str[0]) m_max_keyword_ch = m_key_words[idx].str[0];
	return true;
}

int CStrToken::addKeyword(const TCHAR *file_name, TCHAR *msg_buf)
{
	TCHAR	buf[1024];
	int		i;
	int		start;
	int		cnt;

	if(file_name == NULL || (int)_tcslen(file_name) == 0) return 0;

	CUnicodeArchive uni_ar;
	if(!uni_ar.OpenFile(file_name, _T("r"))) {
		_stprintf(msg_buf, _T("CStrToken::initDefaultKeyword(): fopen error(%s)"), file_name);
		return 1;
	}

	// キーワードの数を数える
	cnt = 0;
	for(;;) {
		if(uni_ar.ReadLine(buf, sizeof(buf)) == NULL) break;
		str_chomp(buf, ' ');
		if(buf[0] == '\0') continue;
		if(buf[0] == '/' && buf[1] == '/') continue;	// コメント行
		cnt++;
	}
	if(cnt == 0) return 0;

	// キーワード構造体の初期化
	if(!realloc_keywords(m_key_word_cnt + cnt, msg_buf)) {
		freeKeyword();
		return 1;
	}

	// ファイルの先頭に戻って，キーワードを設定
	if(!uni_ar.ReOpenFile()) {
		_stprintf(msg_buf, _T("CStrToken::initDefaultKeyword(): fopen error(%s)"), file_name);
		return 1;
	}

	start = m_key_word_cnt;
	m_key_word_cnt = m_key_word_cnt + cnt;
	for(i = start; i < m_key_word_cnt; ) {
		if(uni_ar.ReadLine(buf, sizeof(buf)) == NULL) break;
		str_chomp(buf, ' ');
		if(buf[0] == '\0') continue;
		if(buf[0] == '/' && buf[1] == '/') continue;	// コメント行

		int type = gettype(buf);
		str_chomp(buf, ' ');

		if(!set_keyword(i, type, buf, msg_buf)) {
			freeKeyword();
			return 1;
		}
		i++;
	}

	if(m_keyword_lower_upper) {
		// 大文字も対象にするため，チェックする範囲を広げる
		if(is_lead_byte(m_min_keyword_ch) == FALSE && inline_islower(m_min_keyword_ch)) {
			m_min_keyword_ch = toupper(m_min_keyword_ch);
		}
	}
	if(initKeywordBuf(msg_buf) != 0) {
		freeKeyword();
		return 1;
	}

	postAddKeyword(m_key_words, m_key_word_cnt);

	m_comp_key_word_cnt = m_key_word_cnt;
	m_comp_key_words = m_key_words;

	init_key_word_idx();

	return 0;
}

int CStrToken::initDefaultKeyword(const TCHAR *file_name, TCHAR *msg_buf)
{
	freeKeyword();
	return addKeyword(file_name, msg_buf);
}

__inline static int inline_strncmp_nocase_for_strtoken(
	const TCHAR *p1, const TCHAR *p2, int len)
{
	int		i;
	unsigned int	ch1, ch2;

	for(i = 0; i < len;) {
		ch1 = get_char_nocase(p1);
		ch2 = get_char(p2);
		if(ch1 != ch2) return ch1 - ch2;
		if(ch1 == '\0') break;

		i += get_char_len(p1);
		p1 += get_char_len(p1);
		p2 += get_char_len(p2);
	}
	return 0;
}

int CStrToken::post_binary_search_key_word(const TCHAR *p, 
	const TCHAR *first_word, unsigned int first_word_len, int row)
{
	// 最長一致のキーワードを探す
	int result_row = -1;

	if(m_keyword_lower_upper) {
		for(; row < m_key_word_cnt; row++) {
			if(inline_strncmp(first_word, m_key_words[row].str, first_word_len) != 0) break;
			if(inline_strncmp_nocase_for_strtoken(p, m_key_words[row].str, m_key_words[row].len) == 0) {
				if(isBreakChar(get_char(p + m_key_words[row].len))) result_row = row;
			}
		}
	} else {
		for(; row < m_key_word_cnt; row++) {
			if(inline_strncmp(first_word, m_key_words[row].str, first_word_len) != 0) break;
			if(inline_strncmp(p, m_key_words[row].str, m_key_words[row].len) == 0) {
				if(isBreakChar(get_char(p + m_key_words[row].len))) result_row = row;
			}
		}
	}

	return result_row;
}

#pragma intrinsic(memcpy)
int CStrToken::binary_search_key_word(const TCHAR *p, unsigned int first_word_len)
{
	unsigned int idx;
	{
		if(m_keyword_lower_upper) {
			idx = get_key_word_idx(get_char_nocase(p));
		} else {
			idx = get_key_word_idx(*p);
		}
	}
	// インデックスを使って，lとrを調整する
	int l = m_key_word_idx[idx].start_pos;
	if(l == -1) return -1;

	if(first_word_len == 1 && m_key_word_idx[idx].break_char_flg != BREAK_CHAR_FLG_ONE_CHAR) {
		if(m_key_words[l].len == 1) return l;
		return -1;
	}

	int r = m_key_word_idx[idx].end_pos;

	const TCHAR *first_word;
	if(m_keyword_lower_upper) {
		memcpy(m_key_word_buf, p, first_word_len * sizeof(TCHAR));
		m_key_word_buf[first_word_len] = '\0';
		my_mbslwr(m_key_word_buf);
		first_word = m_key_word_buf;
	} else {
		first_word = p;
	}

	int row, cmp;

	if(m_key_word_idx[idx].break_char_flg) {
		if(first_word_len == 1) {
			//ASSERT(m_key_word_idx[idx].break_char_flg == BREAK_CHAR_FLG_ONE_CHAR);
			// 最長一致のキーワードを探す
			return post_binary_search_key_word(p, first_word, first_word_len, l);
		}

		while(r >= l) {
			row = (l + r) / 2;
			cmp = inline_strncmp(first_word, m_key_words[row].str, first_word_len);

			if(cmp < 0) {
				r = row - 1;
			} else if(cmp > 0) {
				l = row + 1;
			} else { // cmp == 0
				// 最初の単語の先頭位置を探す
				for(; row > 0; row--) {
					if(inline_strncmp(first_word, m_key_words[row - 1].str, first_word_len) != 0) break;
				}
				// 最長一致のキーワードを探す
				return post_binary_search_key_word(p, first_word, first_word_len, row);
			}
		}
	} else {
		while(r >= l) {
			row = (l + r) / 2;
			cmp = inline_strncmp(first_word, m_key_words[row].str, m_key_words[row].len);

			if(cmp < 0) {
				r = row - 1;
			} else if(cmp > 0) {
				l = row + 1;
			} else { // cmp == 0
				if(m_key_words[row].len == first_word_len) return row;
				// first_wordのほうが長い
				l = row + 1;
			}
		}
	}

	// キーワードなし
	return -1;
}
#pragma function(memcpy)

int CStrToken::GetKeywordType(const TCHAR *p, unsigned int *len, unsigned int first_word_len)
{
	int type = 0;

	if(isLangKeywordChar(*p)) {
		int lang_len = 1;
		if(*(p + 1) != '\0' && *(p + 1) != '\n' && *(p + 1) != ' ' && *(p + 1) != '\t') {
			lang_len += get_word_len(p + 1);
		}
		if(lang_len != 1) {
			type = 2;
			if(len != NULL) *len = lang_len;
		}
	}

	if(m_key_word_cnt == 0) return type;
	if(m_min_keyword_ch > *p || m_max_keyword_ch < *p) return type;
	if(first_word_len > m_max_key_word_len) return type;

	// バイナリサーチ
	int row = binary_search_key_word(p, first_word_len);
	if(row == -1) return type;

	if(len != NULL) *len = m_key_words[row].len;

	return m_key_words[row].type;
}

static int binary_search_key_word_for_completion(const TCHAR *word, int *cnt, BOOL reverse,
	struct _key_word_st	*word_list, int word_cnt)
{
	int		l, r, row, cmp;
	int		len;

	l = 0;
	r = word_cnt - 1;
	len = (int)_tcslen(word);

	while(r >= l) {
		row = (l + r) / 2;
		cmp = _tcsncmp(word, word_list[row].str, len);

		if(cmp == 0) {
			if(row == l) {
				int end = word_cnt;

				if(reverse) {
					// 最初の位置のときは，ヒットしない
					if(*cnt == 0) return -1;

					if(*cnt == -1) {
						// 最初の実行のときは，最後の位置を探す
						for(; row < end; row++, (*cnt)++) {
							if(_tcsncmp(word, word_list[row].str, len) != 0) break;
						}
						row--;
					} else {
						// ひとつ前を調べる
						(*cnt)--;
						row = row + *cnt;
						if(row >= end || _tcsncmp(word, word_list[row].str, len) != 0) return -1;
					}
				} else {
					(*cnt)++;
					row = row + *cnt;
					if(row >= end || _tcsncmp(word, word_list[row].str, len) != 0) return -1;
				}

				return row;
			}
			r = row;
		} else if(cmp > 0) {
			l = row + 1;
		} else {
			r = row - 1;
		}
	}

	// キーワードなし
	return -1;
}

int CStrToken::KeywordCompletion(const TCHAR *str, int cnt, TCHAR *buf, 
	unsigned int bufsize, BOOL reverse, struct _key_word_st	*word_list, int word_cnt)
{
	if(word_cnt == 0) return -1;

	int		row;

	if(_tcslen(str) > m_max_key_word_len) return -1;
	_tcscpy(m_key_word_buf, str);

	if(m_keyword_lower_upper) my_mbslwr(m_key_word_buf);

	row = binary_search_key_word_for_completion(m_key_word_buf, &cnt, reverse, word_list, word_cnt);
	if(row == -1) return -1;

	// bufsizeはbyte単位でくるので、* sizeof(TCHAR)してチェックする
	if((word_list[row].len) * sizeof(TCHAR) >= bufsize) return -1;

	_tcscpy(buf, word_list[row].str);

	if(m_keyword_lower_upper & inline_isupper(str[0])) {
		my_mbsupr(buf);
	}

	return cnt;
}

int CStrToken::KeywordCompletion(const TCHAR *str, int cnt, TCHAR *buf, 
	unsigned int bufsize, BOOL reverse)
{
	return KeywordCompletion(str, cnt, buf, bufsize, reverse, m_comp_key_words, m_comp_key_word_cnt);
}

void CStrToken::ClearCharTables()
{
	int		i;
	for(i = 0; i < 256; i++) {
		m_break_char[i] = 0;
		m_quote_char[i] = 0;
		m_comment_char[i] = 0;
		m_bracket_char[i] = 0;
		m_operator_char[i] = 0;
		m_lang_keyword_char[i] = 0;
	}
	m_escape_char = '\0';
	m_break_char['\0'] = 1;
	m_break_char['\t'] = 1;
	m_break_char['\r'] = 1;
	m_break_char['\n'] = 1;
	m_break_char[' '] = 1;
}

void CStrToken::SetEscapeChar(TCHAR ch)
{
	if(ch > 0xff) return;

	m_escape_char = ch;
	m_break_char[ch] = 1;
}

void CStrToken::SetBreakChars(const TCHAR *p)
{
	for(; *p != '\0'; p++) {
		if(*p > 0xff) continue;
		m_break_char[*p] = 1;
	}
}

void CStrToken::SetLangKeywordChars(const TCHAR *p)
{
	for(; *p != '\0'; p++) {
		if(*p > 0xff) continue;
		m_lang_keyword_char[*p] = 1;
	}
}

void CStrToken::SetOperatorChars(const TCHAR *p)
{
	for(; *p != '\0'; p++) {
		if(*p > 0xff) continue;
		m_operator_char[*p] = 1;
		m_break_char[*p] = 1;
	}
}

void CStrToken::SetQuoteChars(const TCHAR *p)
{
	for(; *p != '\0'; p++) {
		if(*p > 0xff) continue;
		m_quote_char[*p] = 1;
		m_break_char[*p] = 1;
		m_comment_char[*p] = 1;
	}
}

void CStrToken::SetBracketChars(const TCHAR *p)
{
	m_bracket_str = p;

	for(; *p != '\0'; p++) {
		if(*p > 0xff) continue;
		m_bracket_char[*p] = CLOSE_BRACKETS_FLG;
		m_break_char[*p] = 1;
		m_bracket_char[getOpenBrackets(*p)] = OPEN_BRACKETS_FLG;
		m_break_char[getOpenBrackets(*p)] = 1;
	}
}

void CStrToken::setCommentChar(const TCHAR *comment)
{
	for(; *comment != '\0'; comment++) {
		if(*comment > 0xff) continue;
		m_comment_char[*comment] = 1;
		m_break_char[*comment] = 1;
	}
}

void CStrToken::SetRowComment(const TCHAR *comment)
{
	if(comment == NULL || _tcslen(comment) == 0) return;
	if(m_row_comment_cnt >= MAX_ROW_COMMENT_CNT) return;

	m_row_comment[m_row_comment_cnt] = (TCHAR *)_tcsdup(comment);
	m_row_comment_len[m_row_comment_cnt] = (int)_tcslen(comment);
	m_row_comment_cnt++;

	setCommentChar(comment);
}

TCHAR *CStrToken::GetRowComment()
{
	if(m_row_comment_cnt == 0) return _T("");	// dummy;
	return m_row_comment[0];
}

void CStrToken::SetOpenComment(const TCHAR *comment)
{
	if(m_open_comment != NULL) {
		free(m_open_comment);
		m_open_comment = NULL;
		m_open_comment_len = 0;
	}
	if(comment == NULL || _tcslen(comment) == 0) return;

	m_open_comment = _tcsdup(comment);
	m_open_comment_len = (int)_tcslen(m_open_comment);

	setCommentChar(comment);
}

void CStrToken::SetCloseComment(const TCHAR *comment)
{
	if(m_close_comment != NULL) {
		free(m_close_comment);
		m_close_comment = NULL;
		m_close_comment_len = 0;
	}
	if(comment == NULL || (int)_tcslen(comment) == 0) return;

	m_close_comment = _tcsdup(comment);
	m_close_comment_len = (int)_tcslen(m_close_comment);

	setCommentChar(comment);
}

TCHAR *CStrToken::GetOpenComment()
{
	if(m_open_comment == NULL) return _T("X");	// dummy;
	return m_open_comment;
}

TCHAR *CStrToken::GetCloseComment()
{
	if(m_close_comment == NULL) return _T("X");	// dummy;
	return m_close_comment;
}

TCHAR CStrToken::getOpenBrackets(unsigned int ch)
{ 
	switch(ch) {
	case ')': return '(';
	case '}': return '{';
	case '>': return '<';
	case ']': return '[';
	}
	return '\0';
}

TCHAR CStrToken::getCloseBrackets(unsigned int ch)
{ 
	switch(ch) {
	case '(': return ')';
	case '{': return '}';
	case '<': return '>';
	case '[': return ']';
	}
	return '\0';
}

int CStrToken::get_word_len_quote(const TCHAR *p, BOOL single_line)
{
	TCHAR	quote_char;
	int		len = 0;
	unsigned int stop_ch = '\0';
	if(single_line) stop_ch = '\n';

	quote_char = *p;
	len++;
	p++;

	for(; (*p != '\0' && *p != stop_ch); len++, p++) {
		if(is_lead_byte(*p)) {
			len++;
			p++;
			if(*p == '\0' || *p == stop_ch) break;
		} else if(isEscapeChar(*p)) {
			len++;
			p++;
			if(*p == '\0' || *p == stop_ch) break;
		} else if(*p == quote_char) {
			len++;
			p++;
			if(*p == quote_char && *p == m_double_bracket_is_escape_char) continue;
			break;
		}
	}

	return len;
}

BOOL CStrToken::HaveRowComment()
{
	if(m_row_comment_cnt > 0) return TRUE;
	return FALSE;
}

BOOL CStrToken::HaveComment()
{
	if(m_row_comment_cnt > 0) return TRUE;
	if(m_open_comment != NULL && m_open_comment[0] != '\0') return TRUE;
	if(m_close_comment != NULL && m_close_comment[0] != '\0') return TRUE;
	return FALSE;
}

static int binary_search_key_word_for_get_completion_list(
	struct _key_word_st *word_list, int word_cnt,
	const TCHAR *word, unsigned int len)
{
	int		l, r, row, cmp;

	l = 0;
	r = word_cnt - 1;

	while(r >= l) {
		row = (l + r) / 2;
		cmp = inline_strncmp(word, word_list[row].lwr_str, len);

		if(cmp < 0) {
			r = row - 1;
		} else if(cmp > 0) {
			l = row + 1;
		} else { // cmp == 0
			// 先頭位置を探す
			for(; row > 0; row--) {
				if(inline_strncmp(word, word_list[row - 1].lwr_str, len) != 0) break;
			}
			return row;
		}
	}

	// キーワードなし
	return -1;
}

BOOL CStrToken::GetCompletionList(CCodeAssistData &grid_data, const TCHAR *str)
{
	unsigned int key_word_len = (unsigned int)_tcslen(str);
	if(key_word_len > m_max_key_word_len || key_word_len >= m_key_word_buf_len) return FALSE;

	BOOL b_disp_comment = getDispCommentFlg();

	grid_data.InitData(b_disp_comment);

	_tcscpy(m_key_word_buf, str);
	my_mbslwr(m_key_word_buf);

	if(m_assist_match_type == ASSIST_FORWARD_MATCH) {
		if(!m_keyword_lower_upper) {
			// 大文字小文字を区別する言語(Java/C++)のとき，全キーワードを調べる
			for(int row = 0; row < m_comp_key_word_cnt; row++) {
				if(inline_strncmp(m_key_word_buf, m_comp_key_words[row].lwr_str, key_word_len) != 0) continue;

				grid_data.AddRow();
				grid_data.SetData(grid_data.Get_RowCnt() - 1, CODE_ASSIST_DATA_NAME, m_comp_key_words[row].org_str);
				grid_data.SetData(grid_data.Get_RowCnt() - 1, CODE_ASSIST_DATA_TYPE, m_comp_key_words[row].type_name);
				if(b_disp_comment) {
					grid_data.SetData(grid_data.Get_RowCnt() - 1, CODE_ASSIST_DATA_COMMENT, m_comp_key_words[row].comment);
				}
			}
		} else {
			// 先頭位置を探す
			int row = binary_search_key_word_for_get_completion_list(
				m_comp_key_words, m_comp_key_word_cnt,
				m_key_word_buf, key_word_len);
			if(row < 0) return FALSE;

			for(; row < m_comp_key_word_cnt; row++) {
				if(inline_strncmp(m_key_word_buf, m_comp_key_words[row].lwr_str, key_word_len) != 0) break;

				grid_data.AddRow();
				grid_data.SetData(grid_data.Get_RowCnt() - 1, CODE_ASSIST_DATA_NAME, m_comp_key_words[row].org_str);
				grid_data.SetData(grid_data.Get_RowCnt() - 1, CODE_ASSIST_DATA_TYPE, m_comp_key_words[row].type_name);
				if(b_disp_comment) {
					grid_data.SetData(grid_data.Get_RowCnt() - 1, CODE_ASSIST_DATA_COMMENT, m_comp_key_words[row].comment);
				}
			}
		}
	} else if(m_assist_match_type == ASSIST_PARTIAL_MATCH) {
		unsigned int ch_map = calc_ch_map(m_key_word_buf);

		for(int row = 0; row < m_comp_key_word_cnt; row++) {
			if((ch_map & m_comp_key_words[row].ch_map) != ch_map) continue;
			if(ostr_strstr_nocase(m_comp_key_words[row].lwr_str, m_key_word_buf, key_word_len) == NULL) continue;

			grid_data.AddRow();
			grid_data.SetData(grid_data.Get_RowCnt() - 1, CODE_ASSIST_DATA_NAME, m_comp_key_words[row].org_str);
			grid_data.SetData(grid_data.Get_RowCnt() - 1, CODE_ASSIST_DATA_TYPE, m_comp_key_words[row].type_name);
			if(b_disp_comment) {
				grid_data.SetData(grid_data.Get_RowCnt() - 1, CODE_ASSIST_DATA_COMMENT, m_comp_key_words[row].comment);
			}
		}
	}

	if(grid_data.Get_RowCnt() == 0) return FALSE;
	return TRUE;
}
