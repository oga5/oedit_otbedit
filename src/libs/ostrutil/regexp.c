/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "local.h"

#include "get_char.h"
#include "bm_local.h"

#ifdef WIN32
#include "mbutil.h"
#endif

extern const unsigned char CHAR_CLASS_w_TBL[ESC_CHAR_CLASS_TABLE_SIZE];

__inline static unsigned int get_char_pbuf(const TCHAR *pbuf, int ic_flg, int iw_flg)
{
	unsigned int ch;
	
	if(ic_flg) {
		ch = get_char_nocase(pbuf);
	} else {
		ch = get_char(pbuf);
	}

	if(iw_flg) {
		ch = get_ascii_char(ch);
	}

	return ch;
}

__inline static int check_any_char(unsigned int ch)
{
	return (ch != '\0' && ch != '\n');
}

__inline static int check_any_char_lf(unsigned int ch)
{
	return (ch != '\0');
}

static int check_w_char_class(unsigned int ch)
{
	if(ch < 0x80 && CHAR_CLASS_w_TBL[ch / 8] & (0x01 << (ch % 8))) {
		return 1;
	}
	return 0;
}

static int check_char_class(CHAR_CLASS *char_class, unsigned int ch)
{
	int		cnt;

	if(ch == '\0') return 0;

	if(char_class->iw_flg) {
		ch = get_ascii_char(ch);
	}

	if(ch <= 0xff) {
		if(char_class->char_tbl[ch / 8] & (0x01 << (ch % 8))) {
			return 1 - char_class->not;
		}
		return char_class->not;
	}

	if(char_class->all_mb_flg) {
		return 1 - char_class->not;
	}

	if(char_class->other) {
		int i;
		int cnt = char_class->other_cnt;
		unsigned int *other = char_class->other;
		for(i = 0; i < cnt; i++) {
			if(ch == other[i]) return 1 - char_class->not;
		}
	}

	for(cnt = 0; cnt < char_class->pair_cnt; cnt++) {
		if(ch >= char_class->pair[cnt].start &&
			ch <= char_class->pair[cnt].end) return 1 - char_class->not;
	}

	return char_class->not;
}

static int realloc_stack_data(OREG_DATA *reg_data)
{
	if(reg_data->stack_cnt >= REGEXP_STACK_SIZE_MAX) return 1;

	{
		REG_STACK *old_stack = reg_data->stack_data;
		REG_STACK *new_stack = (REG_STACK *)malloc(
			sizeof(REG_STACK) * reg_data->stack_size * 2);
		if(new_stack == NULL) return 1;	
		
		memcpy(new_stack, old_stack, sizeof(REG_STACK) *
			reg_data->stack_size);
		reg_data->stack_data = new_stack;
		reg_data->stack_size *= 2;

		free(old_stack);
	}

	return 0;
}

__inline static int push_stack(OREG_DATA *reg_data, REG_STACK **stack)
{
	(reg_data->stack_cnt)++;
	
	if(reg_data->stack_cnt == reg_data->stack_size) {
		if(realloc_stack_data(reg_data) == 1) return 1;
	}

	*stack = &(reg_data->stack_data[reg_data->stack_cnt]);
	**stack = reg_data->stack_data[reg_data->stack_cnt - 1];
	return 0;
}

__inline static int pop_stack(OREG_DATA *reg_data, REG_STACK **stack,
	int start_stack_cnt)
{
	int stack_cnt = reg_data->stack_cnt - 1;

	assert(reg_data->stack_cnt >= start_stack_cnt);
	if(reg_data->stack_cnt == start_stack_cnt) return 1;

	for(; stack_cnt >= start_stack_cnt; stack_cnt--) {
		NfaNodeType t = reg_data->stack_data[stack_cnt].node->type;
		if(t == NODE_LOOP_DETECT) continue;
		if(t == NODE_RECURSIVE) continue;
		break;
	}
	if(stack_cnt < start_stack_cnt) return 1;

	reg_data->stack_cnt = stack_cnt;
	*stack = &(reg_data->stack_data[stack_cnt]);

	// テストしていないルートに進む
	(*stack)->node = (*stack)->node->next[1 - (*stack)->node->first_test];
	return 0;
}

__inline static void back_ref_start(BACK_REF *back_ref,
	OREG_POINT *start_pt, int idx)
{
	back_ref->data[idx].start_pt = *start_pt;
}

__inline static void back_ref_end(BACK_REF *back_ref,
	OREG_POINT *end_pt, int idx, HREG_DATASRC data_src)
{
	back_ref->data[idx].end_pt = *end_pt;

	if(back_ref->data[idx].start_pt.row == end_pt->row) {
		back_ref->data[idx].len =
			end_pt->col - back_ref->data[idx].start_pt.col;
	} else {
		back_ref->data[idx].len = data_src->get_len(data_src->src,
			&(back_ref->data[idx].start_pt), end_pt);
	}

	back_ref->last_match_idx = idx;
}

__inline static int is_end_node(NFA_NODE *node)
{
	return (node == NULL);
}

static int check_loop_detect(OREG_DATA *reg_data, REG_STACK *stack)
{
	int		i;
	for(i = reg_data->stack_cnt - 1; i >= 0; i--) {
		if(stack->node == reg_data->stack_data[i].node) {
			if(stack->cur_pt.row == reg_data->stack_data[i].cur_pt.row &&
				stack->cur_pt.col == reg_data->stack_data[i].cur_pt.col) return 1;
			break;
		}
	}
	return 0;
}

__inline static void init_stack(REG_STACK *stack, OREG_DATA *reg_data,
	OREG_POINT *cur_pt, const TCHAR *pbuf, const TCHAR *prev_pbuf)
{
	int		i;
	for(i = 0; i < reg_data->back_ref_cnt; i++) {
		stack->back_ref.data[i].len = -1;
		stack->back_ref.data[i].start_pt.row = -1;
		stack->back_ref.data[i].start_pt.col = -1;
	}

	stack->back_ref.last_match_idx = -1;

	stack->node = reg_data->nfa_node;
	stack->cur_pt = *cur_pt;
	stack->keep_pt = *cur_pt;
	stack->recursive_idx = -1;

	stack->pbuf = pbuf;
	stack->prev_pbuf = prev_pbuf;
}

__inline static const TCHAR *get_prev_buf_stack(REG_STACK *stack,
	HREG_DATASRC data_src)
{
	static const TCHAR *null_str = _T("");
	OREG_POINT	prev_pt = stack->cur_pt;
	if(prev_pt.col == 0 && prev_pt.row == 0) return null_str;
	if(stack->prev_pbuf) {
		return stack->prev_pbuf;
	} else {
		return data_src->prev_char(data_src->src, &prev_pt);
	}
}

__inline static void next_char_stack(REG_STACK *stack, HREG_DATASRC data_src)
{
	stack->prev_pbuf = stack->pbuf;
	stack->pbuf = next_char_buf_src(stack->pbuf, data_src, &(stack->cur_pt));
}

__inline static void set_cur_pt_stack(REG_STACK *stack, OREG_POINT *pt,
	HREG_DATASRC data_src)
{
	stack->cur_pt = *pt;
	stack->prev_pbuf = NULL;
	stack->pbuf = get_char_buf_src(data_src, pt);
}

static int check_back_ref_stack(REG_STACK *stack, int idx,
	HREG_DATASRC data_src, int ic_flg, int iw_flg, OREG_DATA *reg_data)
{
	UINT_PTR	i;
	size_t		len;
	OREG_POINT	data_pt;
	OREG_POINT	backref_pt;
	const TCHAR *pbuf1, *pbuf2;

	if(idx >= reg_data->back_ref_cnt) return 0;
	if(stack->back_ref.data[idx].len < 0) return 0;

	data_pt = stack->cur_pt;
	backref_pt = stack->back_ref.data[idx].start_pt;

	pbuf1 = get_char_buf_src(data_src, &data_pt);
	pbuf2 = get_char_buf_src(data_src, &backref_pt);
	len = stack->back_ref.data[idx].len;

	for(i = 0; i < len; ) {
		if(get_char_pbuf(pbuf1, ic_flg, iw_flg) != get_char_pbuf(pbuf2, ic_flg, iw_flg)) {
			return 0;
		}
		
		i += get_char_len(pbuf1);
		pbuf1 = next_char_buf_src(pbuf1, data_src, &data_pt);
		pbuf2 = next_char_buf_src(pbuf2, data_src, &backref_pt);
	}

	set_cur_pt_stack(stack, &data_pt, data_src);

	return 1;
}

static int find_name_back_ref_idx(REG_STACK *stack, const TCHAR *name,
	OREG_DATA *reg_data)
{
	int		idx;

	for(idx = 0; idx < reg_data->back_ref_cnt; idx++) {
		if(stack->back_ref.data[idx].len < 0) continue;
		if(_tcscmp(name, reg_data->back_ref_name[idx]) == 0) return idx;
	}

	return -1;
}

static int check_named_back_ref_stack(REG_STACK *stack, const TCHAR *name,
	HREG_DATASRC data_src, int ic_flg, int iw_flg, OREG_DATA *reg_data)
{
	int		idx;

	idx = find_name_back_ref_idx(stack, name, reg_data);
	if(idx < 0) return 0;

	return check_back_ref_stack(stack, idx, data_src, ic_flg, iw_flg, reg_data);
}

__inline static int check_char(NFA_NODE *node, REG_STACK *stack)
{
	int ic_flg = node->data.ch.ic_flg;
	int iw_flg = node->data.ch.iw_flg;
	return (node->data.ch.c == get_char_pbuf(stack->pbuf, ic_flg, iw_flg));
}

__inline static int check_words(NFA_NODE *node, REG_STACK *stack,
	HREG_DATASRC data_src)
{
	int	len = node->data.word.len;
	unsigned int	*buf = node->data.word.buf;
	int ic_flg = node->data.word.ic_flg;
	int iw_flg = node->data.word.iw_flg;

	for(; len > 0; len--) {
		if(get_char_pbuf(stack->pbuf, ic_flg, iw_flg) != *buf) return 0;
		next_char_stack(stack, data_src);
		buf++;
	}
	return 1;
}

__inline static int is_start(OREG_POINT *cur_pt)
{
	return (cur_pt->row == 0 && cur_pt->col == 0);
}

__inline static int is_row_start(REG_STACK *stack, HREG_DATASRC data_src)
{
	return (stack->cur_pt.col == 0 || get_char(get_prev_buf_stack(stack, data_src)) == '\n');
}

__inline static int is_row_end(unsigned int ch, HREG_DATASRC data_src)
{
	return (ch == '\0' || ch == '\n');
}

__inline static int is_word_sepalater(unsigned int ch, unsigned int prev_ch)
{
	return (check_w_char_class(ch) != check_w_char_class(prev_ch));
}

static int set_prev_pt_stack(REG_STACK *stack, int cnt, HREG_DATASRC data_src)
{
	int		i;
	OREG_POINT pt = stack->cur_pt;

	for(i = 0; i < cnt; i++) {
		if(pt.col == 0 && pt.row == 0) return -1;
		data_src->prev_char(data_src->src, &pt);
	}

	set_cur_pt_stack(stack, &pt, data_src);
	return 0;
}

static int regexp_sub(OREG_DATA *reg_data, HREG_DATASRC data_src,
	OREG_POINT *end_pt);

static int is_back_ref_exist(OREG_DATA *reg_data,
	struct regexp_stack_st *stack, int idx)
{
	if(idx >= reg_data->back_ref_cnt) return 0;
	if(stack->back_ref.data[idx].len < 0) return 0;
	return 1;
}

static int is_named_back_ref_exist(OREG_DATA *reg_data,
	struct regexp_stack_st *stack, const TCHAR *name)
{
	int		idx;

	for(idx = 0; idx < reg_data->back_ref_cnt; idx++) {
		if(stack->back_ref.data[idx].len < 0) continue;
		if(_tcscmp(name, reg_data->back_ref_name[idx]) == 0) return 1;
	}

	return 0;
}

static int is_recursive_internal(OREG_DATA *reg_data,
	struct regexp_stack_st *stack, int group_idx)
{
	int		stack_cnt = reg_data->stack_cnt;

	if(stack->recursive_idx < 0) return 0;
	if(group_idx == 0 || group_idx == stack->recursive_idx) return 1;

	return 0;
}

static int check_condition(OREG_DATA *reg_data, HREG_DATASRC data_src,
	struct regexp_stack_st **stack, NFA_NODE *cur_node)
{
	OREG_POINT end_pt;
	int		old_stack_cnt = reg_data->stack_cnt;
	int		ret_v = 0;

	// 今のstackを壊さないようにする
	if(push_stack(reg_data, stack) != 0) return 0;

	(*stack)->node = cur_node;

	if(cur_node->type == NODE_CONDITIONAL_BACK_REF_EXIST) {
		if(is_back_ref_exist(reg_data, *stack, cur_node->data.back_ref.idx)) {
			ret_v = OREGEXP_FOUND;
		}
	} else if(cur_node->type == NODE_CONDITIONAL_NAMED_BACK_REF_EXIST) {
		if(is_named_back_ref_exist(reg_data, *stack,
			cur_node->data.named_back_ref.name)) {
			ret_v = OREGEXP_FOUND;
		}
	} else if(cur_node->type == NODE_CONDITIONAL_RECURSIVE) {
		if(is_recursive_internal(reg_data, *stack,
			cur_node->data.back_ref.idx)) {
			ret_v = OREGEXP_FOUND;
		}
	} else {
		// 正規表現を評価する
		ret_v = regexp_sub(reg_data, data_src, &end_pt);
	}

	// stackの状態を関数呼び出し前に戻す
	reg_data->stack_cnt = old_stack_cnt;
	*stack = &(reg_data->stack_data[old_stack_cnt]);

	return ret_v;
}

static int check_regexp(OREG_DATA *reg_data, HREG_DATASRC data_src,
	struct regexp_stack_st **stack, NFA_NODE **cur_node, int result)
{
	OREG_POINT end_pt;
	int		old_stack_cnt = reg_data->stack_cnt;
	int		ret_v = 0;
	NFA_NODE *end_node = (*cur_node)->data.group.group_end_node;

	assert(end_node && end_node->type == NODE_REGEXP_SUB_END);

	// 今のstackを壊さないようにする
	if(push_stack(reg_data, stack) != 0) return 0;

	(*stack)->node = (*cur_node)->next[0];

	if(regexp_sub(reg_data, data_src, &end_pt) == result) {
		if(old_stack_cnt != reg_data->stack_cnt) {
			// stackをコピー
			reg_data->stack_data[old_stack_cnt]	
				= reg_data->stack_data[reg_data->stack_cnt];
		}

		// nodeを進める
		*cur_node = end_node;

		ret_v = 1;
	}
	// FIXME: エラー処理を入れる

	// stackの状態を関数呼び出し前に戻す
	reg_data->stack_cnt = old_stack_cnt;
	*stack = &(reg_data->stack_data[old_stack_cnt]);

	return ret_v;
}

static int check_no_backtrack(OREG_DATA *reg_data, HREG_DATASRC data_src,
	struct regexp_stack_st **stack, NFA_NODE **cur_node)
{
	return check_regexp(reg_data, data_src, stack, cur_node, OREGEXP_FOUND);
}

static int check_behind(OREG_DATA *reg_data, HREG_DATASRC data_src,
	struct regexp_stack_st **stack, NFA_NODE **cur_node, int result)
{
	OREG_POINT cur_pt = (*stack)->cur_pt;
	int ret_v;

	if(set_prev_pt_stack(*stack, (*cur_node)->data.group.behind_cnt,
		data_src) != 0) {
		if(result == OREGEXP_NOT_FOUND) {
			NFA_NODE *end_node = (*cur_node)->data.group.group_end_node;
			assert(end_node && end_node->type == NODE_REGEXP_SUB_END);
			*cur_node = end_node;
			return 1;
		} else {
			return 0;
		}
	}

	ret_v = check_regexp(reg_data, data_src, stack, cur_node, result);

	set_cur_pt_stack(*stack, &cur_pt, data_src);

	return ret_v;
}

static int check_ahead(OREG_DATA *reg_data, HREG_DATASRC data_src,
	struct regexp_stack_st **stack, NFA_NODE **cur_node, int result)
{
	OREG_POINT cur_pt = (*stack)->cur_pt;
	int ret_v;

	ret_v = check_regexp(reg_data, data_src, stack, cur_node, result);

	set_cur_pt_stack(*stack, &cur_pt, data_src);

	return ret_v;
}

static int check_possessive_caputure(OREG_DATA *reg_data,
	HREG_DATASRC data_src, int min, int max,
	struct regexp_stack_st **stack2) 
{
	int	match_cnt = 0;
	struct regexp_stack_st *stack;
	NFA_NODE	*p_node;
	OREG_POINT	bak_cur_pt = {-1, -1};
	OREG_POINT	end_pt = {-1, -1};
	int		old_stack_cnt = reg_data->stack_cnt;
	int		result;

	stack = &(reg_data->stack_data[reg_data->stack_cnt]);
	p_node = stack->node;
	assert(p_node->type == NODE_POSSESSIVE_CAPUTURE); 

	for(;;) {
		bak_cur_pt = stack->cur_pt;
		stack->node = p_node->next[1];

		result = regexp_sub(reg_data, data_src, &end_pt);

		reg_data->stack_cnt = old_stack_cnt;
		stack = &(reg_data->stack_data[old_stack_cnt]);

		if(result != OREGEXP_FOUND) {
			stack->back_ref = reg_data->back_ref;
			set_cur_pt_stack(stack, &bak_cur_pt, data_src);
			break;
		}

		set_cur_pt_stack(stack, &end_pt, data_src);
		match_cnt++;
		if(max > 0 && match_cnt == max) break;
	}

	*stack2 = stack;
	if(match_cnt >= min && (max == -1 || match_cnt <= max)) {
		stack->node = p_node->next[0];
		return 1;
	}
	return 0;
}

static void start_recursive(OREG_DATA *reg_data, HREG_DATASRC data_src,
	struct regexp_stack_st **stack, int idx)
{
	int		old_stack_cnt = reg_data->stack_cnt;
	int		ret_v = 0;

	assert(idx <= reg_data->back_ref_cnt);

	// 今のstackを壊さないようにする
	if(push_stack(reg_data, stack) != 0) {
		// FIXME: push_stackは関数の外に出す
		assert(0);
		return;
	}

	(*stack)->recursive_idx = idx;
	(*stack)->old_stack_cnt = old_stack_cnt;

	if(idx == 0) {
		(*stack)->node = reg_data->nfa_node;
	} else {
		assert(reg_data->back_ref_nfa_start[idx - 1]->type == NODE_GROUP_START);
		assert(reg_data->back_ref_nfa_start[idx - 1]->data.back_ref.idx == idx - 1);
		(*stack)->node = reg_data->back_ref_nfa_start[idx - 1]->next[0];
	}
}

static void end_recursive(OREG_DATA *reg_data, struct regexp_stack_st *stack)
{
	int		old_stack_cnt = stack->old_stack_cnt;

	assert(reg_data->stack_data[old_stack_cnt].node->type == NODE_RECURSIVE);
	stack->back_ref = reg_data->stack_data[old_stack_cnt].back_ref;

	if(old_stack_cnt == 0) {
		stack->recursive_idx = -1;
	} else {
		struct regexp_stack_st *prev_stack =
			&reg_data->stack_data[old_stack_cnt - 1];
		stack->recursive_idx = prev_stack->recursive_idx;
		stack->old_stack_cnt = prev_stack->old_stack_cnt;
	}
}

__inline static int check_recursive_end(struct regexp_stack_st *stack)
{
	NFA_NODE	*cur_node;

	if(stack->recursive_idx < 0) return 0;

	cur_node = stack->node;

	if(is_end_node(cur_node)) return 1;

	if(cur_node->type == NODE_GROUP_END &&
	   cur_node->data.back_ref.idx == stack->recursive_idx - 1) return 1;

	return 0;
}

static int check_branch(struct regexp_stack_st *stack, NFA_NODE *cur_node)
{
	const unsigned char *b_tbl = cur_node->data.branch.branch_tbl;
	if(b_tbl) {
		unsigned int ch = get_char(stack->pbuf);
		if(ch < 0x80) {
			return check_branch_tbl(b_tbl, ch);
		}
	}

	return 0x03;
}

static int regexp_sub(OREG_DATA *reg_data, HREG_DATASRC data_src,
	OREG_POINT *end_pt)
{
	register int	err_flg = 0;
	NFA_NODE		*cur_node;
	struct regexp_stack_st *stack;
	int				start_stack_cnt = reg_data->stack_cnt;

	stack = &(reg_data->stack_data[reg_data->stack_cnt]);

	for(;;) {
		assert(err_flg == 0);

		/* 
		 * stack->prev_pbuf[0] == '\0'は、
		 * oreg_exec_main_first_check_word_sepalaterでdata_src->prev_charが
		 * ""を返すときに発生する
		 * prev_charがNULLを返すようにすれば、このチェックを削除できる
		 */
#ifdef LINUX
		assert(stack->prev_pbuf == NULL ||
			stack->prev_pbuf[0] == '\0' ||
			stack->prev_pbuf + get_char_len(stack->prev_pbuf) == stack->pbuf ||
			(*(stack->prev_pbuf + get_char_len(stack->prev_pbuf)) == '\r' && *(stack->prev_pbuf + get_char_len(stack->prev_pbuf) + 1) == '\n') && stack->prev_pbuf + get_char_len(stack->prev_pbuf) + 1 == stack->pbuf);
#endif
		if(check_recursive_end(stack)) {
			NFA_NODE *n = reg_data->stack_data[stack->old_stack_cnt].node;
			stack->node = n->next[n->first_test];
			end_recursive(reg_data, stack);
			continue;
		}

		cur_node = stack->node;

		// 終了判定
		if(is_end_node(cur_node)) {
			*end_pt = stack->cur_pt;
			reg_data->back_ref = stack->back_ref;
			return OREGEXP_FOUND;
		}

		switch(cur_node->type) {
		case NODE_WORDS:
			err_flg = (!check_words(cur_node, stack, data_src));
			break;
		case NODE_WORD:
			if(check_char(cur_node, stack)) {
				next_char_stack(stack, data_src);
			} else {
				err_flg = 1;
			}
			break;
		case NODE_EPSILON:
			break;
		case NODE_BRANCH:
			{
				int flg = check_branch(stack, cur_node);

				if(flg == 0x00) {
					err_flg = 1;
				} else if(flg == 0x01) {
					stack->node = cur_node->next[0];
					continue;
				} else if(flg == 0x02) {
					stack->node = cur_node->next[1];
					continue;
				} else {
					assert(flg == 0x03);
					if(push_stack(reg_data, &stack) != 0) {
						return OREGEXP_STACK_OVERFLOW;
					}
				}
			}
			break;
		case NODE_ANY_CHAR:
			if(check_any_char(get_char(stack->pbuf))) {
				next_char_stack(stack, data_src);
			} else {
				err_flg = 1;
			}
			break;
		case NODE_ANY_CHAR_LF:
			if(check_any_char_lf(get_char(stack->pbuf))) {
				next_char_stack(stack, data_src);
			} else {
				err_flg = 1;
			}
			break;
		case NODE_CHAR_CLASS:
			if(check_char_class(cur_node->data.char_class,
				get_char(stack->pbuf))) {
				next_char_stack(stack, data_src);
			} else {
				err_flg = 1;
			}
			break;
		case NODE_FIRST_MATCH:
			err_flg = !is_row_start(stack, data_src);
			break;
		case NODE_FIRST_MATCH_A:
			err_flg = !is_start(&(stack->cur_pt));
			break;
		case NODE_LAST_MATCH:
			err_flg = !(is_row_end(get_char(stack->pbuf), data_src));
			break;
		case NODE_LAST_MATCH_Z:
			if(get_char(stack->pbuf) == '\n') {
				OREG_POINT tmp_pt = stack->cur_pt;
				const TCHAR *pbuf =
					next_char_buf_src(stack->pbuf, data_src, &tmp_pt);
				err_flg = (get_char(pbuf) != '\0');
			} else {
				err_flg = (get_char(stack->pbuf) != '\0' ||
					get_char(get_prev_buf_stack(stack, data_src)) == '\n');
			}
			break;
		case NODE_LAST_MATCH_z:
			err_flg = (get_char(stack->pbuf) != '\0');
			break;
		case NODE_SELECTED_AREA_FIRST:
			if(data_src->is_selected_first) {
				err_flg = !data_src->is_selected_first(data_src->src, &(stack->cur_pt));
			} else {
				err_flg = 1;
			}
			break;
		case NODE_SELECTED_AREA_LAST:
			if(data_src->is_selected_last) {
				err_flg = !data_src->is_selected_last(data_src->src, &(stack->cur_pt));
			} else {
				err_flg = 1;
			}
			break;
		case NODE_SELECTED_AREA_ROW_FIRST:
			if(data_src->is_selected_row_first) {
				err_flg = !data_src->is_selected_row_first(data_src->src, &(stack->cur_pt));
			} else {
				err_flg = 1;
			}
			break;
		case NODE_SELECTED_AREA_ROW_LAST:
			if(data_src->is_selected_row_last) {
				err_flg = !data_src->is_selected_row_last(data_src->src, &(stack->cur_pt));
			} else {
				err_flg = 1;
			}
			break;
		case NODE_GROUP_START:
			back_ref_start(&(stack->back_ref), &(stack->cur_pt),
				cur_node->data.back_ref.idx);
			break;
		case NODE_GROUP_END:
			back_ref_end(&(stack->back_ref), &(stack->cur_pt),
				cur_node->data.back_ref.idx, data_src);
			break;
		case NODE_BACK_REF:
			err_flg = !(check_back_ref_stack(stack,
				cur_node->data.back_ref.idx, data_src,
				cur_node->data.back_ref.ic_flg,
				cur_node->data.back_ref.iw_flg,
				reg_data));
			break;
		case NODE_NAMED_BACK_REF:
			err_flg = !(check_named_back_ref_stack(stack,
				cur_node->data.named_back_ref.name, data_src,
				cur_node->data.named_back_ref.ic_flg,
				cur_node->data.named_back_ref.iw_flg,
				reg_data));
			break;
		case NODE_WORD_SEPALATER:
			err_flg = !is_word_sepalater(get_char(stack->pbuf),
					get_char(get_prev_buf_stack(stack, data_src)));
			break;
		case NODE_WORD_NOT_SEPALATER:
			err_flg = is_word_sepalater(get_char(stack->pbuf),
					get_char(get_prev_buf_stack(stack, data_src)));
			break;
		case NODE_LOOP_DETECT:
			if(check_loop_detect(reg_data, stack) == 0) {
				if(push_stack(reg_data, &stack) != 0) {
					return OREGEXP_STACK_OVERFLOW;
				}
			} else {
				err_flg = 1;
			}
			break;
		case NODE_LOOK_AHEAD_START:
			err_flg = !check_ahead(reg_data, data_src, &stack,
				&cur_node, OREGEXP_FOUND);
			break;
		case NODE_NOT_LOOK_AHEAD_START:
			err_flg = !check_ahead(reg_data, data_src, &stack,
				&cur_node, OREGEXP_NOT_FOUND);
			break;
		case NODE_LOOK_BEHIND_START:
			err_flg = !check_behind(reg_data, data_src, &stack,
				&cur_node, OREGEXP_FOUND);
			break;
		case NODE_NOT_LOOK_BEHIND_START:
			err_flg = !check_behind(reg_data, data_src, &stack,
				&cur_node, OREGEXP_NOT_FOUND);
			break;
		case NODE_NO_BACKTRACK_START:
			err_flg = !check_no_backtrack(reg_data, data_src, &stack,
				&cur_node);
			break;
		case NODE_REGEXP_SUB_END:
			*end_pt = stack->cur_pt;
			return OREGEXP_FOUND;
		case NODE_CONDITIONAL:
			if(check_condition(reg_data, data_src,
				&stack, cur_node->data.condition) == OREGEXP_FOUND) {
				cur_node->first_test = 1;
			} else {
				cur_node->first_test = 0;
			}
			break;
		case NODE_KEEP_PATTERN:
			stack->keep_pt = stack->cur_pt;
			break;
		case NODE_RECURSIVE:
			start_recursive(reg_data, data_src, &stack,
				cur_node->data.recursive.idx);
			continue;
		case NODE_POSSESSIVE_CAPUTURE:
			err_flg = !check_possessive_caputure(reg_data, data_src,
				cur_node->data.possessive_capture.min,
				cur_node->data.possessive_capture.max,
				&stack);
			break;
//		default:
//			err_flg = 1;
		}

		if(err_flg == 0) {
			stack->node = cur_node->next[cur_node->first_test];
		} else {
			// 失敗したら，スタックから次のルートを取り出す
			// スタックが空になったら，全ルート検索完了
			if(pop_stack(reg_data, &stack, start_stack_cnt) != 0) {
				break;
			}

			err_flg = 0;
		}
	}

	return OREGEXP_NOT_FOUND;
}

__inline static int regexp_sub2(OREG_DATA *reg_data, HREG_DATASRC data_src,
    OREG_POINT *start_pt, OREG_POINT *end_pt,
    const TCHAR *pbuf, const TCHAR *prev_pbuf)
{
	int		result;

	reg_data->stack_cnt = 0;

	init_stack(&(reg_data->stack_data[0]), reg_data, start_pt, pbuf, prev_pbuf);

	result = regexp_sub(reg_data, data_src, end_pt);

	if(result == OREGEXP_FOUND) {
		*start_pt = reg_data->stack_data[reg_data->stack_cnt].keep_pt;
	}

	return result;
}

__inline static int is_search_end(const OREG_POINT *cur_pt,
	const OREG_POINT *search_end)
{
	if(search_end->row >= 0) {
		if(cur_pt->row > search_end->row) return 1;
		if(cur_pt->row == search_end->row &&
			cur_pt->col > search_end->col) return 1;
	}
	return 0;
}

int oreg_exec_main_first_check_char(
	HREG_DATA reg_data, HREG_DATASRC data_src,
	OREG_POINT *cur_pt, OREG_POINT *end_pt, OREG_POINT *search_end)
{
	int		hit_flg;
	const TCHAR *pbuf = get_char_buf_src(data_src, cur_pt);
	unsigned int first_check_char = reg_data->exec_data.first_check_char.c;
	int ic_flg = reg_data->exec_data.first_check_char.ic_flg;
	int iw_flg = reg_data->exec_data.first_check_char.iw_flg;

	for(;;) {
		if(get_char_pbuf(pbuf, ic_flg, iw_flg) == first_check_char) {
			hit_flg = regexp_sub2(reg_data, data_src, cur_pt, end_pt,
				pbuf, NULL);
			if(hit_flg != OREGEXP_NOT_FOUND) return hit_flg;
		}
		if(*pbuf == '\0') break;
		pbuf = next_char_buf_src(pbuf, data_src, cur_pt);
		if(is_search_end(cur_pt, search_end)) break;
	}
	return OREGEXP_NOT_FOUND;
}

int oreg_exec_main_first_check_char_class(
	HREG_DATA reg_data, HREG_DATASRC data_src,
	OREG_POINT *cur_pt, OREG_POINT *end_pt, OREG_POINT *search_end)
{
	int		hit_flg;
	const TCHAR *pbuf = get_char_buf_src(data_src, cur_pt);
	CHAR_CLASS *char_class = reg_data->exec_data.first_check_char_class;

	for(;;) {
		if(check_char_class(char_class, get_char(pbuf))) {
			hit_flg = regexp_sub2(reg_data, data_src, cur_pt, end_pt,
				pbuf, NULL);
			if(hit_flg != OREGEXP_NOT_FOUND) return hit_flg;
		}
		if(*pbuf == '\0') break;
		pbuf = next_char_buf_src(pbuf, data_src, cur_pt);
		if(is_search_end(cur_pt, search_end)) break;
	}
	return OREGEXP_NOT_FOUND;
}

int oreg_exec_main_first_check_word_sepalater(
	HREG_DATA reg_data, HREG_DATASRC data_src,
	OREG_POINT *cur_pt, OREG_POINT *end_pt, OREG_POINT *search_end)
{
	int		hit_flg;
	const TCHAR *pbuf = get_char_buf_src(data_src, cur_pt);
	unsigned int ch;
	OREG_POINT prev_pt = *cur_pt;
	const TCHAR *prev_pbuf = data_src->prev_char(data_src->src, &prev_pt);
	unsigned int prev_ch = (prev_pbuf) ? get_char(prev_pbuf) : '\0';

	for(;;) {
		ch = get_char(pbuf);

		if(is_word_sepalater(ch, prev_ch)) {
			hit_flg = regexp_sub2(reg_data, data_src, cur_pt, end_pt,
				pbuf, prev_pbuf);
			if(hit_flg != OREGEXP_NOT_FOUND) return hit_flg;
		}

		if(*pbuf == '\0') break;
		prev_pbuf = pbuf;
		pbuf = next_char_buf_src(pbuf, data_src, cur_pt);
		if(is_search_end(cur_pt, search_end)) break;

		prev_ch = ch;
	}
	return OREGEXP_NOT_FOUND;
}

int oreg_exec_main_default(
	HREG_DATA reg_data, HREG_DATASRC data_src,
	OREG_POINT *cur_pt, OREG_POINT *end_pt, OREG_POINT *search_end)
{
	int		hit_flg;
	const TCHAR *pbuf = get_char_buf_src(data_src, cur_pt);
	const TCHAR *prev_pbuf = NULL;

	for(;;) {
		hit_flg = regexp_sub2(reg_data, data_src, cur_pt, end_pt,
			pbuf, prev_pbuf);
		if(hit_flg != OREGEXP_NOT_FOUND) return hit_flg;
		if(*pbuf == '\0') break;
		prev_pbuf = pbuf;
		pbuf = next_char_buf_src(pbuf, data_src, cur_pt);
		if(is_search_end(cur_pt, search_end)) break;
	}
	return OREGEXP_NOT_FOUND;
}

int oreg_exec_main_first_match(
	HREG_DATA reg_data, HREG_DATASRC data_src,
	OREG_POINT *cur_pt, OREG_POINT *end_pt, OREG_POINT *search_end)
{
	int		hit_flg;
	const TCHAR *pbuf;

	if(!data_src->is_row_first(data_src->src, cur_pt)) {
		pbuf = data_src->next_row(data_src->src, cur_pt);
		if(pbuf == NULL || is_search_end(cur_pt, search_end)) return OREGEXP_NOT_FOUND;
	} else {
		pbuf = data_src->get_char_buf(data_src->src, cur_pt);
		if(pbuf == NULL) return OREGEXP_NOT_FOUND;
	}

	for(;;) {
		hit_flg = regexp_sub2(reg_data, data_src, cur_pt, end_pt, pbuf, NULL);
		if(hit_flg != OREGEXP_NOT_FOUND) return hit_flg;

		pbuf = data_src->next_row(data_src->src, cur_pt);
		if(pbuf == NULL || is_search_end(cur_pt, search_end)) break;
	}
	return OREGEXP_NOT_FOUND;
}

int oreg_exec_main_bm(
	HREG_DATA reg_data, HREG_DATASRC data_src,
	OREG_POINT *cur_pt, OREG_POINT *end_pt, OREG_POINT *search_end)
{
	size_t		buf_len;
	HBM_DATA bm_data = reg_data->exec_data.bm_data;
	const TCHAR *pbuf = data_src->get_row_buf_len(data_src->src, cur_pt->row, &buf_len);
	const TCHAR *p;

	if(pbuf == NULL) return OREGEXP_NOT_FOUND;

	pbuf += cur_pt->col;
	buf_len -= cur_pt->col;

	for(;;) {
		if(buf_len >= bm_data->pattern_len) {
			p = bm_search(bm_data, pbuf, buf_len);

			if(p != NULL) {
				cur_pt->col += (p - pbuf);
				if(is_search_end(cur_pt, search_end)) break;

				if(reg_data->nfa_node == NULL) {
					*end_pt = *cur_pt;
					end_pt->col += bm_data->pattern_len;
					return OREGEXP_FOUND;
				} else {
					int		hit_flg;
					hit_flg = regexp_sub2(reg_data, data_src, cur_pt, end_pt,
						p, NULL);
					if(hit_flg == OREGEXP_FOUND) return OREGEXP_FOUND;
				}

				cur_pt->col += get_char_len(p);
				p += get_char_len(p);
				buf_len -= (p - pbuf);
				pbuf = p;
				continue;
			}
		}

		cur_pt->col = 0;
		(cur_pt->row)++;
		if(is_search_end(cur_pt, search_end)) break;

		pbuf = data_src->get_row_buf_len(data_src->src, cur_pt->row, &buf_len);
		if(pbuf == NULL) break;
		p = pbuf;
	}

	return OREGEXP_NOT_FOUND;
}

static int oreg_exec_main(HREG_DATA reg_data, HREG_DATASRC data_src,
	OREG_POINT *search_start, OREG_POINT *search_end,
	OREG_POINT *result_start, OREG_POINT *result_end)
{
	OREG_POINT	cur_pt = *search_start;
	OREG_POINT	end_pt = {0, 0};
	int			hit_flg;

	if(reg_data == NULL) return 0;

	hit_flg = reg_data->reg_exec_func(reg_data, data_src, &cur_pt, &end_pt,
		search_end);
	if(hit_flg == OREGEXP_FOUND) {
		reg_data->match_start = cur_pt;
		reg_data->match_end = end_pt;
		*result_start = cur_pt;
		*result_end = end_pt;
	}
	return hit_flg;
}

int oreg_exec2(HREG_DATA reg_data, HREG_DATASRC data_src)
{
    OREG_POINT  search_start = {0, 0};
    OREG_POINT  search_end = {-1, -1};
    OREG_POINT  result_start = {0, 0};
    OREG_POINT  result_end = {0, 0};

	return oreg_exec_main(reg_data, data_src,
		&search_start, &search_end, &result_start, &result_end);
}

int oreg_exec(HREG_DATA reg_data, HREG_DATASRC data_src,
	OREG_POINT *search_start, OREG_POINT *search_end,
	OREG_POINT *result_start, OREG_POINT *result_end, int loop_flg)
{
	OREG_POINT zero_pt = {0, 0};

	int ret_v = oreg_exec_main(reg_data, data_src,
		search_start, search_end, result_start, result_end);
	if(ret_v == OREGEXP_FOUND) return ret_v;

	if(loop_flg == 0) return 0;
	if(search_start->row == 0 && search_start->col == 0) return 0;
	if(search_end->row >= 0) return 0;

	return oreg_exec_main(reg_data, data_src,
		&zero_pt, search_start,
		result_start, result_end);
}


