/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>

#include "local.h"
#include "get_char.h"
#include "str_inline.h"

#define OPTIMIZE_SKIP_EPSILON (0x01 << 0)
#define OPTIMIZE_MERGE_SELECTIVE (0x01 << 1)
#define OPTIMIZE_MAKE_WORDS_NODE (0x01 << 2)
#define CHECK_BEHIND_LOOP (0x01 << 3)
#define OPTIMIZE_QUANTIFIER_START_NODE (0x01 << 4)
#define OPTIMIZE_BRANCH (0x01 << 5)

#define MAX_OPTIMIZE_BRANCH 30
#define MAX_BRANCH_CANDIDATE    20

const unsigned char CHAR_CLASS_d_TBL[ESC_CHAR_CLASS_TABLE_SIZE] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
const unsigned char CHAR_CLASS_w_TBL[ESC_CHAR_CLASS_TABLE_SIZE] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03,
	0xfe, 0xff, 0xff, 0x87, 0xfe, 0xff, 0xff, 0x07, };
const unsigned char CHAR_CLASS_s_TBL[ESC_CHAR_CLASS_TABLE_SIZE] = {
	0x00, 0x12, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
const unsigned char CHAR_CLASS_l_TBL[ESC_CHAR_CLASS_TABLE_SIZE] = {
	0x00, 0x36, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

static NFA_NODE *make_nfa_main(OREG_DATA *reg_data, const TCHAR *p, INT_PTR p_len, int b_enable_loop);
static NFA_NODE *make_group(OREG_DATA *reg_data, LexWord *lex_word, int b_cond_flg);
static int get_named_backref(OREG_DATA *reg_data, const TCHAR *data, INT_PTR *p_name_len, TCHAR **p_name);

static int alloc_nfa_buf(OREG_DATA *reg_data, size_t min_size)
{
	size_t		size = ALLOC_BUF_SIZE;
	NFA_BUF *buf;

	if(size < min_size) size = min_size;

	buf = (NFA_BUF *)malloc(size + sizeof(NFA_BUF));
	if(buf == NULL) return 1;

	buf->next = reg_data->alloc_buf.cur;
	reg_data->alloc_buf.cur = buf;
	reg_data->alloc_buf.buf_size = size;
	reg_data->alloc_buf.cur_pt = 0;

	return 0;
}

static int init_reg_data(OREG_DATA *reg_data)
{
	int		i;

	reg_data->nfa_node = NULL;

	reg_data->back_ref_cnt = 0;
	for(i = 0; i < BACK_REF_IDX_MAX; i++) {
		reg_data->back_ref.data[i].len = -1;
		reg_data->back_ref_name[i] = _T("");
	}

	reg_data->reg_exec_mode = REG_EXEC_NONE;
	reg_data->stack_data = NULL;
	reg_data->stack_size = 0;
	reg_data->alloc_buf.cur = NULL;

	if(alloc_nfa_buf(reg_data, 0) != 0) return 1;

	reg_data->reg_switch = 0;
	reg_data->cur_node_id = 0;

	return 0;
}

static OREG_DATA *oreg_alloc()
{
	OREG_DATA	*reg_data;

    reg_data = (OREG_DATA *)malloc(sizeof(OREG_DATA));
    if(reg_data == NULL) return NULL;

    if(init_reg_data(reg_data) != 0) {
        oreg_free(reg_data);
        return NULL;
    }

	return reg_data;
}

void oreg_free(OREG_DATA *reg_data)
{
	NFA_BUF *free_buf, *next_buf;

	if(reg_data == NULL) return;

	if(reg_data->reg_exec_mode == REG_EXEC_BOYER_MOORE &&
		reg_data->exec_data.bm_data != NULL) {
		bm_free(reg_data->exec_data.bm_data);
	}

	if(reg_data->stack_data != NULL) free(reg_data->stack_data);

	for(next_buf = reg_data->alloc_buf.cur; next_buf != NULL;) {
		free_buf = next_buf;
		next_buf = next_buf->next;
		free(free_buf);
	}

	free(reg_data);
}

__inline static void *my_malloc(OREG_DATA *reg_data, size_t size)
{
	void *p;

	if(size % 4 != 0) { // sizeは4の倍数にする(BUSエラーになるため)
		size += 4 - size % 4;
	}

	if(reg_data->alloc_buf.cur_pt + size >= reg_data->alloc_buf.buf_size) {
		if(alloc_nfa_buf(reg_data, size) != 0) return NULL;
	}

	p = reg_data->alloc_buf.cur->buf + reg_data->alloc_buf.cur_pt;
	reg_data->alloc_buf.cur_pt += size;

	return p;
}

static void clear_nfa_node(NFA_NODE *node)
{
	node->type = NODE_EPSILON;
	node->next[0] = NULL;
	node->next[1] = NULL;
	node->first_test = 0;
}

static NFA_NODE *alloc_nfa_node(OREG_DATA *reg_data)
{
	NFA_NODE	*node = NULL;
	node = (NFA_NODE *)my_malloc(reg_data, sizeof(NFA_NODE));
	if(node == NULL) return NULL;

	node->type = NODE_EPSILON;
	node->next[0] = NULL;
	node->next[1] = NULL;
	node->first_test = 0;
	node->optimized = 0;
	node->node_id = reg_data->cur_node_id;
	reg_data->cur_node_id++;

	return node;
}

static NFA_NODE *get_last_node(NFA_NODE *node)
{
	for(; node->next[0] != NULL; node = node->next[0]) ;
	return node;
}

static NFA_NODE *make_word_node(OREG_DATA *reg_data, unsigned int ch)
{
	NFA_NODE *node = alloc_nfa_node(reg_data);
	if(node == NULL) return NULL;

	if(reg_data->reg_switch & REG_SWITCH_ic) ch = inline_tolower_mb(ch);
	if(reg_data->reg_switch & REG_SWITCH_iw) ch = get_ascii_char(ch);

	node->type = NODE_WORD;
	node->data.ch.c = ch;
	node->data.ch.ic_flg = (reg_data->reg_switch & REG_SWITCH_ic) ? 1:0;
	node->data.ch.iw_flg = (reg_data->reg_switch & REG_SWITCH_iw) ? 1:0;

	return node;
}

static NFA_NODE *make_type_node(OREG_DATA *reg_data, NfaNodeType type)
{
	NFA_NODE *node = alloc_nfa_node(reg_data);
	if(node == NULL) return NULL;

	node->type = type;

	return node;
}

static NFA_NODE *make_branch_node(OREG_DATA *reg_data, unsigned int ctrl_ch)
{
	NFA_NODE *node = make_type_node(reg_data, NODE_BRANCH);
	node->data.branch.ctrl_ch = ctrl_ch;
	node->data.branch.branch_tbl = NULL;
	return node;
}

static NFA_NODE *make_recursive_node(OREG_DATA *reg_data, int idx)
{
	NFA_NODE *node = alloc_nfa_node(reg_data);
	if(node == NULL) return NULL;

	node->type = NODE_RECURSIVE;
	node->data.recursive.idx = idx;

	{
		NFA_NODE *loop_detect_node =
			make_type_node(reg_data, NODE_LOOP_DETECT);
		if(loop_detect_node == NULL) return NULL;

		node->next[0] = loop_detect_node;
	}

	return node;
}

static NFA_NODE *make_recursive_node_by_name(OREG_DATA *reg_data, 
	const TCHAR *name)
{
	int		i;

	for(i = 0; i < reg_data->back_ref_cnt; i++) {
		if(_tcscmp(name, reg_data->back_ref_name[i]) == 0) {
			return make_recursive_node(reg_data, i + 1);
		}
	}

	return NULL;
}

static NFA_NODE *add_nfa_node(NFA_NODE *node, NFA_NODE *next_node)
{
	if(node == NULL) return next_node;

	node->next[0] = next_node;

	return get_last_node(next_node);
}

static int check_behind_loop(NFA_NODE *node)
{
	if(node == NULL) return 0;

	if(node->type == NODE_LOOK_AHEAD_START ||
	   node->type == NODE_NOT_LOOK_AHEAD_START) {
		node = node->data.group.group_end_node;
	}

	if(node->optimized & CHECK_BEHIND_LOOP) return 1;
	node->optimized |= CHECK_BEHIND_LOOP;

	if(check_behind_loop(node->next[0]) != 0) return 1;
	if(check_behind_loop(node->next[1]) != 0) return 1;

	node->optimized &= ~CHECK_BEHIND_LOOP;

	return 0;
}

static int check_behind_node(NFA_NODE *node)
{
	int cnt = 0;

	for(; node != NULL; node = node->next[0]) {
		switch(node->type) {
		case NODE_WORD:
		case NODE_ANY_CHAR:
		case NODE_CHAR_CLASS:
			cnt++;
			break;
		case NODE_LOOK_AHEAD_START:
		case NODE_NOT_LOOK_AHEAD_START:
			node = node->data.group.group_end_node;
			break;
		}
		if(node->next[1] != NULL) {
			int cnt1, cnt2;

			cnt1 = check_behind_node(node->next[0]);
			cnt2 = check_behind_node(node->next[1]);
			if(cnt1 == -1 || cnt2 == -1 || cnt1 != cnt2) return -1;

			cnt += cnt1;
			break;
		}
	}

	return cnt;
}

static NFA_NODE *make_conditional(OREG_DATA *reg_data, LexWord *lex_word)
{
	const TCHAR			*p;
	INT_PTR				p_len;
	LexWord				cond_word;
	const TCHAR			*true_str;
	const TCHAR			*false_str;
	INT_PTR				true_len;
	INT_PTR				false_len;
	LexWord				tmp_word;
	NFA_NODE			*node, *cond_node, *true_node, *false_node, *last_node;

	p = lex_word->data + 2;		// (?を読み飛ばす
	p_len = lex_word->len - 3;	// (?と)の分をマイナス

	// conditionの文字列を取得
	if(skip_comment(&p, &p_len) != 0) return NULL;
	if(lexer(p, p_len, &cond_word) != 0) return NULL;
	assert(cond_word.type == LEX_GROUP);
	p += cond_word.len;
	p_len -= cond_word.len;

	// true時の検索文字列を取得
	true_str = p;
	true_len = 0;
	for(;;) {
		if(skip_comment(&p, &p_len) != 0) return NULL;
		if(lexer(p, p_len, &tmp_word) != 0) return NULL;
		p += tmp_word.len;
		p_len -= tmp_word.len;
		if(tmp_word.type == LEX_OR_CHAR) break;
		true_len += tmp_word.len;
		if(p_len == 0) break;
	}

	// false時の検索文字列を取得
	false_str = p;
	false_len = p_len;

	//
	if(cond_word.data[1] >= '1' && cond_word.data[1] <= '9') {
		if(cond_word.len > 3) return NULL;

		cond_node = make_type_node(reg_data, NODE_CONDITIONAL_BACK_REF_EXIST);
		if(cond_node == NULL) return NULL;

		cond_node->data.back_ref.idx = cond_word.data[1] - '0' - 1;
	} else if(cond_word.data[1] == '<') {
		INT_PTR name_len;
		TCHAR *name;
		if(get_named_backref(reg_data, cond_word.data, &name_len, &name)) {
			cond_node = make_type_node(reg_data,
				NODE_CONDITIONAL_NAMED_BACK_REF_EXIST);
			if(cond_node == NULL) return NULL;

			cond_node->data.named_back_ref.name = name;
		} else {
			return NULL;
		}
	} else if(cond_word.data[1] == 'R') {
		cond_node = make_type_node(reg_data, NODE_CONDITIONAL_RECURSIVE);
		if(cond_node == NULL) return NULL;

		if(cond_word.data[2] != ')') {
			if(cond_word.data[2] >= '1' && cond_word.data[2] <= '9') {
				cond_node->data.back_ref.idx = cond_word.data[2] - '0';
			} else {
				// FIXME: R&NAMEに対応する
				return NULL;
			}
		} else {
			cond_node->data.back_ref.idx = 0;
		}
	} else {
		cond_node = make_group(reg_data, &cond_word, 1);
		if(cond_node == NULL) return NULL;
	}

	true_node = make_nfa_main(reg_data, true_str, true_len, 1);
	if(true_node == NULL) return NULL;

	if(false_len == 0) {
		false_node = alloc_nfa_node(reg_data);
	} else {
		false_node = make_nfa_main(reg_data, false_str, false_len, 1);
	}
	if(false_node == NULL) return NULL;

	node = make_type_node(reg_data, NODE_CONDITIONAL);
	if(node == NULL) return NULL;

	node->data.condition = cond_node;
	node->next[0] = false_node;
	node->next[1] = true_node;

	// true, falseの最後で合流する
	last_node = alloc_nfa_node(reg_data);
	if(last_node == NULL) return NULL;
	get_last_node(true_node)->next[0] = last_node;
	get_last_node(false_node)->next[0] = last_node;

	return node;
}

static int is_reg_switch(unsigned int ch)
{
	if(ch == '-' || ch == 's' || ch == 'S' || ch == 'i' || ch == 'w') return 1;
	return 0;
}

static void set_reg_switch(OREG_DATA *reg_data, int sw, int b_minus)
{
	if(b_minus) {
		reg_data->reg_switch &= ~sw;
	} else {
		reg_data->reg_switch |= sw;
	}
}

static NFA_NODE *process_reg_switch(OREG_DATA *reg_data, LexWord *lex_word)
{
	const TCHAR		*data;
	INT_PTR		len;
	int		b_minus = 0;
	int		back_reg_switch = reg_data->reg_switch;
	unsigned int ch;

	// "(?"を読み飛ばす
	data = lex_word->data + 2;
	len = lex_word->len - 3;

	for(; len > 0;) {
		ch = get_char(data);
		len -= get_char_len(data);
		data += get_char_len(data);

		switch(ch) {
		case '-':
			b_minus = 1;
			break;
		case 's':
			set_reg_switch(reg_data, REG_SWITCH_s, b_minus);
			break;
		case 'S':
			set_reg_switch(reg_data, REG_SWITCH_S, b_minus);
			break;
		case 'i':
			set_reg_switch(reg_data, REG_SWITCH_ic, b_minus);
			break;
		case 'w':
			set_reg_switch(reg_data, REG_SWITCH_iw, b_minus);
			break;
		case ':':
			{
				NFA_NODE *result;
				result = make_nfa_main(reg_data, data, len, 1);
				reg_data->reg_switch = back_reg_switch;
				return result;
			}
		default:
			return NULL;
		}
	}

	return alloc_nfa_node(reg_data);
}

static int get_named_backref(OREG_DATA *reg_data, const TCHAR *data, INT_PTR *p_name_len, TCHAR **p_name)
{
	const TCHAR *p = data;
	unsigned int end_ch = '\0';
	int start_len;
	int end_len;

	unsigned int ch;

	assert(get_char(p) == '?' || get_char(p) == '(');
	p += get_char_len(p);

	ch = get_char(p);
	if(ch == '<') {
		start_len = 2;
		end_len = 1;
		end_ch = '>';
		p += get_char_len(p);
	} else if(ch == '\'') {
		start_len = 2;
		end_len = 1;
		end_ch = '\'';
		p += get_char_len(p);
	} else if(ch == 'P') {
		p += get_char_len(p);
		if(get_char(p) != '<') return 0;

		start_len = 3;
		end_len = 1;
		end_ch = '>';
		p += get_char_len(p);
	} else if(ch == '&') {
		start_len = 2;
		end_len = 1;
		end_ch = ')';
		p += get_char_len(p);
	}
	if(end_ch == '\0') return 0;

	for(;;) {
		ch = get_char(p);
		if(ch == '\0') return 0;
		if(ch == end_ch) {
			p += get_char_len(p);
			break;
		}
		p += get_char_len(p);
	}

	{
		INT_PTR name_len;
		INT_PTR copy_len;
		TCHAR *name;

		name_len = p - data;
		copy_len = name_len - start_len - end_len;
		name = (TCHAR *)my_malloc(reg_data, (copy_len + 1) * sizeof(TCHAR));
		_tcsncpy(name, data + start_len, copy_len);
		name[copy_len] = '\0';

		*p_name_len = name_len;
		*p_name = name;
	}

	return 1;
}

static NFA_NODE *make_group(OREG_DATA *reg_data, LexWord *lex_word,
	int b_cond_flg)
{
	NFA_NODE	*start = NULL, *end = NULL, *node = NULL;
	const TCHAR		*data = NULL;
	INT_PTR			len;
	int			b_enable_loop = 1;

	data = lex_word->data + 1;
	len = lex_word->len - 2;

	if(get_char(data) == '?') {
		unsigned int ch1 = get_char(data + 1);

		if(ch1 == ':') {
			data += 2;
			len -= 2;
			return make_nfa_main(reg_data, data, len, 1);
		} else if(ch1 == '>') {
			data += 2;
			len -= 2;
			start = make_type_node(reg_data, NODE_NO_BACKTRACK_START);
			end = make_type_node(reg_data, NODE_REGEXP_SUB_END);
			start->data.group.group_end_node = end;
		} else if(ch1 == '=') {
			data += 2;
			len -= 2;
			start = make_type_node(reg_data, NODE_LOOK_AHEAD_START);
			end = make_type_node(reg_data, NODE_REGEXP_SUB_END);
			start->data.group.group_end_node = end;
		} else if(ch1 == '!') {
			data += 2;
			len -= 2;
			start = make_type_node(reg_data, NODE_NOT_LOOK_AHEAD_START);
			end = make_type_node(reg_data, NODE_REGEXP_SUB_END);
			start->data.group.group_end_node = end;
		} else if(ch1 == '<' && get_char(data + 2) == '=') {
			data += 3;
			len -= 3;
			b_enable_loop = 0;
			start = make_type_node(reg_data, NODE_LOOK_BEHIND_START);
			end = make_type_node(reg_data, NODE_REGEXP_SUB_END);
			start->data.group.group_end_node = end;
		} else if(ch1 == '<' && get_char(data + 2) == '!') {
			data += 3;
			len -= 3;
			b_enable_loop = 0;
			start = make_type_node(reg_data, NODE_NOT_LOOK_BEHIND_START);
			end = make_type_node(reg_data, NODE_REGEXP_SUB_END);
			start->data.group.group_end_node = end;
		} else if(ch1 == '(') {
			return make_conditional(reg_data, lex_word);
		} else if(ch1 == 'R') {
			/* recursive expression not terminated */
			if(get_char(data + 2) != ')') return NULL;
			return make_recursive_node(reg_data, 0);
		} else if(ch1 >= '0' && ch1 <= '9') {
			/* FIXME: 0〜9のみ対応: BACK_REF_IDX_MAXまで拡張する */
			/* recursive expression not terminated */
			if(get_char(data + 2) != ')') return NULL;
			return make_recursive_node(reg_data, ch1 - '0');
		} else if(is_reg_switch(ch1)) {
			return process_reg_switch(reg_data, lex_word);
		} else if(ch1 == '<' || ch1 == '\'' || ch1 == 'P') {
			INT_PTR name_len;
			TCHAR *name;
			if(get_named_backref(reg_data, data, &name_len, &name)) {
				data += name_len;
				len -= name_len;

				start = make_type_node(reg_data, NODE_GROUP_START);
				end = make_type_node(reg_data, NODE_GROUP_END);
				if(start == NULL) return NULL;
				if(end == NULL) return NULL;
				start->data.back_ref.idx = reg_data->back_ref_cnt;
				end->data.back_ref.idx = reg_data->back_ref_cnt;
				reg_data->back_ref_nfa_start[reg_data->back_ref_cnt] = start;
				reg_data->back_ref_name[reg_data->back_ref_cnt] = name;
				(reg_data->back_ref_cnt)++;
			}
		} else if(ch1 == '&') {
			INT_PTR name_len;
			TCHAR *name;
			if(get_named_backref(reg_data, data, &name_len, &name)) {
				data += name_len;
				len -= name_len;

				return make_recursive_node_by_name(reg_data, name);
			}
		} else {
			return NULL;
		}
	} else if(reg_data->back_ref_cnt >= BACK_REF_IDX_MAX) {
		return make_nfa_main(reg_data, data, len, 1);
	} else if(b_cond_flg) {
		return make_nfa_main(reg_data, data, len, 1);
	} else {
		start = make_type_node(reg_data, NODE_GROUP_START);
		end = make_type_node(reg_data, NODE_GROUP_END);
		if(start == NULL) return NULL;
		if(end == NULL) return NULL;
		start->data.back_ref.idx = reg_data->back_ref_cnt;
		end->data.back_ref.idx = reg_data->back_ref_cnt;
		reg_data->back_ref_nfa_start[reg_data->back_ref_cnt] = start;
		(reg_data->back_ref_cnt)++;
	}

	if(start == NULL) return NULL;
	if(end == NULL) return NULL;

	node = make_nfa_main(reg_data, data, len, b_enable_loop);
	if(node == NULL) return NULL;

	if(!b_enable_loop) { // lookbehind 
		if(check_behind_loop(node) != 0) return NULL;
		start->data.group.behind_cnt = check_behind_node(node);
		if(start->data.group.behind_cnt == -1) return NULL;
	}

	start->next[0] = node;
	node = get_last_node(node);
	node->next[0] = end;

	if(!is_epsilon_node(end)) {
		end->next[0] = alloc_nfa_node(reg_data);
		if(end->next[0] == NULL) return NULL;
	}

	return start;
}

static CHAR_CLASS *alloc_char_class(OREG_DATA *reg_data)
{
	CHAR_CLASS	*char_class = NULL;
	char_class = (CHAR_CLASS *)my_malloc(reg_data, sizeof(CHAR_CLASS));

	char_class->not = 0;
	char_class->all_mb_flg = 0;
	char_class->iw_flg = 0;
	char_class->pair_cnt = 0;
	char_class->pair = NULL;
	char_class->other = NULL;
	char_class->other_cnt = 0;

	memset(char_class->char_tbl, 0, sizeof(char_class->char_tbl));

	return char_class;
}

static int is_esc_char_class(unsigned int ch)
{
	if(ch == 'd' || ch == 'D' || ch == 'w' || ch == 'W' ||
		ch == 's' || ch == 'S' || ch == 'l') return 1;
	return 0;
}

static unsigned int convert_escape(unsigned int ch)
{
	switch(ch) {
	case 't': return '\t';
	case 'n': return '\n';
	case 'r': return '\r';
	case 'f': return '\f';
	}
	return ch;
}

static int get_max_pair_cnt(const TCHAR *p, INT_PTR p_len)
{
	int				cnt = 0;
	unsigned int	ch;

	for(; p_len > 0;) {
		ch = get_char(p);
		if(ch == '-') cnt++;

		p_len -= get_char_len(p);
		p += get_char_len(p);
	}

	return cnt;
}

static void make_char_class_ch_data_main(CHAR_CLASS *char_class,
	unsigned int ch)
{
	if(ch <= 0x00ff) {
		char_class->char_tbl[ch / 8] |= (0x01 << (ch % 8));
		return;
	}
	assert(char_class->other != NULL);
	char_class->other[char_class->other_cnt] = ch;
	char_class->other_cnt++;
}

static void make_char_class_ch_data(CHAR_CLASS *char_class, 
	unsigned int ch, int ic_flg, int iw_flg)
{
	if(ic_flg) {
		unsigned int ch2;
		ch = inline_tolower_mb(ch);
		make_char_class_ch_data_main(char_class, ch);
		ch2 = inline_toupper_mb(ch);
		if(ch != ch2) {
			make_char_class_ch_data_main(char_class, ch2);
		}
	} else {
		make_char_class_ch_data_main(char_class, ch);
	}
}

static void make_esc_char_class(OREG_DATA *oreg_data, CHAR_CLASS *char_class,
	unsigned int ch)
{
	const unsigned char *tbl = NULL;
	int		not = 0;
	int		i;

	switch(ch) {
	case 'D': not = 1; // fall
	case 'd': tbl = CHAR_CLASS_d_TBL;
		break;
	case 'W': not = 1; // fall
	case 'w': tbl = CHAR_CLASS_w_TBL;
		break;
	case 'S': not = 1; // fall
	case 's': tbl = CHAR_CLASS_s_TBL;
		break;
	case 'l': tbl = CHAR_CLASS_l_TBL;
		break;
	default:
		return;
	}

	if(not) {
		for(i = 0; i < ESC_CHAR_CLASS_TABLE_SIZE; i++) {
			char_class->char_tbl[i] |= ~tbl[i];
		}
		// 全角文字は全て一致する
		char_class->all_mb_flg = 1;

		// 否定のとき改行文字にマッチさせない
		{
			const char ch = '\n';
			char_class->char_tbl[ch / 8] &= ~(0x01 << (ch % 8));
		}
	} else {
		for(i = 0; i < ESC_CHAR_CLASS_TABLE_SIZE; i++) {
			char_class->char_tbl[i] |= tbl[i];
		}
		if((oreg_data->reg_switch & REG_SWITCH_S) && ch == 's') {
			char n = '\n';
			char_class->char_tbl[n / 8] |= (0x01 << (n % 8));
		}
	}
}

static void make_char_class_pair_to_tbl_main(CHAR_CLASS *char_class, 
	unsigned int start_ch, unsigned int end_ch)
{
	unsigned int ch;

	for(ch = start_ch; ch <= end_ch; ch++) {
		char_class->char_tbl[ch / 8] |= (0x01 << (ch % 8));
	}
}

static void make_char_class_pair_to_tbl(CHAR_CLASS *char_class, 
	unsigned int start_ch, unsigned int end_ch, int ic_flg, int iw_flg)
{
	if(ic_flg) {
		unsigned int u_s = inline_toupper_mb(start_ch);
		unsigned int u_e = inline_toupper_mb(end_ch);
		unsigned int l_s = inline_tolower_mb(start_ch);
		unsigned int l_e = inline_tolower_mb(end_ch);
		make_char_class_pair_to_tbl_main(char_class, u_s, u_e);
		make_char_class_pair_to_tbl_main(char_class, l_s, l_e);
	} else {
		make_char_class_pair_to_tbl_main(char_class, start_ch, end_ch);
	}
}

static int is_posix_char_class(const TCHAR *p)
{
	if(_tcsncmp(p, _T(":ascii:]"), 8) == 0) return 1;
	if(_tcsncmp(p, _T(":alpha:]"), 8) == 0) return 1;
	if(_tcsncmp(p, _T(":alnum:]"), 8) == 0) return 1;
	if(_tcsncmp(p, _T(":digit:]"), 8) == 0) return 1;
	if(_tcsncmp(p, _T(":upper:]"), 8) == 0) return 1;
	if(_tcsncmp(p, _T(":lower:]"), 8) == 0) return 1;
	return 0;
}

static void make_posix_char_class(CHAR_CLASS *char_class, const TCHAR *p,
	int ic_flg, int iw_flg)
{
	if(_tcsncmp(p, _T(":ascii:]"), 8) == 0) {
		make_char_class_pair_to_tbl(char_class, 0x00, 0x7f, 0, iw_flg);
	}
	if(_tcsncmp(p, _T(":alpha:]"), 8) == 0) {
		make_char_class_pair_to_tbl(char_class, 'a', 'z', 1, iw_flg);
	}
	if(_tcsncmp(p, _T(":alnum:]"), 8) == 0) {
		make_char_class_pair_to_tbl(char_class, 'a', 'z', 1, iw_flg);
		make_char_class_pair_to_tbl(char_class, '0', '9', 0, iw_flg);
	}
	if(_tcsncmp(p, _T(":digit:]"), 8) == 0) {
		make_char_class_pair_to_tbl(char_class, '0', '9', 0, iw_flg);
	}
	if(_tcsncmp(p, _T(":upper:]"), 8) == 0) {
		make_char_class_pair_to_tbl(char_class, 'A', 'Z', 0, iw_flg);
	}
	if(_tcsncmp(p, _T(":lower:]"), 8) == 0) {
		make_char_class_pair_to_tbl(char_class, 'a', 'z', 0, iw_flg);
	}
}

static void make_char_class_pair_main(CHAR_CLASS *char_class,
	unsigned int start_ch, unsigned int end_ch)
{
	char_class->pair[char_class->pair_cnt].start = start_ch;
	char_class->pair[char_class->pair_cnt].end = end_ch;
	(char_class->pair_cnt)++;
}

static void make_char_class_pair(CHAR_CLASS *char_class,
	unsigned int start_ch, unsigned int end_ch, int ic_flg, int iw_flg)
{
	if(ic_flg) {
		unsigned int u_s = inline_toupper_mb(start_ch);
		unsigned int u_e = inline_toupper_mb(end_ch);
		unsigned int l_s = inline_tolower_mb(start_ch);
		unsigned int l_e = inline_tolower_mb(end_ch);
		make_char_class_pair_main(char_class, u_s, u_e);
		make_char_class_pair_main(char_class, l_s, l_e);
	} else {
		make_char_class_pair_main(char_class, start_ch, end_ch);
	}
}

static int make_char_class_data(OREG_DATA *reg_data, CHAR_CLASS *char_class,
	const TCHAR *p, INT_PTR p_len, int ic_flg, int iw_flg)
{
	unsigned int	ch;
	unsigned int	prev_ch = '\0';

	for(; p_len > 0;) {
		ch = get_char(p);
		p_len -= get_char_len(p);
		p += get_char_len(p);

		if(ch == '\\') {
			if(p_len == 0) {
				make_char_class_ch_data(char_class, ch, ic_flg, iw_flg);
				break;
			}

			ch = get_char(p);
			p_len -= get_char_len(p);
			p += get_char_len(p);
			
			if(is_esc_char_class(ch)) {
				make_esc_char_class(reg_data, char_class, ch);
				prev_ch = '\0';
				continue;
			}
			ch = convert_escape(ch);
		}

		if(ch == '[' && is_posix_char_class(p)) {
			make_posix_char_class(char_class, p, ic_flg, iw_flg);
			for(; p_len > 0;) {
				ch = get_char(p);
				p_len -= get_char_len(p);
				p += get_char_len(p);
				if(ch == ']') break;
			}
			continue;
		}

		if(ch == '-') {
			if(prev_ch == '\0' || p_len == 0) {
				if(prev_ch != '\0') {
					make_char_class_ch_data(char_class, prev_ch, ic_flg, iw_flg);
				}
				make_char_class_ch_data(char_class, ch, ic_flg, iw_flg);
				if(p_len == 0) break;
			} else {
				// 次の文字を取得
				ch = get_char(p);
				p_len -= get_char_len(p);
				p += get_char_len(p);

				if(iw_flg) {
					prev_ch = get_ascii_char(prev_ch);
					ch = get_ascii_char(ch);
				}

				if(prev_ch > ch) {
					// invalid [] range
					return 1;
				}

				if(prev_ch <= 0xff && ch <= 0xff) {
					make_char_class_pair_to_tbl(char_class, prev_ch, ch, ic_flg, iw_flg);
				} else {
					make_char_class_pair(char_class, prev_ch, ch, ic_flg, iw_flg);
				}

				ch = '\0';
			}
		} else if(get_char(p) != '-') {
			make_char_class_ch_data(char_class, ch, ic_flg, iw_flg);
		}

		prev_ch = ch;
	}

	return 0;
}

static INT_PTR get_char_class_other_len(const TCHAR *p, INT_PTR p_len)
{
	int		other_len = 0;

	for(; p_len > 0;) {
		if(get_char(p) > 0xff) other_len++;
		p_len -= get_char_len(p);
		p += get_char_len(p);
	}

	return other_len;
}

static NFA_NODE *make_char_class(OREG_DATA *reg_data,
	const TCHAR *data, INT_PTR len)
{
	NFA_NODE	*node = NULL;
	CHAR_CLASS	*char_class;
	int			max_pair_cnt = 0;
	INT_PTR		other_len = 0;
	int			ic_flg = (reg_data->reg_switch & REG_SWITCH_ic) ? 1:0;
	int			iw_flg = (reg_data->reg_switch & REG_SWITCH_iw) ? 1:0;

	if(len <= 0 || (len == 1 && get_char(data) == '^')) {
		return alloc_nfa_node(reg_data);
	}

	node = make_type_node(reg_data, NODE_CHAR_CLASS);
	if(node == NULL) return NULL;

	char_class = alloc_char_class(reg_data);
	if(char_class == NULL) return NULL;

	char_class->iw_flg = iw_flg;

	if(get_char(data) == '^') {
		char_class->not = 1;
		data++;
		len--;
	}

	max_pair_cnt = get_max_pair_cnt(data, len);
	if(max_pair_cnt > 0) {
		if(ic_flg) max_pair_cnt *= 2;
		char_class->pair = (CHAR_CLASS_PAIR *)my_malloc(reg_data,
			sizeof(CHAR_CLASS_PAIR) * max_pair_cnt);
		if(char_class->pair == NULL) return NULL;
	}

	other_len = get_char_class_other_len(data, len);
	if(other_len > 0) {
		if(ic_flg) other_len *= 2;
		char_class->other = (unsigned int *)my_malloc(reg_data,
			sizeof(unsigned int) * other_len);
		if(char_class->other == NULL) return NULL;
	}

	if(make_char_class_data(reg_data, char_class, data, len, ic_flg, iw_flg) != 0) {
		return NULL;
	}

	// 否定の検索のとき、改行にマッチさせない
	if(char_class->not) {
		const char ch = '\n';
		char_class->char_tbl[ch / 8] |= (0x01 << (ch % 8));
	}

	node->data.char_class = char_class;

	return node;
}

const TCHAR *get_back_ref_idx(const TCHAR *p, int *val, int back_ref_cnt)
{
	int ret_v = 0;
	const TCHAR *org_p = p;
	unsigned int ch;

	int		bracket_flg = 0;
	int		minus_flg = 0;

	if(get_char(p) == 'g') {
		p += get_char_len(p);
		if(get_char(p) == '{') {
			bracket_flg = 1;
			p += get_char_len(p);
		}
		if(get_char(p) == '-') {
			minus_flg = 1;
			p += get_char_len(p);
		}
	}

	for(; *p != '\0'; p += get_char_len(p)) {
		ch = get_char(p);
		if(*p < '0' || *p > '9') break;
		ret_v *= 10;
		ret_v += *p - '0';
	}

	if(bracket_flg) {
		assert(get_char(p) == '}');
		p += get_char_len(p);
	}

	if(!minus_flg) {
		*val = ret_v - 1;
	} else {
		*val = back_ref_cnt - ret_v;
	}

	if(!bracket_flg && *val >= back_ref_cnt) {
		// 2桁以上の数値を読み込んだとき、後方参照の数を超える場合は、
		// 先頭1桁の指定として処理する
		p = org_p;
		*val = *p - '0' - 1;
		p += get_char_len(p);
	}

	return p;
}

static int get_hex_num(const TCHAR *p, const TCHAR *p_end)
{
	int		ret_v = 0;

	for(; p < p_end; p++) {
		ret_v *= 16;

		if(*p >= '0' && *p <= '9') {
			ret_v += *p - '0';
		} else if(*p >= 'a' && *p <= 'f') {
			ret_v += *p - 'a' + 10;
		} else if(*p >= 'A' && *p <= 'F') {
			ret_v += *p - 'A' + 10;
		} else {
			return -1;
		}
	}

	return ret_v;
}

static NFA_NODE *make_word_nodes_from_escape_data(OREG_DATA *reg_data,
	LexWord *lex_word)
{
	const TCHAR *p = skip_chars(lex_word->data, 2);	// skip '\\Q'
	const TCHAR *p2 = _tcsstr(p, _T("\\E"));
	INT_PTR		word_len;
	NFA_NODE	*start_node = NULL;
	NFA_NODE	*cur_node = NULL;

	start_node = alloc_nfa_node(reg_data);
	if(start_node == NULL) return NULL;

	cur_node = start_node;

	if(p2 != NULL) {
		word_len = lex_word->len - 4;	// -4 for '\Q' and '\E'
	} else {
		word_len = lex_word->len - 2;	// -2 for '\Q'
	}

	for(; word_len; ) {
		cur_node->next[0] = make_word_node(reg_data, get_char(p));
		cur_node = cur_node->next[0];

		word_len -= get_char_len(p);
		p += get_char_len(p);
	}

	return start_node;
}

static NFA_NODE *make_escape_node(OREG_DATA *reg_data, LexWord *lex_word)
{
	const TCHAR *p = skip_chars(lex_word->data, 1);	// slop '\\'
	unsigned int ch = get_char(p);

	switch(ch) {
	case '\0': return NULL;

	case 't': return make_word_node(reg_data, '\t');
	case 'n': return make_word_node(reg_data, '\n');
	case 'r': return make_word_node(reg_data, '\r');
	case 'f': return make_word_node(reg_data, '\f');

	case 'b': return make_type_node(reg_data, NODE_WORD_SEPALATER);
	case 'B': return make_type_node(reg_data, NODE_WORD_NOT_SEPALATER);

	case 'A': return make_type_node(reg_data, NODE_FIRST_MATCH_A);
	case 'Z': return make_type_node(reg_data, NODE_LAST_MATCH_Z);
	case 'z': return make_type_node(reg_data, NODE_LAST_MATCH_z);

	case 'd': return make_char_class(reg_data, _T("\\d"), 2);
	case 'D': return make_char_class(reg_data, _T("\\D"), 2);
	case 'w': return make_char_class(reg_data, _T("\\w"), 2);
	case 'W': return make_char_class(reg_data, _T("\\W"), 2);
	case 's': return make_char_class(reg_data, _T("\\s"), 2);
	case 'S': return make_char_class(reg_data, _T("\\S"), 2);
	case 'l': return make_char_class(reg_data, _T("\\l"), 2);

	case 'K': return make_type_node(reg_data, NODE_KEEP_PATTERN);

	case 'Q': return make_word_nodes_from_escape_data(reg_data, lex_word);
	}

	if(ch == '=') {
		p += get_char_len(p);
		ch = get_char(p);
		switch(ch) {
			case 'A': return make_type_node(reg_data, NODE_SELECTED_AREA_FIRST);
			case 'z': return make_type_node(reg_data, NODE_SELECTED_AREA_LAST);
			case '^': return make_type_node(reg_data, NODE_SELECTED_AREA_ROW_FIRST);
			case '$': return make_type_node(reg_data, NODE_SELECTED_AREA_ROW_LAST);
		}
	} else if(ch == 'k') {
		p += get_char_len(p);
		ch = get_char(p);
		if(ch == '<' || ch == '\'') {
			NFA_NODE *node;
			INT_PTR copy_len = lex_word->len - 4;	// \\k<>の4文字以外をコピー

			node = make_type_node(reg_data, NODE_NAMED_BACK_REF);
			if(node == NULL) return NULL;

			node->data.named_back_ref.name =
				(TCHAR *)my_malloc(reg_data, lex_word->len);
			_tcsncpy(node->data.named_back_ref.name, lex_word->data + 3,
				copy_len);
			node->data.named_back_ref.name[copy_len] = '\0';
			node->data.named_back_ref.ic_flg = (reg_data->reg_switch & REG_SWITCH_ic) ? 1:0;
			node->data.named_back_ref.iw_flg = (reg_data->reg_switch & REG_SWITCH_iw) ? 1:0;

			return node;
		}
	} else if((ch >= '1' && ch <= '9') || ch == 'g') {
		int		idx;
		NFA_NODE *node;
		const TCHAR *p2;
		const TCHAR *p_end = lex_word->data + lex_word->len;

		p2 = get_back_ref_idx(p, &idx, reg_data->back_ref_cnt);
		if(idx < 0 || idx >= BACK_REF_IDX_MAX) return NULL;
		
		node = make_type_node(reg_data, NODE_BACK_REF);
		if(node == NULL) return NULL;

		node->data.back_ref.idx = idx;
		node->data.back_ref.ic_flg = (reg_data->reg_switch & REG_SWITCH_ic) ? 1:0;
		node->data.back_ref.iw_flg = (reg_data->reg_switch & REG_SWITCH_iw) ? 1:0;

		if(p2 != p_end) {
			NFA_NODE *last_node = node;
			for(; p2 != p_end; p2 += get_char_len(p2)) {
				NFA_NODE *word_node = make_word_node(reg_data, get_char(p2));
				if(word_node == NULL) return NULL;

				last_node = add_nfa_node(last_node, word_node);
			}
		}

		return node;
	} else if(ch == 'x') {
		p += get_char_len(p);
		ch = get_char(p);

		if(ch == '{') {
			// +3 for '\x{'
			p = skip_chars(lex_word->data, 3);
			// -4 for '\x{' and '}'
			ch = get_hex_num(p, skip_chars(p, lex_word->len - 4));
		} else {
			// +2 for '\x'
			p = skip_chars(lex_word->data, 2);
			ch = get_hex_num(p, skip_chars(p, 2));
		}
		return make_word_node(reg_data, ch);
	}

	return make_word_node(reg_data, ch);
}

static NFA_NODE *make_next_node(OREG_DATA *reg_data, LexWord *lex_word)
{
	NFA_NODE	*next_node = NULL;

	switch(lex_word->type) {
	case LEX_GROUP:
		next_node = make_group(reg_data, lex_word, 0);
		break;
	case LEX_WORD:
		next_node = make_word_node(reg_data, get_char(lex_word->data));
		break;
	case LEX_ESCAPE_CHAR:
		next_node = make_escape_node(reg_data, lex_word);
		break;
	case LEX_CHAR_CLASS:
		next_node = make_char_class(reg_data, lex_word->data + 1,
			lex_word->len - 2);
		break;
	case LEX_ANY_CHAR:
		if(reg_data->reg_switch & REG_SWITCH_s) {
			next_node = make_type_node(reg_data, NODE_ANY_CHAR_LF);
		} else {
			next_node = make_type_node(reg_data, NODE_ANY_CHAR);
		}
		break;
	case LEX_FIRST_MATCH:
		next_node = make_type_node(reg_data, NODE_FIRST_MATCH);
		break;
	case LEX_LAST_MATCH:
		next_node = make_type_node(reg_data, NODE_LAST_MATCH);
		break;
	}

	return next_node;
}

static int get_quantifier_num(const TCHAR *p, const TCHAR *p_end)
{
	int ret_v = 0;

	for(; p < p_end; p++) {
		if(*p == ' ') continue;
		if(*p < '0' || *p > '9') return -1;
		ret_v *= 10;
		ret_v += *p - '0';
	}

	return ret_v;
}

static int parse_quantifier_node(const TCHAR *p, int *min, int *max)
{
	const TCHAR *p1;

	p++;
	p1 = p;

	for(; *p1 != '\0'; p1++) {
		if(*p1 == '}') {
			*min = get_quantifier_num(p, p1);
			if(*min < 0) return 1;
			*max = *min;
			return 0;
		} else if(*p1 == ',') {
			*min = get_quantifier_num(p, p1);
			if(*min < 0) return 1;
			p1++;
			break;
		}
	}

	if(*p1 == '}') {
		*max = -1;
		return 0;
	}

	p = p1;
	for(; *p1 != '\0'; p1++) {
		if(*p1 == '}') {
			*max = get_quantifier_num(p, p1);
			if(*max < 0) return 1;
			break;
		}
	}

	return 0;
}

static int check_loop_detect(NFA_NODE *node, int depth)
{
	for(; node != NULL;) {
		switch(node->type) {
		case NODE_RECURSIVE:
			// recursive patternの場合、常に番兵を入れる
			// FIXME: 番兵が機能しているか確認する
			return 1;
		case NODE_WORD:
		case NODE_ANY_CHAR:
		case NODE_CHAR_CLASS:
			return 0;
		}
		if(node->next[1] != NULL) {
			if(check_loop_detect(node->next[1], depth + 1) != 0) return 1;
		}
		node = node->next[0];
	}

	return 1;
}

static NFA_NODE *make_quantifier_multi(OREG_DATA *reg_data, NFA_NODE *node,
	unsigned int ctrl_ch, int b_short)
{
	NFA_NODE *start_node;

	start_node = make_branch_node(reg_data, ctrl_ch);
	if(start_node == NULL) return NULL;

	if(check_loop_detect(node, 0)) {
		NFA_NODE *loop_detect_node =
			make_type_node(reg_data, NODE_LOOP_DETECT);
		if(loop_detect_node == NULL) return NULL;

		loop_detect_node->next[0] = node;
		start_node->next[1] = loop_detect_node;
	} else {
		start_node->next[1] = node;
	}
	if(!b_short) start_node->first_test = 1;
	
	get_last_node(node)->next[0] = start_node;

	return start_node;
}

static NFA_NODE *make_quantifier_plus(OREG_DATA *reg_data, NFA_NODE *node,
	unsigned int ctrl_ch, int b_short)
{
	NFA_NODE *start_node;
	NFA_NODE *end_node;

	end_node = make_branch_node(reg_data, ctrl_ch);
	if(end_node == NULL) return NULL;

	start_node = alloc_nfa_node(reg_data);
	if(start_node == NULL) return NULL;
	start_node->next[0] = node;

	if(check_loop_detect(node, 0)) {
		// 番兵を入れる
		NFA_NODE *loop_detect_node =
			make_type_node(reg_data, NODE_LOOP_DETECT);
		if(loop_detect_node == NULL) return NULL;

		end_node->next[1] = loop_detect_node;
		end_node->next[1]->next[0] = start_node->next[0];
	} else {
		end_node->next[1] = start_node->next[0];
	}
	if(!b_short) end_node->first_test = 1;

	get_last_node(node)->next[0] = end_node;

	return start_node;
}

static NFA_NODE *make_quantifier_q(OREG_DATA *reg_data, NFA_NODE *node,
	unsigned int ctrl_ch, int b_short)
{
	NFA_NODE *start_node;
	NFA_NODE *end_node;

	end_node = alloc_nfa_node(reg_data);
	if(end_node == NULL) return NULL;

	start_node = make_branch_node(reg_data, ctrl_ch);
	if(start_node == NULL) return NULL;
	start_node->next[0] = node;
	start_node->next[1] = end_node;
	if(b_short) start_node->first_test = 1;

	get_last_node(node)->next[0] = end_node;

	return start_node;
}

static NFA_NODE *make_quantifier(OREG_DATA *reg_data, NFA_NODE *node,
	unsigned int ctrl_ch, int b_short)
{
	assert(ctrl_ch == '*' || ctrl_ch == '+' || ctrl_ch == '?');

	if(ctrl_ch == '*') {
		return make_quantifier_multi(reg_data, node, ctrl_ch, b_short);
	}
	if(ctrl_ch == '+') {
		return make_quantifier_plus(reg_data, node, ctrl_ch, b_short);
	}

	return make_quantifier_q(reg_data, node, ctrl_ch, b_short);
}

static NFA_NODE *make_quantifier_main(OREG_DATA *reg_data, NFA_NODE *node,
	LexWord *lex_word_q, LexWord *lex_word, int back_ref_cnt,
	int b_enable_loop)
{
	int		min, max;
	int		i;
	int		b_short = 0;
	int		b_no_backtrack = 0;
	NFA_NODE *next_node;
	NFA_NODE *head_node;

	switch(get_char(lex_word_q->data)) {
	case '{':
		if(parse_quantifier_node(lex_word_q->data, &min, &max) != 0) {
			return NULL;
		}
		break;
	case '*':
		min = 0;
		max = -1;
		break;
	case '+':
		min = 1;
		max = -1;
		break;
	case '?':
		min = 0;
		max = 1;
		break;
	}

	if(lex_word_q->len != 1) {
		if(lex_word_q->data[lex_word_q->len - 1] == '?') b_short = 1;
		if(lex_word_q->data[lex_word_q->len - 1] == '+') b_no_backtrack = 1;
	}

	if(!b_enable_loop && min != max) return NULL;

	if(b_no_backtrack) {
		NFA_NODE *start_node;
		NFA_NODE *end_node;

		start_node = alloc_nfa_node(reg_data);
		if(start_node == NULL) return NULL;
		end_node = alloc_nfa_node(reg_data);
		if(end_node == NULL) return NULL;

		start_node->type = NODE_POSSESSIVE_CAPUTURE;
		start_node->data.possessive_capture.min = min;
		start_node->data.possessive_capture.max = max;

		start_node->next[0] = end_node;
		start_node->next[1] = node;

		return start_node;
	}

	if(min == 0 && max == -1) {
		head_node = make_quantifier(reg_data, node, '*', b_short);
	} else if(min == 0 && max == 1) {
		head_node = make_quantifier(reg_data, node, '?', b_short);
	} else if(min == 1 && max == -1) {
		head_node = make_quantifier(reg_data, node, '+', b_short);
	} else {
		i = 1;
		head_node = node;
		node = get_last_node(node);

		if(min > 0) {
			for(; i < min; i++) {
				reg_data->back_ref_cnt = back_ref_cnt;
				next_node = make_next_node(reg_data, lex_word);
				if(next_node == NULL) return NULL;
				node = add_nfa_node(node, next_node);
			}
		}

		if(min == max) {
		} else if(max == -1) {
			reg_data->back_ref_cnt = back_ref_cnt;
			next_node = make_next_node(reg_data, lex_word);
			if(next_node == NULL) return NULL;
			next_node = make_quantifier(reg_data, next_node, '*', b_short);
			if(next_node == NULL) return NULL;
			node = add_nfa_node(node, next_node);
		} else {
			NFA_NODE *end_node = alloc_nfa_node(reg_data);
			if(end_node == NULL) return NULL;

			for(; i < max; i++) {
				reg_data->back_ref_cnt = back_ref_cnt;
				next_node = make_next_node(reg_data, lex_word);
				if(next_node == NULL) return NULL;
				next_node = make_quantifier(reg_data, next_node, '?', b_short);
				if(next_node == NULL) return NULL;

				/* backtrack削減の最適化 (.{2,22}) -> (?>.{2,22}) */
				next_node->type = NODE_BRANCH;
				next_node->next[1] = end_node;

				node = add_nfa_node(node, next_node);
			}
			node = add_nfa_node(node, end_node);
		}

		if(min == 0) {
			// 条件全体のskipを許可する
			head_node = make_quantifier(reg_data, head_node, '?', b_short);
		}
	}

	return head_node;
}

static NFA_NODE *make_branch(OREG_DATA *reg_data, const TCHAR *p, INT_PTR p_len, int b_enable_loop)
{
	NFA_NODE	*head_node = NULL;
	NFA_NODE	*node = NULL;
	NFA_NODE	*next_node = NULL;
	LexWord		lex_word;
	LexWord		lex_word_q;
	int			back_ref_cnt;

	// 空遷移を作る
	node = alloc_nfa_node(reg_data);
	if(node == NULL) return NULL;

	head_node = node;

	for(;;) {
		if(p_len <= 0) break;
		if(*p == '\0') break;

		if(skip_comment(&p, &p_len) != 0) return NULL;
		if(lexer(p, p_len, &lex_word) != 0) return NULL;
		p += lex_word.len;
		p_len -= lex_word.len;

		back_ref_cnt = reg_data->back_ref_cnt;
		next_node = make_next_node(reg_data, &lex_word);
		if(next_node == NULL) return NULL;

		if(skip_comment(&p, &p_len) != 0) return NULL;
		if(p_len > 0 && is_quantifier(get_char(p))) {
			if(lexer(p, p_len, &lex_word_q) != 0) return NULL;
			assert(lex_word_q.type == LEX_QUANTIFIER);
			p += lex_word_q.len;
			p_len -= lex_word_q.len;

			// 量指定子はここで処理する
			next_node = make_quantifier_main(reg_data, next_node,
				&lex_word_q, &lex_word, back_ref_cnt, b_enable_loop);
			if(next_node == NULL) return NULL;
		}

		node = add_nfa_node(node, next_node);
	}

	if(node->next[0] == NULL) {
		node->next[0] = alloc_nfa_node(reg_data);
	}

	return head_node;
}

static NFA_NODE *make_or_nfa(OREG_DATA *reg_data, NFA_NODE *node,
	NFA_NODE *prev_node)
{
	NFA_NODE *start = NULL;
	NFA_NODE *end = NULL;

	// NFAの先頭を作る
	start = make_branch_node(reg_data, '|');
	if(start == NULL) return NULL;

	start->next[0] = prev_node;
	start->next[1] = node;

	// NFAの末尾を作る
	end = alloc_nfa_node(reg_data);
	if(end == NULL) return NULL;

	node = get_last_node(node);
	node->next[0] = end;
	prev_node = get_last_node(prev_node);
	prev_node->next[0] = end;

	return start;
}

static NFA_NODE *make_nfa_main(OREG_DATA *reg_data, const TCHAR *p, INT_PTR p_len, int b_enable_loop)
{
	NFA_NODE	*node = NULL;
	NFA_NODE	*prev_node = NULL;
	const TCHAR	*p_start;
	LexWord	lex_word;
	int			back_reg_switch = reg_data->reg_switch;

	p_start = p;
	for(;;) {
		if(p_len <= 0) break;
		if(*p == '\0') break;

		if(skip_comment(&p, &p_len) != 0) return NULL;
		if(lexer(p, p_len, &lex_word) != 0) return NULL;
		p += lex_word.len;
		p_len -= lex_word.len;

		if(lex_word.type == LEX_OR_CHAR) {
			node = make_branch(reg_data, p_start, p - p_start - 1,
				b_enable_loop);
			if(node == NULL) return NULL;
			p_start = p;
			if(prev_node != NULL) {
				node = make_or_nfa(reg_data, node, prev_node);
			}
			prev_node = node;
		}
	}
	node = make_branch(reg_data, p_start, p - p_start, b_enable_loop);
	if(node == NULL) return NULL;
	if(prev_node != NULL) {
		node = make_or_nfa(reg_data, node, prev_node);
	}

	reg_data->reg_switch = back_reg_switch;
	return node;
}

static NFA_NODE *skip_epsilon_node(NFA_NODE *node)
{
	for(; node != NULL; node = node->next[0]) {
		if(!is_epsilon_node(node) ||
			node->next[0] == NULL || node->next[1] != NULL) break;
	}
	return node;
}

static void make_words_node(OREG_DATA *reg_data, NFA_NODE *node)
{
	NFA_NODE	*start_node;
	NFA_NODE	*words_node;
	int			i;
	int			word_len;
	int			ic_flg;
	int			iw_flg;

	if(node->type != NODE_WORD || node->data.ch.c == '\n') return;

	start_node = node;
	word_len = 0;
	ic_flg = node->data.ch.ic_flg;
	iw_flg = node->data.ch.iw_flg;

	for(node = start_node; node != NULL;) {
		if(node->type != NODE_WORD || node->data.ch.c == '\n') break;
		if(ic_flg != node->data.ch.ic_flg) break;
		if(iw_flg != node->data.ch.iw_flg) break;
		word_len++;
		node->next[0] = skip_epsilon_node(node->next[0]);
		node = node->next[0];
	}
	if(word_len < 2) return;

	words_node = make_type_node(reg_data, NODE_WORDS);
	if(words_node == NULL) return;
	words_node->data.word.len = word_len;
	words_node->data.word.ic_flg = ic_flg;
	words_node->data.word.iw_flg = iw_flg;
	words_node->next[0] = node;

	words_node->data.word.buf = (unsigned int *)my_malloc(reg_data,
		sizeof(unsigned int) * word_len);
	if(words_node->data.word.buf == NULL) return;

	for(node = start_node, i = 0; i < word_len; i++) {
		words_node->data.word.buf[i] = node->data.ch.c;
		node = node->next[0];
	}

	*start_node = *words_node;
}

static int is_null_node(NFA_NODE *node)
{
	if(node != NULL && is_epsilon_node(node) &&
		node->next[0] == NULL && node->next[1] == NULL) return 1;
	return 0;
}

static NFA_NODE *optimize_skip_epsilon(OREG_DATA *reg_data, NFA_NODE *node)
{
	NFA_NODE *start_node = node = skip_epsilon_node(node);

	for(; node != NULL; node = node->next[0]) {
		if(node->optimized & OPTIMIZE_SKIP_EPSILON) break;
		node->optimized |= OPTIMIZE_SKIP_EPSILON;

		node->next[0] = skip_epsilon_node(node->next[0]);

		// 末尾のepsironを除去する(除去すると受理ノードがひとつにならない)
		//if(is_null_node(node->next[0])) node->next[0] = NULL;

		if(node->next[1] != NULL) {
			node->next[1] = optimize_skip_epsilon(reg_data, node->next[1]);
		}
	}

	return start_node;
}

/*
 ab|ac => a(?:b|c)へ変換
 (?:a?){3}a{3} => a{3}(?:a?){3}に変換

 http://swtch.com/~rsc/regexp/regexp1.html
*/
static void optimize_merge_selective(OREG_DATA *reg_data, NFA_NODE *node)
{
	for(; node != NULL; node = node->next[0]) {
		if(node->optimized & OPTIMIZE_MERGE_SELECTIVE) break;
		node->optimized |= OPTIMIZE_MERGE_SELECTIVE;

		// 分岐ノード以外はスキップ
		if(node->next[1] == NULL || node->next[0] == NULL) continue;

		// 先に奥のnodeを解決する
		optimize_merge_selective(reg_data, node->next[0]);

		if(node->type == NODE_BRANCH &&
		   node->next[1]->type == NODE_WORD &&
		   node->next[0]->type == NODE_WORD &&
		   node->next[0]->data.ch.c == node->next[1]->data.ch.c &&
		   node->next[0]->data.ch.ic_flg == node->next[1]->data.ch.ic_flg &&
		   node->next[0]->data.ch.iw_flg == node->next[1]->data.ch.iw_flg) {
			/*
			 * 正規表現ab|acのとき、以下の変換を行う
			 *
			 * 変換前
			 * (1:branch) -> (2:a) -> (4:b)
			 *            -> (3:a) -> (5:c)
			 *
			 * 変換後
			 * (1:a) -> (New:branch) -> (4:b)
			 *                       -> (5:c)
			 *
			 * - node1を(a)に書き換える
			 * - node1の次に新しい分岐ノードを挿入し、node2,3の次のnodeを指す
			*/
			unsigned int ch = node->next[0]->data.ch.c;
			int ic_flg = node->next[0]->data.ch.ic_flg;
			int iw_flg = node->next[0]->data.ch.iw_flg;
			NFA_NODE *new_branch_node = make_branch_node(reg_data, '|');
			new_branch_node->next[0] = node->next[0]->next[0];
			new_branch_node->next[1] = node->next[1]->next[0];
			new_branch_node->first_test = node->first_test;

			clear_nfa_node(node);
			node->type = NODE_WORD;
			node->data.ch.c = ch;
			node->data.ch.ic_flg = ic_flg;
			node->data.ch.iw_flg = iw_flg;
			node->next[0] = new_branch_node;
		}

		optimize_merge_selective(reg_data, node->next[1]);
	}
}

static int check_char_tbl(const unsigned char *char_tbl, unsigned int ch)
{
	if(char_tbl[ch / 8] & (0x01 << (ch % 8))) return 1;
	return 0;
}

// char_classがchar_tblのチェックのみでテスト可能か
static int check_char_class_is_simple(CHAR_CLASS *char_class)
{
	if(char_class->not) return 0;
	if(char_class->all_mb_flg) return 0;
	if(char_class->iw_flg) return 0;
	if(char_class->pair_cnt > 0) return 0;
	if(char_class->other_cnt > 0) return 0;
    return 1;
}

static int check_skip_for_candidate_branch_node(NFA_NODE *node)
{
	switch (node->type) {
	case NODE_EPSILON:
	case NODE_QUANTIFIER_START:
	case NODE_QUANTIFIER_END:
	case NODE_GROUP_START:
	case NODE_GROUP_END:
	case NODE_NO_BACKTRACK_START:
		return 1;
	}
	return 0;
}

static int check_candidate_branch_node(NFA_NODE *node)
{
	if(node->type == NODE_WORD) {
		if(node->data.ch.c >= 0x80) return 0;
		return 1;
	}
	if(node->type == NODE_CHAR_CLASS) {
		if(!check_char_class_is_simple(node->data.char_class)) return 0;
		return 1;
	}
	return 0;
}

struct candidate_branch_st {
	NFA_NODE	*node;
	int			tag;
};

static int candidate_branch_node(NFA_NODE *node, char *node_check_buf,
	struct candidate_branch_st *candidate, int candidate_cnt, int tag)
{
	for(; node != NULL; node = node->next[0]) {
		INT_PTR n = node->node_id;
		if(node_check_buf[n / 8] & (0x01 << (n % 8))) break;
		node_check_buf[n / 8] |= (0x01 << (n % 8));

		if(check_skip_for_candidate_branch_node(node)) continue;

		if(node->next[1] != NULL) {
			candidate_cnt = candidate_branch_node(node->next[1],
				node_check_buf, candidate, candidate_cnt, tag);
			assert(candidate_cnt <= MAX_BRANCH_CANDIDATE);
			if(candidate_cnt < 0) return -1;
			if(candidate_cnt >= MAX_BRANCH_CANDIDATE) return -1;
			continue;
		}

		if(check_candidate_branch_node(node)) {
			if(candidate_cnt >= MAX_BRANCH_CANDIDATE) return -1;
			candidate[candidate_cnt].node = node;
			candidate[candidate_cnt].tag = tag;
			candidate_cnt++;
			break;
		} else {
			return -1;
		}
	}

	// 終端に到達することができる場合、全てのケースを受理する
	if(node == NULL) return -1;
	return candidate_cnt;
}

static int _set_branch_tbl_assert(unsigned char *branch_tbl, unsigned int ch,
	int tag)
{
	char flg = check_branch_tbl(branch_tbl, ch);
	assert(flg <= 0x03);
	assert((tag == 0 && (flg & 0x01)) || (tag == 1 && (flg & 0x02)));
	return 1;
}

static void set_branch_tbl(unsigned char *branch_tbl, unsigned int ch, int tag)
{
	assert(ch < 0x80);
	branch_tbl[ ch / 4 ] |= (0x01 << (((ch % 4) * 2) + tag));

	assert(_set_branch_tbl_assert(branch_tbl, ch, tag));
}

static void merge_branch_tbl_ch(unsigned char *branch_tbl, NFA_NODE *node,
	int tag)
{
	unsigned int ch = node->data.ch.c;

	if(node->data.ch.ic_flg) {
		set_branch_tbl(branch_tbl, inline_tolower_mb(ch), tag);
		set_branch_tbl(branch_tbl, inline_toupper_mb(ch), tag);
	} else {
		set_branch_tbl(branch_tbl, ch, tag);
	}
}

static void merge_branch_tbl(unsigned char *branch_tbl, NFA_NODE *node,
	int tag)
{
	const unsigned char *char_tbl = node->data.char_class->char_tbl;
	unsigned int ch;

	assert(node->data.char_class->not == 0);

	for(ch = 0; ch < 0x80; ch++) {
		if(check_char_tbl(char_tbl, ch)) {
			set_branch_tbl(branch_tbl, ch, tag);
		}
	}
}

static void optimize_branch_node(OREG_DATA *reg_data, NFA_NODE *node,
	unsigned char *node_check_buf)
{
	struct candidate_branch_st candidate[MAX_BRANCH_CANDIDATE];
	unsigned char *branch_tbl;
	int		candidate_cnt = 0;
	int		i;

	// 分岐先を取得
	memset(node_check_buf, 0, (reg_data->cur_node_id / 8) + 1);
	candidate_cnt = candidate_branch_node(node->next[0], node_check_buf,
		candidate, candidate_cnt, 0);
	assert(candidate_cnt <= MAX_BRANCH_CANDIDATE);
	if(candidate_cnt == 0 || candidate_cnt == -1) return;

	memset(node_check_buf, 0, (reg_data->cur_node_id / 8) + 1);
	candidate_cnt = candidate_branch_node(node->next[1], node_check_buf,
		candidate, candidate_cnt, 1);
	assert(candidate_cnt <= MAX_BRANCH_CANDIDATE);
	if(candidate_cnt == 0 || candidate_cnt == -1) return;

	// 候補をマージする
	branch_tbl = (unsigned char *)my_malloc(reg_data, BRANCH_TABLE_SIZE);
	if(branch_tbl == NULL) return;
	memset(branch_tbl, 0, BRANCH_TABLE_SIZE);

	for(i = 0; i < candidate_cnt; i++) {
		NFA_NODE *c_node = candidate[i].node;
		if(c_node->type == NODE_WORD) {
			merge_branch_tbl_ch(branch_tbl, c_node, candidate[i].tag);
		} else if(c_node->type == NODE_CHAR_CLASS) {
			merge_branch_tbl(branch_tbl, c_node, candidate[i].tag);
		}
	}

	node->data.branch.branch_tbl = branch_tbl;
}

static int optimize_branch(OREG_DATA *reg_data, NFA_NODE *node,
	int branch_cnt, unsigned char *node_check_buf)
{
	for(; node != NULL; node = node->next[0]) {
		if(node->optimized & OPTIMIZE_BRANCH) break;
		node->optimized |= OPTIMIZE_BRANCH;

		if(node->type == NODE_BRANCH) {
			optimize_branch_node(reg_data, node, node_check_buf);
			branch_cnt++;
			if(branch_cnt >= MAX_OPTIMIZE_BRANCH) break;
		}
		if(node->next[1] != NULL) {
			branch_cnt = optimize_branch(reg_data, node, branch_cnt,
				node_check_buf);
			if(branch_cnt >= MAX_OPTIMIZE_BRANCH) break;
		}
	}
	return branch_cnt;
}

static void optimize_make_words(OREG_DATA *reg_data, NFA_NODE *node)
{
	for(; node != NULL; node = node->next[0]) {
		if(node->optimized & OPTIMIZE_MAKE_WORDS_NODE) break;
		node->optimized |= OPTIMIZE_MAKE_WORDS_NODE;

		if(node->type == NODE_WORD) make_words_node(reg_data, node);

		if(node->next[1] != NULL) {
			optimize_make_words(reg_data, node->next[1]);
		}
	}
}

static unsigned int get_first_check_char(OREG_DATA *reg_data, NFA_NODE *node)
{
	switch(node->type) {
	case NODE_WORD:
		reg_data->exec_data.first_check_char.c = node->data.ch.c;
		reg_data->exec_data.first_check_char.ic_flg = node->data.ch.ic_flg;
		reg_data->exec_data.first_check_char.iw_flg = node->data.ch.iw_flg;
		return 1;
	case NODE_WORDS:
		reg_data->exec_data.first_check_char.c = node->data.word.buf[0];
		reg_data->exec_data.first_check_char.ic_flg = node->data.word.ic_flg;
		reg_data->exec_data.first_check_char.iw_flg = node->data.word.iw_flg;
		return 1;
	}

	return 0;
}

static NFA_NODE *get_first_check_node(NFA_NODE *node)
{
	for(; node->next[0] != NULL && node->next[1] == NULL;
			node = node->next[0]) {
		switch(node->type) {
		case NODE_EPSILON:
		case NODE_GROUP_START:
		case NODE_NO_BACKTRACK_START:
		case NODE_LOOK_AHEAD_START:
		case NODE_NOT_LOOK_AHEAD_START:
		case NODE_QUANTIFIER_START:
			continue;
		}
		break;
	}

	return node;
}

static int can_boyer_moore(NFA_NODE *node, const TCHAR *pattern,
	TCHAR *pattern_buf, int buf_size)
{
	int		pattern_len = 0;
	unsigned int *words_buf;
	int words_len;

	if(node->type != NODE_WORDS) return 0;
	if(node->data.word.iw_flg) return 0;

	words_buf = node->data.word.buf;
	words_len = node->data.word.len;
	
	for(; words_len > 0; words_len--) {
		if(*words_buf == '\n') return 0;
		if(get_char(pattern) >= 0x80) return 0;
		pattern_len += get_char_len2(*words_buf);
		if(pattern_len >= buf_size) return 0;
		pattern_buf = put_char(pattern_buf, *words_buf);
		words_buf++;
	}
	*pattern_buf = '\0';

	if(pattern_len < 3 || pattern_len > 128) return 0;

	return 1;
}

static int is_bm_only(OREG_DATA *reg_data)
{
	NFA_NODE *node = reg_data->nfa_node;

	for(; node != NULL; node = node->next[0]) {
		if(node->next[1] != NULL) return 0;
		if(is_epsilon_node(node)) continue;
		if(node->type != NODE_WORDS) return 0;
	}

	return 1;
}

static CHAR_CLASS *make_char_class_from_branch_tbl(OREG_DATA *reg_data,
	NFA_NODE *node)
{
	int     i;
	const unsigned char *branch_tbl = node->data.branch.branch_tbl;
	CHAR_CLASS *char_class = alloc_char_class(reg_data);

	for(i = 0; i < 0x80; i++) {
		if(check_branch_tbl(branch_tbl, i)) {
			make_char_class_ch_data_main(char_class, i);
		}
	}

	return char_class;
}

static void set_exec_mode(OREG_DATA *reg_data, const TCHAR *pattern)
{
	NFA_NODE *first_node = get_first_check_node(reg_data->nfa_node);
	TCHAR	bm_pattern[256];

	if(can_boyer_moore(first_node, pattern,
			bm_pattern, sizeof(bm_pattern)/sizeof(TCHAR))) {
		int ic_flg = first_node->data.word.ic_flg;
		reg_data->exec_data.bm_data = bm_create_data(bm_pattern, ic_flg);
		if(reg_data->exec_data.bm_data != NULL) {
			reg_data->reg_exec_mode = REG_EXEC_BOYER_MOORE;
			reg_data->reg_exec_func = &oreg_exec_main_bm;
			if(is_bm_only(reg_data)) reg_data->nfa_node = NULL;
			return;
		}
	}

	if(get_first_check_char(reg_data, first_node)) {
		reg_data->reg_exec_mode = REG_EXEC_FIRST_CHECK_CHAR;
		reg_data->reg_exec_func = &oreg_exec_main_first_check_char;
		return;
	}

	if(first_node->type == NODE_FIRST_MATCH) {
		reg_data->reg_exec_mode = REG_EXEC_FIRST_MATCH;
		reg_data->reg_exec_func = &oreg_exec_main_first_match;
		return;
	}

	if(first_node->type == NODE_CHAR_CLASS) {
		reg_data->reg_exec_mode = REG_EXEC_FIRST_CHECK_CHAR_CLASS;
		reg_data->exec_data.first_check_char_class =
			first_node->data.char_class;
		reg_data->reg_exec_func = &oreg_exec_main_first_check_char_class;
		return;
	}

	if(first_node->type == NODE_BRANCH && first_node->data.branch.branch_tbl) {
		reg_data->reg_exec_mode = REG_EXEC_FIRST_CHECK_CHAR_CLASS;
		reg_data->exec_data.first_check_char_class =
			make_char_class_from_branch_tbl(reg_data, first_node);
		reg_data->reg_exec_func = &oreg_exec_main_first_check_char_class;
		return;
	}

	if(first_node->type == NODE_WORD_SEPALATER) {
		reg_data->reg_exec_mode = REG_EXEC_FIRST_CHECK_WORD_SEPALATER;
		reg_data->reg_exec_func = &oreg_exec_main_first_check_word_sepalater;
		return;
	}

	reg_data->reg_exec_mode = REG_EXEC_DEFAULT;
	reg_data->reg_exec_func = &oreg_exec_main_default;
}

OREG_DATA *oreg_comp_str(const TCHAR *pattern, int i_flg)
{
	OREG_DATA	*reg_data;

	reg_data = oreg_alloc();
	if(reg_data == NULL) return NULL;

	reg_data->exec_data.bm_data = bm_create_data(pattern, i_flg);
	if(reg_data->exec_data.bm_data == NULL) {
		oreg_free(reg_data);
		return NULL;
	}

	reg_data->reg_exec_mode = REG_EXEC_BOYER_MOORE;
	reg_data->reg_exec_func = &oreg_exec_main_bm;

	return reg_data;
}

OREG_DATA *oreg_comp_str2(const TCHAR *pattern, int regexp_opt)
{
	OREG_DATA *result;
	if(regexp_opt & OREGEXP_OPT_IGNORE_WIDTH_ASCII) {
		TCHAR *new_pattern = (TCHAR *)malloc((_tcslen(pattern) + 10) * sizeof(TCHAR));
		if(new_pattern == NULL) return NULL;

		_stprintf(new_pattern, _T("\\Q%s\\E"), pattern);
		result = oreg_comp2(new_pattern, regexp_opt);

		free(new_pattern);
	} else {
		result = oreg_comp_str(pattern, (regexp_opt & OREGEXP_OPT_IGNORE_CASE));
	}

	return result;
}

OREG_DATA *oreg_comp(const TCHAR *pattern, int i_flg)
{
	int		regexp_opt = 0;
	if(i_flg) regexp_opt |= OREGEXP_OPT_IGNORE_CASE;
	return oreg_comp2(pattern, regexp_opt);
}

OREG_DATA *oreg_comp2(const TCHAR *pattern, int regexp_opt)
{
	OREG_DATA	*reg_data;

	reg_data = oreg_alloc();
	if(reg_data == NULL) return NULL;

	if(regexp_opt & OREGEXP_OPT_IGNORE_CASE) reg_data->reg_switch |= REG_SWITCH_ic;
	if(regexp_opt & OREGEXP_OPT_IGNORE_WIDTH_ASCII) reg_data->reg_switch |= REG_SWITCH_iw;

	reg_data->nfa_node = make_nfa_main(reg_data, pattern, _tcslen(pattern), 1);
	if(reg_data->nfa_node == NULL) {
		oreg_free(reg_data);
		return NULL;
	}

	if(!(regexp_opt & OREGEXP_OPT_NO_OPTIMIZE)) {
		reg_data->nfa_node = optimize_skip_epsilon(reg_data, reg_data->nfa_node);

		optimize_merge_selective(reg_data, reg_data->nfa_node);
		{
			unsigned char   node_check_buf[256];
			if((reg_data->cur_node_id / 8) + 1 < (int)sizeof(node_check_buf)) {
				optimize_branch(reg_data, reg_data->nfa_node, 0, node_check_buf);
			}
		}
		optimize_make_words(reg_data, reg_data->nfa_node);
	}

	set_exec_mode(reg_data, pattern);

	if(reg_data->nfa_node != NULL) {
		reg_data->stack_data = (REG_STACK *)malloc(
			sizeof(REG_STACK) * REGEXP_STACK_SIZE_INI);
		if(reg_data->stack_data == NULL) {
			oreg_free(reg_data);
			return NULL;
		}
		reg_data->stack_size = REGEXP_STACK_SIZE_INI;
	}

	return reg_data;
}

static INT_PTR print_nfa_node_elem(NFA_NODE *node)
{
	TCHAR	buf[1024] = _T("");
	TCHAR	buf2[1024] = _T("");

	_stprintf(buf, _T("%Id:"), node->node_id);

	switch(node->type) {
	case NODE_EPSILON:
		_stprintf(buf2, _T("e"));
		break;
	case NODE_BRANCH:
		_stprintf(buf2, _T("branch"));
		if(node->data.branch.branch_tbl) {
			unsigned char *b_tbl = node->data.branch.branch_tbl;
			unsigned int ch;
			int disp_cnt = 0;
			_tcscat(buf2, _T("("));
			for(ch = 0; ch < 0x80; ch++) {
				char flg = check_branch_tbl(b_tbl, ch);
				if(flg) {
					TCHAR tmp_buf[20];
					if(disp_cnt > 0) _tcscat(buf2, _T(","));
					_stprintf(tmp_buf, _T("%c:%d"), ch, flg);
					_tcscat(buf2, tmp_buf);
					disp_cnt++;
					if(disp_cnt > 3) {
						_tcscat(buf2, _T("..."));
						break;
					}
				}
			}
			_tcscat(buf2, _T(")"));
		}
		break;
	case NODE_WORD:
		if(node->data.ch.c >= 0x20 && node->data.ch.c <= 0x7f) {
			_stprintf(buf2, _T("ch('%c')"), node->data.ch.c);
		} else {
			_stprintf(buf2, _T("ch(0x%x)"), node->data.ch.c);
		}
		break;
	case NODE_WORDS:
		{
			TCHAR tmp_buf[512];
			TCHAR *p = tmp_buf;
			unsigned int *words_buf = node->data.word.buf;
			int words_len = node->data.word.len;
			for(; words_len > 0; words_len--) {
				p = put_char(p, *words_buf);
				words_buf++;
			}
			*p = '\0';
			_stprintf(buf2, _T("words(%s)"), tmp_buf);
		}
		break;
	case NODE_ANY_CHAR:
		_stprintf(buf2, _T("."));
		break;
	case NODE_CHAR_CLASS:
		_stprintf(buf2, _T("class"));
		break;
	case NODE_BACK_REF:
		_stprintf(buf2, _T("back_ref"));
		break;
	case NODE_NAMED_BACK_REF:
		_stprintf(buf2, _T("named_back_ref(%s)"), node->data.named_back_ref.name);
		break;
	case NODE_FIRST_MATCH:
		_stprintf(buf2, _T("^"));
		break;
	case NODE_FIRST_MATCH_A:
		_stprintf(buf2, _T("\\A"));
		break;
	case NODE_LAST_MATCH:
		_stprintf(buf2, _T("$"));
		break;
	case NODE_LAST_MATCH_Z:
		_stprintf(buf2, _T("\\Z"));
		break;
	case NODE_LAST_MATCH_z:
		_stprintf(buf2, _T("\\z"));
		break;
	case NODE_WORD_SEPALATER:
		_stprintf(buf2, _T("\\b"));
		break;
	case NODE_WORD_NOT_SEPALATER:
		_stprintf(buf2, _T("\\B"));
		break;
	case NODE_GROUP_START:
		_stprintf(buf2, _T("group_start"));
		break;
	case NODE_GROUP_END:
		_stprintf(buf2, _T("group_end  "));
		break;
	case NODE_LOOP_DETECT:
		_stprintf(buf2, _T("loop_detect"));
		break;
	case NODE_RECURSIVE:
		_stprintf(buf2, _T("recursive(%d)"), node->data.recursive.idx);
		break;
	case NODE_POSSESSIVE_CAPUTURE:
		_stprintf(buf2, _T("possessive_capture(min:%d, max:%d)"),
			node->data.possessive_capture.min,
			node->data.possessive_capture.max);
		break;
	default:
		_stprintf(buf2, _T("unknown(%d)"), node->type);
		break;
	}
	_tcscat(buf, buf2);

	_tprintf(_T("%s"), buf);
	return _tcslen(buf);
}

static void print_nfa_node_main(NFA_NODE *node, INT_PTR sp, unsigned char *node_check_buf)
{
	INT_PTR		n;

	if(node == NULL) return;

	n = node->node_id;
	if(node_check_buf[n / 8] & (0x01 << (n % 8))) {
		printf("[%Id]", node->node_id);
		return;
	}

	sp += print_nfa_node_elem(node);
	node_check_buf[n / 8] |= (0x01 << (n % 8));

	if(node->next[node->first_test]) {
		printf(" -> ");
		print_nfa_node_main(node->next[node->first_test], sp + 4,
			node_check_buf);
	}
	if(node->next[!node->first_test]) {
		int		i;
		printf("\n");
		for(i = 0; i < sp; i++) printf(" ");
		printf(" -> ");
		print_nfa_node_main(node->next[!node->first_test], sp + 4,
			node_check_buf);
	}
}

void print_nfa_node2(OREG_DATA *reg_data, NFA_NODE *nfa_node)
{
	unsigned char node_check_buf[1024];

	if(nfa_node == NULL) {
		if(reg_data->reg_exec_mode == REG_EXEC_BOYER_MOORE) {
			printf("Boyer Moore Search");
		}
		return;
	}

	if((reg_data->cur_node_id / 8) + 1 >= (int)sizeof(node_check_buf)) {
		printf("regexp is very large\n");
		return;
	}
	memset(node_check_buf, 0, (reg_data->cur_node_id / 8) + 1);
	print_nfa_node_main(nfa_node, 0, node_check_buf);
	printf("\n");
}

void print_nfa_node(OREG_DATA *reg_data)
{
	print_nfa_node2(reg_data, reg_data->nfa_node);
}

int oregexp_print_nfa(const TCHAR *pattern, int regexp_opt)
{
	OREG_DATA *reg_data = oreg_comp2(pattern, regexp_opt);
	if(reg_data != NULL) {
		print_nfa_node(reg_data);
	} else {
		printf("compile error!\n");
	}
	return 0;
}

#ifdef TEST
struct parse_q_node_st {
    TCHAR    *data;
    int     min;
    int     max;
};

struct parse_q_node_st parse_q_data[] = {
    {_T("{1}"), 1, 1},
    {_T("{1,4}"), 1, 4},
    {_T("{1, 5}"), 1, 5},
    {_T("{4,}"), 4, -1},
    {_T("{123,456}"), 123, 456},
};

int parse_q_node_test()
{
    int     ret_v, min, max, i;
    int     test_cnt = sizeof(parse_q_data)/sizeof(parse_q_data[0]);
    int     fail = 0;

    printf("parse_quantifier_node test:\n\n");

    for(i = 0; i < test_cnt; i++) {
        ret_v = parse_quantifier_node(parse_q_data[i].data, &min, &max);

        printf("test %d: %s ", i, parse_q_data[i].data);

        if(ret_v != 0 || min != parse_q_data[i].min ||
            max != parse_q_data[i].max) {
            fail++;
            printf("--- fail!!: %d, %d\n", min, max);
        } else {
            printf("--- OK\n");
        }
    }

    printf("\nparse_quantifier_node test: %d tests, %d fail \n\n",
        test_cnt, fail);

	return 0;
}

#endif



