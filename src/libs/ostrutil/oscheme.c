/*
 * oscheme interpreter
 *
 * Copyright (c) 2025 Atsushi Ogawa
 *
 * Derived from MiniSCHEME, incorporating ideas from TinySCHEME.
 * Refer to LICENSE_MINISCHEME and LICENSE_TINYSCHEME for respective license details.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 /* The following lines contain the original credits from Mini-Scheme. */
 /*
  *      ---------- Mini-Scheme Interpreter Version 0.85 ----------
  *
  *                coded by Atsushi Moriwaki (11/5/1989)
  *
  *            E-MAIL :  moriwaki@kurims.kurims.kyoto-u.ac.jp
  *
  *               THIS SOFTWARE IS IN THE PUBLIC DOMAIN
  *               ------------------------------------
  * This software is completely free to copy, modify and/or re-distribute.
  * But I would appreciate it if you left my name on the code as the author.
  *
  */


#define USE_ALLOC_SET   1
/*
#define _GC_DEBUG_ 1
*/

#define USE_SETJMP

#ifdef WIN32
#include <tchar.h>
#include <windows.h>
#include <locale.h>
#define ENABLE_CANCEL
#else
#define OSCHEME_MAIN
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>

#ifdef WIN32
typedef struct _stat stat_st;
#else
typedef struct stat stat_st;
#endif

#ifdef WIN32
#include <io.h>
#define F_OK 0
#else
#include <unistd.h>
#endif

#ifdef USE_SETJMP
#include <setjmp.h>
#endif

#include "oregexp.h"
#include "ohash.h"
#include "ommgr.h"

#include "oscheme.h"

/* ========== string operator ========== */
typedef unsigned int  uint;
typedef unsigned int  och;

/* ========== multibyte character ========== */
#define MAX_CHAR_SIZE	4

#include "get_char.h"
static int _is_lead_byte(TCHAR c) { return is_lead_byte(c); }
static och _get_char(TCHAR *p) { return get_char(p); }
static TCHAR *_put_char(TCHAR *p, och ch) { return put_char(p, ch); }
static int _get_char_len(TCHAR *p) { return get_char_len(p); }
static int _get_char_len2(och ch) { return get_char_len2(ch); }
static TCHAR *_next_char(TCHAR *p) { return p + _get_char_len(p); }

/* ========== char operations ========== */
#define _isdigit(c)	((c) >= '0' && (c) <= '9')
#define _isspace(c)	((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r')
#define _char_upper_p(ch)    (('A' <= (ch)) && ((ch) <= 'Z'))
#define _char_lower_p(ch)    (('a' <= (ch)) && ((ch) <= 'z'))
#define _char_upcase(ch)     (_char_lower_p(ch) ? ((ch) - ('a' - 'A')) : (ch))
#define _char_downcase(ch)   (_char_upper_p(ch) ? ((ch) + ('a' - 'A')) : (ch))

static int _string_cmp(TCHAR *s1, TCHAR *s2)
{
	for(;; s1 = _next_char(s1), s2 = _next_char(s2)) {
		if(_get_char(s1) != _get_char(s2)) break;
		if(*s1 == '\0') break;
	}
	return _get_char(s1) - _get_char(s2);
}

static int _string_cmp_ci(TCHAR *s1, TCHAR *s2)
{
	for(;; s1 = _next_char(s1), s2 = _next_char(s2)) {
		if(_char_downcase(_get_char(s1)) !=
			_char_downcase(_get_char(s2))) break;
		if(*s1 == '\0') break;
	}
	return _char_downcase(_get_char(s1)) - _char_downcase(_get_char(s2));
}

/* ========== defines ========== */
#define CELL_SEGSIZE		(1024 * 4)
#define CELL_NSEGMENT		 100
#define INPORT_READ_SIZE	1024
#define FILL_INPORT_THR		  10
#define STR_BUF_SIZE		1024
#define ARG_STACK_SIZE		1024
#define GC_THRESHOLD_SIZE	(1024 * 1024 * 8)

#define prompt		"> "
#define InitFile	"init.scm"

#define _OP_DEFINE_TABLE_ \
	_OP_DEF(op_load,OP_LOAD,_T("load")) \
	_OP_DEF(op_require,OP_REQUIRE,_T("require")) \
	_OP_DEF(op_use,OP_USE,NULL) \
	_OP_DEF(op_load_t0lvl,OP_LOAD_T0LVL,NULL) \
	_OP_DEF(op_t0lvl,OP_T0LVL,NULL) \
	_OP_DEF(op_t1lvl,OP_T1LVL,NULL) \
	_OP_DEF(op_read,OP_READ,_T("read")) \
	_OP_DEF(op_read_char,OP_READ_CHAR,_T("read-char")) \
	_OP_DEF(op_peek_char,OP_PEEK_CHAR,_T("peek-char")) \
	_OP_DEF(op_valueprint,OP_VALUEPRINT,NULL) \
	_OP_DEF(op_values_print,OP_VALUES_PRINT,NULL) \
	_OP_DEF(op_eval,OP_EVAL,NULL) \
	_OP_DEF(op_e0args,OP_E0ARGS,NULL) \
	_OP_DEF(op_e1args,OP_E1ARGS,NULL) \
	_OP_DEF(op_apply,OP_APPLY,NULL) \
	_OP_DEF(op_domacro,OP_DOMACRO,NULL) \
	_OP_DEF(op_lambda,OP_LAMBDA,NULL) \
	_OP_DEF(op_quote,OP_QUOTE,NULL) \
	_OP_DEF(op_qquote0,OP_QQUOTE0,NULL) \
	_OP_DEF(op_qquote1,OP_QQUOTE1,NULL) \
	_OP_DEF(op_qquote2,OP_QQUOTE2,NULL) \
	_OP_DEF(op_qquote2_add,OP_QQUOTE2_ADD,NULL) \
	_OP_DEF(op_qquote2_dot,OP_QQUOTE2_DOT,NULL) \
	_OP_DEF(op_qquote2_unqsp,OP_QQUOTE2_UNQSP,NULL) \
	_OP_DEF(op_qquote_vec0,OP_QQUOTE_VEC0,NULL) \
	_OP_DEF(op_qquote_vec1,OP_QQUOTE_VEC1,NULL) \
	_OP_DEF(op_def0,OP_DEF0,NULL) \
	_OP_DEF(op_def1,OP_DEF1,NULL) \
	_OP_DEF(op_begin,OP_BEGIN,NULL) \
	_OP_DEF(op_if0,OP_IF0,NULL) \
	_OP_DEF(op_if1,OP_IF1,NULL) \
	_OP_DEF(op_set0,OP_SET0,NULL) \
	_OP_DEF(op_set1,OP_SET1,NULL) \
	_OP_DEF(op_let0,OP_LET0,NULL) \
	_OP_DEF(op_let_optional0,OP_LET_OPTIONAL0,NULL) \
	_OP_DEF(op_let_optional1,OP_LET_OPTIONAL1,NULL) \
	_OP_DEF(op_let1,OP_LET1,NULL) \
	_OP_DEF(op_let2,OP_LET2,NULL) \
	_OP_DEF(op_let0ast,OP_LET0AST,NULL) \
	_OP_DEF(op_let1ast,OP_LET1AST,NULL) \
	_OP_DEF(op_let2ast,OP_LET2AST,NULL) \
	_OP_DEF(op_let0rec,OP_LET0REC,NULL) \
	_OP_DEF(op_let1rec,OP_LET1REC,NULL) \
	_OP_DEF(op_let2rec,OP_LET2REC,NULL) \
	_OP_DEF(op_cond0,OP_COND0,NULL) \
	_OP_DEF(op_cond1,OP_COND1,NULL) \
	_OP_DEF(op_and0,OP_AND0,NULL) \
	_OP_DEF(op_and1,OP_AND1,NULL) \
	_OP_DEF(op_or0,OP_OR0,NULL) \
	_OP_DEF(op_or1,OP_OR1,NULL) \
	_OP_DEF(op_0macro,OP_0MACRO,NULL) \
	_OP_DEF(op_1macro,OP_1MACRO,NULL) \
	_OP_DEF(op_peval,OP_PEVAL,_T("eval")) \
	_OP_DEF(op_papply,OP_PAPPLY,_T("apply")) \
	_OP_DEF(op_continuation,OP_CONTINUATION,_T("call-with-current-continuation")) \
	_OP_DEF(op_add,OP_ADD,_T("+")) \
	_OP_DEF(op_sub,OP_SUB,_T("-")) \
	_OP_DEF(op_mul,OP_MUL,_T("*")) \
	_OP_DEF(op_div,OP_DIV,_T("/")) \
	_OP_DEF(op_rem,OP_REM,_T("remainder")) \
	_OP_DEF(op_modulo,OP_MODULO,_T("modulo")) \
	_OP_DEF(op_car,OP_CAR,_T("car")) \
	_OP_DEF(op_cdr,OP_CDR,_T("cdr")) \
	_OP_DEF(op_cons,OP_CONS,_T("cons")) \
	_OP_DEF(op_setcar,OP_SETCAR,_T("set-car!")) \
	_OP_DEF(op_setcdr,OP_SETCDR,_T("set-cdr!")) \
	_OP_DEF(op_eq,OP_EQ,_T("eq?")) \
	_OP_DEF(op_eqv,OP_EQV,_T("eqv?")) \
	_OP_DEF(op_equal,OP_EQUAL,_T("equal?")) \
	_OP_DEF(op_not,OP_NOT,_T("not")) \
	_OP_DEF(op_is_bool,OP_BOOLP,_T("boolean?")) \
	_OP_DEF(op_is_null,OP_NULLP,_T("null?")) \
	_OP_DEF(op_neq,OP_NEQ,_T("=")) \
	_OP_DEF(op_less,OP_LESS,_T("<")) \
	_OP_DEF(op_gre,OP_GRE,_T(">")) \
	_OP_DEF(op_leq,OP_LEQ,_T("<=")) \
	_OP_DEF(op_geq,OP_GEQ,_T(">=")) \
	_OP_DEF(op_is_number,OP_NUMBERP,_T("number?")) \
	_OP_DEF(op_is_integer,OP_INTEGERP,_T("integer?")) \
	_OP_DEF(op_number_to_string,OP_NUMBER_TO_STRING,_T("number->string")) \
	_OP_DEF(op_string_to_number,OP_STRING_TO_NUMBER,_T("string->number")) \
	_OP_DEF(op_is_proc,OP_PROCP,_T("procedure?")) \
	_OP_DEF(op_is_pair,OP_PAIRP,_T("pair?")) \
	_OP_DEF(op_is_closure,OP_CLOSUREP,_T("closure?")) \
	_OP_DEF(op_is_macro,OP_MACROP,_T("macro?")) \
	_OP_DEF(op_is_list,OP_LISTP,_T("list?")) \
	_OP_DEF(op_list_length,OP_LIST_LENGTH,_T("length")) \
	_OP_DEF(op_restore_outport,OP_RESTORE_OUTPORT,NULL) \
	_OP_DEF(op_restore_inport,OP_RESTORE_INPORT,NULL) \
	_OP_DEF(op_write,OP_WRITE,_T("write")) \
	_OP_DEF(op_write_char,OP_WRITE_CHAR,_T("write-char")) \
	_OP_DEF(op_display,OP_DISPLAY,_T("display")) \
	_OP_DEF(op_newline,OP_NEWLINE,_T("newline")) \
	_OP_DEF(op_print_list0,OP_P0LIST,NULL) \
	_OP_DEF(op_print_list1,OP_P1LIST,NULL) \
	_OP_DEF(op_err0,OP_ERR0,_T("error")) \
	_OP_DEF(op_err1,OP_ERR1,NULL) \
	_OP_DEF(op_reverse,OP_REVERSE,_T("reverse")) \
	_OP_DEF(op_append,OP_APPEND,_T("append")) \
	_OP_DEF(op_int_env,OP_INT_ENV,_T("interaction-environment")) \
	_OP_DEF(op_eof,OP_EOF,NULL) \
	_OP_DEF(op_exit,OP_EXIT,_T("exit")) \
	_OP_DEF(op_gc,OP_GC,_T("gc")) \
	_OP_DEF(op_read_pr,OP_RDSEXPR,NULL) \
	_OP_DEF(op_read_list,OP_RDLIST,NULL) \
	_OP_DEF(op_read_dot,OP_RDDOT,NULL) \
	_OP_DEF(op_read_quote,OP_RDQUOTE,NULL) \
	_OP_DEF(op_read_quote_vec,OP_RDQUOTE_VEC,NULL) \
	_OP_DEF(op_read_qquote,OP_RDQQUOTE,NULL) \
	_OP_DEF(op_read_unquote,OP_RDUNQUOTE,NULL) \
	_OP_DEF(op_read_unquote_sp,OP_RDUQTSP,NULL) \
	_OP_DEF(op_read_qquote_vec,OP_RDQQUOTE_VEC,NULL) \
	_OP_DEF(op_quote_vec0,OP_QUOTE_VEC0,NULL) \
	_OP_DEF(op_quote_vec1,OP_QUOTE_VEC1,NULL) \
	_OP_DEF(op_print_vector,OP_PVECTOR,NULL) \
	_OP_DEF(op_is_vector,OP_VECTORP,_T("vector?")) \
	_OP_DEF(op_mk_vector,OP_MKVEC,_T("make-vector")) \
	_OP_DEF(op_vector_ref,OP_VECREF,_T("vector-ref")) \
	_OP_DEF(op_vector_set,OP_VECSET,_T("vector-set!")) \
	_OP_DEF(op_vector_length,OP_VECLEN,_T("vector-length")) \
	_OP_DEF(op_is_char,OP_CHARP,_T("char?")) \
	_OP_DEF(op_char_to_int,OP_CHAR_TO_INT,_T("char->integer")) \
	_OP_DEF(op_int_to_char,OP_INT_TO_CHAR,_T("integer->char")) \
	_OP_DEF(op_is_symbol,OP_SYMBOLP,_T("symbol?")) \
	_OP_DEF(op_is_symbol_bound,OP_SYMBOL_BOUNDP,_T("symbol-bound?")) \
	_OP_DEF(op_symbol_to_string,OP_SYMBOL_TO_STRING,_T("symbol->string")) \
	_OP_DEF(op_is_string,OP_STRINGP,_T("string?")) \
	_OP_DEF(op_mk_string,OP_MKSTRING,_T("make-string")) \
	_OP_DEF(op_string_length,OP_STRING_LENGTH,_T("string-length")) \
	_OP_DEF(op_string_to_symbol,OP_STRING_TO_SYMBOL,_T("string->symbol")) \
	_OP_DEF(op_string_ref,OP_STRING_REF,_T("string-ref")) \
	_OP_DEF(op_string_set,OP_STRING_SET,_T("string-set!")) \
	_OP_DEF(op_substring,OP_SUBSTRING,_T("substring")) \
	_OP_DEF(op_string_append,OP_STRING_APPEND,_T("string-append")) \
	_OP_DEF(op_string_cmp,OP_STRING_CMP,_T("_string-cmp?")) \
	_OP_DEF(op_string_cmp_ci,OP_STRING_CMP_CI,_T("_string-cmp-ci?")) \
	_OP_DEF(op_get_closure_code,OP_GET_CLOSURE,_T("get-closure-code")) \
	_OP_DEF(op_macroexpand_1,OP_MACROEXPAND_1,_T("macroexpand-1")) \
	_OP_DEF(op_macroexpand,OP_MACROEXPAND,_T("macroexpand")) \
	_OP_DEF(op_macroexpand_main,OP_MACROEXPAND_MAIN,NULL) \
	_OP_DEF(op_gensym,OP_GENSYM,_T("gensym")) \
	_OP_DEF(op_is_gensym,OP_GENSYMP,_T("gensym?")) \
	_OP_DEF(op_is_output_port,OP_OUTPUT_PORTP,_T("output-port?")) \
	_OP_DEF(op_current_output_port,OP_CURRENT_OUTPUT_PORT,_T("current-output-port")) \
	_OP_DEF(op_open_output_file,OP_OPEN_OUTPUT_FILE,_T("open-output-file")) \
	_OP_DEF(op_open_output_string,OP_OPEN_OUTPUT_STRING,_T("open-output-string")) \
	_OP_DEF(op_get_output_string,OP_GET_OUTPUT_STRING,_T("get-output-string")) \
	_OP_DEF(op_clear_output_string,OP_CLEAR_OUTPUT_STRING,_T("clear-output-string")) \
	_OP_DEF(op_close_output_port,OP_CLOSE_OUTPUT_PORT,_T("close-output-port")) \
	_OP_DEF(op_is_input_port,OP_INPUT_PORTP,_T("input-port?")) \
	_OP_DEF(op_current_input_port,OP_CURRENT_INPUT_PORT,_T("current-input-port")) \
	_OP_DEF(op_open_input_file,OP_OPEN_INPUT_FILE,_T("open-input-file")) \
	_OP_DEF(op_open_input_string,OP_OPEN_INPUT_STRING,_T("open-input-string")) \
	_OP_DEF(op_close_input_port,OP_CLOSE_INPUT_PORT,_T("close-input-port")) \
	_OP_DEF(op_is_port_closed,OP_PORT_CLOSEDP,_T("port-closed?")) \
	_OP_DEF(op_is_eof_object,OP_EOF_OBJECTP,_T("eof-object?")) \
	_OP_DEF(op_is_file_exists,OP_FILE_EXISTP,_T("file-exists?")) \
	_OP_DEF(op_call_cb_func,OP_CALL_CB_FUNC,NULL) \
	_OP_DEF(op_values,OP_VALUES,_T("values")) \
	_OP_DEF(op_call_with_values0,OP_CALL_WITH_VALUES0,_T("call-with-values")) \
	_OP_DEF(op_call_with_values1,OP_CALL_WITH_VALUES1,NULL) \
	_OP_DEF(op_is_regexp,OP_REGEXPP,_T("regexp?")) \
	_OP_DEF(op_regexp_to_string,OP_REGEXP_TO_STRING,_T("regexp->string")) \
	_OP_DEF(op_string_to_regexp,OP_STRING_TO_REGEXP,_T("string->regexp")) \
	_OP_DEF(op_rxmatch,OP_RXMATCH,_T("rxmatch")) \
	_OP_DEF(op_is_regmatch,OP_REGMATCHP,_T("regmatch?")) \
	_OP_DEF(op_rxmatch_num_matches,OP_RX_NUM_MATCHES,_T("rxmatch-num-matches")) \
	_OP_DEF(op_rxmatch_substring,OP_RXMATCHE_SUBSTRING,_T("rxmatch-substring")) \
	_OP_DEF(op_rxmatch_start,OP_RXMATCHE_START,_T("rxmatch-start")) \
	_OP_DEF(op_rxmatch_end,OP_RXMATCHE_END,_T("rxmatch-end")) \
	_OP_DEF(op_regexp_replace,OP_REGEXP_REPLACE,_T("regexp-replace")) \
	_OP_DEF(op_regexp_replace_all,OP_REGEXP_REPLACE_ALL,_T("regexp-replace-all")) \
	_OP_DEF(op_mk_hash,OP_MAKE_HASH,_T("make-hash-table")) \
	_OP_DEF(op_hash_table_type,OP_HASH_TYPE_TABLE,_T("hash-table-type")) \
	_OP_DEF(op_hash_put,OP_HASH_PUT,_T("hash-table-put!")) \
	_OP_DEF(op_hash_delete,OP_HASH_DELETE,_T("hash-table-delete!")) \
	_OP_DEF(op_hash_get,OP_HASH_GET,_T("hash-table-get")) \
	_OP_DEF(op_hash_exists,OP_HASH_EXISTS,_T("hash-table-exists?")) \
	_OP_DEF(op_hash_num_entries,OP_HASH_NUM_ENTRIES,_T("hash-table-num-entries")) \
	_OP_DEF(op_hash_table_for_each0,OP_HASH_TABLE_FOR_EACH0,_T("hash-table-for-each")) \
	_OP_DEF(op_hash_table_for_each1,OP_HASH_TABLE_FOR_EACH1,NULL) \
	_OP_DEF(op_hash_table_map0,OP_HASH_TABLE_MAP0,_T("hash-table-map")) \
	_OP_DEF(op_hash_table_map1,OP_HASH_TABLE_MAP1,NULL) \
	_OP_DEF(op_is_hash_table,OP_IS_HASH_TABLE,_T("hash-table?")) \
	_OP_DEF(op_define_class,OP_DEFINE_CLASS,_T("_define-class")) \
	_OP_DEF(op_make_object,OP_MAKE_OBJECT,_T("make")) \
	_OP_DEF(op_slot_ref,OP_SLOT_REF,_T("slot-ref")) \
	_OP_DEF(op_slot_set,OP_SLOT_SET,_T("slot-set!")) \
	_OP_DEF(op_sys_time,OP_SYS_TIME,_T("sys-time")) \
	_OP_DEF(op_sys_localtime,OP_SYS_LOCALTIME,_T("sys-localtime")) \
	_OP_DEF(op_sys_strftime,OP_SYS_STRFTIME,_T("sys-strftime")) \
	_OP_DEF(op_sys_stat,OP_SYS_STAT,_T("sys-stat")) \
	_OP_DEF(op_sys_getenv, OP_SYS_GETENV, _T("sys-getenv")) \
	_OP_DEF(op_win_exec,OP_WIN_EXEC,_T("win-exec")) \
	_OP_DEF(op_win_get_clipboard_text,OP_WIN_GET_CLIPBOARD_TEXT,_T("win-get-clipboard-text")) \
	_OP_DEF(op_debug_info,OP_DEBUG_INFO,_T("_debug_info")) \
	_OP_DEF(op_optional0,OP_OPTIONAL0,NULL) \
	_OP_DEF(op_nop,OP_NOP,_T("_nop"))

enum scheme_opcodes {
#define _OP_DEF(FN,OP,NAME) OP,
	_OP_DEFINE_TABLE_
	OP_MAX_DEFINED
#undef _OP_DEF
};

/* ========== type definition ========== */
typedef struct _st_string {
	size_t	_bufsize;
	TCHAR	_svalue[1];
} ostr;
typedef ostr *pstr;

typedef struct _st_symbol {
	uint	_symnum;
	TCHAR	*_symname;
} osymbol;
typedef osymbol *psymbol;

typedef struct _st_vector {
	INT_PTR	_item_cnt;
	INT_PTR	_arr_size;
	
	pcell	_arr[1];
} ovector;
typedef ovector *pvector;

typedef struct _st_hash_entry {
	pcell	_key;
	pcell	_data;
} _hash_entry;

typedef struct _st_hash_oblist_entry {
	TCHAR	*_name;
	pcell	_symbol;
} _hash_oblist_entry;

enum _hash_type {
	HASH_TYPE_EQ,
	HASH_TYPE_EQV,
	HASH_TYPE_EQUAL,
	HASH_TYPE_STRING,
	HASH_TYPE_PTR,
	HASH_TYPE_OBLIST,
};

typedef struct _st_hash_table {
	ohash			_hash;
	enum _hash_type	_type;
} _hash_table;
typedef _hash_table *phash;

typedef struct _st_number {
	INT_PTR		_ivalue;
} onumber;

typedef struct _st_dump_stack_frame {
	enum scheme_opcodes operator;
	pcell	args;
	pcell	envir;
	pcell	code;
} ostackframe;

typedef struct _st_continuation {
	int		_dump_stack_cnt;
	struct _st_dump_stack_frame _dump_stack[1];
} ocontinuation;

#define PORT_INPORT		(0x01 << 0)
#define PORT_OUTPORT	(0x01 << 1)
#define PORT_CLOSED		(0x01 << 2)
#define PORT_STRING		(0x01 << 3)
#define PORT_UNICODE	(0x01 << 4)

typedef struct _st_port {
	int		_flg;
	FILE	*_fp;
	INT_PTR	_buf_size;
	TCHAR	*_buf;
	TCHAR	*_cur_buf;
	TCHAR	*_end_buf;
	oscheme_portout_func	_out_func;
	TCHAR	_file_name[1];
} oport;
typedef oport *pport;

typedef int (*slot_set_func)(oscheme *sc, pcell o, pcell v, int idx);
typedef pcell (*slot_get_func)(oscheme *sc, pcell o, int idx);
typedef struct _st_slot_accesser {
	slot_get_func	getter;
	slot_set_func	setter;
} slot_accesser;

typedef struct _st_class {
	pcell		_supers;
	pcell		_slots;
	slot_accesser	*_accessers;
	int			_slot_cnt;
	TCHAR		_class_name[1];
} oclass;
typedef oclass *pclass;

typedef struct _st_object {
	pcell	_class;
	pcell	_vslots[1];
} oobject;
typedef oobject *pobject;

struct _st_cell {
	unsigned short _flag;
	union {
		struct {
			pcell	_car;
			pcell	_cdr;
		} _cons;
		onumber		_number;
		enum scheme_opcodes _procnum;
		void *		_ptr;
		och			_ch;
	} _object;
};

struct _st_scheme {
	/* arrays for segments */
	pcell	cell_seg[CELL_NSEGMENT];
	int		cell_seg_size[CELL_NSEGMENT];
	int		last_cell_seg;
	int		cur_cell_seg;
	pcell	next_cell;
	pcell	next_cell_last;

	pcell	args;			/* register for arguments of function */
	pcell	envir;			/* stack register for current environment */
	pcell	code;			/* register for current code */

	pcell	oblist;			/* pointer to symbol table */
	pcell	global_env;		/* pointer to global environment */
	pcell	global_var;		/* pointer to global variable */

	pcell	infp;			/* input file */
	pcell	outfp;			/* output file */
	pcell	errfp;			/* err file */
	pcell	iofp;			/* read buffer */
	pcell	tmpfp;

	/* pointer to special symbols */
	ocell	_NIL, _T, _F, _UNDEF;
	pcell	NIL;				/* special cell representing empty cell */
	pcell	T;					/* special cell representing #t */
	pcell	F;					/* special cell representing #f */
	pcell	UNDEF;				/* special cell representing #undef */

	pcell	LAMBDA;				/* pointer to syntax lambda */
	pcell	QUOTE;				/* pointer to syntax quote */
	pcell	QUOTE_VEC;			/* pointer to syntax quote-vec */
	pcell	QQUOTE;				/* pointer to syntax quasiquote */
	pcell	UNQUOTE;			/* pointer to syntax unquote */
	pcell	UNQUOTESP;			/* pointer to syntax unquote-splicing */
	pcell	QQUOTE_VEC;			/* pointer to syntax quasiquote-vec */

	/* for reading */
	TCHAR	err_buf[STR_BUF_SIZE];

	/* for eval cycle */
	int		tok;
	int		print_flag;
	pcell	value;
	enum scheme_opcodes operator;

	/* for values/call-with-values */
	pcell	values;

	/* stack frame */
	ostackframe *dump_stack;
	int		cur_frame;
	int		dump_stack_cnt;

#ifdef USE_ALLOC_SET
	/* memory context */
	alloc_set	aset;
	size_t		aset_allocated_size;
	size_t		aset_gc_threshold_size;
#endif

	/* other */
	uint	gensym_counter;
	char	interactive;
	char	gc_verbose;
	int		used_cell_cnt;
	int		all_cell_cnt;

#ifdef ENABLE_CANCEL
	int		cancel_counter;
	int		cancel_flg;
	int		cancel_fvirt;
	int		cancel_keycode;
#endif

#ifdef USE_SETJMP
	jmp_buf	jmp_env;
#endif

	/* temporary register */
	int		dont_gc_flg;

	/* for application extention */
	void	*user_data;
};

#define MAX_CB_FUNC_ARGS		32
typedef struct _st_callback_func {
	oscheme_callback_func	_fp;
	int		_min_args;
	int		_max_args;
	TCHAR	_type_chk_str[MAX_CB_FUNC_ARGS];
	TCHAR	_func_name[1];
} ocbfunc;
typedef ocbfunc *pcbfunc;

typedef struct _st_regexp_data {
	HREG_DATA	_hregdata;
	int			_ci_flg;
	TCHAR		_regexp_pattern[1];
} oregexp_data;
typedef oregexp_data *pregexp_data;

typedef struct _st_regmatch_data {
	INT_PTR		_start_pt;
	INT_PTR		_end_pt;
} oregmatch_data;

typedef struct _st_regmatch {
	int				_cnt;
	pcell			_str;
	oregmatch_data	_match_data[1];
} oregmatch;
typedef oregmatch *pregmatch;

/* mark functions */
void _gc_mark_continuation(oscheme *sc, pcell p);
void _gc_mark_vector(oscheme *sc, pcell p);
void _gc_mark_regmatch(oscheme *sc, pcell p);
void _gc_mark_hash(oscheme *sc, pcell p);
void _gc_mark_class(oscheme *sc, pcell p);
void _gc_mark_object(oscheme *sc, pcell p);

/* finalize functions */
void _finalize_cell(oscheme *sc, pcell p);
void _finalize_continuation(oscheme *sc, pcell p);
void _finalize_port(oscheme *sc, pcell p);
void _finalize_regexp(oscheme *sc, pcell p);
void _finalize_hash(oscheme *sc, pcell p);
void _finalize_hash_iter(oscheme *sc, pcell p);
void _finalize_class(oscheme *sc, pcell p);

#define _TYPE_DEFINE_TABLE_ \
	_TYPE_DEF(_T_NOTHING_,NULL,NULL) /* DUMMY */\
	_TYPE_DEF(T_PAIR,NULL,NULL) \
	_TYPE_DEF(T_SYNTAX,NULL,NULL) \
	_TYPE_DEF(T_PROC,NULL,NULL) \
	_TYPE_DEF(T_SYMBOL,NULL,_finalize_cell) \
	_TYPE_DEF(T_CLOSURE,NULL,NULL) \
	_TYPE_DEF(T_CONTINUATION,_gc_mark_continuation,_finalize_continuation) \
	_TYPE_DEF(T_MACRO,NULL,NULL) \
	_TYPE_DEF(T_VECTOR,_gc_mark_vector,_finalize_cell) \
	_TYPE_DEF(T_PORT,NULL,_finalize_port) \
	_TYPE_DEF(T_EOF,NULL,NULL) \
	_TYPE_DEF(T_NUMBER,NULL,NULL) \
	_TYPE_DEF(T_STRING,NULL,_finalize_cell) \
	_TYPE_DEF(T_CHAR,NULL,NULL) \
	_TYPE_DEF(T_CB_FUNC,NULL,_finalize_cell) \
	_TYPE_DEF(T_REGEXP,NULL,_finalize_regexp) \
	_TYPE_DEF(T_REGMATCH,_gc_mark_regmatch,_finalize_cell) \
	_TYPE_DEF(T_HASH,_gc_mark_hash,_finalize_hash) \
	_TYPE_DEF(T_HASH_ITER,NULL,_finalize_hash_iter) \
	_TYPE_DEF(T_CLASS,_gc_mark_class,_finalize_class) \
	_TYPE_DEF(T_OBJECT,_gc_mark_object,_finalize_cell) \
	_TYPE_DEF(T_MEMBLOCK,NULL,_finalize_cell)\
	_TYPE_DEF(_T_NOTHING1_,NULL,NULL) /* DUMMY */\
	_TYPE_DEF(_T_NOTHING2_,NULL,NULL) /* DUMMY */\
	_TYPE_DEF(_T_NOTHING3_,NULL,NULL) /* DUMMY */\
	_TYPE_DEF(_T_NOTHING4_,NULL,NULL) /* DUMMY */\
	_TYPE_DEF(_T_NOTHING5_,NULL,NULL) /* DUMMY */\
	_TYPE_DEF(_T_NOTHING6_,NULL,NULL) /* DUMMY */\
	_TYPE_DEF(_T_NOTHING7_,NULL,NULL) /* DUMMY */\
	_TYPE_DEF(_T_NOTHING8_,NULL,NULL) /* DUMMY */\
	_TYPE_DEF(_T_NOTHING9_,NULL,NULL) /* DUMMY */\
	_TYPE_DEF(_T_NOTHING10_,NULL,NULL) /* DUMMY */

enum type_id_enum {
#define _TYPE_DEF(_TYPE_ID_,_TYPE_MARK_FUNC_,_TYPE_FINALIZE_FUNC_) _TYPE_ID_,
	_TYPE_DEFINE_TABLE_
	TYPE_MAX_DEFINED
#undef _TYPE_DEF
};

typedef void (*_type_mark_func)(oscheme *sc, pcell x);
typedef void (*_type_finalize_func)(oscheme *sc, pcell x);

struct type_table_st {
	int					id;
	_type_mark_func		mark_func;
	_type_finalize_func	finalize_func;
};

struct type_table_st type_table[] = {
#define _TYPE_DEF(_TYPE_ID_,_TYPE_MARK_FUNC_,_TYPE_FINALIZE_FUNC_) \
	{_TYPE_ID_,_TYPE_MARK_FUNC_,_TYPE_FINALIZE_FUNC_},
	_TYPE_DEFINE_TABLE_
};

#define TYPEMASK		     31 /* 0000000000011111 */

#define T_MACROCACHE       8192	/* 0010000000000000 */	/* macro cache */
#define T_ATOM            16384	/* 0100000000000000 */	/* only for gc */
#define MARK              32768	/* 1000000000000000 */	/* only for gc */

/* for type check */
#define TC_NONE					_T("")
#define TC_ANY					_T("A")
#define TC_OBJECT				_T("b")
#define TC_CHAR					_T("C")
#define TC_CLOSURE				_T("c")
#define TC_LAST_ARGS			_T("D")
#define TC_ENVIRONMENT			_T("E")	/* FIXME: type_chk not implemented */
#define TC_HASH					_T("H")
#define TC_IPORT				_T("I")
#define TC_CLASS				_T("k")
#define TC_LIST					_T("L")
#define TC_REGMATCH				_T("M")
#define TC_NUMBER				_T("N")
#define TC_POSITIVE_NUMBER		_T("+")
#define TC_OPORT				_T("O")
#define TC_OSTRPORT				_T("o")
#define TC_PAIR					_T("P")
#define TC_PROCEDURE			_T("p")
#define TC_REGEXP				_T("R")
#define TC_STRING				_T("S")
#define TC_STRING_OR_PROCEDURE	_T("s")
#define TC_PORT					_T("T")
#define TC_VECTOR				_T("V")
#define TC_SYMBOL				_T("Y")

/* ========== macros for cell operations ========== */
#define typeflag(p)			((p)->_flag)
#define type(p)				(typeflag(p) & TYPEMASK)
#define settype(p, t)		typeflag(p) = (typeflag(p) & ~TYPEMASK) | t;

#define ptrvalue(p)			((p)->_object._ptr)

#define isstring(p)			(type(p) == T_STRING)
#define strobj(p)			((pstr)ptrvalue(p))
#define strvalue(p)			(strobj(p)->_svalue)
#define strbufsize(p)		(strobj(p)->_bufsize)

#define isnumber(p)			(type(p) == T_NUMBER)
#define ivalue(p)			((p)->_object._number._ivalue)
#define ivalue_unchecked(p)	((p)->_object._number._ivalue)

#define ischar(p)			(type(p) == T_CHAR)
#define chvalue(p)			((p)->_object._ch)

#define ispair(p)			(type(p) == T_PAIR)

#ifdef CAR_DEBUG
#else
#define car(p)				((p)->_object._cons._car)
#define cdr(p)				((p)->_object._cons._cdr)
#endif

#define issymbol(p)			(type(p) == T_SYMBOL)
#define symvalue(p)			((psymbol)ptrvalue(p))
#define symnum(p)			(symvalue(p)->_symnum)
#define symname(p)			(symvalue(p)->_symname)
#define isgensym(p)			(symnum(p) != 0)

#define isproc(p)			(type(p) == T_PROC)
#define procnum(p)			((p)->_object._procnum)

#define issyntax(p)			(type(p) == T_SYNTAX)
#define syntaxnum(p)		((p)->_object._procnum)

#define isclosure(p)		(type(p) == T_CLOSURE)
#define closure_code(p)		car(p)
#define closure_env(p)		cdr(p)

#define ismacro(p)			(type(p) == T_MACRO)

#define iscontinuation(p)	(type(p) == T_CONTINUATION)
#define getcontinuation(p)	((ocontinuation *)ptrvalue(p))

#define isvector(p)			(type(p) == T_VECTOR)
#define vector_obj(p)		((pvector)ptrvalue(p))
#define vector_cnt(p)		(vector_obj(p)->_item_cnt)
#define vector_arr(p)		(vector_obj(p)->_arr)
#define vector_size(p)		(vector_obj(p)->_arr_size)

#define ishash(p)			(type(p) == T_HASH)
#define hash_obj(p)			((phash)ptrvalue(p))
#define hash_hashdata(p)	(hash_obj(p)->_hash)
#define hash_type(p)		(hash_obj(p)->_type)

#define hash_iter(p)		((ohash_iter)ptrvalue(p))

#define iseof_object(p)		(type(p) == T_EOF)

#define isport(p)			(type(p) == T_PORT)
#define portobj(p)			((pport)ptrvalue(p))
#define port_fp(p)			(portobj(p)->_fp)
#define port_flg(p)			(portobj(p)->_flg)
#define port_fname(p)		(portobj(p)->_file_name)
#define port_buf_size(p)	(portobj(p)->_buf_size)
#define port_buf(p)			(portobj(p)->_buf)
#define port_cur_buf(p)		(portobj(p)->_cur_buf)
#define port_end_buf(p)		(portobj(p)->_end_buf)
#define port_out_func(p)	(portobj(p)->_out_func)
#define is_inport(p)		(port_flg(p) & PORT_INPORT)
#define is_outport(p)		(port_flg(p) & PORT_OUTPORT)
#define is_stringport(p)	(port_flg(p) & PORT_STRING)
#define is_strport(p)		(port_fp(p) == NULL)
#define is_closed_port(p)	(port_flg(p) & PORT_CLOSED)

#define is_cb_func(p)		(type(p) == T_CB_FUNC)
#define cb_funcobj(p)		((pcbfunc)ptrvalue(p))
#define cb_func_min_args(p)	(cb_funcobj(p)->_min_args)
#define cb_func_max_args(p)	(cb_funcobj(p)->_max_args)
#define cb_func_type_chk_str(p)	(cb_funcobj(p)->_type_chk_str)
#define cb_func_fp(p)		(cb_funcobj(p)->_fp)
#define cb_func_name(p)		(cb_funcobj(p)->_func_name)

#define isregexp(p)			(type(p) == T_REGEXP)
#define regexp_obj(p)		((pregexp_data)ptrvalue(p))
#define regexp_hregdata(p)	(regexp_obj(p)->_hregdata)
#define regexp_ci(p)		(regexp_obj(p)->_ci_flg)
#define regexp_pattern(p)	(regexp_obj(p)->_regexp_pattern)

#define isregmatch(p)		(type(p) == T_REGMATCH)
#define regmatch_obj(p)		((pregmatch)ptrvalue(p))
#define regmatch_cnt(p)		(regmatch_obj(p)->_cnt)
#define regmatch_str(p)		(regmatch_obj(p)->_str)
#define regmatch_data(p)	(regmatch_obj(p)->_match_data)

#define isclass(p)			(type(p) == T_CLASS)
#define class_obj(p)		((pclass)ptrvalue(p))
#define class_supers(p)		(class_obj(p)->_supers)
#define class_slots(p)		(class_obj(p)->_slots)
#define class_accessers(p)	(class_obj(p)->_accessers)
#define class_slot_cnt(p)	(class_obj(p)->_slot_cnt)
#define class_name(p)		(class_obj(p)->_class_name)

#define isobject(p)			(type(p) == T_OBJECT)
#define object_ptr(p)		((pobject)ptrvalue(p))
#define object_class(p)		(object_ptr(p)->_class)
#define object_vslots(p)	(object_ptr(p)->_vslots)

#define need_mark(p)		(type_table[type(p)].mark_func != NULL)
#define do_mark(sc, p)		(type_table[type(p)].mark_func((sc), (p)))
#define need_finalize(p)	(type_table[type(p)].finalize_func != NULL)
#define do_finalize(sc, p)	(type_table[type(p)].finalize_func((sc), (p)))

#define is_macrocache(p)	(typeflag(p) & T_MACROCACHE)
#define set_macrocache(p)	typeflag(p) |= T_MACROCACHE

#define isatom(p)			(typeflag(p) & T_ATOM)
#define setatom(p)			typeflag(p) |= T_ATOM
#define clratom(p)			typeflag(p) &= ~T_ATOM

#define ismark(p)			(typeflag(p) & MARK)
#define setmark(p)			typeflag(p) |= MARK
#define clrmark(p)			typeflag(p) &= ~MARK

#define caar(p)				car(car(p))
#define cadr(p)				car(cdr(p))
#define cdar(p)				cdr(car(p))
#define cddr(p)				cdr(cdr(p))
#define cadar(p)			car(cdr(car(p)))
#define caddr(p)			car(cdr(cdr(p)))
#define cdaar(p)			cdr(car(car(p)))
#define cdadr(p)			cdr(car(cdr(p)))
#define cdddr(p)			cdr(cdr(cdr(p)))
#define cadaar(p)			car(cdr(car(car(p))))
#define cadddr(p)			car(cdr(cdr(cdr(p))))
#define cddddr(p)			cdr(cdr(cdr(cdr(p))))

pcell eval_cycle(oscheme *sc, enum scheme_opcodes op, int interactive);
void _clear_outportbuf(oscheme *sc, pcell p);
void _char_port_out(oscheme *sc, pcell p, och ch);
void _string_port_out(oscheme *sc, pcell p, const TCHAR *str);
void port_out(oscheme *sc, pcell p, TCHAR *str);
const TCHAR *proc_name(enum scheme_opcodes op);

pcell copy_stack_to_heap(oscheme *sc, pcell b);
pcell copy_list(oscheme *sc, pcell b);

int eq(pcell a, pcell b);
int eqv(pcell a, pcell b);
int equal(pcell a, pcell b);

/* ========== Error ========== */
void FatalError(oscheme *sc, const TCHAR *msg)
{
    _stprintf(sc->err_buf, _T("fatal error: %s"), msg);
#ifdef WIN32
	MessageBoxW(NULL, sc->err_buf, _T("scheme fatal error"), MB_ICONERROR | MB_OK);
#else
    port_out(sc, sc->errfp, sc->err_buf);
#endif

#ifdef USE_SETJMP
    longjmp(sc->jmp_env, 1);
#else
    exit(1);
#endif
}

/* ========== memory allocation ========== */
void *oscm_malloc(oscheme *sc, size_t size)
{
#ifdef USE_ALLOC_SET
	void *p = alloc_set_alloc(sc->aset, size);
	if(p == NULL) FatalError(sc, _T("no memory"));

	sc->aset_allocated_size += alloc_set_get_chunk_size(p);

	return p;
#else
	return malloc(size);
#endif
}

void *osm_realloc(oscheme *sc, void *p, size_t size)
{
#ifdef USE_ALLOC_SET
	size_t old_p_size = alloc_set_get_chunk_size(p);

	p = alloc_set_realloc(sc->aset, p, size);
	if(p == NULL) FatalError(sc, _T("no memory"));

	sc->aset_allocated_size -= old_p_size;
	sc->aset_allocated_size += alloc_set_get_chunk_size(p);

	return p;
#else
	return realloc(p, size);
#endif
}

void oscm_free(oscheme *sc, void *p)
{
#ifdef USE_ALLOC_SET
	size_t old_p_size = alloc_set_get_chunk_size(p);
	alloc_set_free(p);
	sc->aset_allocated_size -= old_p_size;
#else
	free(p);
#endif
}

int alloc_cellseg(oscheme *sc, int seg_size)
{
	pcell		p;
	long		i;

	if(sc->last_cell_seg >= CELL_NSEGMENT - 1) {
		FatalError(sc, _T("no memory"));
		return 0;
	}

	p = (pcell)malloc(seg_size * sizeof(ocell));
	if(p == NULL) {
		FatalError(sc, _T("no memory"));
		return 0;
	}

	sc->last_cell_seg += 1;
	sc->all_cell_cnt += seg_size;

	sc->cell_seg[sc->last_cell_seg] = p;
	sc->cell_seg_size[sc->last_cell_seg] = seg_size;

	for(i = 0; i < seg_size; i++, p++) typeflag(p) = 0;

	return 1;
}

void set_cur_seg(oscheme *sc, int seg_idx)
{
	sc->cur_cell_seg = seg_idx;
	sc->next_cell = sc->cell_seg[sc->cur_cell_seg];
	sc->next_cell_last = sc->next_cell + sc->cell_seg_size[sc->cur_cell_seg];
}

/* ========== garbage collector ========== */
void _gc_mark_e(oscheme *sc, pcell a);
#define _gc_mark(sc, a)		if(!ismark(a)) _gc_mark_e(sc, a)

void _gc_mark_continuation(oscheme *sc, pcell p)
{
	ocontinuation *c = getcontinuation(p);
	int		i;

/* 
 現在のstackをバックアップ
 continuationのstackを書き戻す
 gc
 stackをリストア
*/
	for(i = 0; i < c->_dump_stack_cnt; i++) {
		_gc_mark(sc, c->_dump_stack[i].args);
		_gc_mark(sc, c->_dump_stack[i].envir);
		_gc_mark(sc, c->_dump_stack[i].code);
	}
}

void _gc_mark_vector(oscheme *sc, pcell p)
{
	INT_PTR		i;
	INT_PTR		cnt = vector_cnt(p);
	pcell	*arr = vector_arr(p);
	for(i = 0; i < cnt; i++) _gc_mark(sc, arr[i]);
}

void _gc_mark_hash(oscheme *sc, pcell p)
{
	/* mark all key & data */
	ohash_iter iter = ohash_iter_init(hash_hashdata(p));

	if(hash_type(p) == HASH_TYPE_OBLIST) {
		_hash_oblist_entry *entry;
		for(;;) {
			entry = (_hash_oblist_entry *)ohash_iter_next(iter);
			if(entry == NULL) break;
			_gc_mark(sc, entry->_symbol);
		}
	} else {
		_hash_entry *entry;
		for(;;) {
			entry = (_hash_entry *)ohash_iter_next(iter);
			if(entry == NULL) break;
			_gc_mark(sc, entry->_key);
			_gc_mark(sc, entry->_data);
		}
	}

	ohash_iter_delete(iter);
}

void _gc_mark_class(oscheme *sc, pcell p)
{
	_gc_mark(sc, class_supers(p));
	_gc_mark(sc, class_slots(p));
}

void _gc_mark_object(oscheme *sc, pcell p)
{
	int		i, cnt;
	_gc_mark(sc, object_class(p));

	cnt = class_slot_cnt(object_class(p));
	for(i = 0; i < cnt; i++) {
		_gc_mark(sc, object_vslots(p)[i]);
	}
}

void _gc_mark_regmatch(oscheme *sc, pcell p)
{
	_gc_mark(sc, regmatch_str(p));
}

/* --
 * We use algorithm E (Knuth, The Art of Computer Programming Vol.1,
 * sec.3.5) for marking.
 */
void _gc_mark_e(oscheme *sc, pcell a)
{
	pcell		t, q, p;

//E1:
	t = NULL;
	p = a;

E2:
	if(need_mark(p) && !ismark(p)) {
		setmark(p);
		do_mark(sc, p);
	}
	setmark(p);
	if(isatom(p)) goto E6;

//E4:	
	q = car(p);
	if(q && !ismark(q)) {
		setatom(p);
		car(p) = t;
		t = p;
		p = q;
		goto E2;
	}

E5:
	q = cdr(p);
	if(q && !ismark(q)) {
		cdr(p) = t;
		t = p;
		p = q;
		goto E2;
	}

E6:
	if(!t) return;
	q = t;
	if(isatom(q)) {
		clratom(q);
		t = car(q);
		car(q) = p;
		p = q;
		goto E5;
	} else {
		t = cdr(q);
		cdr(q) = p;
		p = q;
		goto E6;
	}
}

void _finalize_cell(oscheme *sc, pcell a)
{
	oscm_free(sc, ptrvalue(a));
}

void _finalize_continuation(oscheme *sc, pcell a)
{
	oscm_free(sc, ptrvalue(a));
}

void _finalize_regexp(oscheme *sc, pcell a)
{
	oreg_free(regexp_hregdata(a));
	oscm_free(sc, ptrvalue(a));
}

void _finalize_hash(oscheme *sc, pcell a)
{
	ohash_delete(hash_hashdata(a));
	oscm_free(sc, ptrvalue(a));
}

void _finalize_hash_iter(oscheme *sc, pcell a)
{
	ohash_iter_delete(hash_iter(a));
}

void _finalize_class(oscheme *sc, pcell a)
{
	if(class_accessers(a) != NULL) oscm_free(sc, class_accessers(a));
	oscm_free(sc, ptrvalue(a));
}

void _finalize_port(oscheme *sc, pcell a)
{
	if(!is_closed_port(a)) {
		FILE *fp = port_fp(a);
		if(fp != NULL && fp != stdin && fp != stdout && fp != stderr) {
			fclose(fp);
		}
	}
	if(port_buf(a) != NULL) oscm_free(sc, port_buf(a));

	oscm_free(sc, ptrvalue(a));
}

static pcell _gc_sweep(oscheme *sc, int all_flg)
{
	pcell	p = sc->next_cell;
	pcell	last_p = sc->next_cell_last;

	for(;;) {
		if(p == last_p) {
			if(sc->cur_cell_seg == sc->last_cell_seg) break;

			set_cur_seg(sc, sc->cur_cell_seg + 1);
			p = sc->next_cell;
			last_p = sc->next_cell_last;
		}
		if(ismark(p)) {
			sc->used_cell_cnt++;
			clrmark(p);
		} else {
			if(need_finalize(p)) do_finalize(sc, p);
			typeflag(p) = 0;
			if(!all_flg) return p;
		}
		p++;
	}

	return NULL;
}

static void _free_unmarked_cell(oscheme *sc)
{
	int idx;
	int last_idx = sc->last_cell_seg;

	for(idx = 0; idx <= last_idx; idx++) {
		pcell	p = sc->cell_seg[idx];
		pcell	last_p = p + sc->cell_seg_size[idx];
		for(;;) {
			if(p == last_p) break;
			if(!ismark(p)) {
				if(need_finalize(p)) do_finalize(sc, p);
				typeflag(p) = 0;
			}
			p++;
		}
	}
}

void gc(oscheme *sc, pcell a, pcell b, int force)
{
	int		i;

	if(sc->gc_verbose) printf("gc...");

	/* sweep all segments */
	if(force) _gc_sweep(sc, 1);

	/* mark system globals */
	_gc_mark(sc, sc->oblist);
	_gc_mark(sc, sc->global_env);
	_gc_mark(sc, sc->global_var);

	/* mark current registers */
	_gc_mark(sc, sc->args);
	_gc_mark(sc, sc->envir);
	_gc_mark(sc, sc->code);
	_gc_mark(sc, sc->value);
	if(sc->values != NULL) _gc_mark(sc, sc->values);

	/* mark port */
	_gc_mark(sc, sc->infp);
	_gc_mark(sc, sc->outfp);
	_gc_mark(sc, sc->errfp);
	_gc_mark(sc, sc->iofp);
	_gc_mark(sc, sc->tmpfp);

	/* mark stack frame */
	for(i = 0; i < sc->cur_frame; i++) {
		_gc_mark(sc, sc->dump_stack[i].args);
		_gc_mark(sc, sc->dump_stack[i].envir);
		_gc_mark(sc, sc->dump_stack[i].code);
	}

	/* mark variables a, b */
	_gc_mark(sc, a);
	_gc_mark(sc, b);

	/* lazy sweep */
	set_cur_seg(sc, 0);
	sc->used_cell_cnt = 0;

	if(force) _free_unmarked_cell(sc);
}

/* ========== cell ========== */
/* get new cell. parameter a, b is marked by gc. */
pcell get_cell(oscheme *sc, pcell a, pcell b)
{
	pcell	x;

#ifdef _GC_DEBUG_
	if(!sc->dont_gc_flg) {
		gc(sc, a, b, 1);
		gc(sc, a, b, 1);
	}
#endif
	if(!sc->dont_gc_flg && sc->aset_allocated_size > sc->aset_gc_threshold_size) {
		gc(sc, a, b, 1);

		if(sc->aset_allocated_size > (sc->aset_gc_threshold_size / 2)) {
			sc->aset_gc_threshold_size *= 2;
		}
	}

	x = _gc_sweep(sc, 0);

	if(x == NULL) {
		if(!sc->dont_gc_flg &&
			(sc->all_cell_cnt - sc->used_cell_cnt) > (sc->all_cell_cnt / 2)) {
			gc(sc, a, b, 0);
			x = _gc_sweep(sc, 0);
		}
		if(x == NULL) {
			alloc_cellseg(sc, CELL_SEGSIZE);
			set_cur_seg(sc, sc->cur_cell_seg + 1);
			x = sc->next_cell;
		}
	}
	sc->next_cell = x + 1;

	return x;
}

/* get new cons cell */
pcell cons(oscheme *sc, pcell a, pcell b)
{
	pcell x = get_cell(sc, a, b);

	typeflag(x) = T_PAIR;
	car(x) = a;
	cdr(x) = b;
	return x;
}

void push_env(oscheme *sc, pcell a, pcell b)
{
	if(ishash(car(sc->envir))) {
		_hash_entry *entry;
		entry = (_hash_entry *)ohash_enter(hash_hashdata(car(sc->envir)), &a);
		if(entry == NULL) {
			FatalError(sc, _T("no memory"));
			return;
		}
		entry->_data = b;
	} else {
		car(sc->envir) = cons(sc, cons(sc, a, b), car(sc->envir));
	}
}

void new_env_frame(oscheme *sc)
{
	sc->envir = cons(sc, sc->NIL, sc->envir);
}

pcell *find_frame(oscheme *sc, pcell f, pcell a)
{
	pcell	y;

	if(ishash(f)) {
		_hash_entry *entry;
		entry = (_hash_entry *)ohash_search(hash_hashdata(f), &a);
		if(entry != NULL) return &(entry->_data);
	} else {
		for(y = f; y != sc->NIL; y = cdr(y)) {
			if(caar(y) == a) return &(cdar(y));
		}
	}
	return NULL;
}

pcell *find_env(oscheme *sc, pcell a)
{
	pcell x;
	pcell *r;

	for(x = sc->envir; x != sc->NIL; x = cdr(x)) {
		r = find_frame(sc, car(x), a);
		if(r != NULL) return r;
	}
	return NULL;
}

/* get eof */
pcell mk_eof(oscheme *sc)
{
	pcell x = get_cell(sc, sc->NIL, sc->NIL);
	typeflag(x) = T_EOF | T_ATOM;
	return x;
}

/* get number atom */
pcell mk_number(oscheme *sc, INT_PTR num)
{
	pcell x = get_cell(sc, sc->NIL, sc->NIL);

	typeflag(x) = T_NUMBER | T_ATOM;
	ivalue(x) = num;
	return x;
}

INT_PTR safe_ivalue(pcell x)
{
	if(x == NULL || !isnumber(x)) return 0;
	return ivalue(x);
}

/* clear input port */
void _clear_inportbuf(oscheme *sc, pcell p)
{
	_tcscpy(port_buf(p), _T("\0"));
	port_cur_buf(p) = port_buf(p);
	port_end_buf(p) = port_cur_buf(p);
}

/* clear input port */
void _clear_outportbuf(oscheme *sc, pcell p)
{
	_tcscpy(port_buf(p), _T("\0"));
	port_cur_buf(p) = port_buf(p);
}

/* create port */
pcell mk_port(oscheme *sc, FILE *fp, int flg, const TCHAR *file_name, size_t buf_size)
{
	pcell	x = get_cell(sc, sc->NIL, sc->NIL);
	size_t	fname_len = _tcslen(file_name);

	typeflag(x) = T_PORT | T_ATOM;
	ptrvalue(x) = oscm_malloc(sc, sizeof(oport) + (fname_len * sizeof(TCHAR)));
	port_fp(x) = fp;
	port_flg(x) = flg;
	port_out_func(x) = NULL;
	_tcscpy(port_fname(x), file_name);

	port_buf_size(x) = buf_size;
	if(buf_size > 0) {
		port_buf(x) = oscm_malloc(sc, buf_size * sizeof(TCHAR));
		_clear_inportbuf(sc, x);
	} else {
		port_buf(x) = NULL;
	}

	return x;
}

pcell mk_string_input_port(oscheme *sc, TCHAR *str)
{
	size_t	len = _tcslen(str);
	pcell	x = mk_port(sc, NULL, PORT_INPORT | PORT_STRING, _T("(string)"), (len + 1));

	_tcscpy(port_cur_buf(x), str);
	port_end_buf(x) = port_cur_buf(x) + len;

	return x;
}

pcell mk_string_output_port(oscheme *sc, int ini_buf_size)
{
	pcell	x = mk_port(sc, NULL, PORT_OUTPORT | PORT_STRING,
		_T("(string)"), ini_buf_size);

	port_end_buf(x) = port_cur_buf(x) + port_buf_size(x);

	return x;
}

int is_port_eof(oscheme *sc, pcell p)
{
	if((is_strport(p) || feof(port_fp(p))) &&
	   port_end_buf(p) == port_cur_buf(p)) return 1;
	return 0;
}

int is_file_exists(TCHAR *filename)
{
	if(_taccess(filename, F_OK) == 0) return 1;
	return 0;
}

int is_full_path_name(TCHAR *filename)
{
#ifdef WIN32
	if(_char_upper_p(filename[0]) &&
		filename[1] == ':' &&
		filename[2] == '\\') return 1;
#else
	if(filename[0] == '/') return 1;
#endif

	return 0;
}

void close_port(oscheme *sc, pcell p)
{
	if(is_closed_port(p)) return;
	if(port_fp(p) != NULL) fclose(port_fp(p));
	port_flg(p) |= PORT_CLOSED;
}

/* create vector */
pcell mk_vector(oscheme *sc, INT_PTR vec_size, int b_init, pcell init_value_list)
{
	pcell	vec = get_cell(sc, sc->NIL, sc->NIL);

	typeflag(vec) = T_VECTOR | T_ATOM;
	ptrvalue(vec) = oscm_malloc(sc, sizeof(ovector) + (sizeof(ocell) * vec_size));
	vector_cnt(vec) = 0;
	vector_size(vec) = vec_size;

	if(b_init) {
		int		i;
		for(i = 0; i < vec_size; i++) {
			vector_arr(vec)[i] = sc->NIL;
		}
		vector_cnt(vec) = vec_size;
	}

	if(init_value_list != NULL) {
		int		i;
		pcell	x;
		pcell	*arr = vector_arr(vec);
			
		for(x = init_value_list, i = 0; ispair(x); x = cdr(x), i++) {
			arr[i] = car(x);
		}
		vector_cnt(vec) = vec_size;
	}

	return vec;
}

/* create hash */
unsigned int cell_hash_eq(const void *key, size_t keysize)
{
	pcell p = *((pcell *)key);
	if(isnumber(p)) {
		INT_PTR i = ivalue(p);
		return ohash_default_hash(&i, sizeof(i));
	}
	if(ischar(p)) {
		och ch = chvalue(p);
		return ohash_default_hash(&ch, sizeof(ch));
	}
	return ohash_default_hash(&p, keysize);
}

unsigned int cell_hash_equal(const void *key, size_t keysize)
{
	pcell p = *((pcell *)key);
	if(isnumber(p)) {
		INT_PTR i = ivalue(p);
		return ohash_default_hash(&i, sizeof(i));
	}
	if(ischar(p)) {
		och ch = chvalue(p);
		return ohash_default_hash(&ch, sizeof(ch));
	}
	if(isstring(p)) {
		TCHAR *s = strvalue(p);
		return ohash_string_hash(&s, 0);
	}
	return type(p);
}

unsigned int cell_hash_string(const void *key, size_t keysize)
{
	pcell p = *((pcell *)key);
	if(isstring(p)) {
		TCHAR *s = strvalue(p);
		return ohash_string_hash(&s, 0);
	}
	return type(p);
}

int cell_match_ptr(void *match_arg, const void *key1, const void *key2,
	size_t keysize)
{
	return !(*(pcell *)key1 == *(pcell *)key2);
}

int cell_match_eq(void *match_arg, const void *key1, const void *key2,
	size_t keysize)
{
	return !eq(*(pcell *)key1, *(pcell *)key2);
}

int cell_match_eqv(void *match_arg, const void *key1, const void *key2,
	size_t keysize)
{
	return !eqv(*(pcell *)key1, *(pcell *)key2);
}

int cell_match_equal(void *match_arg, const void *key1, const void *key2,
	size_t keysize)
{
	return !equal(*(pcell *)key1, *(pcell *)key2);
}

int cell_match_string(void *match_arg, const void *key1, const void *key2,
	size_t keysize)
{
	if(!isstring(*(pcell *)key1) || !isstring(*(pcell *)key2)) return 1;
	return _string_cmp(strvalue(*(pcell *)key1), strvalue(*(pcell *)key2));
}

int cell_match_str(void *match_arg, const void *key1, const void *key2,
	size_t keysize)
{
	return _string_cmp(*(TCHAR **)key1, *(TCHAR **)key2);
}

pcell mk_hash(oscheme *sc, int table_size, TCHAR *type)
{
	pcell	x = get_cell(sc, sc->NIL, sc->NIL);
	_hash_match_func match = NULL;
	_hash_value_func hash_value_func = NULL;
	enum _hash_type t;

	typeflag(x) = T_HASH | T_ATOM;
	ptrvalue(x) = oscm_malloc(sc, sizeof(_hash_table));

	if(_tcscmp(type, _T("eq?")) == 0) {
		match = cell_match_eq;
		hash_value_func = cell_hash_eq;
		t = HASH_TYPE_EQ;
	} else if(_tcscmp(type, _T("eqv?")) == 0) {
		match = cell_match_eqv;
		hash_value_func = cell_hash_eq;
		t = HASH_TYPE_EQV;
	} else if(_tcscmp(type, _T("equal?")) == 0) {
		match = cell_match_equal;
		hash_value_func = cell_hash_equal;
		t = HASH_TYPE_EQUAL;
	} else if(_tcscmp(type, _T("string=?")) == 0) {
		match = cell_match_string;
		hash_value_func = cell_hash_string;
		t = HASH_TYPE_STRING;
	} else if(_tcscmp(type, _T("ptr-eq?")) == 0) {
		match = cell_match_ptr;
		hash_value_func = ohash_default_hash;
		t = HASH_TYPE_PTR;
	} else if(_tcscmp(type, _T("oblist")) == 0) {
		match = cell_match_str;
		hash_value_func = ohash_string_hash;
		t = HASH_TYPE_OBLIST;
	} else {
		FatalError(sc, _T("mk_hash: unsupported type"));
		return NULL;
	}

	hash_type(x) = t;

	if(t == HASH_TYPE_OBLIST) {
		hash_hashdata(x) = ohash_create(table_size,
			sizeof(TCHAR *), sizeof(_hash_oblist_entry), hash_value_func,
			(_hash_alloc_func)oscm_malloc, (_hash_free_func)oscm_free, sc, match, x);
	} else {
		hash_hashdata(x) = ohash_create(table_size,
			sizeof(pcell), sizeof(_hash_entry), hash_value_func,
			(_hash_alloc_func)oscm_malloc, (_hash_free_func)oscm_free, sc, match, x);
	}
	if(hash_hashdata(x) == NULL) FatalError(sc, _T("no memory"));

	return x;
}

pcell mk_hash_iter(oscheme *sc, ohash hash)
{
	pcell	x = get_cell(sc, sc->NIL, sc->NIL);

	typeflag(x) = T_HASH_ITER | T_ATOM;
	ptrvalue(x) = ohash_iter_init(hash);

	return x;
}

/* memory buffer */
pcell mk_memblock(oscheme *sc, size_t size, void *p)
{
	pcell	x = get_cell(sc, sc->NIL, sc->NIL);

	typeflag(x) = T_MEMBLOCK | T_ATOM;
	ptrvalue(x) = oscm_malloc(sc, size);

	if(p) memcpy(ptrvalue(x), p, size);

	return x;
}

/* get new string */
ostr *mk_ostr(oscheme *sc, size_t len)
{
	ostr *p = oscm_malloc(sc, sizeof(ostr) + len * sizeof(TCHAR));
	p->_bufsize = len;
	return p;
}

pcell mk_string_cell(oscheme *sc, size_t len)
{
	pcell x = get_cell(sc, sc->NIL, sc->NIL);

	typeflag(x) = T_STRING | T_ATOM;
	ptrvalue(x) = mk_ostr(sc, len);
	return x;
}

pcell mk_string(oscheme *sc, const TCHAR *str)
{
	pcell s = mk_string_cell(sc, _tcslen(str));
	_tcscpy(strvalue(s), str);
	return s;
}

/* get new char */
pcell mk_char(oscheme *sc, och ch)
{
	pcell x = get_cell(sc, sc->NIL, sc->NIL);

	typeflag(x) = T_CHAR | T_ATOM;
	chvalue(x) = ch;
	return x;
}

/* get new symbol */
pcell search_oblist(oscheme *sc, const TCHAR *name)
{
	_hash_oblist_entry *entry;

	entry = (_hash_oblist_entry *)ohash_search(
		hash_hashdata(sc->oblist), &name);
	if(entry == NULL) return NULL;
	return entry->_symbol;
}

pcell mk_symbol(oscheme *sc, const TCHAR *name)
{
	_hash_oblist_entry *entry;
	pcell x;

	/* first check oblist */
	x = search_oblist(sc, name);
	if(x != NULL) return x;

	x = get_cell(sc, sc->NIL, sc->NIL);
	typeflag(x) = T_SYMBOL | T_ATOM;
	ptrvalue(x) = oscm_malloc(sc, sizeof(osymbol) + ((_tcslen(name) + 1) * sizeof(TCHAR)));
	symname(x) = (TCHAR *)((char *)ptrvalue(x) + sizeof(osymbol));
	_tcscpy(symname(x), name);
	symnum(x) = 0;

	entry = (_hash_oblist_entry *)ohash_enter(hash_hashdata(sc->oblist),
		&symname(x));
	if(entry == NULL) {
		FatalError(sc, _T("no memory"));
		return NULL;
	}
	entry->_symbol = x;

	return x;
}

/* make symbol or number atom from string */
pcell mk_atom(oscheme *sc, TCHAR *str)
{
	TCHAR	c;
	TCHAR	*p = str;

	c = _get_char(p);
	p = _next_char(p);
	if(!_isdigit(c)) {
		if(c != '+' && c != '-') return mk_symbol(sc, str);
		if(!_isdigit(_get_char(p))) return mk_symbol(sc, str);
	}

	for(;; p = _next_char(p)) {
		c = _get_char(p);
		if(c == '\0') break;
		if(!_isdigit(c)) return mk_symbol(sc, str);
	}

	return mk_number(sc, _ttoi(str));
}

/* create callback func */
pcell mk_callback_func(oscheme *sc, TCHAR *name, oscheme_callback_func fp,
	int min_args, int max_args, TCHAR *type_chk_str)
{
	pcell	s;
	pcell	x;

	s = mk_symbol(sc, name);
	x = get_cell(sc, s, sc->NIL);

	typeflag(x) = T_CB_FUNC | T_ATOM;
	ptrvalue(x) = oscm_malloc(sc, sizeof(ocbfunc) + _tcslen(name) * sizeof(TCHAR));
	cb_func_fp(x) = fp;
	cb_func_min_args(x) = min_args;
	cb_func_max_args(x) = max_args;
	_tcscpy(cb_func_type_chk_str(x), type_chk_str);
	_tcscpy(cb_func_name(x), name);

	push_env(sc, s, x);

	return x;
}

/* create regex object */
pcell mk_regexp(oscheme *sc, TCHAR *pattern, int ci_flg)
{
	HREG_DATA	regdata;
	pcell	x;

	regdata = oreg_comp(pattern, ci_flg);
	if(regdata == NULL) {
		_stprintf(sc->err_buf, _T("invalid regexp pattern"));
		return NULL;
	}

	x = get_cell(sc, sc->NIL, sc->NIL);

	typeflag(x) = T_REGEXP | T_ATOM;
	ptrvalue(x) = oscm_malloc(sc, sizeof(oregexp_data) + _tcslen(pattern) * sizeof(TCHAR));
	regexp_hregdata(x) = regdata;
	regexp_ci(x) = ci_flg;
	_tcscpy(regexp_pattern(x), pattern);

	return x;
}

/* make regmatch object */
pcell mk_regmatch(oscheme *sc, pcell r, pcell s)
{
	pcell	x = get_cell(sc, sc->NIL, sc->NIL);
	int		submatch_cnt = oreg_get_backref_cnt(regexp_hregdata(r));
	int		i;
	OREG_POINT	start_pt, end_pt;

	typeflag(x) = T_REGMATCH | T_ATOM;
	ptrvalue(x) = oscm_malloc(sc, sizeof(oregmatch) +
		submatch_cnt * sizeof(oregmatch_data));

	regmatch_cnt(x) = submatch_cnt + 1;
	regmatch_str(x) = s;

	oreg_get_match_data(regexp_hregdata(r), &start_pt, &end_pt);
	regmatch_data(x)[0]._start_pt = start_pt.col;
	regmatch_data(x)[0]._end_pt = end_pt.col;

	for(i = 1; i <= submatch_cnt; i++) {
		oreg_get_backref_data(regexp_hregdata(r), i - 1, &start_pt, &end_pt);
		regmatch_data(x)[i]._start_pt = start_pt.col;
		regmatch_data(x)[i]._end_pt = end_pt.col;
	}

	return x;
}

/* make constant */
pcell mk_const(oscheme *sc, TCHAR *name)
{
	TCHAR	tmp[256];
	long	x;

	if(*name == '\\') {
		och		ch;
		name = _next_char(name);

		if(_string_cmp_ci(name, _T("space")) == 0) {
			ch = ' ';
		} else if(_string_cmp_ci(name, _T("newline")) == 0) {
			ch = '\n';
		} else if(_string_cmp_ci(name, _T("return")) == 0) {
			ch = '\r';
		} else if(_string_cmp_ci(name, _T("tab")) == 0) {
			ch = '\t';
		} else {
			ch = _get_char(name);
		}
		return mk_char(sc, ch);
	}
	if(_tcscmp(name, _T("t")) == 0) return sc->T;
	if(_tcscmp(name, _T("f")) == 0) return sc->F;
	if(_tcscmp(name, _T("o")) == 0) { /* #o (octal) */
		_stprintf(tmp, _T("0%s"), &name[1]);
		_stscanf(tmp, _T("%lo"), &x);
		return mk_number(sc, x);
	}
	if(_tcscmp(name, _T("o")) == 0) { /* #d (decimal) */
		_stscanf(&name[1], _T("%ld"), &x);
		return mk_number(sc, x);
	}
	if(_tcscmp(name, _T("x")) == 0) { /* #x (hex) */
		_stprintf(tmp, _T("0x%s"), &name[1]);
		_stscanf(tmp, _T("%lx"), &x);
		return mk_number(sc, x);
	}
	return sc->NIL;
}

/* ========== routines for reading ========== */
enum _token {
	TOK_LPAREN,
	TOK_RPAREN,
	TOK_DOT,
	TOK_ATOM,
	TOK_QUOTE,
	TOK_QUOTE_VEC,
	TOK_COMMENT,
	TOK_DQUOTE,
	TOK_BQUOTE,
	TOK_BQUOTE_VEC,
	TOK_COMMA,
	TOK_ATMARK,
	TOK_SHARP,
	TOK_BLOCK_COMMENT,
	TOK_REGEXP,
	TOK_EOF,
};

static void _prompt(oscheme *sc)
{
	printf(prompt);
}

static void _fill_in_buf(oscheme *sc, pcell in, size_t ch_cnt)
{
	size_t		read_cnt;
	memcpy(port_buf(in), port_cur_buf(in), ch_cnt * sizeof(TCHAR));

#ifdef _UNICODE
	if(!(port_flg(in) & PORT_UNICODE)) {
		size_t buf_read_cnt;
		size_t buf_len = (port_buf_size(in) - ch_cnt) / 4 - 1;
		unsigned char buf[INPORT_READ_SIZE];

		if(buf_len <= 0) buf_len = 10;
		if(buf_len >= sizeof(buf)) buf_len = sizeof(buf) - 1;

		buf_read_cnt = fread(buf, sizeof(unsigned char), buf_len, port_fp(in));
		buf[buf_read_cnt] = '\0';

		{
			wchar_t *wp = port_buf(in) + ch_cnt;
			unsigned char *p = buf;
			int mbsize;
			wchar_t *wps = wp;

			for(; *p;) {
				if(*p >= 0x80 && *(p + 1) == '\0') {
					fseek(port_fp(in), -1, SEEK_CUR);
					break;
				}
				mbsize = mbtowc(wp, p, MB_CUR_MAX);
				if(mbsize == -1) {
					p++;
					continue;
				}
				wp++;
				p += mbsize;
			}
			*wp = L'\0';

			port_cur_buf(in) = port_buf(in);
			port_end_buf(in) = wp;
		}
		return;
	}
#endif /* _UNICODE */

	read_cnt = fread(port_buf(in) + ch_cnt, sizeof(TCHAR), port_buf_size(in) - ch_cnt, port_fp(in));

	port_cur_buf(in) = port_buf(in);
	port_end_buf(in) = port_buf(in) + ch_cnt + read_cnt;
}

static och _read_in_buf(oscheme *sc)
{
	pcell	in = sc->infp;
	och		ch;
	
	if(!isport(in) || !is_inport(in) || is_closed_port(in)) return '\0';

	if(port_fp(in) == stdin) {
		if(port_end_buf(in) == port_cur_buf(in)) {
			// FIXME: _fgettsの2番目の引数がintのためCASTしている
			if(_fgetts(port_buf(in), (int)port_buf_size(in), port_fp(in)) == NULL) {
				return '\0';
			}
			port_cur_buf(in) = port_buf(in);
			port_end_buf(in) = port_buf(in) + _tcslen(port_buf(in));
		}
	} else {
		if(port_end_buf(in) - port_cur_buf(in) < FILL_INPORT_THR) {
			if(!is_strport(in) && !feof(port_fp(in))) {
				_fill_in_buf(sc, in, port_end_buf(in) - port_cur_buf(in));
			}
			if(port_end_buf(in) == port_cur_buf(in)) return '\0';
		}
	}

	ch = _get_char(port_cur_buf(in));
	return ch;
}

static void _next_in_buf(oscheme *sc)
{
	if(port_cur_buf(sc->infp) == port_end_buf(sc->infp)) return;
	port_cur_buf(sc->infp) = _next_char(port_cur_buf(sc->infp));
}

static och _read_next_buf(oscheme *sc)
{
	if(port_cur_buf(sc->infp) == port_end_buf(sc->infp)) return '\0';
	return _get_char(_next_char(port_cur_buf(sc->infp)));
}

/* read next line (for comment) */
static void _skip_comment(oscheme *sc)
{
	for(;;) {
		och ch = _read_in_buf(sc);
		_next_in_buf(sc);
		if(ch == '\n' || ch == '\0') break;
	}
}

static void _skip_block_comment(oscheme *sc)
{
	int		nested_level = 1;

	for(;;) {
		och ch = _read_in_buf(sc);
		if(ch == '#') {
			_next_in_buf(sc);
			if(_read_in_buf(sc) == '|') nested_level++;
		} else if(ch == '|') {
			_next_in_buf(sc);
			if(_read_in_buf(sc) == '#') {
				nested_level--;
				if(nested_level == 0) {
					_next_in_buf(sc);
					break;
				}
			}
		}
		_next_in_buf(sc);
		if(ch == '\0') break;
	}
}

/* read characters to delimiter */
TCHAR *_readstr(oscheme *sc)
{
	pcell	oport = sc->iofp;
	och		ch;

	_clear_outportbuf(sc, oport);

	for(;;) {
		ch = _read_in_buf(sc);
		if(ch == '\0' || ch == '(' || ch == ')' || ch == ';' ||
			_isspace(ch)) break;
		_char_port_out(sc, oport, ch);
		_next_in_buf(sc);
	}
	_char_port_out(sc, oport, '\0');
	return port_buf(oport);
}

/* read string expression "xxx...xxx" */
TCHAR *_readstrexp(oscheme *sc, och stop_char)
{
	pcell	oport = sc->iofp;
	och		ch;

	_clear_outportbuf(sc, oport);

	for(;;) {
		ch = _read_in_buf(sc);
		_next_in_buf(sc);

		if(ch == '\0' || ch == stop_char) break;

		if(ch == '\\') {
			ch = _read_in_buf(sc);
			_next_in_buf(sc);
			if(ch == 'n') {
				_char_port_out(sc, oport, '\n');
			} else if(ch == 'r') {
				_char_port_out(sc, oport, '\r');
			} else if(ch == 't') {
				_char_port_out(sc, oport, '\t');
			} else {
				_char_port_out(sc, oport, ch);
			}
		} else {
			_char_port_out(sc, oport, ch);
		}
	}
	_char_port_out(sc, oport, '\0');
	return port_buf(oport);
}

/* read string for regexp */
TCHAR *_readstr_regexp(oscheme *sc, och stop_char)
{
	pcell	oport = sc->iofp;
	och		ch;

	_clear_outportbuf(sc, oport);

	for(;;) {
		ch = _read_in_buf(sc);
		_next_in_buf(sc);

		if(ch == '\0' || ch == stop_char) break;

		if(ch == '\\') {
			ch = _read_in_buf(sc);
			_next_in_buf(sc);

			if(ch != '/') {
				_char_port_out(sc, oport, '\\');
			}
			if(ch == '\0') break;
			_char_port_out(sc, oport, ch);
		} else {
			_char_port_out(sc, oport, ch);
		}
	}
	_char_port_out(sc, oport, '\0');
	return port_buf(oport);
}

/* read string for sharp */
TCHAR *_readstr_sharp(oscheme *sc)
{
	pcell	oport = sc->iofp;
	och		ch;

	_clear_outportbuf(sc, oport);

	if(_read_in_buf(sc) == '\\') {
		ch = _read_in_buf(sc);
		_char_port_out(sc, oport, ch);
		_next_in_buf(sc);

		ch = _read_in_buf(sc);
		_char_port_out(sc, oport, ch);
		_next_in_buf(sc);
	}

	for(;;) {
		ch = _read_in_buf(sc);
		if(ch == '\0' || ch == '(' || ch == ')' || ch == ';' ||
			_isspace(ch)) break;
		_char_port_out(sc, oport, ch);
		_next_in_buf(sc);
	}
	_char_port_out(sc, oport, '\0');
	return port_buf(oport);
}

/* skip white characters */
och _skipspace(oscheme *sc)
{
	och		ch;

	for(;;) {
		ch = _read_in_buf(sc);
		if(!_isspace(ch)) return ch;
		_next_in_buf(sc);
	}
}

int _get_token(oscheme *sc)
{
	och ch = _skipspace(sc);

	switch(ch) {
	case '\0':
		return TOK_EOF;
	case '(':
		_next_in_buf(sc);
		return TOK_LPAREN;
	case ')':
		_next_in_buf(sc);
		return TOK_RPAREN;
	case '.':
		if(_isspace(_read_next_buf(sc))) {
			_next_in_buf(sc);
			return TOK_DOT;
		} else {
			return TOK_ATOM;
		}
	case '\'':
		_next_in_buf(sc);
		if(_read_in_buf(sc) == '#' && _read_next_buf(sc) == '(') {
			_next_in_buf(sc);
			return TOK_QUOTE_VEC;
		}
		return TOK_QUOTE;
	case ';':
		_next_in_buf(sc);
		return TOK_COMMENT;
	case '"':
		_next_in_buf(sc);
		return TOK_DQUOTE;
	case '`':
		_next_in_buf(sc);
		if(_read_in_buf(sc) == '#' && _read_next_buf(sc) == '(') {
			_next_in_buf(sc);
			return TOK_BQUOTE_VEC;
		}
		return TOK_BQUOTE;
	case ',':
		_next_in_buf(sc);
		if(_read_in_buf(sc) == '@') {
			_next_in_buf(sc);
			return TOK_ATMARK;
		} else {
			return TOK_COMMA;
		}
	case '#':
		_next_in_buf(sc);
		if(_read_in_buf(sc) == '(') {
			return TOK_QUOTE_VEC;
		}
		if(_read_in_buf(sc) == '!') {
			return TOK_COMMENT;
		}
		if(_read_in_buf(sc) == '/') {
			_next_in_buf(sc);
			return TOK_REGEXP;
		}
		if(_read_in_buf(sc) == '|') {
			_next_in_buf(sc);
			return TOK_BLOCK_COMMENT;
		}
		return TOK_SHARP;
	default:
		return TOK_ATOM;
	}
}

/* ========== Rootines for Printing ========== */
#define	ok_abbrev(sc, x)	(ispair(x) && cdr(x) == (sc)->NIL)

void _alloc_string_port(oscheme *sc, pcell p, size_t len)
{
	if(port_cur_buf(p) + len >= port_end_buf(p)) {
		size_t cur_len;
		size_t need_size = port_end_buf(p) - port_cur_buf(p) + len;
		size_t buf_size = port_buf_size(p) * 2;
		for(; need_size > buf_size;) buf_size *= 2;
		
		/* realloc buffer */
		cur_len = port_cur_buf(p) - port_buf(p);
		port_buf(p) = osm_realloc(sc, port_buf(p), buf_size * sizeof(TCHAR));
		port_cur_buf(p) = port_buf(p) + cur_len;
		port_end_buf(p) = port_cur_buf(p) + port_buf_size(p);
		port_buf_size(p) = buf_size;
	}
}

void _char_port_out(oscheme *sc, pcell p, och ch)
{
	int len = _get_char_len2(ch);
	_alloc_string_port(sc, p, len);
	port_cur_buf(p) = _put_char(port_cur_buf(p), ch);
}

void _string_port_out(oscheme *sc, pcell p, const TCHAR *str)
{
	size_t len = _tcslen(str);
	_alloc_string_port(sc, p, len);
	_tcscat(port_cur_buf(p), str);
	port_cur_buf(p) = port_cur_buf(p) + len;

	{
		TCHAR *z1 = port_buf(p);
		TCHAR *z2 = port_cur_buf(p);
	}
}

void port_out(oscheme *sc, pcell p, TCHAR *str)
{
	if(!isport(p) || !is_outport(p) || is_closed_port(p)) return;

	if(is_stringport(p)) {
		_string_port_out(sc, p, str);
	} else if(port_out_func(p)) {
		port_out_func(p)(sc, str);
	} else if(port_fp(p) != NULL) {
		_fputts(str, port_fp(p));
	}
}

TCHAR *strunquote(oscheme *sc, TCHAR *s)
{
	pcell	oport = sc->iofp;
	och		ch;

	_clear_outportbuf(sc, oport);
	_char_port_out(sc, oport, '"');

	for ( ; ch = _get_char(s); s = _next_char(s)) {
		if(ch == '"') {
			_char_port_out(sc, oport, '\\');
			_char_port_out(sc, oport, '"');
		} else if(ch == '\n') {
			_char_port_out(sc, oport, '\\');
			_char_port_out(sc, oport, 'n');
		} else if(ch == '\r') {
			_char_port_out(sc, oport, '\\');
			_char_port_out(sc, oport, 'r');
		} else if(ch == '\t') {
			_char_port_out(sc, oport, '\\');
			_char_port_out(sc, oport, 't');
		} else if(ch == '\\') {
			_char_port_out(sc, oport, '\\');
			_char_port_out(sc, oport, '\\');
		} else {
			_char_port_out(sc, oport, ch);
		}
	}

	_char_port_out(sc, oport, '"');
	_char_port_out(sc, oport, '\0');
	return port_buf(oport);
}

/* print atoms */
size_t printatom(oscheme *sc, pcell l, int f)
{
	TCHAR	*p;
	TCHAR	buf[100];

	if(l == sc->NIL) {
		p = _T("()");
	} else if(l == sc->T) {
		p = _T("#t");
	} else if(l == sc->F) {
		p = _T("#f");
	} else if(l == sc->UNDEF) {
		p = _T("#<undef>");
	} else if(isnumber(l)) {
		p = buf;
		_stprintf(p, _T("%Id"), ivalue(l));
	} else if(isstring(l)) {
		if(!f) {
			p = strvalue(l);
		} else {
			p = strunquote(sc, strvalue(l));
		}
	} else if(ischar(l)) {
		if(!f) {
			TCHAR *pbuf = buf;
			pbuf = _put_char(pbuf, chvalue(l));
			pbuf = _put_char(pbuf, '\0');
			p = buf;
		} else {
			if(chvalue(l) == '\n') {
				p =  _T("#\\newline");
			} else if(chvalue(l) == '\r') {
				p = _T("#\\return");
			} else if(chvalue(l) == ' ') {
				p = _T("#\\space");
			} else if(chvalue(l) == '\t') {
				p = _T("#\\tab");
			} else {
				TCHAR *pbuf = buf;
				pbuf = _put_char(pbuf, '#');
				pbuf = _put_char(pbuf, '\\');
				pbuf = _put_char(pbuf, chvalue(l));
				pbuf = _put_char(pbuf, '\0');
				p = buf;
			}
		}
	} else if(issymbol(l)) {
		p = symname(l);
	} else if(issyntax(l)) {
		p = _T("#<syntax>");
	} else if(isproc(l)) {
		_clear_outportbuf(sc, sc->iofp);
		_string_port_out(sc, sc->iofp, _T("#<subr "));
		_string_port_out(sc, sc->iofp, proc_name(procnum(l)));
		_string_port_out(sc, sc->iofp, _T(">"));
		p = port_buf(sc->iofp);
	} else if(ismacro(l)) {
		p = _T("#<macro>");
	} else if(isclosure(l)) {
		p = _T("#<closure>");
	} else if(iscontinuation(l)) {
		p = _T("#<continuation>");
	} else if(is_cb_func(l)) {
		_clear_outportbuf(sc, sc->iofp);
		_string_port_out(sc, sc->iofp, _T("#<callback function "));
		_string_port_out(sc, sc->iofp, cb_func_name(l));
		_string_port_out(sc, sc->iofp, _T(">"));
		p = port_buf(sc->iofp);
	} else if(isport(l)) {
		_clear_outportbuf(sc, sc->iofp);
		if(is_outport(l)) {
			_string_port_out(sc, sc->iofp, _T("#<oport"));
		} else {
			_string_port_out(sc, sc->iofp, _T("#<iport"));
		}
		if(is_closed_port(l)) {
			_string_port_out(sc, sc->iofp, _T("(closed)"));
		}
		_string_port_out(sc, sc->iofp, _T(" "));
		_string_port_out(sc, sc->iofp, port_fname(l));
		_string_port_out(sc, sc->iofp, _T(">"));
		p = port_buf(sc->iofp);
	} else if(iseof_object(l)) {
		p = _T("#<eof>");
	} else if(isregexp(l)) {
		_clear_outportbuf(sc, sc->iofp);
		_string_port_out(sc, sc->iofp, _T("#/"));
		//_string_port_out(sc, sc->iofp, regexp_pattern(l));
		{
			TCHAR *s = regexp_pattern(l);
			och ch;
			for(;; s = _next_char(s)) {
				ch = _get_char(s);
				if(ch == '\0') break;
				if(ch == '/') _char_port_out(sc, sc->iofp, '\\');
				_char_port_out(sc, sc->iofp, ch);
			}
			_put_char(port_cur_buf(sc->iofp), '\0');
		}
		_string_port_out(sc, sc->iofp, _T("/"));
		if(regexp_ci(l)) {
			_string_port_out(sc, sc->iofp, _T("i"));
		}
		p = port_buf(sc->iofp);
	} else if(isregmatch(l)) {
		p = _T("#<regmatch>");
	} else if(ishash(l)) {
		TCHAR *hash_type = _T("unknown");
		switch(hash_type(l)) {
		case HASH_TYPE_EQ: hash_type = _T("eq?"); break;
		case HASH_TYPE_EQV: hash_type = _T("eqv?"); break;
		case HASH_TYPE_EQUAL: hash_type = _T("equal?"); break;
		case HASH_TYPE_STRING: hash_type = _T("string=?"); break;
		}
		p = buf;
		_stprintf(p, _T("#<hash-table %s>"), hash_type);
	} else if(isclass(l)) {
		_clear_outportbuf(sc, sc->iofp);
		_string_port_out(sc, sc->iofp, _T("#<class "));
		_string_port_out(sc, sc->iofp, class_name(l));
		_string_port_out(sc, sc->iofp, _T(">"));
		p = port_buf(sc->iofp);
	} else if(isobject(l)) {
		_clear_outportbuf(sc, sc->iofp);
		_string_port_out(sc, sc->iofp, _T("#<"));
		_string_port_out(sc, sc->iofp, class_name(object_class(l)));
		_string_port_out(sc, sc->iofp, _T(" "));
		_stprintf(buf, _T("0x%p"), l);
		_string_port_out(sc, sc->iofp, buf);
		_string_port_out(sc, sc->iofp, _T(">"));
		p = port_buf(sc->iofp);
	} else {
		p = _T("#<unknown>");
	}

	if(f < 0) return _tcslen(p);

	port_out(sc, sc->outfp, p);
	return 0;
}

/* ========== Routines for Evaluation Cycle ========== */

/* make closure */
pcell mk_closure(oscheme *sc, pcell code, pcell env)
{
	pcell	x = get_cell(sc, code, env);

	typeflag(x) = T_CLOSURE;
	car(x) = code;
	cdr(x) = env;
	return x;
}

/* make continuation */
pcell mk_continuation(oscheme *sc)
{
	pcell	x;
	ocontinuation *c;
	int		i;

	x = get_cell(sc, sc->NIL, sc->NIL);

	typeflag(x) = T_CONTINUATION | T_ATOM;
	ptrvalue(x) = oscm_malloc(sc, sizeof(ocontinuation) +
		sizeof(ostackframe) * sc->cur_frame);

	c = getcontinuation(x);

	memcpy(c->_dump_stack, sc->dump_stack, sizeof(ostackframe) * sc->cur_frame);
	c->_dump_stack_cnt = sc->cur_frame;

	oscheme_set_dont_gc_flg(sc);
	for(i = 0; i < sc->cur_frame; i++) {
		c->_dump_stack[i].args = copy_list(sc, c->_dump_stack[i].args);
	}
	oscheme_clear_dont_gc_flg(sc);

	return x;
}

/* restore continuation */
void dump_continuation(oscheme *sc, pcell x)
{
	ocontinuation *c = getcontinuation(x);
	int	i;

	memcpy(sc->dump_stack, c->_dump_stack,
		sizeof(ostackframe) * c->_dump_stack_cnt);
	sc->cur_frame = c->_dump_stack_cnt;

	oscheme_set_dont_gc_flg(sc);
	for(i = 0; i < sc->cur_frame; i++) {
		sc->dump_stack[i].args = copy_list(sc, sc->dump_stack[i].args);
	}
	oscheme_clear_dont_gc_flg(sc);
}

/* reverse list -- make new cells */
/* a must be checked by gc */
pcell reverse(oscheme *sc, pcell a)
{
	pcell	p = sc->NIL;
	for(; ispair(a); a = cdr(a)) {
		/* protect a from the collection by GC */
		p = cons(sc, a, p);
		car(p) = car(a);
	}
	return p;
}

/* reverse list -- no make new cells */
pcell reverse_in_place(oscheme *sc, pcell term, pcell list)
{
	pcell	p = list;
	pcell	r = term;
	pcell	q;

	while(p != sc->NIL) {
		q = cdr(p);
		cdr(p) = r;
		r = p;
		p = q;
	}
	return r;
}

/* copy list -- make new cells */
pcell copy_list(oscheme *sc, pcell b)
{
	pcell	n = sc->NIL;

	for(; b != sc->NIL; b = cdr(b)) {
		n = cons(sc, car(b), n);
	}
	n = reverse_in_place(sc, sc->NIL, n);

	return n;
}

/* append list -- make new cells */
pcell append(oscheme *sc, pcell a, pcell b)
{
	pcell	p = b;
	pcell	q;

	if(a != sc->NIL) {
		a = reverse(sc, a);
		while(a != sc->NIL) {
			q = cdr(a);
			cdr(a) = p;
			p = a;
			a = q;
		}
	}

	return p;
}

/* count of list length */
int list_length(oscheme *sc, pcell list)
{
	pcell	x = list;
	pcell	slow = list;
	int		len = 0;

	for(;;) {
		if(x == sc->NIL) break;
		if(!ispair(x)) return -1;

		x = cdr(x);
		len++;
		if(x == sc->NIL) break;
		if(!ispair(x)) return -1;

		x = cdr(x);
		slow = cdr(slow);
		if(x == slow) return -2;

		len++;
	}

	return len;
}

int islist(oscheme *sc, pcell x)
{
	if(list_length(sc, x) >= 0) return 1;
	return 0;
}

/* equivalence of strings */
int string_length(TCHAR *s)
{
	int		len;
	for(len = 0; _get_char(s) != '\0'; s = _next_char(s), len++) ;
	return len;
}

int string_length_range(TCHAR *s, TCHAR *end)
{
	int		len;
	for(len = 0; s < end && _get_char(s) != '\0'; s = _next_char(s), len++) ;
	return len;
}

size_t string_length_byte(TCHAR *s)
{
	return _tcslen(s) * sizeof(TCHAR);
}

/* equivalence of atoms */
int eq(pcell a, pcell b)
{
	if(isnumber(a)) return (isnumber(b) && ivalue(a) == ivalue(b));
	if(ischar(a)) return (ischar(b) && chvalue(a) == chvalue(b));
	return (a == b);
}

int eqv(pcell a, pcell b)
{
	return eq(a, b);
}

int vector_equal(pcell a, pcell b)
{
	INT_PTR		i, len;
	pcell	*arr_a, *arr_b;

	if(vector_cnt(a) != vector_cnt(b)) return 0;

	len = vector_cnt(a);
	arr_a = vector_arr(a);
	arr_b = vector_arr(b);

	for(i = 0; i < len; i++) {
		if(!equal(arr_a[i], arr_b[i])) return 0;
	}
	return 1;
}

int equal(pcell a, pcell b)
{
	if(ispair(a)) {
		return (ispair(b) && equal(car(a), car(b)) && equal(cdr(a), cdr(b)));
	}
	if(isstring(a)) {
		return (isstring(b) && _string_cmp(strvalue(a), strvalue(b)) == 0);
	}
	if(isvector(a)) {
		return (isvector(b) && vector_equal(a, b));
	}
	return eqv(a, b);
}

/* true or false value macro */
#define istrue(sc, p)	((p) != sc->F)
#define isfalse(sc, p)	((p) == sc->F)

/* Error macro */
#define Error_0(sc, s) do {									\
	sc->args = cons(sc, mk_string(sc, (s)), sc->NIL);		\
	sc->operator = OP_ERR0;									\
	return sc->T;											\
} while(0)

#define Error_1(sc, s, a) do {								\
	sc->args = cons(sc, (a), sc->NIL);						\
	sc->args = cons(sc, mk_string(sc, (s)), sc->args);		\
	sc->operator = OP_ERR0;									\
	return sc->T;											\
} while(0)

/* control macros for eval_cycle */
#define s_goto(sc, a) do {										\
	sc->operator = (a);											\
	return sc->T;												\
} while(0)

static void s_save(oscheme *sc, enum scheme_opcodes op, pcell args, pcell code)
{
	struct _st_dump_stack_frame *next_frame;

	if(sc->cur_frame >= sc->dump_stack_cnt) {
		sc->dump_stack_cnt *= 2;
		if(sc->dump_stack_cnt == 0) sc->dump_stack_cnt = 32;

		sc->dump_stack = (struct _st_dump_stack_frame *)osm_realloc(
				sc, sc->dump_stack,
				sizeof(struct _st_dump_stack_frame) * sc->dump_stack_cnt);
	}

	next_frame = &(sc->dump_stack[sc->cur_frame]);
	next_frame->operator = op;
	next_frame->args = args;
	next_frame->envir = sc->envir;
	next_frame->code = code;

	sc->cur_frame++;
}

static pcell _s_return_values(oscheme *sc, pcell a, pcell values)
{
	struct _st_dump_stack_frame *frame;

	sc->cur_frame--;
	frame = &(sc->dump_stack[sc->cur_frame]);
	
	sc->value = a;
	sc->values = values;
	sc->operator = frame->operator;
	sc->args = frame->args;
	sc->envir = frame->envir;
	sc->code = frame->code;

	return sc->T;
}

static pcell _s_return(oscheme *sc, pcell a)
{
	return _s_return_values(sc, a, NULL);
}

static void _dump_stack_reset(oscheme *sc)
{
	sc->cur_frame = 0;
}

#define s_return(sc, a) return _s_return(sc, a);
#define s_retbool(sc, tf)	s_return(sc, (tf) ? sc->T : sc->F)

/* ========== get arguments ========== */
#define GET_ARG_TYPE_CHK_ERR		1
#define GET_ARG_FEW_ARGUMENTS_ERR	2

#define _type_chk_err(_sc, _type_name, _name) do {					\
	_stprintf(sc->err_buf, _T("%s: %s required:"), _name, _type_name);	\
	return GET_ARG_TYPE_CHK_ERR;									\
} while(0)

int _type_chk(oscheme *sc, TCHAR *tc_flg, pcell arg, const TCHAR *name)
{
	switch(*tc_flg) {
	case 'C':
		if(!ischar(arg)) _type_chk_err(sc, _T("character"), name);
		break;
	case 'c':
		if(!isclosure(arg)) _type_chk_err(sc, _T("closure"), name);
		break;
	case 'p':
		if(!isclosure(arg) && !isproc(arg))
			_type_chk_err(sc, _T("procedure"), name);
		break;
	case 'N':
		if(!isnumber(arg))  _type_chk_err(sc, _T("number"), name);
		break;
	case '+':
		if(!isnumber(arg) || ivalue(arg) < 0) {
			_type_chk_err(sc, _T("positive number"), name);
		}
		break;
	case 'S':
		if(!isstring(arg)) _type_chk_err(sc, _T("string"), name);
		break;
	case 's':
		if(!isstring(arg) && !isclosure(arg) && !isproc(arg))
			_type_chk_err(sc, _T("string or procedure"), name);
		break;
	case 'Y':
		if(!issymbol(arg)) _type_chk_err(sc, _T("symbol"), name);
		break;
	case 'P':
		if(!ispair(arg)) _type_chk_err(sc, _T("pair"), name);
		break;
	case 'V':
		if(!isvector(arg)) _type_chk_err(sc, _T("vector"), name);
		break;
	case 'I':
		if(!isport(arg) || !is_inport(arg))
			_type_chk_err(sc, _T("input port"), name);
		break;
	case 'O':
		if(!isport(arg) || !is_outport(arg))
			_type_chk_err(sc, _T("output port"), name);
		break;
	case 'o':
		if(!isport(arg) || !is_outport(arg) || !is_stringport(arg))
			_type_chk_err(sc, _T("output string port"), name);
		break;
	case 'T':
		if(!isport(arg)) _type_chk_err(sc, _T("port"), name);
		break;
	case 'L':
		if(!islist(sc, arg)) _type_chk_err(sc, _T("list"), name);
		break;
	case 'M':
		if(!isregmatch(arg) && !isfalse(sc, arg))
			_type_chk_err(sc, _T("regmatch"), name);
		break;
	case 'R':
		if(!isregexp(arg)) _type_chk_err(sc, _T("regexp"), name);
		break;
	case 'H':
		if(!ishash(arg)) _type_chk_err(sc, _T("hash-table"), name);
		break;
	case 'k':
		if(!isclass(arg)) _type_chk_err(sc, _T("class"), name);
		break;
	case 'b':
		if(!isobject(arg)) _type_chk_err(sc, _T("object"), name);
		break;
	}
	return 0;
}

int _get_args(oscheme *sc, const TCHAR *name, int min_argc, int max_argc,
	TCHAR *type_chk_str, pcell *argv, int *argc)
{
	int 	i;
	pcell	x = (sc)->args;
	TCHAR	*t = type_chk_str;

	for(i = 0; i < max_argc; i++) {
		if(!ispair(x)) break;
		if(*t == 'D') {	/* last doted args */
			argv[i] = x;
			i++;
			break;
		}
		if(*t != '\0') {
			if(_type_chk(sc, t, car(x), name) != 0) {
				argv[0] = car(x);
				return GET_ARG_TYPE_CHK_ERR;
			}
			t++;
		}
		argv[i] = car(x);
		x = cdr(x);
	}
	*argc = i;
	if(*argc < min_argc) {
		_stprintf(sc->err_buf, _T("wrong number of arguments: ")
			_T("%s requires %d, but got %d"),
			name, min_argc, *argc);
		return GET_ARG_FEW_ARGUMENTS_ERR;
	}
	return 0;
}

#define _TYPE_CHK(_sc, _tc_flg, _arg) do {								\
	if(_type_chk(_sc, _tc_flg, _arg, proc_name(sc->operator)) != 0) {	\
		Error_1(_sc, (_sc)->err_buf, _arg);								\
	}																	\
} while(0)

#define _PORT_OPEN_CHK(_sc, _port) do {									\
	if(is_closed_port((_port))) {										\
		Error_1(_sc, _T("I/O attempted on closed port"), (_port));			\
	}																	\
} while(0)

#define GET_ARGS(_sc, _min_argc, _max_argc, _type_chk_str)				\
pcell	argv[(_max_argc)];												\
int		argc;															\
do {																	\
	int _get_args_result = _get_args(_sc, proc_name(sc->operator),		\
		_min_argc, _max_argc, _type_chk_str, argv, &argc);				\
	if(_get_args_result == GET_ARG_FEW_ARGUMENTS_ERR) {					\
		Error_0(_sc, (_sc)->err_buf);									\
	} else if(_get_args_result == GET_ARG_TYPE_CHK_ERR) {				\
		Error_1(_sc, (_sc)->err_buf, argv[0]);							\
	}																	\
} while(0)

/* ========== Evaluation Cycle ========== */
static pcell _load_file(oscheme *sc, const TCHAR *file_name);

static pcell _eval_symbol(oscheme *sc, pcell code)
{
	pcell *x = find_env(sc, code);
	if(x != NULL) return *x;
	return NULL;
}

pcell op_eval(oscheme *sc)
{
	pcell x, y;

	if(issymbol(sc->code)) {
		x = _eval_symbol(sc, sc->code);
		if(x == NULL) Error_1(sc, _T("unbounded variable"), sc->code);
		s_return(sc, x);
	} else if(ispair(sc->code)) {
		x = car(sc->code);
		if(issymbol(x)) {
			y = _eval_symbol(sc, x);
			if(y == NULL) Error_1(sc, _T("unbounded variable"), x);
			if(issyntax(y)) {
				sc->code = cdr(sc->code);
				if(!islist(sc, sc->code)) Error_0(sc, _T("syntax error"));
				s_goto(sc, syntaxnum(y));
			} else {
				sc->value = y;
				s_goto(sc, OP_E0ARGS);
			}
		}
		s_save(sc, OP_E0ARGS, sc->NIL, sc->code);
		sc->code = x;
		s_goto(sc, OP_EVAL);
	} else {
		s_return(sc, sc->code);
	}
}

pcell op_e0args(oscheme *sc)
{
	if(ismacro(sc->value)) {
		if(is_macrocache(sc->code)) {
			sc->value = cdr(sc->code);
			s_goto(sc, OP_DOMACRO);
		}
		s_save(sc, OP_DOMACRO, sc->NIL, sc->code);
		sc->args = cdr(sc->code);
		sc->code = sc->value;
		s_goto(sc, OP_APPLY);
	} else {
		sc->args = sc->NIL;
		sc->code = cdr(sc->code);
		s_goto(sc, OP_E1ARGS);
	}
}

pcell op_e1args(oscheme *sc)
{
OP_E1ARGS_TOP:
	sc->args = cons(sc, sc->value, sc->args);

	if(ispair(sc->code)) {
		pcell a = car(sc->code);
		if(ispair(a)) {
			s_save(sc, OP_E1ARGS, sc->args, cdr(sc->code));
			sc->code = a;
			sc->args = sc->NIL;
			s_goto(sc, OP_EVAL);
		} else {
			if(issymbol(a)) {
				pcell v = _eval_symbol(sc, a);
				if(v == NULL) Error_1(sc, _T("unbounded variable"), a);
				sc->value = v;
			} else {
				sc->value = a;
			}
			sc->code = cdr(sc->code);
			goto OP_E1ARGS_TOP;
		}
	} else {
		sc->args = reverse_in_place(sc, sc->NIL, sc->args);
		sc->code = car(sc->args);
		sc->args = cdr(sc->args);
		s_goto(sc, OP_APPLY);
	}
}

pcell op_call_cb_func(oscheme *sc)
{
	pcell	argv[MAX_CB_FUNC_ARGS];
	int		argc;
	pcell	f = sc->code;
	pcell	result;

	int _get_args_result = _get_args(sc, cb_func_name(f),
		cb_func_min_args(f), cb_func_max_args(f),
		cb_func_type_chk_str(f), argv, &argc);
	if(_get_args_result == GET_ARG_FEW_ARGUMENTS_ERR) {
		Error_0(sc, sc->err_buf);
	} else if(_get_args_result == GET_ARG_TYPE_CHK_ERR) {
		Error_1(sc, sc->err_buf, argv[0]);
	}

	result = cb_func_fp(f)(sc, argv, argc);
	if(sc->cur_frame == 0) return sc->NIL;

	s_return(sc, result);
}

#ifdef ENABLE_CANCEL
static int _check_cancel_key(oscheme *sc)
{
	if(GetAsyncKeyState(sc->cancel_keycode) >= 0) return 0;
	if((sc->cancel_fvirt & FALT) && GetAsyncKeyState(VK_MENU) >= 0) return 0;
	if((sc->cancel_fvirt & FCONTROL) && GetAsyncKeyState(VK_CONTROL) >= 0) return 0;
	if((sc->cancel_fvirt & FSHIFT) && GetAsyncKeyState(VK_SHIFT) >= 0) return 0;
	return 1;
}
#endif

static int _check_optional(pcell x)
{
	if(!issymbol(x)) return 0;
	if(_tcscmp(symname(x), _T(":optional")) == 0) return 1;
	return 0;
}

pcell op_optional0(oscheme *sc)
{
	pcell x = sc->args;

	if(!_check_optional(car(x))) {
		push_env(sc, caar(x), sc->value);
	}
	x = cdr(x);

	for(; ispair(x); x = cdr(x)) {
		if(ispair(car(x))) {
			s_save(sc, OP_OPTIONAL0, x, sc->code);
			sc->code = cadar(x);
			s_goto(sc, OP_EVAL);
		} else {
			push_env(sc, car(x), sc->UNDEF);
		}
	}

	if(x == sc->NIL) {
	} else if(issymbol(x)) {
		push_env(sc, x, sc->NIL);
	} else {
		Error_0(sc, _T("syntax error in closure"));
	}
	sc->code = cdr(closure_code(sc->code));
	sc->args = sc->NIL;
	s_goto(sc, OP_BEGIN);
}

pcell op_apply(oscheme *sc)
{
	pcell x, y;

#ifdef ENABLE_CANCEL
	sc->cancel_counter++;

	if(sc->cancel_counter > 500) {
		if(_check_cancel_key(sc)) {
			sc->cancel_flg = 1;
			Error_0(sc, _T("canceled"));
		} else {
			sc->cancel_counter = 0;
		}
	}
#endif

	if(isproc(sc->code)) {
		s_goto(sc, procnum(sc->code));			/* procedure */
	} else if(isclosure(sc->code) || ismacro(sc->code)) {	/* closure */
		/* make environment */
		sc->envir = closure_env(sc->code);
		new_env_frame(sc);
		for(x = car(closure_code(sc->code)), y = sc->args;
			ispair(x); x = cdr(x), y = cdr(y)) {
			if(y == sc->NIL) {
				if(_check_optional(car(x))) {
					sc->args = x;
					s_goto(sc, OP_OPTIONAL0);
				}
				Error_0(sc, _T("few arguments"));
			} else {
				if(!ispair(y)) Error_0(sc, _T("syntax-error"));
				push_env(sc, car(x), car(y));
			}
		}
		if(x == sc->NIL) {
		} else if(issymbol(x)) {
			push_env(sc, x, y);
		} else {
			Error_0(sc, _T("syntax error in closure"));
		}
		sc->code = cdr(closure_code(sc->code));
		sc->args = sc->NIL;
		s_goto(sc, OP_BEGIN);
	} else if(iscontinuation(sc->code)) {		/* continuation */
		pcell a = sc->NIL;
		pcell args = NULL;
		if(ispair(sc->args)) {
			a = car(sc->args);
			args = sc->args;
		}
		dump_continuation(sc, sc->code);
		return _s_return_values(sc, a, args);
	} else if(is_cb_func(sc->code)) {
		s_goto(sc, OP_CALL_CB_FUNC);
	} else {
		Error_1(sc, _T("illegal function"), sc->code);
	}
}

pcell op_domacro(oscheme *sc)
{
	if(!is_macrocache(sc->code)) {
		set_macrocache(sc->code);
		cdr(sc->code) = sc->value;
	}
	sc->code = sc->value;
	s_goto(sc, OP_EVAL);
}

pcell op_if0(oscheme *sc)
{
	s_save(sc, OP_IF1, sc->NIL, cdr(sc->code));
	sc->code = car(sc->code);
	s_goto(sc, OP_EVAL);
}

pcell op_if1(oscheme *sc)
{
	if(istrue(sc, sc->value)) {
		sc->code = car(sc->code);
	} else {
		if(!ispair(cdr(sc->code))) s_return(sc, sc->UNDEF);
		sc->code = cadr(sc->code);
	}
	s_goto(sc, OP_EVAL);
}

pcell op_set0(oscheme *sc)
{
	s_save(sc, OP_SET1, sc->NIL, car(sc->code));
	sc->code = cadr(sc->code);
	s_goto(sc, OP_EVAL);
}

pcell op_set1(oscheme *sc)
{
	pcell *x = find_env(sc, sc->code);
	if(x != NULL) {
		*x = sc->value;
		s_return(sc, sc->code);
	}
	Error_1(sc, _T("unbounded variable"), sc->code);
}

pcell op_let_optional0(oscheme *sc)
{
	if(issymbol(car(sc->code))) {
		// (define (func arg1 . args) (let-optionals args ((a 10) (b 20)) body)
		sc->value = _eval_symbol(sc, car(sc->code));
		s_goto(sc, OP_LET_OPTIONAL1);
	} else {
		// (let-optionals (1 2) ((a 10) (b 20)) body)
		sc->value = car(sc->code);
		s_goto(sc, OP_LET_OPTIONAL1);
	}
}

pcell op_let_optional1(oscheme *sc)
{
	pcell args = sc->value;
	pcell defs = cadr(sc->code);
	pcell x, y;

	for(x = args, y = defs; x != sc->NIL; x = cdr(x), y = cdr(y)) {
		if(y == sc->NIL) {
			Error_0(sc, _T("too many arguments"));
		}
		cadar(y) = car(x);
	}

	sc->code = cdr(sc->code);
	s_goto(sc, OP_LET0);
}

pcell op_let0(oscheme *sc)
{
	sc->args = sc->NIL;
	sc->value = sc->code;
	// named-letかチェック
	sc->code = issymbol(car(sc->code)) ? cadr(sc->code) : car(sc->code);
	s_goto(sc, OP_LET1);
}

pcell op_let1(oscheme *sc)
{
	sc->args = cons(sc, sc->value, sc->args);
	if(ispair(sc->code)) {
		if(!ispair(car(sc->code)) || !ispair(cdar(sc->code))) {
			Error_0(sc, _T("let: invalid operation"));
		}
		s_save(sc, OP_LET1, sc->args, cdr(sc->code));
		sc->code = cadar(sc->code);
		sc->args = sc->NIL;
		s_goto(sc, OP_EVAL);
	} else {
		sc->args = reverse(sc, sc->args);
		sc->code = car(sc->args);
		sc->args = cdr(sc->args);
		s_goto(sc, OP_LET2);
	}
}

pcell op_let2(oscheme *sc)
{
	pcell x, y;

	new_env_frame(sc);
	for(x = issymbol(car(sc->code)) ? cadr(sc->code) : car(sc->code),
		y = sc->args; y != sc->NIL; x = cdr(x), y = cdr(y)) {
		push_env(sc, caar(x), car(y));
	}
	if(issymbol(car(sc->code))) {
		for(x = cadr(sc->code), sc->args = sc->NIL; x != sc->NIL; x = cdr(x)) {
			if(!ispair(x)) {
				Error_0(sc, _T("let: invalid operation"));
			}
			sc->args = cons(sc, caar(x), sc->args);
		}
		x = mk_closure(sc, cons(sc, reverse(sc, sc->args), cddr(sc->code)),
			sc->envir);
		push_env(sc, car(sc->code), x);
		sc->code = cddr(sc->code);
		sc->args = sc->NIL;
	} else {
		sc->code = cdr(sc->code);
		sc->args = sc->NIL;
	}
	s_goto(sc, OP_BEGIN);
}

pcell op_let0ast(oscheme *sc)
{
	if(car(sc->code) == sc->NIL) {
		new_env_frame(sc);
		sc->code = cdr(sc->code);
		s_goto(sc, OP_BEGIN);
	}
	s_save(sc, OP_LET1AST, cdr(sc->code), car(sc->code));
	if(!ispair(car(sc->code)) || !ispair(caar(sc->code)) ||
		!ispair(cdaar(sc->code))) {
		Error_0(sc, _T("syntax error"));
	}
	sc->code = cadaar(sc->code);
	s_goto(sc, OP_EVAL);
}

pcell op_let1ast(oscheme *sc)
{
	new_env_frame(sc);
	s_goto(sc, OP_LET2AST);
}

pcell op_let2ast(oscheme *sc)
{
	push_env(sc, caar(sc->code), sc->value);
	sc->code = cdr(sc->code);
	if(ispair(sc->code)) {
		s_save(sc, OP_LET2AST, sc->args, sc->code);
		sc->code = cadar(sc->code);
		sc->args = sc->NIL;
		s_goto(sc, OP_EVAL);
	} else {
		sc->code = sc->args;
		sc->args = sc->NIL;
		s_goto(sc, OP_BEGIN);
	}
}

pcell op_let0rec(oscheme *sc)
{
	new_env_frame(sc);
	sc->args = sc->NIL;
	sc->value = sc->code;
	sc->code = car(sc->code);
	s_goto(sc, OP_LET1REC);
}

pcell op_let1rec(oscheme *sc)
{
	sc->args = cons(sc, sc->value, sc->args);
	if(ispair(sc->code)) {
		s_save(sc, OP_LET1REC, sc->args, cdr(sc->code));
		sc->code = cadar(sc->code);
		sc->args = sc->NIL;
		s_goto(sc, OP_EVAL);
	} else {
		sc->args = reverse(sc, sc->args);
		sc->code = car(sc->args);
		sc->args = cdr(sc->args);
		s_goto(sc, OP_LET2REC);
	}
}

pcell op_let2rec(oscheme *sc)
{
	pcell x, y;

	for(x = car(sc->code), y = sc->args; y != sc->NIL; x = cdr(x), y = cdr(y)) {
		push_env(sc, caar(x), car(y));
	}
	sc->code = cdr(sc->code);
	sc->args = sc->NIL;
	s_goto(sc, OP_BEGIN);
}

pcell op_lambda(oscheme *sc)
{
	s_return(sc, mk_closure(sc, sc->code, sc->envir));
}

pcell op_quote(oscheme *sc)
{
	s_return(sc, car(sc->code));
}

pcell op_qquote0(oscheme *sc)
{
	sc->code = car(sc->code);
	sc->value = sc->NIL;
	sc->args = cons(sc, sc->NIL, mk_number(sc, 0));
	s_goto(sc, OP_QQUOTE1);
}

pcell op_qquote_vec0(oscheme *sc)
{
	s_save(sc, OP_QQUOTE_VEC1, sc->NIL, sc->NIL);
	sc->code = car(sc->code);
	sc->value = sc->NIL;
	sc->args = cons(sc, sc->NIL, mk_number(sc, 0));
	s_goto(sc, OP_QQUOTE1);
}

pcell op_qquote_vec1(oscheme *sc)
{
	sc->args = sc->value;
	s_goto(sc, OP_QUOTE_VEC1);
}

pcell op_qquote1(oscheme *sc)
{
	INT_PTR level = ivalue_unchecked(cdr(sc->args));

	if(ispair(sc->code)) {
		if(car(sc->code) == sc->QQUOTE) {
			s_save(sc, OP_QQUOTE2_ADD, sc->QQUOTE, sc->NIL);
			sc->code = cadr(sc->code);
			sc->args = cons(sc, sc->NIL, mk_number(sc, level + 1));
			s_goto(sc, OP_QQUOTE1);
		} else if(car(sc->code) == sc->QQUOTE_VEC) {
			s_save(sc, OP_QQUOTE2_ADD, sc->QQUOTE_VEC, sc->NIL);
			sc->code = cadr(sc->code);
			sc->args = cons(sc, sc->NIL, mk_number(sc, level + 1));
			s_goto(sc, OP_QQUOTE1);
		} else if(car(sc->code) == sc->UNQUOTE ||
					car(sc->code) == sc->UNQUOTESP) {
			if(!ispair(cdr(sc->code))) Error_0(sc, _T("syntax error"));
			if(level == 0) {
				sc->code = cadr(sc->code);
				sc->args = sc->NIL;
				s_goto(sc, OP_EVAL);
			} else {
				s_save(sc, OP_QQUOTE2_ADD, car(sc->code), sc->NIL);
				sc->code = cadr(sc->code);
				sc->args = cons(sc, sc->NIL, mk_number(sc, level - 1));
				s_goto(sc, OP_QQUOTE1);
			}
		} else {
			if(ispair(cdr(sc->code))) {
				if(cadr(sc->code) == sc->UNQUOTE ||
					cadr(sc->code) == sc->QQUOTE) {
					/* dot detected */
					s_save(sc, OP_QQUOTE2_DOT, sc->args, sc->NIL);
				}
				if(cadr(sc->code) == sc->UNQUOTESP) {
					Error_0(sc, _T("unquote-splicing in invalid context"));
				}
			}
			if(ispair(car(sc->code))) {
				if(caar(sc->code) == sc->UNQUOTESP && level == 0) {
					s_save(sc, OP_QQUOTE2_UNQSP, sc->args, cdr(sc->code));
				} else {
					s_save(sc, OP_QQUOTE2, sc->args, cdr(sc->code));
				}
				sc->code = car(sc->code);
				sc->args = cons(sc, sc->NIL, mk_number(sc, level));
				s_goto(sc, OP_QQUOTE1);
			} else {
				s_save(sc, OP_QQUOTE2, sc->args, cdr(sc->code));
				sc->code = car(sc->code);
				sc->args = cons(sc, sc->NIL, mk_number(sc, level));
				s_goto(sc, OP_QQUOTE1);
			}
		}
	} else {
		sc->args = reverse_in_place(sc, sc->code, car(sc->args));
		s_return(sc, sc->args);
	}
}

pcell op_qquote2(oscheme *sc)
{
	car(sc->args) = cons(sc, sc->value, car(sc->args));
	s_goto(sc, OP_QQUOTE1);
}

pcell op_qquote2_add(oscheme *sc)
{
	sc->value = cons(sc, sc->args, cons(sc, sc->value, sc->NIL));
	s_return(sc, sc->value);
}

pcell op_qquote2_dot(oscheme *sc)
{
	sc->args = reverse_in_place(sc, sc->value, car(sc->args));
	s_return(sc, sc->args);
}

pcell op_qquote2_unqsp(oscheme *sc)
{
	pcell	x = sc->value;
	if(ispair(x)) {
		for(; x != sc->NIL; x = cdr(x)) {
			car(sc->args) = cons(sc, car(x), car(sc->args));
		}
	} else {
		if(x != sc->NIL) {
			/* dot detected */
			sc->args = reverse_in_place(sc, x, car(sc->args));
			s_return(sc, sc->args);
		}
	}
	s_goto(sc, OP_QQUOTE1);
}

pcell op_load(oscheme *sc)
{
	TCHAR	filepath[1024];
	TCHAR	*filename;
	GET_ARGS(sc, 1, 1, TC_STRING);

	filename = strvalue(argv[0]);

	if(filename[0] == '.' || is_full_path_name(filename)) {
		_tcscpy(filepath, filename);
	} else {
		pcell	paths = _eval_symbol(sc, mk_symbol(sc, _T("*load-path*")));

		for(;;) {
			if(paths == NULL || !ispair(paths)) {
				Error_1(sc, _T("load: can not find file"), argv[0]);
			}

			_stprintf(filepath, _T("%s/%s"),
				strvalue(car(paths)), filename);
			if(is_file_exists(filepath)) break;

			paths = cdr(paths);
		}
	}

	if(_load_file(sc, filepath) == sc->F) {
		Error_1(sc, _T("load: can not find file"), argv[0]);
	}
	if(sc->cur_frame == 0) {
		if(sc->interactive) {
			s_save(sc, OP_T0LVL, sc->NIL, sc->NIL);
			s_goto(sc, OP_NEWLINE);
		} else {
			return sc->NIL;
		}
	}

	s_return(sc, sc->T);
}

pcell op_require(oscheme *sc)
{
	TCHAR	filename[1024];
	GET_ARGS(sc, 1, 1, TC_STRING);

	_stprintf(filename, _T("%s.scm"), strvalue(argv[0]));
	sc->args = cons(sc, mk_string(sc, filename), sc->NIL);

	s_goto(sc, OP_LOAD);
}

pcell op_use(oscheme *sc)
{
	TCHAR	filename[1024];
	pcell	x = car(sc->code);
	_TYPE_CHK(sc, TC_SYMBOL, x);

	_stprintf(filename, _T("%s.scm"), symname(x));
	sc->args = cons(sc, mk_string(sc, filename), sc->NIL);

	s_goto(sc, OP_LOAD);
}

pcell op_load_t0lvl(oscheme *sc)
{
	if(is_port_eof(sc, car(sc->args))) return sc->NIL;
	s_save(sc, OP_LOAD_T0LVL, sc->args, sc->NIL);
	s_save(sc, OP_T1LVL, sc->NIL, sc->NIL);
	s_goto(sc, OP_READ);
}

pcell op_t0lvl(oscheme *sc)
{
	if(is_port_eof(sc, sc->infp)) return sc->NIL;
	s_save(sc, OP_T0LVL, sc->NIL, sc->NIL);
	s_save(sc, OP_VALUEPRINT, sc->NIL, sc->NIL);
	s_save(sc, OP_T1LVL, sc->NIL, sc->NIL);
	if(isport(sc->infp) && port_fp(sc->infp) == stdin) _prompt(sc);
	s_goto(sc, OP_READ);
}

pcell op_t1lvl(oscheme *sc)
{
	sc->code = sc->value;
	s_goto(sc, OP_EVAL);
}

pcell op_read_char(oscheme *sc)
{
	och		ch;
	pcell	back_p = sc->infp;
	GET_ARGS(sc, 0, 1, TC_IPORT);
	_PORT_OPEN_CHK(sc, argv[0]);

	if(argc == 1) sc->infp = argv[0];

	ch = _read_in_buf(sc);
	_next_in_buf(sc);
	sc->infp = back_p;

	if(ch == '\0') s_return(sc, mk_eof(sc));
	s_return(sc, mk_char(sc, ch));
}

pcell op_peek_char(oscheme *sc)
{
	och		ch;
	pcell	back_p = sc->infp;
	GET_ARGS(sc, 0, 1, TC_IPORT);
	_PORT_OPEN_CHK(sc, argv[0]);

	if(argc == 1) sc->infp = argv[0];

	ch = _read_in_buf(sc);
	sc->infp = back_p;

	if(ch == '\0') s_return(sc, mk_eof(sc));
	s_return(sc, mk_char(sc, ch));
}

pcell op_read(oscheme *sc)
{
	GET_ARGS(sc, 0, 1, TC_IPORT);
	if(argc == 1) {
		_PORT_OPEN_CHK(sc, argv[0]);
		s_save(sc, OP_RESTORE_INPORT, sc->infp, sc->NIL);
		sc->infp = argv[0];
	}
	sc->tok = _get_token(sc);
	s_goto(sc, OP_RDSEXPR);
}

pcell op_valueprint(oscheme *sc)
{
	sc->print_flag = 1;
	if(sc->values != NULL) {
		sc->args = sc->values;
		s_goto(sc, OP_VALUES_PRINT);
	} else {
		sc->args = sc->value;
		s_save(sc, OP_NEWLINE, sc->NIL, sc->NIL);
		s_goto(sc, OP_P0LIST);
	}
}

pcell op_values_print(oscheme *sc)
{
	if(sc->args == sc->NIL) s_return(sc, sc->UNDEF);

	s_save(sc, OP_VALUES_PRINT, cdr(sc->args), sc->NIL);
	sc->args = car(sc->args);
	s_save(sc, OP_NEWLINE, sc->NIL, sc->NIL);
	s_goto(sc, OP_P0LIST);
}

pcell op_def0(oscheme *sc)
{
	pcell x;

	if(ispair(car(sc->code))) {
		x = caar(sc->code);
		sc->code = cons(sc, sc->LAMBDA,
			cons(sc, cdar(sc->code), cdr(sc->code)));
	} else {
		x = car(sc->code);
		sc->code = cadr(sc->code);
	}
	if(!issymbol(x)) {
		Error_0(sc, _T("Variable is not symbol"));
	}
	s_save(sc, OP_DEF1, sc->NIL, x);
	s_goto(sc, OP_EVAL);
}

pcell op_def1(oscheme *sc)
{
	pcell *x = find_frame(sc, car(sc->envir), sc->code);
	if(x != NULL) {
		*x = sc->value;
		s_return(sc, sc->code);
	}
	push_env(sc, sc->code, sc->value);
	s_return(sc, sc->code);
}

pcell op_is_symbol_bound(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_SYMBOL);
	if(_eval_symbol(sc, argv[0]) == NULL) s_return(sc, sc->F);
	s_return(sc, sc->T);
}

pcell op_begin(oscheme *sc)
{
	if(!ispair(sc->code)) {
		s_return(sc, sc->code);
	}
	if(cdr(sc->code) != sc->NIL) {
		s_save(sc, OP_BEGIN, sc->NIL, cdr(sc->code));
	}
	sc->code = car(sc->code);
	s_goto(sc, OP_EVAL);
}

pcell op_cond0(oscheme *sc)
{
	if(!ispair(sc->code)) {
		Error_0(sc, _T("Syntax error in cond"));
	}
	if(!ispair(car(sc->code))) {
		Error_0(sc, _T("Syntax error in cond"));
	}
	s_save(sc, OP_COND1, sc->NIL, sc->code);
	sc->code = caar(sc->code);
	s_goto(sc, OP_EVAL);
}

pcell op_cond1(oscheme *sc)
{
	if(istrue(sc, sc->value)) {
		sc->code = cdar(sc->code);
		if(sc->code == sc->NIL) {
			s_return(sc, sc->value);
		}
		if(ispair(sc->code) && issymbol(car(sc->code)) &&
			_tcscmp(symname(car(sc->code)), _T("=>")) == 0) {
			sc->code = cons(sc, cadr(sc->code), sc->NIL);
			cdr(sc->code) = cons(sc, cons(sc, sc->QUOTE, sc->NIL), sc->NIL);
			cdadr(sc->code) = cons(sc, sc->value, sc->NIL);
			s_goto(sc, OP_EVAL);
		}
		s_goto(sc, OP_BEGIN);
	} else {
		sc->code = cdr(sc->code);
		if(sc->code == sc->NIL) s_return(sc, sc->UNDEF);
		s_save(sc, OP_COND1, sc->NIL, sc->code);
		if(!ispair(car(sc->code))) {
			Error_0(sc, _T("Syntax error in cond"));
		}
		sc->code = caar(sc->code);
		s_goto(sc, OP_EVAL);
	}
}

pcell op_and0(oscheme *sc)
{
	if(sc->code == sc->NIL) s_return(sc, sc->T);
	s_save(sc, OP_AND1, sc->NIL, cdr(sc->code));
	sc->code = car(sc->code);
	s_goto(sc, OP_EVAL);
}

pcell op_and1(oscheme *sc)
{
	if(isfalse(sc, sc->value)) s_return(sc, sc->value);
	if(sc->code == sc->NIL) s_return(sc, sc->value);
	s_save(sc, OP_AND1, sc->NIL, cdr(sc->code));
	sc->code = car(sc->code);
	s_goto(sc, OP_EVAL);
}

pcell op_or0(oscheme *sc)
{
	if(sc->code == sc->NIL) s_return(sc, sc->F);
	s_save(sc, OP_OR1, sc->NIL, cdr(sc->code));
	sc->code = car(sc->code);
	s_goto(sc, OP_EVAL);
}

pcell op_or1(oscheme *sc)
{
	if(istrue(sc, sc->value)) s_return(sc, sc->value);
	if(sc->code == sc->NIL) s_return(sc, sc->value);
	s_save(sc, OP_OR1, sc->NIL, cdr(sc->code));
	sc->code = car(sc->code);
	s_goto(sc, OP_EVAL);
}

pcell op_debug_info(oscheme *sc)
{
	s_return(sc, sc->global_var);
}

pcell op_nop(oscheme *sc)
{
	return _s_return_values(sc, sc->value, sc->values);
}

pcell op_int_env(oscheme *sc)
{
	s_return(sc, sc->global_env);
}

pcell op_0macro(oscheme *sc)
{
	pcell x;

	if(ispair(car(sc->code))) {
		x = caar(sc->code);
		sc->code = cons(sc, sc->LAMBDA,
			cons(sc, cdar(sc->code), cdr(sc->code)));
	} else {
		x = car(sc->code);
		sc->code = cadr(sc->code);
	}
	if(!issymbol(x)) {
		Error_0(sc, _T("variable is not a symbol"));
	}
	s_save(sc, OP_1MACRO, sc->NIL, x);
	s_goto(sc, OP_EVAL);
}

pcell op_1macro(oscheme *sc)
{
	settype(sc->value, T_MACRO);
	s_goto(sc, OP_DEF1);
}

pcell op_peval(oscheme *sc)
{
	GET_ARGS(sc, 1, 2, TC_ANY TC_ENVIRONMENT);
/*  FIXME: ignore environment
	if(argc == 2) sc->envir = argv[1];
*/
	sc->code = argv[0];
	sc->args = sc->NIL;
	s_goto(sc, OP_EVAL);
}

pcell op_papply(oscheme *sc)
{
	pcell	x;
	sc->code = car(sc->args);
	x = cdr(sc->args);

	s_save(sc, OP_NOP, sc->args, sc->NIL);

	sc->args = sc->NIL;
	for(; cdr(x) != sc->NIL; x = cdr(x)) {
		sc->args = cons(sc, car(x), sc->args);
	}
	if(!islist(sc, car(x))) {
		Error_0(sc, _T("last argument must be list"));
	}
	for(x = car(x); x != sc->NIL; x = cdr(x)) {
		sc->args = cons(sc, car(x), sc->args);
	}
	sc->args = reverse_in_place(sc, sc->NIL, sc->args);
	s_goto(sc, OP_APPLY);
}

pcell op_continuation(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	sc->code = argv[0];
	sc->args = cons(sc, mk_continuation(sc), sc->NIL);
	s_goto(sc, OP_APPLY);
}

pcell op_add(oscheme *sc)
{
	pcell	x;
	INT_PTR	v = 0;

	for(x = sc->args, v = 0; x != sc->NIL; x = cdr(x)) {
		_TYPE_CHK(sc, TC_NUMBER, car(x));
		v += ivalue(car(x));
	}
	s_return(sc, mk_number(sc, v));
}

pcell op_sub(oscheme *sc)
{
	pcell	x;
	INT_PTR	v = 0;

	if(cdr(sc->args) == sc->NIL) {
		x = sc->args;
		v = 0;
	} else {
		x = cdr(sc->args);
		_TYPE_CHK(sc, TC_NUMBER, car(sc->args));
		v = ivalue(car(sc->args));
	}

	for(; x != sc->NIL; x = cdr(x)) {
		_TYPE_CHK(sc, TC_NUMBER, car(x));
		v -= ivalue(car(x));
	}
	s_return(sc, mk_number(sc, v));
}

pcell op_mul(oscheme *sc)
{
	pcell	x;
	INT_PTR	v = 0;

	for(x = sc->args, v = 1; x != sc->NIL; x = cdr(x)) {
		_TYPE_CHK(sc, TC_NUMBER, car(x));
		v *= ivalue(car(x));
	}
	s_return(sc, mk_number(sc, v));
}

pcell op_div(oscheme *sc)
{
	pcell	x;
	INT_PTR	v = 0;

	_TYPE_CHK(sc, TC_NUMBER, car(sc->args));

	for(x = cdr(sc->args), v = ivalue(car(sc->args));
		x != sc->NIL; x = cdr(x)) {
		_TYPE_CHK(sc, TC_NUMBER, car(x));
		if(ivalue(car(x)) != 0) {
			v /= ivalue(car(x));
		} else {
			Error_0(sc, _T("Divided by zero"));
		}
	}
	s_return(sc, mk_number(sc, v));
}

pcell op_rem(oscheme *sc)
{
	pcell	x;
	INT_PTR	v;

	_TYPE_CHK(sc, TC_NUMBER, car(sc->args));

	for(x = cdr(sc->args), v = ivalue(car(sc->args));
		x != sc->NIL; x = cdr(x)) {
		_TYPE_CHK(sc, TC_NUMBER, car(x));
		if(ivalue(car(x)) != 0) {
			v %= ivalue(car(x));
		} else {
			Error_0(sc, _T("Dividev by zero"));
		}
	}
	s_return(sc, mk_number(sc, v));
}

pcell op_modulo(oscheme *sc)
{
	INT_PTR	v1;
	INT_PTR	v2;
	INT_PTR	result;
	GET_ARGS(sc, 2, 2, TC_NUMBER TC_NUMBER);

	v1 = ivalue(argv[0]);
	v2 = ivalue(argv[1]);

	if(v2 == 0) Error_0(sc, _T("Dividev by zero"));

	result = v1 % v2;
	if(v1 < 0 && v2 > 0 || v1 > 0 && v2 < 0) {
		result += v2;
	}

	s_return(sc, mk_number(sc, result));
}

pcell op_car(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_PAIR);
	s_return(sc, car(argv[0]));
}

pcell op_cdr(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_PAIR);
	s_return(sc, cdr(argv[0]));
}

pcell op_cons(oscheme *sc)
{
	GET_ARGS(sc, 2, 2, TC_ANY TC_ANY);
	sc->args = cons(sc, argv[0], argv[1]);
	s_return(sc, sc->args);
}

pcell op_setcar(oscheme *sc)
{
	GET_ARGS(sc, 2, 2, TC_PAIR TC_ANY);
	car(argv[0]) = argv[1];
	s_return(sc, sc->UNDEF);
}

pcell op_setcdr(oscheme *sc)
{
	GET_ARGS(sc, 2, 2, TC_PAIR TC_ANY);
	cdr(argv[0]) = argv[1];
	s_return(sc, sc->UNDEF);
}

pcell op_not(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	s_retbool(sc, isfalse(sc, argv[0]));
}

pcell op_is_bool(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	s_retbool(sc, argv[0] == sc->F || argv[0] == sc->T);
}

pcell op_is_null(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	s_retbool(sc, argv[0] == sc->NIL);
}

pcell op_neq(oscheme *sc)
{
	pcell x, n1, n2;

	n1 = car(sc->args);
	_TYPE_CHK(sc, TC_NUMBER, n1);

	for(x = cdr(sc->args); x != sc->NIL; x = cdr(x)) {
		n2 = car(x);
		_TYPE_CHK(sc, TC_NUMBER, n2);
		if(!(ivalue(n1) == ivalue(n2))) s_return(sc, sc->F);
		n1 = n2;
	}
	s_return(sc, sc->T);
}

pcell op_less(oscheme *sc)
{
	pcell x, n1, n2;

	n1 = car(sc->args);
	_TYPE_CHK(sc, TC_NUMBER, n1);

	for(x = cdr(sc->args); x != sc->NIL; x = cdr(x)) {
		n2 = car(x);
		_TYPE_CHK(sc, TC_NUMBER, n2);
		if(!(ivalue(n1) < ivalue(n2))) s_return(sc, sc->F);
		n1 = n2;
	}
	s_return(sc, sc->T);
}

pcell op_gre(oscheme *sc)
{
	pcell x, n1, n2;

	n1 = car(sc->args);
	_TYPE_CHK(sc, TC_NUMBER, n1);

	for(x = cdr(sc->args); x != sc->NIL; x = cdr(x)) {
		n2 = car(x);
		_TYPE_CHK(sc, TC_NUMBER, n2);
		if(!(ivalue(n1) > ivalue(n2))) s_return(sc, sc->F);
		n1 = n2;
	}
	s_return(sc, sc->T);
}

pcell op_leq(oscheme *sc)
{
	pcell x, n1, n2;

	n1 = car(sc->args);
	_TYPE_CHK(sc, TC_NUMBER, n1);

	for(x = cdr(sc->args); x != sc->NIL; x = cdr(x)) {
		n2 = car(x);
		_TYPE_CHK(sc, TC_NUMBER, n2);
		if(!(ivalue(n1) <= ivalue(n2))) s_return(sc, sc->F);
		n1 = n2;
	}
	s_return(sc, sc->T);
}

pcell op_geq(oscheme *sc)
{
	pcell x, n1, n2;

	n1 = car(sc->args);
	_TYPE_CHK(sc, TC_NUMBER, n1);

	for(x = cdr(sc->args); x != sc->NIL; x = cdr(x)) {
		n2 = car(x);
		_TYPE_CHK(sc, TC_NUMBER, n2);
		if(!(ivalue(n1) >= ivalue(n2))) s_return(sc, sc->F);
		n1 = n2;
	}
	s_return(sc, sc->T);
}

pcell op_get_closure_code(oscheme *sc)
{
	sc->args = car(sc->args);

	if(isclosure(sc->args) || ismacro(sc->args)) {
		s_return(sc, cons(sc, sc->LAMBDA, closure_code(sc->value)));
	}
	if(iscontinuation(sc->args)) {
		ocontinuation *c = getcontinuation(sc->args);
		s_return(sc, cons(sc, sc->LAMBDA,
			c->_dump_stack[c->_dump_stack_cnt - 1].code));
	}
	s_return(sc, sc->F);
}

pcell op_is_closure(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	s_retbool(sc, isclosure(argv[0]));
}

pcell op_is_macro(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	s_retbool(sc, ismacro(argv[0]));
}

pcell op_is_proc(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	s_retbool(sc, isproc(argv[0]) || isclosure(argv[0]) ||
		iscontinuation(argv[0]));
}

pcell op_is_pair(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	s_retbool(sc, ispair(argv[0]));
}

pcell op_is_symbol(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	s_retbool(sc, issymbol(argv[0]));
}

pcell op_symbol_to_string(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_SYMBOL);
	s_return(sc, mk_string(sc, symname(argv[0])));
}

pcell op_is_number(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	s_retbool(sc, isnumber(argv[0]));
}

pcell op_is_integer(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	s_retbool(sc, isnumber(argv[0]));
}

pcell op_number_to_string(oscheme *sc)
{
	TCHAR	buf[100];
	GET_ARGS(sc, 1, 1, TC_NUMBER);

	_stprintf(buf, _T("%Id"), ivalue(argv[0]));
	s_return(sc, mk_string(sc, buf));
}

pcell op_string_to_number(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_STRING);
	s_return(sc, mk_number(sc, _ttoi(strvalue(argv[0]))));
}

pcell op_is_string(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	s_retbool(sc, isstring(argv[0]));
}

pcell op_is_char(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	s_retbool(sc, ischar(argv[0]));
}

pcell op_char_to_int(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_CHAR);
	s_return(sc, mk_number(sc, chvalue(argv[0])));
}

pcell op_int_to_char(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_NUMBER);
	s_return(sc, mk_char(sc, (och)ivalue(argv[0])));
}

pcell op_is_vector(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	s_retbool(sc, isvector(argv[0]));
}

pcell op_quote_vec0(oscheme *sc)
{
	sc->args = car(sc->code);
	s_goto(sc, OP_QUOTE_VEC1);
}

pcell op_quote_vec1(oscheme *sc)
{
	pcell	vec;
	int		len;

	len = list_length(sc, sc->args);
	if(len < 0) {
		Error_1(sc, _T("vector constructer require list:"), sc->args);
	}

	vec = mk_vector(sc, len, 0, sc->args);

	s_return(sc, vec);
}

pcell op_mk_vector(oscheme *sc)
{
	pcell	vec;
	pcell	fill = sc->NIL;
	INT_PTR		len;
	INT_PTR		i;
	pcell	*arr;
	GET_ARGS(sc, 1, 2, TC_POSITIVE_NUMBER TC_ANY);

	len = ivalue(argv[0]);
	if(argc == 2) fill = argv[1];

	vec = mk_vector(sc, len, 0, NULL);
	arr = vector_arr(vec);

	for(i = 0; i < len; i++) arr[i] = fill;
	vector_cnt(vec) = len;

	s_return(sc, vec);
}

pcell op_vector_ref(oscheme *sc)
{
	INT_PTR		idx;
	GET_ARGS(sc, 2, 2, TC_VECTOR TC_NUMBER);

	idx = ivalue(argv[1]);
	if(idx < 0 || idx >= (int)vector_cnt(argv[0])) {
		Error_1(sc, _T("vector-ref index out of range:"), argv[1]);
	}
	s_return(sc, vector_arr(argv[0])[idx]);
}

pcell op_vector_set(oscheme *sc)
{
	INT_PTR		idx;
	GET_ARGS(sc, 3, 3, TC_VECTOR TC_NUMBER TC_ANY);

	idx = ivalue(argv[1]);
	if(idx < 0 || idx >= (int)vector_cnt(argv[0])) {
		Error_1(sc, _T("vector-set! index out of range:"), argv[1]);
	}

	vector_arr(argv[0])[idx] = argv[2];
	s_return(sc, sc->UNDEF);
}

pcell op_vector_length(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_VECTOR);
	s_return(sc, mk_number(sc, vector_cnt(argv[0])));
}

pcell _op_string_cmp(oscheme *sc, int ci_flg)
{
	int		result;
	GET_ARGS(sc, 3, 3, TC_PROCEDURE TC_STRING TC_STRING);

	if(ci_flg == 0) {
		result = _string_cmp(strvalue(argv[1]), strvalue(argv[2]));
	} else {
		result = _string_cmp_ci(strvalue(argv[1]), strvalue(argv[2]));
	}

	if(isproc(argv[0])) {
		switch(procnum(argv[0])) {
		case OP_NEQ: s_retbool(sc, result == 0);
		case OP_LESS: s_retbool(sc, result < 0);
		case OP_GRE: s_retbool(sc, result > 0);
		case OP_LEQ: s_retbool(sc, result <= 0);
		case OP_GEQ: s_retbool(sc, result >= 0);
		}
	}

	Error_1(sc, _T("unsupported procedure"), argv[0]);
}

pcell op_string_cmp(oscheme *sc)
{
	return _op_string_cmp(sc, 0);
}

pcell op_string_cmp_ci(oscheme *sc)
{
	return _op_string_cmp(sc, 1);
}

pcell op_string_length(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_STRING);
	s_return(sc, mk_number(sc, string_length(strvalue(argv[0]))));
}

#define _string_fwd(sc, p, cnt) do {								\
	INT_PTR _i = (cnt);													\
	for(; _i > 0; _i--) {											\
		if(_get_char(p) == '\0') {									\
			Error_0((sc), _T("out of string buffer"));				\
		}															\
		(p) = _next_char(p);										\
	}																\
} while(0)

pcell op_string_ref(oscheme *sc)
{
	TCHAR	*p;
	GET_ARGS(sc, 2, 2, TC_STRING TC_POSITIVE_NUMBER);
	p = strvalue(argv[0]);
	_string_fwd(sc, p, ivalue(argv[1]));
	s_return(sc, mk_char(sc, _get_char(p)));
}

pcell op_string_set(oscheme *sc)
{
	TCHAR	*p_start, *p;
	och		ch;
	GET_ARGS(sc, 3, 3, TC_STRING TC_POSITIVE_NUMBER TC_CHAR);

	p_start = strvalue(argv[0]);
	p = p_start;
	ch = chvalue(argv[2]);

	_string_fwd(sc, p, ivalue(argv[1]));

	if(_get_char_len2(ch) == _get_char_len2(_get_char(p))) {
		p = _put_char(p, ch);
	} else {
		ostr *new_str = mk_ostr(sc, _tcslen(p_start) +
			((INT_PTR)_get_char_len2(ch) - _get_char_len2(_get_char(p))));
		TCHAR *np = new_str->_svalue;
		memcpy(np, p_start, p - p_start);
		np += p - p_start;
		np = _put_char(np, ch);
		_tcscpy(np, _next_char(p));
		oscm_free(sc, ptrvalue(argv[0]));
		ptrvalue(argv[0]) = new_str;
	}

	s_return(sc, argv[0]);
}

pcell op_substring(oscheme *sc)
{
	TCHAR	*p, *copy_start;
	INT_PTR	start, end;
	pcell	s;
	GET_ARGS(sc, 3, 3, TC_STRING TC_POSITIVE_NUMBER TC_POSITIVE_NUMBER);

	p = strvalue(argv[0]);
	start = ivalue(argv[1]);
	end = ivalue(argv[2]);

	if(start > end) {
		Error_0(sc, _T("substring: invalid argument (start > end)"));
	}

	_string_fwd(sc, p, start);
	copy_start = p;
	_string_fwd(sc, p, end - start);

	s = mk_string_cell(sc, p - copy_start + 1);
	memcpy(strvalue(s), copy_start, (p - copy_start) * sizeof(TCHAR));
	strvalue(s)[p - copy_start] = '\0';

	s_return(sc, s);
}

pcell op_string_append(oscheme *sc)
{
	pcell	x, s;
	TCHAR	*p;
	size_t		total_len = 0;

	for(x = sc->args; x != sc->NIL; x = cdr(x)) {
		_TYPE_CHK(sc, TC_STRING, car(x));
		total_len += string_length_byte(strvalue(car(x)));
	}

	s = mk_string_cell(sc, total_len);
	p = strvalue(s);

	for(x = sc->args; x != sc->NIL; x = cdr(x)) {
		size_t l = string_length_byte(strvalue(car(x)));
		memcpy(p, strvalue(car(x)), l);
		p = (TCHAR *)((char *)p + l);
	}
	*p = '\0';

	s_return(sc, s);
}

pcell op_mk_string(oscheme *sc)
{
	pcell   str;
	INT_PTR    len;
	INT_PTR    i;
	och     ch = ' ';
	TCHAR   *s;
	GET_ARGS(sc, 1, 2, TC_POSITIVE_NUMBER TC_CHAR);

	len = ivalue(argv[0]);
	if(argc == 2) ch = chvalue(argv[1]);

	str = mk_string_cell(sc, len * _get_char_len2(ch));
	s = strvalue(str);

	for(i = 0; i < len; i++) s = _put_char(s, ch);
	*s = '\0';

	s_return(sc, str);
}

pcell op_string_to_symbol(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_STRING);
	s_return(sc, mk_symbol(sc, strvalue(argv[0])));
}

pcell op_gc(oscheme *sc)
{
	gc(sc, sc->NIL, sc->NIL, 1);
	s_return(sc, sc->UNDEF);
}

pcell op_restore_outport(oscheme *sc)
{
	sc->outfp = sc->args;
	sc->args = sc->NIL;
	s_return(sc, sc->value);
}

pcell op_restore_inport(oscheme *sc)
{
	sc->infp = sc->args;
	sc->args = sc->NIL;
	s_return(sc, sc->value);
}

pcell op_write(oscheme *sc)
{
	GET_ARGS(sc, 1, 2, TC_ANY TC_OPORT);
	if(argc == 2) {
		_PORT_OPEN_CHK(sc, argv[1]);
		s_save(sc, OP_RESTORE_OUTPORT, sc->outfp, sc->NIL);
		sc->outfp = argv[1];
	}
	sc->print_flag = 1;
	sc->args = argv[0];
	s_goto(sc, OP_P0LIST);
}

pcell op_display(oscheme *sc)
{
	GET_ARGS(sc, 1, 2, TC_ANY TC_OPORT);
	if(argc == 2) {
		_PORT_OPEN_CHK(sc, argv[1]);
		s_save(sc, OP_RESTORE_OUTPORT, sc->outfp, sc->NIL);
		sc->outfp = argv[1];
	}
	sc->print_flag = 0;
	sc->args = argv[0];
	s_goto(sc, OP_P0LIST);
}

pcell op_write_char(oscheme *sc)
{
	TCHAR	chbuf[MAX_CHAR_SIZE + 1];
	TCHAR	*pbuf = chbuf;
	pcell	p = sc->outfp;

	GET_ARGS(sc, 1, 2, TC_CHAR TC_OPORT);
	if(argc == 2) {
		_PORT_OPEN_CHK(sc, argv[1]);
		p = argv[1];
	}

	pbuf = _put_char(pbuf, chvalue(argv[0]));
	pbuf = _put_char(pbuf, '\0');

	port_out(sc, p, chbuf);
	s_return(sc, sc->UNDEF);
}

pcell op_newline(oscheme *sc)
{
	pcell	p = sc->outfp;
	GET_ARGS(sc, 0, 1, TC_OPORT);

	if(argc == 1) {
		_PORT_OPEN_CHK(sc, argv[0]);
		p = argv[0];
	}
	port_out(sc, p, _T("\n"));
	s_return(sc, sc->UNDEF);
}

pcell op_err0(oscheme *sc)
{
	sc->tmpfp = sc->outfp;
	sc->outfp = sc->errfp;

	port_out(sc, sc->outfp, _T("Error:"));
	if(ispair(sc->args) && isstring(car(sc->args))) {
		port_out(sc, sc->outfp, _T(" "));
		port_out(sc, sc->outfp, strvalue(car(sc->args)));
		sc->args = cdr(sc->args);
	}

	s_goto(sc, OP_ERR1);
}

pcell op_err1(oscheme *sc)
{
	port_out(sc, sc->outfp, _T(" "));

	if(sc->args != sc->NIL) {
		s_save(sc, OP_ERR1, cdr(sc->args), sc->NIL);
		sc->args = car(sc->args);
		sc->print_flag = 1;
		s_goto(sc, OP_P0LIST);
	} else {
		port_out(sc, sc->outfp, _T("\n"));
		sc->outfp = sc->tmpfp;
		sc->tmpfp = sc->NIL;

		_dump_stack_reset(sc);
		_clear_inportbuf(sc, sc->infp);
		sc->envir = sc->global_env;
		oscheme_clear_dont_gc_flg(sc);

		if(sc->interactive == 0) return sc->NIL;
		s_goto(sc, OP_T0LVL);
	}
}

pcell op_reverse(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_LIST);
	s_return(sc, reverse(sc, argv[0]));
}

pcell op_append(oscheme *sc)
{
	pcell x, y;
	pcell prev_node;

	if(sc->args == sc->NIL) s_return(sc, sc->NIL);
	if(cdr(sc->args) == sc->NIL) s_return(sc, car(sc->args));

	prev_node = car(sc->args);

	for(x = car(sc->args), y = cdr(sc->args); y != sc->NIL; y = cdr(y)) {
		_TYPE_CHK(sc, TC_LIST, prev_node);
		prev_node = car(y);
		x = append(sc, x, car(y));
	}
	s_return(sc, x);
}

pcell op_eof(oscheme *sc)
{
	s_return(sc, mk_eof(sc));
}

pcell op_exit(oscheme *sc)
{
	return sc->NIL;
}

pcell op_eq(oscheme *sc)
{
	GET_ARGS(sc, 2, 2, TC_ANY TC_ANY);
	s_retbool(sc, eq(argv[0], argv[1]));
}

pcell op_eqv(oscheme *sc)
{
	GET_ARGS(sc, 2, 2, TC_ANY TC_ANY);
	s_retbool(sc, eqv(argv[0], argv[1]));
}

pcell op_equal(oscheme *sc)
{
	GET_ARGS(sc, 2, 2, TC_ANY TC_ANY);
	s_retbool(sc, equal(argv[0], argv[1]));
}

pcell op_print_vector(oscheme *sc)
{
	INT_PTR		i = ivalue_unchecked(cdr(sc->args));
	pcell	vec = car(sc->args);
	INT_PTR		len = vector_cnt(vec);
	
	if(i == len) {
		port_out(sc, sc->outfp, _T(")"));
		s_return(sc, sc->UNDEF);
	}
	if(i != 0) port_out(sc, sc->outfp, _T(" "));

	ivalue_unchecked(cdr(sc->args)) = i + 1;
	s_save(sc, OP_PVECTOR, sc->args, sc->NIL);
	sc->args = vector_arr(vec)[i];
	s_goto(sc, OP_P0LIST);
}

pcell op_read_pr(oscheme *sc)
{
	pcell x;

	switch(sc->tok) {
	case TOK_COMMENT:
		_skip_comment(sc);
		sc->tok = _get_token(sc);
		s_goto(sc, OP_RDSEXPR);
	case TOK_BLOCK_COMMENT:
		_skip_block_comment(sc);
		sc->tok = _get_token(sc);
		s_goto(sc, OP_RDSEXPR);
	case TOK_QUOTE_VEC:
		s_save(sc, OP_RDQUOTE_VEC, sc->NIL, sc->NIL);
		sc->tok = _get_token(sc);
		s_goto(sc, OP_RDSEXPR);
	case TOK_LPAREN:
		sc->tok = _get_token(sc);
		if(sc->tok == TOK_RPAREN) {
			s_return(sc, sc->NIL);
		} else if(sc->tok == TOK_DOT) {
			Error_0(sc, _T("syntax error -- illegal dot expression"));
		} else {
			s_save(sc, OP_RDLIST, sc->NIL, sc->NIL);
			s_goto(sc, OP_RDSEXPR);
		}
	case TOK_QUOTE:
		s_save(sc, OP_RDQUOTE, sc->NIL, sc->NIL);
		sc->tok = _get_token(sc);
		s_goto(sc, OP_RDSEXPR);
	case TOK_BQUOTE_VEC:
		s_save(sc, OP_RDQQUOTE_VEC, sc->NIL, sc->NIL);
		sc->tok = _get_token(sc);
		s_goto(sc, OP_RDSEXPR);
	case TOK_BQUOTE:
		s_save(sc, OP_RDQQUOTE, sc->NIL, sc->NIL);
		sc->tok = _get_token(sc);
		s_goto(sc, OP_RDSEXPR);
	case TOK_COMMA:
		s_save(sc, OP_RDUNQUOTE, sc->NIL, sc->NIL);
		sc->tok = _get_token(sc);
		s_goto(sc, OP_RDSEXPR);
	case TOK_ATMARK:
		s_save(sc, OP_RDUQTSP, sc->NIL, sc->NIL);
		sc->tok = _get_token(sc);
		s_goto(sc, OP_RDSEXPR);
	case TOK_ATOM:
		s_return(sc, mk_atom(sc, _readstr(sc)));
	case TOK_DQUOTE:
		s_return(sc, mk_string(sc, _readstrexp(sc, '"')));
	case TOK_REGEXP:
		{
			int	ci_flg = 0;
			TCHAR *pattern = _readstr_regexp(sc, '/');

			if(_read_in_buf(sc) == 'i') {
				_next_in_buf(sc);
				ci_flg = 1;
			}

			x = mk_regexp(sc, pattern, ci_flg);
			if(x == NULL) {
				Error_0(sc, sc->err_buf);
			} else {
				s_return(sc, x);
			}
		}
	case TOK_SHARP:
		x = mk_const(sc, _readstr_sharp(sc));
		if(x == sc->NIL) {
			Error_0(sc, _T("Undefined sharp expression"));
		} else {
			s_return(sc, x);
		}
	case TOK_EOF:
		s_goto(sc, OP_EOF);
	default:
		Error_0(sc, _T("syntax error -- illegal token"));
	}
}

pcell op_read_list(oscheme *sc)
{
	sc->args = cons(sc, sc->value, sc->args);

	for(;;) {
		sc->tok = _get_token(sc);

		if(sc->tok == TOK_COMMENT) {
			_skip_comment(sc);
		} else if(sc->tok == TOK_BLOCK_COMMENT) {
			_skip_block_comment(sc);
		} else {
			break;
		}
	}

	if(sc->tok == TOK_EOF) {
		Error_0(sc, _T("syntax error -- EOF inside a list"));
	} if(sc->tok == TOK_RPAREN) {
		s_return(sc, reverse_in_place(sc, sc->NIL, sc->args));
	} else if(sc->tok == TOK_DOT) {
		s_save(sc, OP_RDDOT, sc->args, sc->NIL);
		sc->tok = _get_token(sc);
		s_goto(sc, OP_RDSEXPR);
	} else {
		s_save(sc, OP_RDLIST, sc->args, sc->NIL);
		s_goto(sc, OP_RDSEXPR);
	}
}

pcell op_read_dot(oscheme *sc)
{
	sc->tok = _get_token(sc);
	if(sc->tok != TOK_RPAREN) {
		Error_0(sc, _T("syntax error -- illegal dot expression"));
	} else {
		s_return(sc, reverse_in_place(sc, sc->value, sc->args));
	}
}

pcell op_read_quote(oscheme *sc)
{
	s_return(sc, cons(sc, sc->QUOTE, cons(sc, sc->value, sc->NIL)));
}

pcell op_read_quote_vec(oscheme *sc)
{
	pcell	vec;
	int	len;

	len = list_length(sc, sc->value);
	if(len < 0) {
		Error_1(sc, _T("vector constructer require list:"), sc->args);
	}

	vec = mk_vector(sc, len, 0, sc->value);

	s_return(sc, vec);
}

pcell op_read_qquote(oscheme *sc)
{
	s_return(sc, cons(sc, sc->QQUOTE, cons(sc, sc->value, sc->NIL)));
}

pcell op_read_qquote_vec(oscheme *sc)
{
	s_return(sc, cons(sc, sc->QQUOTE_VEC, cons(sc, sc->value, sc->NIL)));
}

pcell op_read_unquote(oscheme *sc)
{
	s_return(sc, cons(sc, sc->UNQUOTE, cons(sc, sc->value, sc->NIL)));
}

pcell op_read_unquote_sp(oscheme *sc)
{
	s_return(sc, cons(sc, sc->UNQUOTESP, cons(sc, sc->value, sc->NIL)));
}

pcell op_print_list0(oscheme *sc)
{
	if(isvector(sc->args)) {
		port_out(sc, sc->outfp, _T("#("));
		sc->args = cons(sc, sc->args, mk_number(sc, 0));
		s_goto(sc, OP_PVECTOR);
	} else if(!ispair(sc->args)) {
		printatom(sc, sc->args, sc->print_flag);
		s_return(sc, sc->UNDEF);
	} else if(car(sc->args) == sc->QUOTE && ok_abbrev(sc, cdr(sc->args))) {
		port_out(sc, sc->outfp, _T("'"));
		sc->args = cadr(sc->args);
		s_goto(sc, OP_P0LIST);
	} else if(car(sc->args) == sc->QUOTE_VEC && ok_abbrev(sc, cdr(sc->args))) {
		/* ここにはこない？ */
		port_out(sc, sc->outfp, _T("'#"));
		sc->args = cadr(sc->args);
		s_goto(sc, OP_P0LIST);
	} else if(car(sc->args) == sc->QQUOTE && ok_abbrev(sc, cdr(sc->args))) {
		port_out(sc, sc->outfp, _T("`"));
		sc->args = cadr(sc->args);
		s_goto(sc, OP_P0LIST);
	} else if(car(sc->args) == sc->QQUOTE_VEC && ok_abbrev(sc, cdr(sc->args))) {
		port_out(sc, sc->outfp, _T("`#"));
		sc->args = cadr(sc->args);
		s_goto(sc, OP_P0LIST);
	} else if(car(sc->args) == sc->UNQUOTE && ok_abbrev(sc, cdr(sc->args))) {
		port_out(sc, sc->outfp, _T(","));
		sc->args = cadr(sc->args);
		s_goto(sc, OP_P0LIST);
	} else if(car(sc->args) == sc->UNQUOTESP && ok_abbrev(sc, cdr(sc->args))) {
		port_out(sc, sc->outfp, _T(",@"));
		sc->args = cadr(sc->args);
		s_goto(sc, OP_P0LIST);
	} else {
		port_out(sc, sc->outfp, _T("("));
		s_save(sc, OP_P1LIST, cdr(sc->args), sc->NIL);
		sc->args = car(sc->args);
		s_goto(sc, OP_P0LIST);
	}
}

pcell op_print_list1(oscheme *sc)
{
	if(ispair(sc->args)) {
		s_save(sc, OP_P1LIST, cdr(sc->args), sc->NIL);
		port_out(sc, sc->outfp, _T(" "));
		sc->args = car(sc->args);
		s_goto(sc, OP_P0LIST);
	} else if(sc->args != sc->NIL) {
		s_save(sc, OP_P1LIST, sc->NIL, sc->NIL);
		port_out(sc, sc->outfp, _T(" . "));
		s_goto(sc, OP_P0LIST);
	} else {
		port_out(sc, sc->outfp, _T(")"));
		s_return(sc, sc->UNDEF);
	}
}

pcell op_list_length(oscheme *sc)
{
	pcell list = car(sc->args);
	int len = list_length(sc, list);
	if(len < 0) {
		_stprintf(sc->err_buf, _T("%s: %s required:"), _T("length"), _T("list"));
		Error_1(sc, sc->err_buf, list);
	}
	s_return(sc, mk_number(sc, len));
}

pcell op_is_list(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	s_retbool(sc, islist(sc, argv[0]));
}

pcell op_gensym(oscheme *sc)
{
	TCHAR	name[100];
	pcell	x;

	_stprintf(name, _T("|G:%d|"), sc->gensym_counter);
	x = mk_symbol(sc, name);
	symnum(x) = sc->gensym_counter;

	sc->gensym_counter++;
	
	s_return(sc, x);
}

pcell op_is_gensym(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	s_retbool(sc, isgensym(argv[0]));
}

pcell op_is_output_port(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	if(!isport(argv[0]) || !is_outport(argv[0])) s_return(sc, sc->F);
	s_return(sc, sc->T);
}

pcell op_is_input_port(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	if(!isport(argv[0]) || !is_inport(argv[0])) s_return(sc, sc->F);
	s_return(sc, sc->T);
}

pcell op_is_port_closed(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_PORT);
	s_retbool(sc, is_closed_port(argv[0]));
}

pcell op_is_eof_object(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	s_retbool(sc, iseof_object(argv[0]));
}

pcell op_is_file_exists(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_STRING);
	s_retbool(sc, is_file_exists(strvalue(argv[0])));
}

pcell op_current_output_port(oscheme *sc)
{
	s_return(sc, sc->outfp);
}

pcell op_current_input_port(oscheme *sc)
{
	s_return(sc, sc->infp);
}

static int _check_unicode(FILE *fp)
{
	size_t		read_cnt;
	unsigned char	buf[1024];
	unsigned char	*p;

	fread(buf, 1, 2, fp);
	if(buf[0] == 0xff && buf[1] == 0xfe) return 1;

	fseek(fp, 0, SEEK_SET);

	read_cnt = fread(buf, sizeof(unsigned char), sizeof(buf), fp);
	fseek(fp, 0, SEEK_SET);

	p = buf;
	for(; read_cnt; read_cnt--) {
		if(*p == '\0') return 1;
		p++;
	}

	return 0;
}

pcell _op_open_file(oscheme *sc, int flg, TCHAR *mode, uint buf_size)
{
	pcell	p;
	FILE	*fp;
	GET_ARGS(sc, 1, 1, TC_STRING);

	fp = _tfopen(strvalue(argv[0]), mode);
	if(fp == NULL) Error_1(sc, _T("file cannot open"), argv[0]);

	if(*mode == 'r') {
		if(_check_unicode(fp)) {
			flg |= PORT_UNICODE;
		}
	}

	p = mk_port(sc, fp, flg, strvalue(argv[0]), buf_size);
	s_return(sc, p);
}

pcell op_open_output_file(oscheme *sc)
{
	return _op_open_file(sc, PORT_OUTPORT, _T("wb"), 0);
}

pcell op_get_output_string(oscheme *sc)
{
	pcell	x;
	GET_ARGS(sc, 1, 1, TC_OSTRPORT);
	x = mk_string(sc, port_buf(argv[0]));
	s_return(sc, x);
}

pcell op_clear_output_string(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_OSTRPORT);
	_clear_outportbuf(sc, argv[0]);
	s_return(sc, sc->UNDEF);
}

pcell op_open_output_string(oscheme *sc)
{
	pcell	port;
	port = mk_string_output_port(sc, 64);
	s_return(sc, port);
}

pcell op_open_input_file(oscheme *sc)
{
	return _op_open_file(sc, PORT_INPORT, _T("rb"),
		INPORT_READ_SIZE + FILL_INPORT_THR);
}

pcell op_open_input_string(oscheme *sc)
{
	pcell	port;
	GET_ARGS(sc, 1, 1, TC_STRING);
	port = mk_string_input_port(sc, strvalue(argv[0]));
	s_return(sc, port);
}

pcell op_close_output_port(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_OPORT);
	if(!is_closed_port(argv[0])) close_port(sc, argv[0]);
	s_return(sc, sc->UNDEF);
}

pcell op_close_input_port(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_IPORT);
	if(!is_closed_port(argv[0])) close_port(sc, argv[0]);
	s_return(sc, sc->UNDEF);
}

pcell op_macroexpand_1(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	sc->value = argv[0];
	sc->args = sc->F;
	s_goto(sc, OP_MACROEXPAND_MAIN);
}

pcell op_macroexpand(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	sc->value = argv[0];
	sc->args = sc->T;
	s_goto(sc, OP_MACROEXPAND_MAIN);
}

pcell op_macroexpand_main(oscheme *sc)
{
	pcell	x, y;

	if(!ispair(sc->value)) {
		s_return(sc, sc->value);
	}
	x = car(sc->value);

	if(issymbol(x)) {
		y = _eval_symbol(sc, x);
		if(y == NULL) Error_1(sc, _T("unbounded variable"), x);

		if(ismacro(y)) {
			if(sc->args == sc->T) {
				s_save(sc, OP_MACROEXPAND_MAIN, sc->T, sc->NIL);
			}
			sc->args = cdr(sc->value);
			sc->code = y;
			s_goto(sc, OP_APPLY);
		}
	}

	s_return(sc, sc->value);
}

pcell op_values(oscheme *sc)
{
	if(!ispair(sc->args)) {
		return _s_return_values(sc, sc->UNDEF, sc->NIL);
	}
	return _s_return_values(sc, car(sc->args), sc->args);
}

pcell op_call_with_values0(oscheme *sc)
{
	GET_ARGS(sc, 2, 2, TC_PROCEDURE TC_PROCEDURE);
	s_save(sc, OP_CALL_WITH_VALUES1, sc->NIL, argv[1]);
	sc->code = argv[0];
	sc->args = sc->NIL;
	s_goto(sc, OP_APPLY);
}

pcell op_call_with_values1(oscheme *sc)
{
	if(sc->values != NULL) {
		sc->args = sc->values;
	} else {
		sc->args = cons(sc, sc->value, sc->NIL);
	}
	s_goto(sc, OP_APPLY);
}

pcell op_is_regexp(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	s_retbool(sc, isregexp(argv[0]));
}

pcell op_regexp_to_string(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_REGEXP);
	s_return(sc, mk_string(sc, regexp_pattern(argv[0])));
}

pcell op_string_to_regexp(oscheme *sc)
{
	int		ci_flg = 0;
	pcell	regexp;
	GET_ARGS(sc, 1, 2, TC_STRING TC_ANY);

	if(argc == 2 && istrue(sc, argv[1])) ci_flg = 1;

	regexp = mk_regexp(sc, strvalue(argv[0]), ci_flg);
	if(regexp == NULL) {
		Error_0(sc, sc->err_buf);
	} else {
		s_return(sc, regexp);
	}
}

static int _rxmatch_main(oscheme *sc, HREG_DATA hreg, TCHAR *str, INT_PTR start)
{
	INT_PTR	result_start, result_len;

	return oreg_exec_str(hreg, str, start, &result_start, &result_len);
}

pcell op_rxmatch(oscheme *sc)
{
	GET_ARGS(sc, 2, 2, TC_REGEXP TC_STRING);

	if(_rxmatch_main(sc, regexp_hregdata(argv[0]),
		strvalue(argv[1]), 0) == OREGEXP_FOUND) {
		s_return(sc, mk_regmatch(sc, argv[0], argv[1]));
	}

	s_return(sc, sc->F);
}

struct _regexp_replace_cb_st {
	oscheme *sc;
	pcell	reg;
	pcell	str;
	pcell	replace_code;
};

TCHAR *_regexp_replace_cb(void *callback_arg, HREG_DATA reg_data)
{
	struct _regexp_replace_cb_st *arg =
		(struct _regexp_replace_cb_st *)callback_arg;
	oscheme *sc = arg->sc;
	pcell regmatch = mk_regmatch(sc, arg->reg, arg->str);
	pcell r;

	s_save(sc, OP_EXIT, sc->NIL, sc->NIL);
	sc->args = cons(sc, regmatch, sc->NIL);
	sc->code = arg->replace_code;
	r = eval_cycle(sc, OP_APPLY, 0);
	if(sc->cur_frame == 0) return NULL;

	return oscheme_tostring(sc, r);
}

pcell _regexp_replace_main(oscheme *sc, pcell reg,
	pcell str, pcell replace, int all_flg)
{
	pcell			x;
	TCHAR result_buf[1024];
	TCHAR *result_str;

	if(isstring(replace)) {
		result_str = oreg_replace_str(regexp_hregdata(reg), strvalue(str),
			strvalue(replace), all_flg, result_buf, sizeof(result_buf));
	} else {
		struct _regexp_replace_cb_st arg = {sc, reg, str, replace};

		s_save(sc, OP_NOP, sc->args, sc->NIL);

		result_str = oreg_replace_str_cb(regexp_hregdata(reg), strvalue(str),
			_regexp_replace_cb, &arg,
			all_flg, result_buf, sizeof(result_buf));
		if(sc->cur_frame == 0) {
			if(sc->interactive) {
				s_save(sc, OP_T0LVL, sc->NIL, sc->NIL);
				s_goto(sc, OP_NEWLINE);
			} else {
				return sc->NIL;
			}
		}
	}
	if(result_str == NULL) FatalError(sc, _T("no memory"));

	if(result_str == strvalue(str)) s_return(sc, str);

	x = mk_string(sc, result_str);
	if(result_str != result_buf) free(result_str);

	s_return(sc, x);
}

pcell op_regexp_replace(oscheme *sc)
{
	GET_ARGS(sc, 3, 3, TC_REGEXP TC_STRING TC_STRING_OR_PROCEDURE);

	return _regexp_replace_main(sc, argv[0], argv[1], argv[2], 0);
}

pcell op_regexp_replace_all(oscheme *sc)
{
	GET_ARGS(sc, 3, 3, TC_REGEXP TC_STRING TC_STRING_OR_PROCEDURE);

	return _regexp_replace_main(sc, argv[0], argv[1], argv[2], 1);
}

pcell op_rxmatch_substring(oscheme *sc)
{
	INT_PTR		idx = 0;
	INT_PTR	start, end;
	size_t  len;
	pcell	org_str, s;
	GET_ARGS(sc, 1, 2, TC_REGMATCH TC_NUMBER);
	if(isfalse(sc, argv[0])) s_return(sc, argv[0]);

	if(argc == 2) idx = ivalue(argv[1]);
	if(idx < 0 || idx >= regmatch_cnt(argv[0])) {
		Error_0(sc, _T("submatch index out of range"));
	}

	start = regmatch_data(argv[0])[idx]._start_pt;
	if(start == -1) s_return(sc, sc->F);
	end = regmatch_data(argv[0])[idx]._end_pt;
	len = end - start;
	org_str = regmatch_str(argv[0]);

	if(len == 0) {
		s_return(sc, mk_string(sc, _T("")));
	}

	if(end > (int)strbufsize(org_str)) {
		Error_0(sc, _T("out of string buffer"));
	}

	org_str = regmatch_str(argv[0]);
	s = mk_string_cell(sc, len);
	memcpy(strvalue(s), strvalue(org_str) + start, len * sizeof(TCHAR));
	strvalue(s)[len] = '\0';

	s_return(sc, s);
}

pcell op_rxmatch_start(oscheme *sc)
{
	INT_PTR		idx = 0;
	TCHAR	*p, *p_end;
	GET_ARGS(sc, 1, 2, TC_REGMATCH TC_NUMBER);
	if(isfalse(sc, argv[0])) s_return(sc, argv[0]);

	if(argc == 2) idx = ivalue(argv[1]);
	if(idx < 0 || idx >= regmatch_cnt(argv[0])) {
		Error_0(sc, _T("submatch index out of range"));
	}
	if(regmatch_data(argv[0])[idx]._start_pt == -1) s_return(sc, sc->F);

	p = strvalue(regmatch_str(argv[0]));
	p_end = p + regmatch_data(argv[0])[idx]._start_pt;
	s_return(sc, mk_number(sc, string_length_range(p, p_end)));
}

pcell op_rxmatch_end(oscheme *sc)
{
	INT_PTR		idx = 0;
	TCHAR	*p, *p_end;
	GET_ARGS(sc, 1, 2, TC_REGMATCH TC_NUMBER);
	if(isfalse(sc, argv[0])) s_return(sc, argv[0]);

	if(argc == 2) idx = ivalue(argv[1]);
	if(idx < 0 || idx >= regmatch_cnt(argv[0])) {
		Error_0(sc, _T("submatch index out of range"));
	}
	if(regmatch_data(argv[0])[idx]._start_pt == -1) s_return(sc, sc->F);

	p = strvalue(regmatch_str(argv[0]));
	p_end = p + regmatch_data(argv[0])[idx]._end_pt;
	s_return(sc, mk_number(sc, string_length_range(p, p_end)));
}

pcell op_is_regmatch(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	s_retbool(sc, isregmatch(argv[0]));
}

pcell op_rxmatch_num_matches(oscheme *sc)
{
	int	num = 0;
	GET_ARGS(sc, 1, 1, TC_REGMATCH);
	if(isregmatch(argv[0])) num = regmatch_cnt(argv[0]);
	s_return(sc, mk_number(sc, num));
}

pcell op_mk_hash(oscheme *sc)
{
	pcell   h;
	TCHAR   *hash_type;
	GET_ARGS(sc, 0, 1, TC_SYMBOL);

	if(argc == 1) {
		if(_tcscmp(symname(argv[0]), _T("eq?")) != 0 &&
			_tcscmp(symname(argv[0]), _T("eqv?")) != 0 &&
			_tcscmp(symname(argv[0]), _T("equal?")) != 0 &&
			_tcscmp(symname(argv[0]), _T("string=?")) != 0) {
			Error_1(sc, _T("unsupported hash type: "), argv[0]);
		}
		hash_type = symname(argv[0]);
	} else {
		hash_type = _T("eq?");
	}
	h = mk_hash(sc, 256, hash_type);

	s_return(sc, h);
}

pcell op_hash_table_type(oscheme *sc)
{
	TCHAR *hash_type = _T("unknown");
	GET_ARGS(sc, 1, 1, TC_HASH);

	switch(hash_type(argv[0])) {
	case HASH_TYPE_EQ: hash_type = _T("eq?"); break;
	case HASH_TYPE_EQV: hash_type = _T("eqv?"); break;
	case HASH_TYPE_EQUAL: hash_type = _T("equal?"); break;
	case HASH_TYPE_STRING: hash_type = _T("string=?"); break;
	}
	s_return(sc, mk_symbol(sc, hash_type));
}

pcell op_hash_put(oscheme *sc)
{
	_hash_entry *entry;
	GET_ARGS(sc, 3, 3, TC_HASH TC_ANY TC_ANY);

	if(hash_type(argv[0]) == HASH_TYPE_STRING)
		_TYPE_CHK(sc, TC_STRING, argv[1]);

	entry = (_hash_entry *)ohash_enter(hash_hashdata(argv[0]), &argv[1]);
	if(entry == NULL) {
		FatalError(sc, _T("no memory"));
		return NULL;
	}

	entry->_data = argv[2];

	s_return(sc, sc->UNDEF);
}

pcell op_hash_delete(oscheme *sc)
{
	_hash_entry *entry;
	GET_ARGS(sc, 2, 2, TC_HASH TC_ANY);

	entry = (_hash_entry *)ohash_remove(hash_hashdata(argv[0]), &argv[1]);
	if(entry) s_return(sc, sc->T);
	s_return(sc, sc->F);
}

pcell op_hash_get(oscheme *sc)
{
	_hash_entry *entry;
	GET_ARGS(sc, 2, 3, TC_HASH TC_ANY TC_ANY);

	entry = (_hash_entry *)ohash_search(hash_hashdata(argv[0]), &argv[1]);
	if(entry == NULL) {
		if(argc == 3) s_return(sc, argv[2]);
		Error_1(sc, _T("hash-table-get: key not found"), argv[1]);
	}

	s_return(sc, entry->_data);
}

pcell op_hash_exists(oscheme *sc)
{
	_hash_entry *entry;
	GET_ARGS(sc, 2, 3, TC_HASH TC_ANY TC_ANY);

	entry = (_hash_entry *)ohash_search(hash_hashdata(argv[0]), &argv[1]);
	s_retbool(sc, (entry != NULL));
}

pcell op_hash_num_entries(oscheme *sc)
{
	pcell	n;
	GET_ARGS(sc, 1, 1, TC_HASH);

	n = mk_number(sc, ohash_num_entries(hash_hashdata(argv[0])));
	s_return(sc, n);
}

pcell op_is_hash_table(oscheme *sc)
{
	GET_ARGS(sc, 1, 1, TC_ANY);
	s_retbool(sc, ishash(argv[0]));
}

pcell op_hash_table_for_each0(oscheme *sc)
{
	pcell	x;
	GET_ARGS(sc, 2, 2, TC_HASH TC_PROCEDURE);

	x = mk_hash_iter(sc, hash_hashdata(argv[0]));
	sc->args = cons(sc, x, sc->args);

	s_goto(sc, OP_HASH_TABLE_FOR_EACH1);
}

pcell op_hash_table_for_each1(oscheme *sc)
{
	_hash_entry *entry;

	entry = (_hash_entry *)ohash_iter_next(hash_iter(car(sc->args)));
	if(entry == NULL) {
		s_return(sc, sc->UNDEF);
	}

	s_save(sc, OP_HASH_TABLE_FOR_EACH1, sc->args, sc->code);
	sc->code = caddr(sc->args);
	sc->args = cons(sc, entry->_data, sc->NIL);
	sc->args = cons(sc, entry->_key, sc->args);
	s_goto(sc, OP_APPLY);
}

pcell op_hash_table_map0(oscheme *sc)
{
	pcell	x;
	GET_ARGS(sc, 2, 2, TC_HASH TC_PROCEDURE);

	x = mk_hash_iter(sc, hash_hashdata(argv[0]));
	sc->args = cons(sc, x, sc->args);
	sc->args = cons(sc, sc->NIL, sc->args);
	sc->value = sc->NIL;

	s_goto(sc, OP_HASH_TABLE_MAP1);
}

pcell op_hash_table_map1(oscheme *sc)
{
	_hash_entry *entry;
	car(sc->args) = cons(sc, sc->value, car(sc->args));

	entry = (_hash_entry *)ohash_iter_next(hash_iter(cadr(sc->args)));
	if(entry == NULL) {
		pcell x = reverse_in_place(sc, sc->NIL, car(sc->args));
		s_return(sc, cdr(x));
	}

	s_save(sc, OP_HASH_TABLE_MAP1, sc->args, sc->code);
	sc->code = cadddr(sc->args);
	sc->args = cons(sc, entry->_data, sc->NIL);
	sc->args = cons(sc, entry->_key, sc->args);
	s_goto(sc, OP_APPLY);
}

/* object system */
pcell mk_class(oscheme *sc, const TCHAR *name, pcell supers,
	pcell slots, slot_accesser *accessers)
{
	pcell	x = get_cell(sc, supers, slots);

	typeflag(x) = T_CLASS | T_ATOM;
	ptrvalue(x) = oscm_malloc(sc, sizeof(oclass) + _tcslen(name) * sizeof(TCHAR));

	class_supers(x) = supers;
	class_slots(x) = slots;
	class_accessers(x) = accessers;
	class_slot_cnt(x) = list_length(sc, slots);
	_tcscpy(class_name(x), name);

	return x;
}

int object_find_vslot_idx(oscheme *sc, pcell o, pcell s)
{
	pcell	x;
	int		idx = 0;
	for(x = class_slots(object_class(o)); x != sc->NIL; x = cdr(x), idx++) {
		if(issymbol(car(x)) && car(x) == s) return idx;
	}
	return -1;
}

pcell object_slot_ref(oscheme *sc, pcell o, pcell s)
{
	int		idx = object_find_vslot_idx(sc, o, s);
	slot_accesser *accessers = class_accessers(object_class(o));
	if(idx < 0) {
		_tcscpy(sc->err_buf, _T("slot-ref: object doesn't have such slot:"));
		return NULL;
	}

	if(accessers != NULL && accessers[idx].getter != NULL) {
		return accessers[idx].getter(sc, o, idx);
	}

	return object_vslots(o)[idx];
}

pcell object_slot_ref_by_name(oscheme *sc, pcell o, const TCHAR *slot_name)
{
	pcell	s = mk_symbol(sc, slot_name);
	return object_slot_ref(sc, o, s);
}

int object_slot_set(oscheme *sc, pcell o, pcell s, pcell v)
{
	int		idx = object_find_vslot_idx(sc, o, s);
	slot_accesser *accessers = class_accessers(object_class(o));
	if(idx < 0) {
		_tcscpy(sc->err_buf, _T("slot-ref: object doesn't have such slot:"));
		return -1;
	}

	if(accessers != NULL && accessers[idx].setter != NULL) {
		return accessers[idx].setter(sc, o, v, idx);
	}

	object_vslots(o)[idx] = v;
	return idx;
}

int object_slot_set_by_name(oscheme *sc, pcell o, const TCHAR *slot_name,
	pcell v)
{
	pcell	s = mk_symbol(sc, slot_name);
	return object_slot_set(sc, o, s, v);
}

pcell mk_object(oscheme *sc, pcell k)
{
	pcell	x = get_cell(sc, k, sc->NIL);
	int		i;
	int		slot_cnt = class_slot_cnt(k);

	typeflag(x) = T_OBJECT | T_ATOM;
	ptrvalue(x) = oscm_malloc(sc, sizeof(oobject) + (sizeof(pcell) * slot_cnt));

	object_class(x) = k;
	for(i = 0; i < slot_cnt; i++) {
		object_vslots(x)[i] = sc->UNDEF;
	}

	return x;
}

pcell op_define_class(oscheme *sc)
{
	pcell	x;
	GET_ARGS(sc, 3, 3, TC_STRING TC_LIST TC_LIST);

	x = mk_class(sc, strvalue(argv[0]), argv[1], argv[2], NULL);

	s_return(sc, x);
}

pcell op_make_object(oscheme *sc)
{
	pcell	x;
	GET_ARGS(sc, 1, 1, TC_CLASS);

	x = mk_object(sc, argv[0]);

	s_return(sc, x);
}

pcell op_slot_ref(oscheme *sc)
{
	pcell	x;
	GET_ARGS(sc, 2, 2, TC_OBJECT TC_SYMBOL);

	x = object_slot_ref(sc, argv[0], argv[1]);
	if(x == NULL) Error_1(sc, sc->err_buf, argv[1]);

	s_return(sc, x);
}

pcell op_slot_set(oscheme *sc)
{
	int		idx;
	GET_ARGS(sc, 3, 3, TC_OBJECT TC_SYMBOL TC_ANY);

	idx = object_slot_set(sc, argv[0], argv[1], argv[2]);
	if(idx < 0) Error_1(sc, sc->err_buf, argv[2]);

	s_return(sc, sc->UNDEF);
}

pcell op_sys_time(oscheme *sc)
{
	time_t t = time(NULL);
	s_return(sc, mk_number(sc, t));
}

pcell mk_object_by_name(oscheme *sc, const TCHAR *name)
{
	pcell k;

	k = _eval_symbol(sc, mk_symbol(sc, name));
	if(k == NULL) FatalError(sc, _T("class not found"));

	return mk_object(sc, k);
}

int is_instance_of(oscheme *sc, pcell o, const TCHAR *name)
{
	pcell k = _eval_symbol(sc, mk_symbol(sc, name));
	if(k == NULL) FatalError(sc, _T("class not found"));

	return (object_class(o) == k);
}

pcell mk_sys_stat(oscheme *sc, stat_st *pst)
{
	pcell	x;
	oscheme_set_dont_gc_flg(sc);

	x = mk_object_by_name(sc, _T("<sys-stat>"));

	object_slot_set_by_name(sc, x, _T("size"), mk_number(sc, pst->st_size));
	object_slot_set_by_name(sc, x, _T("atime"), mk_number(sc, pst->st_atime));
	object_slot_set_by_name(sc, x, _T("mtime"), mk_number(sc, pst->st_mtime));
	object_slot_set_by_name(sc, x, _T("ctime"), mk_number(sc, pst->st_ctime));

	oscheme_clear_dont_gc_flg(sc);

	return x;
}

pcell mk_sys_tm(oscheme *sc, struct tm *ptm)
{
	pcell	x;

	oscheme_set_dont_gc_flg(sc);

	x = mk_object_by_name(sc, _T("<sys-tm>"));

	object_slot_set_by_name(sc, x, _T("sec"), mk_number(sc, ptm->tm_sec));
	object_slot_set_by_name(sc, x, _T("min"), mk_number(sc, ptm->tm_min));
	object_slot_set_by_name(sc, x, _T("hour"), mk_number(sc, ptm->tm_hour));
	object_slot_set_by_name(sc, x, _T("mday"), mk_number(sc, ptm->tm_mday));
	object_slot_set_by_name(sc, x, _T("mon"), mk_number(sc, ptm->tm_mon));
	object_slot_set_by_name(sc, x, _T("year"), mk_number(sc, ptm->tm_year));
	object_slot_set_by_name(sc, x, _T("wday"), mk_number(sc, ptm->tm_wday));
	object_slot_set_by_name(sc, x, _T("yday"), mk_number(sc, ptm->tm_yday));
	object_slot_set_by_name(sc, x, _T("isdst"), mk_number(sc, ptm->tm_isdst));

	oscheme_clear_dont_gc_flg(sc);

	return x;
}

pcell op_sys_localtime(oscheme *sc)
{
	struct tm	tmdata;
	time_t	t;
	pcell	x;
	GET_ARGS(sc, 1, 1, TC_NUMBER);

	t = ivalue(argv[0]);
	tmdata = *localtime(&t);

	x = mk_sys_tm(sc, &tmdata);

	s_return(sc, x);
}

void make_tm_from_sys_tm(oscheme *sc, pcell x, struct tm *ptm)
{
	ptm->tm_sec = (int)safe_ivalue(object_slot_ref_by_name(sc, x, _T("sec")));
	ptm->tm_min = (int)safe_ivalue(object_slot_ref_by_name(sc, x, _T("min")));
	ptm->tm_hour = (int)safe_ivalue(object_slot_ref_by_name(sc, x, _T("hour")));
	ptm->tm_mday = (int)safe_ivalue(object_slot_ref_by_name(sc, x, _T("mday")));
	ptm->tm_mon = (int)safe_ivalue(object_slot_ref_by_name(sc, x, _T("mon")));
	ptm->tm_year = (int)safe_ivalue(object_slot_ref_by_name(sc, x, _T("year")));
	ptm->tm_wday = (int)safe_ivalue(object_slot_ref_by_name(sc, x, _T("wday")));
	ptm->tm_yday = (int)safe_ivalue(object_slot_ref_by_name(sc, x, _T("yday")));
	ptm->tm_isdst = (int)safe_ivalue(object_slot_ref_by_name(sc, x, _T("isdst")));
}

pcell op_sys_strftime(oscheme *sc)
{
	struct tm t;
	TCHAR buf[1024];
	GET_ARGS(sc, 2, 2, TC_STRING TC_OBJECT);

	if(!is_instance_of(sc, argv[1], _T("<sys-tm>"))) {
		_stprintf(sc->err_buf, _T("%s: %s required:"), _T("sys-strftime"), _T("<sys-tm>"));
		Error_1(sc, sc->err_buf, argv[1]);
	}

	make_tm_from_sys_tm(sc, argv[1], &t);
	_tcsftime(buf, sizeof(buf)/sizeof(buf[0]), strvalue(argv[0]), &t);

	s_return(sc, mk_string(sc, buf));
}

pcell op_sys_stat(oscheme *sc)
{
	stat_st st;
	GET_ARGS(sc, 1, 1, TC_STRING);

	if(_tstat(strvalue(argv[0]), &st)) {
		_stprintf(sc->err_buf, _T("%s: stat error:"), _T("sys-stat"));
		Error_1(sc, sc->err_buf, argv[0]);
	}

	s_return(sc, mk_sys_stat(sc, &st));
}

#if defined(WIN32) && defined(_UNICODE)
#define oscheme_getenv _wgetenv
#else
#define oscheme_getenv getenv
#endif

pcell op_sys_getenv(oscheme* sc)
{
	GET_ARGS(sc, 1, 1, TC_STRING);
	const TCHAR* name = strvalue(argv[0]);
	const TCHAR* val = oscheme_getenv(name);
	if (!val) s_return(sc, mk_string(sc, _T("")));
	s_return(sc, mk_string(sc, val));
}

/*-----------------------------------------------------------------*/

#define CMD_BUF_SIZE	(size_t)1024

pcell op_win_exec(oscheme *sc)
{
	pcell	x;
	TCHAR	cmd[CMD_BUF_SIZE] = _T("");
	size_t	l = CMD_BUF_SIZE - 2;
	TCHAR	*p;

	_tcscpy(cmd, _T(""));
	for(x = sc->args; x != sc->NIL; x = cdr(x)) {
		_TYPE_CHK(sc, TC_STRING, car(x));
		p = strvalue(car(x));
		if((int)_tcslen(p) > l) Error_0(sc, _T("command is too long"));

		_tcscat(cmd, strvalue(car(x)));
		_tcscat(cmd, _T(" "));
		l -= (_tcslen(p) + 1);
	}

#ifdef WIN32
	{
		PROCESS_INFORMATION		pi;
		struct _STARTUPINFOW	si;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);

		if(CreateProcessW(NULL, cmd, NULL, NULL, FALSE,
			0, NULL, NULL, &si, &pi) == FALSE) {
			Error_1(sc, _T("win-exec: can not execute"), mk_string(sc, cmd));
		}

		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
#else
	printf("%s\n", cmd);
#endif

	s_return(sc, mk_number(sc, 0));
}

pcell op_win_get_clipboard_text(oscheme *sc)
{
	pcell	s;

#ifdef WIN32
	{
		HANDLE hData;
		TCHAR *pstr;

		if(!IsClipboardFormatAvailable(CF_UNICODETEXT)) s_return(sc, mk_string(sc, _T("")));
		if(!OpenClipboard(NULL)) s_return(sc, mk_string(sc, _T("")));
		hData = GetClipboardData(CF_UNICODETEXT);
		if(!hData) s_return(sc, mk_string(sc, _T("")));
		
		pstr =(TCHAR *)GlobalLock(hData);
		s = mk_string(sc, pstr);

		GlobalUnlock(hData);
		CloseClipboard();
	}
#else
	s = mk_string(sc, _T(""));
#endif
	s_return(sc, s);
}

/*-----------------------------------------------------------------*/

typedef pcell (*_dispatch_func)(oscheme *sc);

struct dispatch_table_st {
	_dispatch_func	func;
	enum scheme_opcodes op;
	TCHAR	*proc_name;
};

struct dispatch_table_st dispatch_table[] = {
#define _OP_DEF(FN,OP,NAME) {FN,OP,NAME},
	_OP_DEFINE_TABLE_
};

const TCHAR *proc_name(enum scheme_opcodes op)
{
	static const TCHAR *null_str = _T("");
	TCHAR *name = dispatch_table[op].proc_name;
	if(name == NULL) return null_str;
	return name;
}

pcell eval_cycle(oscheme *sc, enum scheme_opcodes op, int interactive)
{
	int		back_interactive = sc->interactive;

#ifdef ENABLE_CANCEL
	sc->cancel_flg = 0;
#endif

	sc->interactive = interactive;
	sc->operator = op;

	for(;;) {
		if((*dispatch_table[sc->operator].func)(sc) == sc->NIL) break;
	}

	sc->interactive = back_interactive;
	return sc->value;
}

static pcell _load_file(oscheme *sc, const TCHAR *file_name)
{
	pcell	p;
	pcell	back_p = sc->infp;
	FILE  *fp;
	int		flg = PORT_INPORT;

	fp = _tfopen(file_name, _T("rb"));
	if(fp == NULL) return sc->F;

	if(_check_unicode(fp)) {
		flg |= PORT_UNICODE;
	}

	p = mk_port(sc, fp, flg, file_name,
		INPORT_READ_SIZE + FILL_INPORT_THR);
	sc->args = cons(sc, p, sc->NIL);

	eval_cycle(sc, OP_LOAD_T0LVL, 0);

	close_port(sc, p);
	sc->args = sc->NIL;
	sc->infp = back_p;

	return sc->T;
}

static pcell _eval_str(oscheme *sc, const TCHAR *str)
{
	pcell	p, r;
	pcell	back_p = sc->infp;

	p = mk_string_input_port(sc, (TCHAR *)str);
	sc->args = cons(sc, p, sc->NIL);

	r = eval_cycle(sc, OP_LOAD_T0LVL, 0);

	close_port(sc, p);
	sc->args = sc->NIL;
	sc->infp = back_p;

	return r;
}

/* ========== initialization of internal keywords ========== */
void mk_syntax(oscheme *sc, enum scheme_opcodes op, TCHAR *name)
{
	pcell x, y;

	x = mk_symbol(sc, name);
	y = get_cell(sc, x, sc->NIL);
	typeflag(y) = (T_SYNTAX | T_ATOM);
	syntaxnum(y) = op;
	push_env(sc, x, y);
}

void mk_proc(oscheme *sc, enum scheme_opcodes op, TCHAR *name)
{
	pcell x, y;

	x = mk_symbol(sc, name);
	y = get_cell(sc, x, sc->NIL);
	typeflag(y) = (T_PROC | T_ATOM);
	procnum(y) = op;
	push_env(sc, x, y);
}

/* ========== initialization  ========== */
static void _init_global_vars(oscheme *sc)
{
	sc->last_cell_seg = -1;

	/* init NIL */
	sc->NIL = &(sc->_NIL);
	typeflag(sc->NIL) = (T_ATOM | MARK);
	car(sc->NIL) = cdr(sc->NIL) = sc->NIL;

	/* init T */
	sc->T = &(sc->_T);
	typeflag(sc->T) = (T_ATOM | MARK);
	car(sc->T) = cdr(sc->T) = sc->T;

	/* init F */
	sc->F = &(sc->_F);
	typeflag(sc->F) = (T_ATOM | MARK);
	car(sc->F) = cdr(sc->F) = sc->F;

	/* init UNDEF */
	sc->UNDEF = &(sc->_UNDEF);
	typeflag(sc->UNDEF) = (T_ATOM | MARK);
	car(sc->UNDEF) = cdr(sc->UNDEF) = sc->UNDEF;

	/* init environment */
	sc->global_env = sc->NIL;

	/* init registers */
	sc->value = sc->NIL;
	sc->args = sc->NIL;
	sc->envir = sc->NIL;
	sc->code = sc->NIL;
	sc->oblist = sc->NIL;

	/* init stack */
	sc->dump_stack = NULL;
	sc->cur_frame = 0;
	sc->dump_stack_cnt = 0;

	sc->gensym_counter = 1;
	sc->interactive = 0;
	sc->gc_verbose = 0;
	sc->used_cell_cnt = 0;
	sc->all_cell_cnt = 0;
	sc->global_var = sc->NIL;
	sc->dont_gc_flg = 0;

	/* port */
	sc->infp = sc->NIL;
	sc->outfp = sc->NIL;
	sc->errfp = sc->NIL;
	sc->iofp = sc->NIL;
	sc->tmpfp = sc->NIL;

#ifdef ENABLE_CANCEL
	sc->cancel_flg = 0;
	sc->cancel_counter = 0;
	sc->cancel_fvirt = 0;
	sc->cancel_keycode = VK_CANCEL;
#endif

	sc->aset = NULL;
	sc->aset_allocated_size = 0;
	sc->aset_gc_threshold_size = GC_THRESHOLD_SIZE;
	sc->user_data = NULL;
}

static void _init_syntax(oscheme *sc)
{
	pcell x;

	mk_syntax(sc, OP_LAMBDA, _T("lambda"));
	mk_syntax(sc, OP_QUOTE, _T("quote"));
	mk_syntax(sc, OP_QUOTE_VEC0, _T("quote-vector"));
	mk_syntax(sc, OP_DEF0, _T("define"));
	mk_syntax(sc, OP_IF0, _T("if"));
	mk_syntax(sc, OP_BEGIN, _T("begin"));
	mk_syntax(sc, OP_SET0, _T("set!"));
	mk_syntax(sc, OP_LET0, _T("let"));
	mk_syntax(sc, OP_LET_OPTIONAL0, _T("let-optionals"));
	mk_syntax(sc, OP_LET0AST, _T("let*"));
	mk_syntax(sc, OP_LET0REC, _T("letrec"));
	mk_syntax(sc, OP_COND0, _T("cond"));
	mk_syntax(sc, OP_AND0, _T("and"));
	mk_syntax(sc, OP_OR0, _T("or"));

	mk_syntax(sc, OP_0MACRO, _T("define-macro"));

	mk_syntax(sc, OP_QQUOTE0, _T("quasiquote"));
	mk_syntax(sc, OP_QQUOTE_VEC0, _T("_quasiquote-vec"));

	mk_syntax(sc, OP_USE, _T("use"));

	/* init else */
	x = mk_symbol(sc, _T("else"));
	push_env(sc, x, sc->T);
}

static void _init_procs(oscheme* sc)
{
	int		i;
	for(i = 0; i < sizeof(dispatch_table)/sizeof(dispatch_table[0]); i++) {
		if(dispatch_table[i].proc_name != NULL) {
			mk_proc(sc, dispatch_table[i].op, dispatch_table[i].proc_name);
		}
	}
}

void _init_port(oscheme *sc)
{
	sc->infp = mk_port(sc, stdin, PORT_INPORT, _T("(stdin)"),
		INPORT_READ_SIZE + FILL_INPORT_THR);
	sc->outfp = mk_port(sc, stdout, PORT_OUTPORT, _T("(stdout)"), 0);
	sc->errfp = mk_port(sc, stderr, PORT_OUTPORT, _T("(stderr)"), 0);
	sc->iofp = mk_string_output_port(sc, STR_BUF_SIZE);
}

struct _st_slotdef {
	TCHAR	*name;
	slot_get_func   getter;
	slot_set_func   setter;
};

int sys_num_setter(oscheme *sc, pcell o, pcell v, int idx)
{
	if(!isnumber(v)) {
		_tcscpy(sc->err_buf, _T("exact integer required, but got"));
		return -1;
	}
	object_vslots(o)[idx] = v;
	return idx;
}

struct _st_slotdef sys_tm_slots[] = {
	{_T("sec"), NULL, sys_num_setter},
	{_T("min"), NULL, sys_num_setter},
	{_T("hour"), NULL, sys_num_setter},
	{_T("mday"), NULL, sys_num_setter},
	{_T("mon"), NULL, sys_num_setter},
	{_T("year"), NULL, sys_num_setter},
	{_T("wday"), NULL, sys_num_setter},
	{_T("yday"), NULL, sys_num_setter},
	{_T("isdst"), NULL, sys_num_setter},
	{NULL, NULL, NULL},
};

struct _st_slotdef sys_stat_slots[] = {
	{_T("size"), NULL, sys_num_setter},
	{_T("atime"), NULL, sys_num_setter},
	{_T("mtime"), NULL, sys_num_setter},
	{_T("ctime"), NULL, sys_num_setter},
	{NULL, NULL, NULL},
};

struct _st_classdef {
	TCHAR	*name;
	TCHAR	*supers;
	struct _st_slotdef *slots;
} internal_class_def[] = {
	{ _T("<sys-tm>"), _T("'()"), sys_tm_slots },
	{ _T("<sys-stat>"), _T("'()"), sys_stat_slots },
};

void _init_internal_class(oscheme *sc)
{
	int		i;
	int		cnt = sizeof(internal_class_def)/sizeof(internal_class_def[0]);

	oscheme_set_dont_gc_flg(sc);

	for(i = 0; i < cnt; i++) {
		struct _st_classdef *c = &(internal_class_def[i]);
		struct _st_slotdef *s = c->slots;
		int		i, slot_cnt;
		pcell	supers;
		pcell	slots = sc->NIL;
		pcell	name;
		pcell	k;
		slot_accesser	*accessers;

		name = mk_symbol(sc, c->name);
		supers = _eval_str(sc, c->supers);

		for(slot_cnt = 0; s[slot_cnt].name != NULL; slot_cnt++) {
			slots = cons(sc, mk_symbol(sc, s[slot_cnt].name), slots);
		}
		slots = reverse_in_place(sc, sc->NIL, slots);

		accessers = (slot_accesser *)oscm_malloc(sc, 
			sizeof(slot_accesser) * slot_cnt);
		for(i = 0; i < slot_cnt; i++) {
			accessers[i].getter = s[i].getter;
			accessers[i].setter = s[i].setter;
		}

		k = mk_class(sc, c->name, supers, slots, accessers);
		push_env(sc, name, k);
	}
	oscheme_clear_dont_gc_flg(sc);
}

/* ========== external function ========== */
void clear_oscheme(oscheme *sc)
{
	int		i;
	pcell	p, last_p;

	if(sc == NULL) return;

	set_cur_seg(sc, 0);
	p = sc->next_cell;
	last_p = sc->next_cell_last;

	for(;;) {
		if(p == last_p) {
			if(sc->cur_cell_seg == sc->last_cell_seg) break;
			set_cur_seg(sc, sc->cur_cell_seg + 1);
			p = sc->next_cell;
			last_p = sc->next_cell_last;
		}
		if(need_finalize(p)) do_finalize(sc, p);
		p++;
	}

	oscm_free(sc, sc->dump_stack);

	for(i = 0; i <= sc->last_cell_seg; i++) free(sc->cell_seg[i]);

#ifdef USE_ALLOC_SET
	if(sc->aset != NULL) alloc_set_delete(sc->aset);
#endif
	free(sc);
}

oscheme *init_oscheme()
{
	pcell	h;

	oscheme *sc = (oscheme *)malloc(sizeof(oscheme));
	if(sc == NULL) return NULL;

#ifdef _UNICODE
	setlocale(LC_ALL, "Japanese");
#endif

	_init_global_vars(sc);

#ifdef USE_SETJMP
	if(setjmp(sc->jmp_env) != 0) {
		clear_oscheme(sc);
		return NULL;
	}
#endif

#ifdef USE_ALLOC_SET
    sc->aset = alloc_set_create(1024 * 32, 1024 * 32);
    if(sc->aset == NULL) FatalError(sc, _T("no memory"));
#endif

	alloc_cellseg(sc, CELL_SEGSIZE);
	set_cur_seg(sc, 0);

	/* init global_env */
	h = mk_hash(sc, 1024, _T("ptr-eq?"));
	sc->global_env = cons(sc, h, sc->NIL);
	sc->envir = sc->global_env;
	sc->oblist = mk_hash(sc, 1024, _T("oblist"));

	_init_port(sc);
	_init_syntax(sc);
	_init_procs(sc);

	/* initialization of global pointers to special symbols */
	sc->LAMBDA = mk_symbol(sc, _T("lambda"));
	sc->QUOTE = mk_symbol(sc, _T("quote"));
	sc->QUOTE_VEC = mk_symbol(sc, _T("quote-vector"));
	sc->QQUOTE = mk_symbol(sc, _T("quasiquote"));
	sc->UNQUOTE = mk_symbol(sc, _T("unquote"));
	sc->UNQUOTESP = mk_symbol(sc, _T("unquote-splicing"));
	sc->QQUOTE_VEC = mk_symbol(sc, _T("_quasiquote-vec"));

	_init_internal_class(sc);

	return sc;
}

#define BLANK_CODE	0
#ifdef USE_SETJMP
#define _SET_JMP_BUF_(_sc, _err_value, _err_code)				\
jmp_buf	_back_jmp_env;											\
do {															\
	memcpy(_back_jmp_env, (_sc)->jmp_env, sizeof(jmp_buf));		\
	if(setjmp((_sc)->jmp_env) != 0) {							\
		_err_code;												\
		return _err_value;										\
	}															\
} while(0)
#define _RESTORE_JMP_BUF_(_sc) do {								\
	memcpy((_sc)->jmp_env, _back_jmp_env, sizeof(jmp_buf));		\
} while(0)
#else
#define _SET_JMP_BUF_(_sc)
#define _RESTORE_JMP_BUF_(_sc)
#endif

int oscheme_regist_callback_func(oscheme *sc, TCHAR *name,
	oscheme_callback_func func, int min_argc, int max_argc, TCHAR *type_chk_str)
{
	pcell r;
	_SET_JMP_BUF_(sc, 1, BLANK_CODE);
	r = mk_callback_func(sc, name, func, min_argc, max_argc, type_chk_str);
	_RESTORE_JMP_BUF_(sc);

	// FIXME: error check
	return 0;
}

pcell oscheme_eval_str(oscheme *sc, const TCHAR *str)
{
	pcell r;
	_SET_JMP_BUF_(sc, sc->F, BLANK_CODE);
	r = _eval_str(sc, str);
	_RESTORE_JMP_BUF_(sc);

	return r;
}

pcell oscheme_apply(oscheme *sc, pcell x)
{
	pcell r;
	_SET_JMP_BUF_(sc, sc->F, BLANK_CODE);

	s_save(sc, OP_EXIT, sc->NIL, sc->NIL);
	sc->args = sc->NIL;
	sc->code = x;
	r = eval_cycle(sc, OP_APPLY, 0);
	
	_RESTORE_JMP_BUF_(sc);

	return r;
}

TCHAR *oscheme_tostring(oscheme *sc, pcell lst)
{
	pcell string_port, r;
	pcell back_fp = sc->outfp;
	TCHAR *str, *str_end;
	_SET_JMP_BUF_(sc, _T(""), sc->outfp = back_fp);

	string_port = mk_string_output_port(sc, 64);
	sc->print_flag = 0;
	sc->outfp = string_port;

	s_save(sc, OP_EXIT, sc->NIL, back_fp);
	sc->args = lst;
	r = eval_cycle(sc, OP_P0LIST, 0);

	sc->outfp = back_fp;
	str = port_buf(string_port);
	str_end = port_cur_buf(string_port);

	_RESTORE_JMP_BUF_(sc);

	return str;
}

void oscheme_error(oscheme *sc, const TCHAR *msg)
{
	sc->args = cons(sc, mk_string(sc, msg), sc->NIL);
	eval_cycle(sc, OP_ERR0, 0);
}

int oscheme_bound_global(oscheme *sc, pcell x)
{
	_SET_JMP_BUF_(sc, 1, BLANK_CODE);
	sc->global_var = cons(sc, x, sc->global_var);
	_RESTORE_JMP_BUF_(sc);
	return 0;
}

int oscheme_unbound_global(oscheme *sc, pcell x)
{
	pcell y = sc->global_var;
	pcell *prev = &(sc->global_var);

	for(y = sc->global_var; y != sc->NIL; y = cdr(y)) {
		if(car(y) == x) {
			*prev = cdr(y);
			break;
		}
		prev = &(cdr(y));
	}
	return 0;
}

int oscheme_load_file(oscheme *sc, const TCHAR *file_name)
{
	pcell r;

	_SET_JMP_BUF_(sc, 1, BLANK_CODE);

	r =	_load_file(sc, file_name);

	_RESTORE_JMP_BUF_(sc);
	if(r != sc->T) {
		_stprintf(sc->err_buf, _T("fatal error: file cannot open %s"), file_name);
		return 1;
	}
	return 0;
}

pcell oscheme_true(oscheme *sc)
{
	return sc->T;
}

pcell oscheme_false(oscheme *sc)
{
	return sc->F;
}

pcell oscheme_nil(oscheme *sc)
{
	return sc->NIL;
}

pcell oscheme_cons_cell(oscheme *sc, pcell a, pcell b)
{
	return cons(sc, a, b);
}

int oscheme_is_false(oscheme *sc, pcell p)
{
	return isfalse(sc, p);
}

pcell oscheme_mk_string(oscheme *sc, const TCHAR *str)
{
	pcell	x;
	_SET_JMP_BUF_(sc, NULL, BLANK_CODE);
	x = mk_string(sc, str);
	_RESTORE_JMP_BUF_(sc);
	return x;
}

pcell oscheme_mk_number(oscheme *sc, int n)
{
	pcell	x;
	_SET_JMP_BUF_(sc, NULL, BLANK_CODE);
	x = mk_number(sc, n);
	_RESTORE_JMP_BUF_(sc);
	return x;
}

pcell oscheme_mk_char(oscheme *sc, unsigned int ch)
{
    pcell   x;
    _SET_JMP_BUF_(sc, NULL, BLANK_CODE);
    x = mk_char(sc, ch);
    _RESTORE_JMP_BUF_(sc);
    return x;
}

pcell oscheme_mk_eof(oscheme *sc)
{
    pcell   x;
    _SET_JMP_BUF_(sc, NULL, BLANK_CODE);
    x = mk_eof(sc);
    _RESTORE_JMP_BUF_(sc);
    return x;
}

void oscheme_set_dont_gc_flg(oscheme *sc)
{
	sc->dont_gc_flg = 1;
}

void oscheme_clear_dont_gc_flg(oscheme *sc)
{
	sc->dont_gc_flg = 0;
}

TCHAR *oscheme_get_strvalue(oscheme *sc, pcell x)
{
	if(!isstring(x)) return _T("");
	return strvalue(x);
}

INT_PTR oscheme_get_ivalue(oscheme *sc, pcell x)
{
	if(!isnumber(x)) return 0;
	return ivalue(x);
}

int oscheme_get_int_value(oscheme *sc, pcell x)
{
	if (!isnumber(x)) return 0;
	return (int)ivalue(x);
}

long oscheme_get_long_value(oscheme *sc, pcell x)
{
	if (!isnumber(x)) return 0;
	return (long)ivalue(x);
}

TCHAR *oscheme_get_err_msg(oscheme *sc)
{
	return sc->err_buf;
}

void oscheme_set_out_func(oscheme *sc, oscheme_portout_func func)
{
	port_out_func(sc->outfp) = func;
}

void oscheme_set_err_func(oscheme *sc, oscheme_portout_func func)
{
	port_out_func(sc->errfp) = func;
}

void _debug_cell_print(oscheme *sc, pcell x)
{
	_tprintf(_T("%s\n"), oscheme_tostring(sc, x));
}

void oscheme_set_user_data(oscheme *sc, void *ptr)
{
	sc->user_data = ptr;
}

void *oscheme_get_user_data(oscheme *sc)
{
	return sc->user_data;
}

#ifdef ENABLE_CANCEL
void oscheme_set_cancel_key(oscheme *sc, int fvirt, int keycode)
{
	sc->cancel_fvirt = fvirt;
	sc->cancel_keycode = keycode;
}

int oscheme_canceled(oscheme *sc)
{
	return sc->cancel_flg;
}
#endif

#ifdef OSCHEME_MAIN
pcell c_func_test(oscheme *sc, pcell *argv, int argc)
{
	int		i;

	printf("c_func_test(%d):", argc);
	for(i = 0; i < argc; i++) {
		printf("%s,", strvalue(argv[i]));
	}
	printf("\n");
	return sc->T;
}

int main(int argc, char **argv)
{
	oscheme *sc = NULL;
	int		ret_v = 0;

	sc = init_oscheme();
	if(sc == NULL) {
		fprintf(stderr, "no memory\n");
		return 1;
	}
#ifdef USE_SETJMP
	if(setjmp(sc->jmp_env) != 0) {
		fprintf(stderr, "%s\n", sc->err_buf);
		clear_oscheme(sc);
		return 1;
	}
#endif

	_eval_str(sc, _T("(define *load-path* '(\"./lib\"))"));
	_load_file(sc, InitFile);

	mk_callback_func(sc, "c-test", c_func_test, 1, 3,
		TC_STRING TC_STRING TC_STRING);

	if(argc == 2) {
		if(_load_file(sc, argv[1]) == sc->F) {
			fprintf(stderr, "Error: can not find file \"%s\"\n", argv[1]);
			ret_v = 1;
		}
	} else if(argc == 3) {
		if(_tcscmp(argv[1], _T("-e")) == 0) {
			_eval_str(sc, argv[2]);
		} else if(_tcscmp(argv[1], _T("-E")) == 0) {
			if(_tcslen(argv[2]) > STR_BUF_SIZE - 100) {
				fprintf(stderr, "Error: code too large\n");
			} else {
				TCHAR code_buf[STR_BUF_SIZE];
				_stprintf(code_buf, "(display %s) (newline)", argv[2]);
				_eval_str(sc, code_buf);
			}
		} else {
			fprintf(stderr, "Error: bad option\n");
			ret_v = 1;
		}
	} else {
		eval_cycle(sc, OP_T0LVL, 1);
	}

	clear_oscheme(sc);

	return ret_v;
}
#endif

