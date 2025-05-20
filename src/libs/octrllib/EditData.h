/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef _EDIT_DATA_H_INCLUDE
#define _EDIT_DATA_H_INCLUDE

#define OPE_INPUT			1
#define OPE_BACKSPACE		2
#define OPE_DELETE			3
#define OPE_SAVE_CURSOR_POS	4
#define OPE_SWAP_ROW		5

#define CHAR_TYPE_NORMAL		1
#define CHAR_TYPE_IN_COMMENT	2
#define CHAR_TYPE_IN_ROWCOMMENT	3
#define CHAR_TYPE_IN_QUOTE		4

enum INDENT_MODE {
	INDENT_MODE_NONE,
	INDENT_MODE_AUTO,
	INDENT_MODE_SMART,
};

#define MODE_UNDO				1
#define MODE_REDO				2

#include "Undo.h"
#include "EditRowData.h"
#include "EditRowBuffer.h"
#include "strtoken.h"
#include "UnicodeArchive.h"

#include "EditDispData.h"

#include "oregexp.h"

enum GET_WORD_OPT {
	GET_WORD_NORMAL,
	GET_WORD_CURRENT_PT,
};
enum GET_WORD_MOVE_METHOD_OPT {
	GET_WORD_MOVE_METHOD_NORMAL,
	GET_WORD_MOVE_METHOD_BREAK_CHAR,
};

#define GET_WORD_EX_OPT_MOVE_CURSOR				(0x0001 << 0)
#define GET_WORD_EX_OPT_CONSIDER_QUOTE_WORD		(0x0001 << 1)


// FIXME classにして、メモリの管理をクラス無いで行う (ini_bufの管理、デストラクタで解放など)
#define CHECK_CHAR_TYPE_CACHE_BUF_SIZE	256
struct check_char_type_cache {
	char			*char_type_arr;
	char			ini_buf[CHECK_CHAR_TYPE_CACHE_BUF_SIZE];
	int				char_type_arr_cnt;
	TCHAR			*p;
	int				row;
	int				cur_x;
	char			cur_type;
};

#define ECS_SHOW_LINE_FEED		((QWORD)1 << 0)
#define ECS_SHOW_TAB			((QWORD)1 << 1)
#define ECS_INVERT_BRACKETS		((QWORD)1 << 2)
#define ECS_GRID_EDITOR			((QWORD)1 << 3)
#define ECS_ON_DIALOG			((QWORD)1 << 4)
#define ECS_SHOW_ROW_NUM		((QWORD)1 << 5)
#define ECS_SHOW_COL_NUM		((QWORD)1 << 6)
#define ECS_NO_COMMENT_CHECK	((QWORD)1 << 7)
#define ECS_HTML_MODE			((QWORD)1 << 8)
#define ECS_SHOW_2BYTE_SPACE	((QWORD)1 << 9)
#define ECS_CLICKABLE_URL		((QWORD)1 << 10)
#define ECS_SEARCH_LOOP_MSG		((QWORD)1 << 11)
#define ECS_DISABLE_KEY_DOWN	((QWORD)1 << 12)	// アクセラレータキーをカスタマイズするときに設定する
#define ECS_DRAG_DROP_EDIT		((QWORD)1 << 13)
#define ECS_SHOW_ROW_LINE		((QWORD)1 << 14)
#define ECS_SHOW_EDIT_ROW		((QWORD)1 << 15)
#define ECS_DRAG_SELECT_COPY	((QWORD)1 << 16)
#define ECS_V_SCROLL_EX			((QWORD)1 << 17)	// 編集行が画面の下端にくっつかないようにする
#define ECS_NO_BLINK_CARET		((QWORD)1 << 18)
#define ECS_ROW_COPY_AT_NO_SEL	((QWORD)1 << 19)	// 選択範囲がないときは，行コピーする
#define ECS_SHOW_SPACE			((QWORD)1 << 20)
#define ECS_SHOW_LINE_END		((QWORD)1 << 21)
#define ECS_NO_INVERT_SELECT_TEXT	((QWORD)1 << 22)
#define ECS_IME_CARET_COLOR		((QWORD)1 << 23)
#define ECS_TEXT_DROP_EDIT		((QWORD)1 << 24)
#define ECS_SHOW_BRACKETS_BOLD	((QWORD)1 << 25)	// 現在のカーソル位置に対応する括弧を強調表示
#define ECS_WORD_SELECT_MODE	((QWORD)1 << 26)	// ダブルクリックで選択開始したとき、単語単位で選択する
#define ECS_DRAW_ANTIALIAS		((QWORD)1 << 27)	// TrueTypeFontのとき、文字をアンチエイリアスで描画する
#define ECS_FULL_SCREEN_MODE	((QWORD)1 << 28)	// フルスクリーンモード
#define ECS_SHOW_RULED_LINE		((QWORD)1 << 29)	// 罫線表示
#define ECS_CLEAR_AFTER_COPY	((QWORD)1 << 30)	// コピーしたら選択範囲をクリアする
#define ECS_MODIFIED_BK_COLOR	((QWORD)1 << 31)	// 編集されたとき背景色を変更する

#define ECS_BRACKET_MULTI_COLOR_ENABLE	((QWORD)1 << 32)	// 入れ子になったカッコを色分けして表示を有効にするか
#define ECS_BRACKET_MULTI_COLOR	((QWORD)1 << 33)	// 入れ子になったカッコを色分けして表示

#define ECS_TEXT_ALIGN_RIGHT	((QWORD)1 << 34)	// 右寄せ表示


// ((QWORD)1 << 63)まで利用可能

class CEditDataListener
{
public:
	virtual void NoticeUpdate() = 0;
};

class CEditData
{
private:
	CEditRowBuffer	m_row_buffer;
	int		m_char_cnt;
	int		m_limit_char_cnt;

	CUndo	*m_undo;

	POINT	m_cur_pt;

	BOOL	m_tab_as_space;
	INDENT_MODE		m_indent_mode;

	int		m_max_disp_width;
	CEditRowData *m_max_disp_width_row;

	int		m_disp_col;

	static CStrToken	m_def_str_token;
	CStrToken	*m_str_token;

	int		m_saved_undo_sequence;	// データを保存したときのundo_seq
	int		m_undo_redo_mode;

	UINT	m_edit_seq;				// データが編集されるたびに増える

	unsigned int	m_prev_input_char;

	CEditDispData	m_disp_data;

	TCHAR	*m_last_confinement;
	TCHAR	*m_first_confinement;
	BOOL			m_word_wrap;
	BOOL			m_confinement_flg;

	OREG_DATASRC	m_data_src;

	int				m_copy_line_type;
	TCHAR			m_copy_line_type_str[3];
	int				m_copy_line_type_len;

public:
	CEditData();
	virtual ~CEditData();

	int load_file(const TCHAR *file_name, TCHAR *msg_buf);
	int save_file(const TCHAR *file_name, int kanji_code, int line_type);

	int load_file(CArchive &ar, int &kanji_code, int &line_type);
	int save_file(CArchive &ar, int kanji_code, int line_type);

	int save_file_main(CUnicodeArchive &ar);

	int append_file(CArchive &ar, int &kanji_code, int &line_type);

	void set_str_token(CStrToken *str_token) { m_str_token = str_token; }

	int input_tab(CFontWidthHandler *dchandler);
	int input_char(unsigned int data);
	int back_space();
	int delete_key();

	void calc_del_rows_pos(int row1, int row2, POINT *pt1, POINT *pt2);
	int del_row(int row) { return del_rows(row, row); }
	int del_rows(int row1, int row2);

	CString get_all_text();
	CString get_point_text(POINT pt1, POINT pt2);
	CString get_selected_text();
	CString get_selected_text_box();

	CString get_word(GET_WORD_OPT opt, GET_WORD_MOVE_METHOD_OPT move_method, UINT ex_opt = 0);
	CString get_prev_word();
	
	int copy_text_data(POINT *pt1, POINT *pt2, TCHAR *buf, int bufsize);
	int copy_text_data_box(TCHAR *buf, int bufsize);

	int calc_buf_size(POINT *pt1, POINT *pt2);
	int calc_buf_size_box();

	int calc_char_cnt(POINT *pt1, POINT *pt2);
	int calc_char_cnt_box();

	int del_all();
	int del(POINT *pt1, POINT *pt2, BOOL recalc);
	int del_box(BOOL recalc);

	int search_text_regexp(POINT *pt, int dir, BOOL loop, POINT *start_pt, 
		HWND loop_msg_wnd, HREG_DATA reg_data, BOOL *b_looped);
	int search_text_regexp(const TCHAR *reg_str, POINT *pt,
		BOOL b_distinct_lwr_upr, POINT *start_pt);
	// end_ptも指定できるようにしたバージョン
	int search_text_regexp_end_pt(POINT *pt, int dir, 
		BOOL loop, POINT *start_pt, POINT *end_pt, 
		HWND loop_msg_wnd, HREG_DATA reg_data, BOOL *b_looped);

	int get_search_text_regexp(POINT *pt, int dir, HREG_DATA reg_data, 
		const TCHAR *replace_str, CString *str);

	int replace_str(POINT *pt1, POINT *pt2, const TCHAR *str);
	int replace_str_box(const TCHAR *str);

	int replace_str_all_regexp(const TCHAR *replace_str,
		BOOL b_regexp, POINT *start_pt, POINT *end_pt, 
		int *preplace_cnt, HREG_DATA reg_data);
	int replace_str_all_regexp(const TCHAR *reg_str, const TCHAR *replace_str,
		BOOL b_distinct_lwr_upr, int *preplace_cnt);
	int replace_str_all_regexp_box(CFontWidthHandler *dchandler,
		const TCHAR *replace_str,
		BOOL b_regexp, int *preplace_cnt, HREG_DATA reg_data);

	CString make_back_ref_str(POINT *pt1, int len, const TCHAR *replace_str,
		HREG_DATA reg_data);
	void calc_find_last_pos(POINT *pt, int len);

	int filter_str(int start_row, int end_row, int *pdel_row_cnt, 
		HREG_DATA reg_data, BOOL b_del_flg);

	int replace_all(const TCHAR *pstr);
	virtual int paste(const TCHAR *pstr, BOOL recalc = TRUE);
	int paste_no_undo(const TCHAR *pstr);
	int put_char(TCHAR ch) { return add_char(ch); }

	TCHAR *get_row_buf(int row);
	const TCHAR *get_cur_buf();
	int get_row_len(int row);
	int get_row_cnt() { return m_row_buffer.get_row_cnt(); }
	int get_max_disp_width() { return m_max_disp_width; }
	BOOL is_max_disp_width_row(int row) { return is_max_disp_width_row(get_edit_row_data(row)); }

	void recalc_disp_size() { 
		TRACE(_T("recalc_disp_size called\n"));
	}

	unsigned int get_char(int row, int col);
	unsigned int get_cur_char() { return get_pt_char(&m_cur_pt); }
	unsigned int get_prev_char();
	const TCHAR *get_disp_row_text(int row);
	
	int get_cur_col() { return m_cur_pt.x; }
	int get_cur_row() { return m_cur_pt.y; }
	int get_cur_split();
	POINT get_cur_pt() { return m_cur_pt; }

	POINT get_end_of_text_pt();

	int get_tabstop() { return m_disp_data.GetFontData()->GetTabStop(); }
	void set_tabstop(int tabstop);

	BOOL get_tab_as_space() { return m_tab_as_space; }
	void set_tab_as_space(BOOL b_tab_as_space) { m_tab_as_space = b_tab_as_space; }

	void set_indent_mode(INDENT_MODE mode) { m_indent_mode = mode; }
	INDENT_MODE get_indent_mode() { return m_indent_mode; }

	void move_cur_col(int cnt);
	void move_cur_row(int cnt);
	void set_cur(int row, int col);

	void move_line_end();
	void move_document_end();

	void get_point(int disp_row, int disp_col, POINT *pt);
	void set_valid_point(POINT *pt);

	void reset_undo();
	int undo();
	int redo();
	BOOL can_undo();
	BOOL can_redo();
	int get_undo_sequence() { return m_undo->get_undo_sequence(); }
	void clear_cur_operation() { m_undo->clear_cur_operation(); }

	UINT get_edit_sequence() { return m_edit_seq; }

	void set_row_data_flg(int row, int flg) { get_edit_row_data(row)->set_data_flg(flg); }
	void unset_row_data_flg(int row, int flg) { get_edit_row_data(row)->unset_data_flg(flg); }
	void toggle_row_data_flg(int row, int flg) { get_edit_row_data(row)->toggle_data_flg(flg); }
	int get_row_data_flg(int row, int flg) { return get_edit_row_data(row)->get_data_flg(flg); }

	int get_row_bracket_cnt(int row) { return get_edit_row_data(row)->get_bracket_cnt(); }
	void set_row_bracket_cnt(int row, int cnt) { get_edit_row_data(row)->set_bracket_cnt(cnt); }

	BOOL is_edit_data() { return (m_undo->get_undo_sequence() != m_saved_undo_sequence); }
	BOOL is_edit_row(int row) { return (get_edit_row_data(row)->get_edit_undo_sequence() != -1); }

	CStrToken *get_str_token() { return m_str_token; }

	void set_undo_cnt(int cnt);

	void recalc_max_disp_width();

	void set_limit_text(int limit);
	int get_limit_text() { return m_limit_char_cnt; }
	int get_file_size(int line_size) { return m_char_cnt + (get_row_cnt() - 1) * line_size; }
	int get_data_size() { return get_file_size(m_copy_line_type_len); }

	int reverse_rows(int start_row, int end_row);
	int sort_rows(int start_row, int end_row, BOOL desc);

	void skip_space(int dir);
	void move_word(int dir);
	void move_break_char(int dir);
	void move_chars(int dir, unsigned int *ch_arr, int ch_cnt);

	int get_row_col() { return m_disp_col; }
	void set_row_col(int col) { m_disp_col = col; }

	int strstr_dir_regexp(int row, int col, int dir, int *hit_len, HREG_DATA reg_data);

	char check_char_type() { return check_char_type(m_cur_pt.y, m_cur_pt.x); }
	char check_char_type(int row, int col, struct check_char_type_cache *cache = NULL);

	BOOL search_brackets(unsigned int open_char, unsigned int close_char, 
		int start_cnt, POINT *pt);
	BOOL search_brackets_ex(unsigned int open_char, unsigned int close_char, 
		int start_cnt, POINT *p_end_pt, unsigned int end_char, int arrow, POINT *pt);

	POINT move_pt(POINT pt, int cnt);
	POINT skip_comment_and_space(POINT pt, BOOL b_skip_2byte_space, BOOL b_comment_only);
	void skip_comment_and_space();
	void skip_comment_and_space(BOOL b_skip_2byte_space);
	void skip_comment();

	int space_to_tab(const POINT *start_pt, const POINT *end_pt);
	int tab_to_space(const POINT *start_pt, const POINT *end_pt);
	int tab_to_space_all();

	void get_box_x2(int row, int *start_x, int *end_x);

	void save_cursor_pos();
	void move_last_edit_pos();

	CEditRowData *get_edit_row_data(int row) { return m_row_buffer.get_edit_row_data(row); }

	int get_scroll_row(int row) { return get_edit_row_data(row)->get_scroll_row(); }
	int get_split_cnt(int row) { return get_edit_row_data(row)->get_split_cnt(); }

	CUndo *get_undo() { return m_undo; }

	CEditDispData *get_disp_data() { return &m_disp_data; }

	void calc_disp_width(int row, CFontWidthHandler *dchandler);
	int get_disp_width_pt(POINT pt, CFontWidthHandler *dchandler);
	int get_x_from_width_pt(int row, int width, CFontWidthHandler *dchandler);
	int calc_disp_row_split_cnt(int row, CFontWidthHandler *dchandler);

	int split_rows(POINT *start_pt, POINT *end_pt);

	int hankaku_to_zenkaku(POINT *start_pt, POINT *end_pt, BOOL b_alpha, BOOL b_kata);
	int zenkaku_to_hankaku(POINT *start_pt, POINT *end_pt, BOOL b_alpha, BOOL b_kata);
	int to_lower_without_comment_and_literal(POINT* start_pt, POINT* end_pt);
	int to_upper_without_comment_and_literal(POINT* start_pt, POINT* end_pt);
	
	void set_word_wrap(BOOL b_wrap);
	void set_last_confinement(const TCHAR *str);
	void set_first_confinement(const TCHAR *str);
	BOOL is_confinement_mode() { return m_confinement_flg; }

	int get_disp_offset(int idx, int *psplit_idx = NULL);
	int get_disp_row(int idx) { return get_disp_data()->GetDispRowArr()->GetData(idx); }

	BOOL is_blank_row(int row);
	BOOL is_empty();

	BOOL is_first_of_text() { return (get_cur_row() == 0 && get_cur_col() == 0); }
	BOOL is_first_of_line() { return (get_cur_col() == 0); }
	BOOL is_end_of_line() { return (get_cur_col() == get_row_len(get_cur_row())); }
	BOOL is_end_line() { return (get_cur_row() == get_row_cnt() - 1); }
	BOOL is_end_of_text() { return (is_end_line() && is_end_of_line()); }

	void set_copy_line_type(int line_type);
	int get_copy_line_type() { return m_copy_line_type; }

	OREG_DATASRC *get_oreg_data_src() { return &m_data_src; }

	BOOL is_valid_point(const POINT pt);

	int get_next_line_len(const TCHAR *pstr, int width, 
		CDC *pdc, CFontWidthData *font_width_data);

	int get_caret_col(int row, int col, CFontWidthHandler *dchandler);
	int get_cur_caret_col(CFontWidthHandler *dchandler) {
		return get_caret_col(get_cur_row(), get_cur_col(), dchandler);
	}
	int get_col_from_caret_col(int row, int disp_col, CFontWidthHandler *dchandler);

	void check_comment_row(int start_row, int end_row, QWORD ex_style, BOOL *p_invalidate);

private:
	int fast_new_line(CEditRowData **&new_rows, int &new_rows_alloc_cnt,
		int &new_row_cnt);
	int fast_paste_main(const TCHAR *pstr);
	int paste_main(const TCHAR *pstr);

	int init_editdata();
	void free_editdata();

	int init_undo();
	void free_undo();

	int add_char(TCHAR data);
	int back();
	int delete_char();
	int new_line();

	unsigned int get_pt_char(const POINT *pt);

	int del_main(POINT *pt1, POINT *pt2, BOOL recalc, BOOL make_undo = TRUE);
	int copy_text_data_main(POINT *pt1, POINT *pt2, TCHAR *buf, int bufsize);
	int copy_text_data_box(POINT *pt1, POINT *pt2, TCHAR *buf, int bufsize);
	int replace_str_main(POINT *pt1, POINT *pt2, const TCHAR *str);

	void fill_space(int w, CFontWidthHandler *dchandler);

	int replace_str_all_regexp_main(const TCHAR *replace_str,
		BOOL b_regexp, POINT *start_pt, POINT *end_pt, 
		int *preplace_cnt, HREG_DATA reg_data,
		BOOL box, CFontWidthHandler *dchandler);

	void move_row(int cnt) { move_row(&m_cur_pt, cnt); }
	void move_row(POINT *pt, int cnt);
	void move_col(int cnt) { move_col(&m_cur_pt, cnt); }
	void move_col(POINT *pt, int cnt);
	void move_left() { move_left(&m_cur_pt); }
	void move_left(POINT *pt);
	void move_right() { move_right(&m_cur_pt); }
	void move_right(POINT *pt);

	void set_cur_row(int row);
	void set_cur_col(int col);

	void set_max_disp_width_row(int row);
	BOOL is_max_disp_width_row(CEditRowData *row) { return (row == m_max_disp_width_row); }
	void clear_disp_col() { m_disp_col = -1; }

	int search_text_loop_regexp(POINT *pt, int dir, int row, int loop_cnt, HREG_DATA reg_data);

	int swap_row(int row1, int row2, BOOL b_make_undo = TRUE);
	int sort_rows_asc(int left, int right);
	int sort_rows_desc(int left, int right);

	BOOL same_char_type(unsigned int ch1, unsigned int ch2);
	int get_char_type(unsigned int ch);

	void set_edit_undo_sequence(int row);
	void set_all_edit_undo_sequence();

	BOOL is_last_confinement_char(unsigned int ch);
	BOOL is_first_confinement_char(unsigned int ch);
	int check_confinement(const TCHAR *p_start, int offset);
	int check_word_wrap(const TCHAR *p_start, int offset, int prev_offset);
	void check_confinement_flg();

	int hankaku_zenkaku_convert(POINT *start_pt, POINT *end_pt, BOOL b_alpha, BOOL b_kata,
		BOOL b_hankaku_to_zenkaku);

	int without_comment_and_literal_convert(POINT* start_pt, POINT* end_pt, BOOL to_lower, BOOL to_upper);

	void notice_update();
};

__inline TCHAR *CEditData::get_row_buf(int row)
{
	ASSERT(row >= 0 && row < get_row_cnt());
	if(row < 0 || row >= get_row_cnt()) return _T("");

	return get_edit_row_data(row)->get_buffer();
}

__inline const TCHAR *CEditData::get_cur_buf()
{
	return get_edit_row_data(m_cur_pt.y)->get_buffer() + m_cur_pt.x;
}

__inline int CEditData::get_row_len(int row)
{
	ASSERT(row >= 0 && row < get_row_cnt());
	if(row < 0 || row >= get_row_cnt()) return 0;

	return get_edit_row_data(row)->get_char_cnt() - 1;
}

class CEditDataSaveCurPos
{
public:
	CEditDataSaveCurPos(CEditData *e) {
		m_edit_data = e;
		m_cur_pt.y = m_edit_data->get_cur_row();
		m_cur_pt.x = m_edit_data->get_cur_col();
	}
	~CEditDataSaveCurPos() {
		m_edit_data->set_cur(m_cur_pt.y, m_cur_pt.x);
	}

private:
	CEditData *m_edit_data;
	POINT	m_cur_pt;
};

#endif _EDIT_DATA_H_INCLUDE
