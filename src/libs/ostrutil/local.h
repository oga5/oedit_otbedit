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
#define __inline
#endif

#ifdef WIN32
#include <basetsd.h>
#endif

#include "oregexp.h"
#include "get_char.h"
#include "bm_search.h"
#include <assert.h>

typedef enum {
	LEX_WORD,
	LEX_CHAR_CLASS,
	LEX_ANY_CHAR,
	LEX_GROUP,
	LEX_COMMENT,
	LEX_QUANTIFIER,
	LEX_OR_CHAR,
	LEX_ESCAPE_CHAR,
	LEX_FIRST_MATCH,
	LEX_LAST_MATCH,
} RegWordType;

typedef struct lex_word {
	RegWordType		type;
	const TCHAR 	*data;
	INT_PTR			len;
} LexWord;

typedef enum {
	NODE_EPSILON,
	NODE_BRANCH,

	NODE_WORD,
	NODE_WORDS,
	NODE_ANY_CHAR,
	NODE_ANY_CHAR_LF,
	NODE_CHAR_CLASS,
	NODE_BACK_REF,
	NODE_NAMED_BACK_REF,

	NODE_FIRST_MATCH,
	NODE_FIRST_MATCH_A,
	NODE_LAST_MATCH,
	NODE_LAST_MATCH_Z,
	NODE_LAST_MATCH_z,
	NODE_WORD_SEPALATER,
	NODE_WORD_NOT_SEPALATER,

	NODE_GROUP_START,
	NODE_GROUP_END,

	NODE_LOOP_DETECT,

	NODE_LOOK_AHEAD_START,
	NODE_NOT_LOOK_AHEAD_START,

	NODE_NO_BACKTRACK_START,
	NODE_LOOK_BEHIND_START,
	NODE_NOT_LOOK_BEHIND_START,
	NODE_REGEXP_SUB_END,

	NODE_CONDITIONAL,
	NODE_CONDITIONAL_BACK_REF_EXIST,
	NODE_CONDITIONAL_NAMED_BACK_REF_EXIST,
	NODE_CONDITIONAL_RECURSIVE,

	NODE_SELECTED_AREA_FIRST,
	NODE_SELECTED_AREA_LAST,
	NODE_SELECTED_AREA_ROW_FIRST,
	NODE_SELECTED_AREA_ROW_LAST,

	NODE_KEEP_PATTERN,

	NODE_RECURSIVE,
	NODE_POSSESSIVE_CAPUTURE,

	NODE_QUANTIFIER_START,
	NODE_QUANTIFIER_END,
} NfaNodeType;

typedef struct char_class_pair_st {
	unsigned int	start;
	unsigned int	end;
} CHAR_CLASS_PAIR;


#define ESC_CHAR_CLASS_TABLE_SIZE	16	// 128 / 8
#define CHAR_CLASS_TABLE_SIZE		32	// 256 / 8
#define BRANCH_TABLE_SIZE           32  // 128 / 4

typedef struct char_class_st {
	unsigned int	not:1;
	unsigned int	all_mb_flg:1;
	unsigned int	iw_flg:1;
	int				pair_cnt;
	CHAR_CLASS_PAIR *pair;
	int				other_cnt;
	unsigned int	*other;
	unsigned char	char_tbl[CHAR_CLASS_TABLE_SIZE];
} CHAR_CLASS;

typedef struct nfa_node_st {
	NfaNodeType			type;

	union {
		struct {
			unsigned int	c;
			unsigned int	ic_flg:1;
			unsigned int	iw_flg:1;
		} ch;
		CHAR_CLASS	*char_class;
		struct {
			int				idx;
			unsigned int	ic_flg:1;
			unsigned int	iw_flg:1;
		} back_ref;
		struct {
			TCHAR			*name;
			unsigned int	ic_flg:1;
			unsigned int	iw_flg:1;
		} named_back_ref;
		struct {
			unsigned int	ctrl_ch;
			unsigned char	*branch_tbl;
		} branch;
		struct {
			int				len;
			unsigned int	ic_flg:1;
			unsigned int	iw_flg:1;
			unsigned int	*buf;
		} word;
		struct {
			int behind_cnt;
			struct nfa_node_st	*group_end_node;
		} group;
		struct {
			int				idx;
		} recursive;
		struct {
			int				min;
			int				max;
		} possessive_capture;
		struct nfa_node_st	*condition;
	} data;

	struct nfa_node_st	*next[2];

	/*
	 * first_test: 最初にcheckする方向を制御するフラグ
	 * next[0]とnext[1]を入れ替えることも出来そうだが、next[0]をたどって
	 * 末尾に到達できることを保証したいため、このフラグがある
	 */
	char	first_test;

	char	optimized;
	INT_PTR	node_id;
} NFA_NODE;

#define ALLOC_BUF_SIZE  (sizeof(NFA_NODE) * 32)

typedef struct nfa_alloc_st {
	struct nfa_alloc_st *next;
	char buf[1];
} NFA_BUF;

#define BACK_REF_IDX_MAX	10

typedef struct back_ref_st {
	struct {
		OREG_POINT	start_pt;
		OREG_POINT	end_pt;
		size_t		len;
	} data[BACK_REF_IDX_MAX];
	int		last_match_idx;
} BACK_REF;

typedef struct regexp_stack_st REG_STACK;
struct regexp_stack_st {
	NFA_NODE	*node;
	BACK_REF	back_ref;
	OREG_POINT	cur_pt;
	OREG_POINT	keep_pt;
	int			recursive_idx;
	int			old_stack_cnt;

	const TCHAR *pbuf;
	const TCHAR *prev_pbuf;
};

#define REGEXP_STACK_SIZE_MAX	1024 * 32
#define REGEXP_STACK_SIZE_INI	32

typedef enum {
	REG_EXEC_NONE,
	REG_EXEC_DEFAULT,
	REG_EXEC_FIRST_CHECK_CHAR,
	REG_EXEC_BOYER_MOORE,
	REG_EXEC_FIRST_MATCH,
	REG_EXEC_FIRST_CHECK_CHAR_CLASS,
	REG_EXEC_FIRST_CHECK_WORD_SEPALATER,
} RegExecMode;

#define REG_SWITCH_s (0x01 << 0)
#define REG_SWITCH_S (0x01 << 1)
#define REG_SWITCH_ic (0x01 << 2)
#define REG_SWITCH_iw (0x01 << 3)

typedef int (*RegExecFunc)(
	HREG_DATA reg_data, HREG_DATASRC data_src,
	OREG_POINT *cur_pt, OREG_POINT *end_pt, OREG_POINT *search_end);

typedef struct oreg_data_st {
	struct {
		NFA_BUF	*cur;
		size_t	buf_size;
		size_t	cur_pt;
	} alloc_buf;

	NFA_NODE	*nfa_node;

	RegExecMode reg_exec_mode;
	RegExecFunc reg_exec_func;
	union {
		struct {
			unsigned int	c;
			unsigned int	ic_flg:1;
			unsigned int	iw_flg:1;
		} first_check_char;
		NFA_NODE	*first_check_node;
		CHAR_CLASS  *first_check_char_class;
		HBM_DATA	bm_data;
	} exec_data;

	BACK_REF	back_ref;
	int			back_ref_cnt;
	TCHAR		*back_ref_name[BACK_REF_IDX_MAX];
	NFA_NODE	*back_ref_nfa_start[BACK_REF_IDX_MAX];
	OREG_POINT	match_start;
	OREG_POINT	match_end;

	REG_STACK	*stack_data;
	int			stack_cnt;
	int			stack_size;

	int			reg_switch;
	INT_PTR		cur_node_id;
} OREG_DATA;

int is_quantifier(unsigned int ch);
int skip_comment(const TCHAR **pp, INT_PTR *pp_len);
int lexer(const TCHAR *p, INT_PTR p_len, LexWord *lex_word);

const TCHAR *get_back_ref_idx(const TCHAR *p, int *val,
	int back_ref_cnt);

__inline static int check_branch_tbl(const unsigned char *branch_tbl,
    unsigned int ch)
{
	int s = (ch % 4) * 2;
	assert((ch / 4) < BRANCH_TABLE_SIZE);
	return (branch_tbl[ ch / 4 ] & (0x03 << s)) >> s;
}

__inline static INT_PTR oreg_pt_cmp(const OREG_POINT *pt1, const OREG_POINT *pt2)
{
	if(pt1->row != pt2->row) return pt1->row - pt2->row;
	return pt1->col - pt2->col;
}

__inline static int is_epsilon_node(NFA_NODE *node)
{
	return (node->type == NODE_EPSILON);
}

__inline static const TCHAR *get_char_buf_src(HREG_DATASRC data_src,
    OREG_POINT *pt)
{
	return data_src->get_char_buf(data_src->src, pt);
}

__inline static const TCHAR *next_char_buf_src(
	const TCHAR *pbuf, HREG_DATASRC data_src, OREG_POINT *pt)
{
	int     ch_len = get_char_len(pbuf);
	pbuf += ch_len;
	if(*pbuf != '\0') {
		pt->col += ch_len;
		if(*pbuf == '\r' && *(pbuf + 1) == '\n') {
			pt->col += 1;
			pbuf++;
		}
		return pbuf;
	}
	return data_src->next_char(data_src->src, pt);
}

int oreg_exec_main_first_check_char(
	HREG_DATA reg_data, HREG_DATASRC data_src,
	OREG_POINT *cur_pt, OREG_POINT *end_pt, OREG_POINT *search_end);
int oreg_exec_main_default(
	HREG_DATA reg_data, HREG_DATASRC data_src,
	OREG_POINT *cur_pt, OREG_POINT *end_pt, OREG_POINT *search_end);
int oreg_exec_main_bm(
	HREG_DATA reg_data, HREG_DATASRC data_src,
	OREG_POINT *cur_pt, OREG_POINT *end_pt, OREG_POINT *search_end);
int oreg_exec_main_first_match(
	HREG_DATA reg_data, HREG_DATASRC data_src,
	OREG_POINT *cur_pt, OREG_POINT *end_pt, OREG_POINT *search_end);
int oreg_exec_main_first_check_char_class(
    HREG_DATA reg_data, HREG_DATASRC data_src,
    OREG_POINT *cur_pt, OREG_POINT *end_pt, OREG_POINT *search_end);
int oreg_exec_main_first_check_word_sepalater(
    HREG_DATA reg_data, HREG_DATASRC data_src,
    OREG_POINT *cur_pt, OREG_POINT *end_pt, OREG_POINT *search_end);

void print_nfa_node(OREG_DATA *reg_data);

int oregexp_lwr_main2(const TCHAR *pattern,
    const TCHAR *str, INT_PTR search_start_col,
    INT_PTR *start, INT_PTR *len, int regexp_opt, int simple_str_flg);

#ifdef  __cplusplus
}
#endif

