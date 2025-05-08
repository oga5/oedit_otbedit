/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef OEDIT_GLOBAL_H_INCLUDED
#define OEDIT_GLOBAL_H_INCLUDED

#ifdef MAIN_PROG
#define	EXTERN
#else
#define EXTERN extern
#endif

#include "EditCtrl.h"
#include "sharedata.h"
#include "SearchDlgData.h"
#include "LanguageSetting.h"
#include "oscheme.h"

#define GLOBAL_H_OEDIT

EXTERN oscheme		*g_sc;

EXTERN CSearchDlgData	g_search_data;

EXTERN CShareData	g_share_data;		// over process shared memory

EXTERN CFont		g_font;				// global font
EXTERN POINT		g_cur_pos;			// cursor position of current textctrl
EXTERN CStrToken	g_str_token;
EXTERN CString		g_file_type_name;
EXTERN int			g_open_kanji_code;
EXTERN BOOL			g_full_screen_mode;

#define MAX_LANGUAGE_CNT	15
EXTERN CLanguageSettingList	g_lang_setting_list;

struct _st_option {
	struct {
		int		tabstop{ 4 };				// tabstop count of textctrl
		int		row_space{ 0 };
		int		char_space{ 0 };
		int		top_space{ 0 };
		int		left_space{ 0 };
		BOOL	show_line_feed{ FALSE };
		BOOL	show_line_end{ FALSE };
		BOOL	show_tab{ FALSE };
		BOOL	show_row_num{ FALSE };
		BOOL	show_col_num{ FALSE };
		BOOL	show_col_num_split_window{ FALSE };
		BOOL	show_space{ FALSE };
		BOOL	show_2byte_space{ FALSE };
		BOOL	show_row_line{ FALSE };
		BOOL	show_edit_row{ FALSE };
		BOOL	show_brackets_bold{ FALSE };
		BOOL	show_ruled_line{ FALSE };
		int		brackets_blink_time{ 0 };
		int		line_mode{ 0 };
		int		line_len{ 0 };
		COLORREF	color[EDIT_CTRL_COLOR_CNT];
		int		indent_mode{ 0 };
		BOOL	tab_as_space{ FALSE };
		BOOL	clickable_url{ FALSE };
		BOOL	drag_drop_edit{ FALSE };
		BOOL	mouse_select_copy{ FALSE };
		BOOL	row_copy_at_no_sel{ FALSE };
		BOOL	clear_after_copy{ FALSE };
		int		wheel_scroll_row{ 3 };
	
		BOOL	use_keyword_window{ FALSE };

		BOOL	enable_word_wrap{ FALSE };
		BOOL	enable_confinement{ FALSE };
	} text_editor;

	CString	first_confinement_str;
	CString	last_confinement_str;

	BOOL		boot_on_ime{ FALSE };
	BOOL		save_modified{ TRUE };
	CString		initial_dir;
	CString		kb_macro_dir;
	int			edit_mode{ 0 };
	BOOL		window_title_is_path_name{ FALSE };
	BOOL		search_loop_msg{ FALSE };
	BOOL		search_dlg_close_at_found{ FALSE };
	BOOL		enable_tag_jump{ FALSE };
	int			max_mru{ 4 };

	CString		browser_01;
	CString		browser_02;
	CString		browser_03;

	struct _st_option_font {
		CString		face_name;
		int			point{ 0 };
		int			weight{ 0 };
		BOOL		italic{ FALSE };
	} font;

	int			default_kanji_code{ 0 };
	int			default_line_type{ 0 };
	CString		default_edit_mode;

	int			file_open_kanji_code{ 0 };

	struct _st_option_ogrep {
		BOOL	search_dir_as_current_dir{ TRUE };
	} ogrep;

	struct _st_full_screen {
		int		top_space{ 0 };
		int		left_space{ 0 };
		int		height{ 400 };
		int		width{ 800 };
		CString	bg_image_file;
	} full_screen;
};
EXTERN _st_option	g_option;

#endif /* OEDIT_GLOBAL_H_INCLUDED */