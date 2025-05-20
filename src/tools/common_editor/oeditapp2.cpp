/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */


#include "stdafx.h"
#include "oedit.h"

#include "fileutil.h"
#include "optionSheet.h"
#include "oeditDoc.h"
#include "ColorUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static const char *THIS_FILE = __FILE__;
#endif

void COeditApp::LoadOption()
{
	g_option.text_editor.tabstop = GetIniFileInt(_T("TEXT_EDITOR"), _T("TABSTOP"), 4);
	g_option.text_editor.row_space = GetIniFileInt(_T("TEXT_EDITOR"), _T("ROW_SPACE"), 0);
	g_option.text_editor.char_space = GetIniFileInt(_T("TEXT_EDITOR"), _T("CHAR_SPACE"), 0);
	g_option.text_editor.top_space = GetIniFileInt(_T("TEXT_EDITOR"), _T("TOP_SPACE"), 5);
	g_option.text_editor.left_space = GetIniFileInt(_T("TEXT_EDITOR"), _T("LEFT_SPACE"), 5);
	g_option.text_editor.show_line_feed = GetIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_LINE_FEED"), TRUE);
	g_option.text_editor.show_line_end = GetIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_LINE_END"), FALSE);
	g_option.text_editor.show_tab = GetIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_TAB"), TRUE);
	g_option.text_editor.show_space = GetIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_SPACE"), FALSE);
	g_option.text_editor.show_2byte_space = GetIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_2BYTE_SPACE"), TRUE);
	g_option.text_editor.show_row_num = GetIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_ROW_NUM"), TRUE);
	g_option.text_editor.show_col_num = GetIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_COL_NUM"), TRUE);
#ifdef GLOBAL_H_OEDIT
	g_option.text_editor.show_col_num_split_window = GetIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_COL_NUM_SPLIT_WINDOW"), FALSE);
#endif
	g_option.text_editor.show_row_line = GetIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_ROW_LINE"), TRUE);
	g_option.text_editor.show_ruled_line = GetIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_RULED_LINE"), FALSE);
	g_option.text_editor.show_edit_row = GetIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_EDIT_ROW"), TRUE);
	g_option.text_editor.show_brackets_bold = GetIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_BRACKETS_BOLD"), TRUE);
	g_option.text_editor.brackets_blink_time = GetIniFileInt(_T("TEXT_EDITOR"), _T("BRACKETS_BLINK_TIME"), GetCaretBlinkTime());
	if(g_option.text_editor.brackets_blink_time < 0) g_option.text_editor.brackets_blink_time = 0;
	g_option.text_editor.line_len = GetIniFileInt(_T("TEXT_EDITOR"), _T("LINE_LEN"), 80);

	g_option.text_editor.indent_mode = GetIniFileInt(_T("TEXT_EDITOR"), _T("INDENT_MODE"), FALSE);
	
	g_option.text_editor.tab_as_space = GetIniFileInt(_T("TEXT_EDITOR"), _T("TAB_AS_SPACE"), FALSE);
	g_option.text_editor.clickable_url = GetIniFileInt(_T("TEXT_EDITOR"), _T("CLICKABLE_URL"), TRUE);
	g_option.text_editor.drag_drop_edit = GetIniFileInt(_T("TEXT_EDITOR"), _T("DRAG_DROP_EDIT"), FALSE);
#ifdef GLOBAL_H_OTBEDIT
	g_option.text_editor.text_drop_edit = GetIniFileInt(_T("TEXT_EDITOR"), _T("TEXT_DROP_EDIT"), FALSE);
#endif
	g_option.text_editor.mouse_select_copy = GetIniFileInt(_T("TEXT_EDITOR"), _T("MOUSE_SELECT_COPY"), FALSE);
	g_option.text_editor.row_copy_at_no_sel = GetIniFileInt(_T("TEXT_EDITOR"), _T("ROW_COPY_AT_NO_SEL"), TRUE);
	g_option.text_editor.clear_after_copy = GetIniFileInt(_T("TEXT_EDITOR"), _T("CLEAR_AFTER_COPY"), FALSE);
	g_option.text_editor.wheel_scroll_row = GetIniFileInt(_T("TEXT_EDITOR"), _T("WHEEL_SCROLL_ROW"), 3);
	g_option.text_editor.use_keyword_window = GetIniFileInt(_T("TEXT_EDITOR"), _T("USE_KEYWORD_WINDOW"), TRUE);

	g_option.text_editor.color[TEXT_COLOR] = GetIniFileInt(_T("TEXT_EDITOR"), _T("TEXT_COLOR"), RGB(0, 0, 0));
	g_option.text_editor.color[KEYWORD_COLOR] = GetIniFileInt(_T("TEXT_EDITOR"), _T("KEYWORD_COLOR"), RGB(0, 0, 205));
	g_option.text_editor.color[KEYWORD2_COLOR] = GetIniFileInt(_T("TEXT_EDITOR"), _T("KEYWORD2_COLOR"), RGB(0, 128, 192));
	g_option.text_editor.color[COMMENT_COLOR] = GetIniFileInt(_T("TEXT_EDITOR"), _T("COMMENT_COLOR"), RGB(0, 120, 0));
	g_option.text_editor.color[BG_COLOR] = GetIniFileInt(_T("TEXT_EDITOR"), _T("BG_COLOR"), RGB(255, 255, 255));
	g_option.text_editor.color[PEN_COLOR] =	GetIniFileInt(_T("TEXT_EDITOR"), _T("PEN_COLOR"), RGB(0, 150, 150));
	g_option.text_editor.color[QUOTE_COLOR] = GetIniFileInt(_T("TEXT_EDITOR"), _T("QUOTE_COLOR"), RGB(220, 0, 0));
	g_option.text_editor.color[SEARCH_COLOR] = GetIniFileInt(_T("TEXT_EDITOR"), _T("SRARCH_COLOR"), RGB(200, 200, 200));
	g_option.text_editor.color[SELECTED_COLOR] = GetIniFileInt(_T("TEXT_EDITOR"), _T("SELECTED_COLOR"), RGB(0, 0, 50));
	g_option.text_editor.color[RULER_COLOR] = GetIniFileInt(_T("TEXT_EDITOR"), _T("RULER_COLOR"), RGB(0, 100, 0));
	g_option.text_editor.color[OPERATOR_COLOR] = GetIniFileInt(_T("TEXT_EDITOR"), _T("OPERATOR_COLOR"), RGB(128, 0, 0));
	g_option.text_editor.color[IME_CARET_COLOR] = GetIniFileInt(_T("TEXT_EDITOR"), _T("IME_CARET_COLOR"), RGB(0x00, 0xa0, 0x00));
	g_option.text_editor.color[RULED_LINE_COLOR] = GetIniFileInt(_T("TEXT_EDITOR"), _T("RULED_LINE_COLOR"), RGB(180, 180, 180));

	g_option.text_editor.enable_word_wrap = GetIniFileInt(_T("TEXT_EDITOR"), _T("ENABLE_WORD_WRAP"), FALSE);
	g_option.text_editor.enable_confinement = GetIniFileInt(_T("TEXT_EDITOR"), _T("ENABLE_CONFINEMENT"), FALSE);
	g_option.first_confinement_str = GetIniFileString(_T("TEXT_EDITOR"), _T("FIRST_CONFINEMENT_STR"), _T("、。，．・？！ー々）］｝」』"));
	g_option.last_confinement_str = GetIniFileString(_T("TEXT_EDITOR"), _T("LAST_CONFINEMENT_STR"), _T("（［｛「『"));

	g_option.boot_on_ime = GetIniFileInt(_T("APPLICATION"), _T("BOOT_ON_IME"), FALSE);
	g_option.save_modified = GetIniFileInt(_T("APPLICATION"), _T("SAVE_MODIFIED"), TRUE);
	g_option.initial_dir = GetIniFileString(_T("APPLICATION"), _T("INITIAL_DIR"), _T(""));
	if(g_option.initial_dir == "") g_option.initial_dir = GetAppPath();

	g_option.kb_macro_dir = GetIniFileString(_T("APPLICATION"), _T("KB_MACRO_DIR"), _T(""));
	if(g_option.kb_macro_dir == "") g_option.kb_macro_dir = GetAppPath();

	g_option.window_title_is_path_name = GetIniFileInt(_T("APPLICATION"), _T("WINDOW_TITLE_IS_PATH_NAME"), TRUE);
	g_option.search_loop_msg = GetIniFileInt(_T("APPLICATION"), _T("SEARCH_LOOP_MSG"), 0);
	g_option.search_dlg_close_at_found = GetIniFileInt(_T("APPLICATION"), _T("SEARCH_DLG_CLOSE_AT_FOUND"), 0);
	g_option.enable_tag_jump = GetIniFileInt(_T("APPLICATION"), _T("ENABLE_TAG_JUMP"), FALSE);
	g_option.max_mru = GetIniFileInt(_T("APPLICATION"), _T("MAX_MRU"), 4);

	g_option.browser_01 = GetIniFileString(_T("APPLICATION"), _T("BROWSER_01"), _T("C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe"));
	g_option.browser_02 = GetIniFileString(_T("APPLICATION"), _T("BROWSER_02"), _T("C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe"));
	g_option.browser_03 = GetIniFileString(_T("APPLICATION"), _T("BROWSER_03"), _T("C:\\Program Files\\Netscape\\Communicator\\Program\\netscape.exe"));

	g_option.default_kanji_code = GetIniFileInt(_T("APPLICATION"), _T("DEFAULT_KANJI_CODE"), KANJI_CODE_SJIS);
	g_option.default_line_type = GetIniFileInt(_T("APPLICATION"), _T("DEFAULT_LINE_TYPE"), LINE_TYPE_CR_LF);
	g_option.default_edit_mode = GetIniFileString(_T("APPLICATION"), _T("DEFAULT_EDIT_MODE_STR"), _T("NONE"));
	if(g_lang_setting_list.GetSettingIdx(g_option.default_edit_mode) == -1) {
		g_option.default_edit_mode = _T("NONE");
	}

	g_option.file_open_kanji_code = GetIniFileInt(_T("APPLICATION"), _T("FILE_OPEN_KANJI_CODE"), UnknownKanjiCode);
	g_open_kanji_code = g_option.file_open_kanji_code;

#ifdef GLOBAL_H_OTBEDIT
	g_option.tab_title_is_path_name = GetIniFileInt(_T("APPLICATION"), _T("TAB_TITLE_IS_PATH_NAME"), FALSE);
	g_option.tab_close_at_mclick = GetIniFileInt(_T("APPLICATION"), _T("TAB_CLOSE_AT_MCLICK"), FALSE);
	g_option.tab_drag_move = GetIniFileInt(_T("APPLICATION"), _T("TAB_DRAG_MOVE"), TRUE);
	g_option.close_btn_on_tab = GetIniFileInt(_T("APPLICATION"), _T("CLOSE_BTN_ON_TAB"), TRUE);
	g_option.show_tab_tooltip = GetIniFileInt(_T("APPLICATION"), _T("SHOW_TAB_TOOLTIP"), TRUE);
	g_option.tab_create_at_dblclick = GetIniFileInt(_T("APPLICATION"), _T("TAB_CREATE_AT_DBLCLICK"), FALSE);
	g_option.file_list_filter = GetIniFileString(_T("APPLICATION"), _T("FILE_LIST_FILTER"),
		_T("*.txt;*.htm;*.html;*.c;*.cpp;*.h;*.java;*.pl;*.cgi;*.sql;*.ora;*.log"));
	g_option.disable_double_boot = GetIniFileInt(_T("APPLICATION"), _T("DISABLE_DOUBLE_BOOT"), FALSE);
	g_option.hide_tab_bar_at_one = GetIniFileInt(_T("APPLICATION"), _T("HIDE_TAB_BAR_AT_ONE"), FALSE);
	g_option.tab_multi_row = GetIniFileInt(_T("APPLICATION"), _T("TAB_MULTI_ROW"), FALSE);

	g_option.file_tab.color[FILETAB_TEXT_COLOR] = GetIniFileInt(_T("FILETAB"), _T("TEXT_COLOR"), ::GetSysColor(COLOR_WINDOWTEXT));
	g_option.file_tab.color[FILETAB_ACTIVE_BK_COLOR] = GetIniFileInt(_T("FILETAB"), _T("ACTIVE_BK_COLOR"), ::GetSysColor(COLOR_BTNFACE));
	g_option.file_tab.color[FILETAB_NO_ACTIVE_BK_COLOR] = GetIniFileInt(_T("FILETAB"), _T("NO_ACTIVE_BK_COLOR"), 
		make_dark_color(::GetSysColor(COLOR_BTNFACE), 0.85));
	g_option.file_tab.color[FILETAB_BTN_COLOR] = GetIniFileInt(_T("FILETAB"), _T("BTN_COLOR"), ::GetSysColor(COLOR_BTNFACE));
#endif
	g_option.ogrep.search_dir_as_current_dir = GetIniFileInt(_T("OGREP"), _T("SEARCH_DIR_AS_CURRENT_DIR"), FALSE);

#ifndef GLOBAL_H_OTBEDIT
	CRect rectDesktop;
	::GetWindowRect(::GetDesktopWindow(), &rectDesktop);
	g_option.full_screen.top_space = GetIniFileInt(_T("FULL_SCREEN"), _T("TOP_SPACE"), 50);
	g_option.full_screen.left_space = GetIniFileInt(_T("FULL_SCREEN"), _T("LEFT_SPACE"), 100);
	g_option.full_screen.height = GetIniFileInt(_T("FULL_SCREEN"), _T("HEIGHT"), rectDesktop.Height() - 150);
	g_option.full_screen.width = GetIniFileInt(_T("FULL_SCREEN"), _T("WIDTH"), rectDesktop.Width() - 400);
	g_option.full_screen.bg_image_file = GetIniFileString(_T("FULL_SCREEN"), _T("BG_IMAGE_FILE"), _T(""));
#endif
}

void COeditApp::SaveOption()
{
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("TABSTOP"), g_option.text_editor.tabstop);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("ROW_SPACE"), g_option.text_editor.row_space);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("CHAR_SPACE"), g_option.text_editor.char_space);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("TOP_SPACE"), g_option.text_editor.top_space);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("LEFT_SPACE"), g_option.text_editor.left_space);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_LINE_FEED"), g_option.text_editor.show_line_feed);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_LINE_END"), g_option.text_editor.show_line_end);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_SPACE"), g_option.text_editor.show_space);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_2BYTE_SPACE"), g_option.text_editor.show_2byte_space);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_TAB"), g_option.text_editor.show_tab);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_ROW_NUM"), g_option.text_editor.show_row_num);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_COL_NUM"), g_option.text_editor.show_col_num);
#ifdef GLOBAL_H_OEDIT
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_COL_NUM_SPLIT_WINDOW"), g_option.text_editor.show_col_num_split_window);
#endif
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_ROW_LINE"), g_option.text_editor.show_row_line);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_RULED_LINE"), g_option.text_editor.show_ruled_line);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_EDIT_ROW"), g_option.text_editor.show_edit_row);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("SHOW_BRACKETS_BOLD"), g_option.text_editor.show_brackets_bold);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("BRACKETS_BLINK_TIME"), g_option.text_editor.brackets_blink_time);
//	WriteIniFileInt(_T("TEXT_EDITOR"), _T("LINE_MODE"), g_option.text_editor.line_mode);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("LINE_LEN"), g_option.text_editor.line_len);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("INDENT_MODE"), g_option.text_editor.indent_mode);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("TAB_AS_SPACE"), g_option.text_editor.tab_as_space);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("CLICKABLE_URL"), g_option.text_editor.clickable_url);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("DRAG_DROP_EDIT"), g_option.text_editor.drag_drop_edit);
#ifdef GLOBAL_H_OTBEDIT
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("TEXT_DROP_EDIT"), g_option.text_editor.text_drop_edit);
#endif
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("MOUSE_SELECT_COPY"), g_option.text_editor.mouse_select_copy);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("ROW_COPY_AT_NO_SEL"), g_option.text_editor.row_copy_at_no_sel);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("CLEAR_AFTER_COPY"), g_option.text_editor.clear_after_copy);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("WHEEL_SCROLL_ROW"), g_option.text_editor.wheel_scroll_row);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("USE_KEYWORD_WINDOW"), g_option.text_editor.use_keyword_window);

	WriteIniFileInt(_T("TEXT_EDITOR"), _T("TEXT_COLOR"), g_option.text_editor.color[TEXT_COLOR]);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("KEYWORD_COLOR"), g_option.text_editor.color[KEYWORD_COLOR]);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("KEYWORD2_COLOR"), g_option.text_editor.color[KEYWORD2_COLOR]);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("COMMENT_COLOR"), g_option.text_editor.color[COMMENT_COLOR]);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("BG_COLOR"), g_option.text_editor.color[BG_COLOR]);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("PEN_COLOR"), g_option.text_editor.color[PEN_COLOR]);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("QUOTE_COLOR"), g_option.text_editor.color[QUOTE_COLOR]);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("SRARCH_COLOR"), g_option.text_editor.color[SEARCH_COLOR]);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("SELECTED_COLOR"), g_option.text_editor.color[SELECTED_COLOR]);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("RULER_COLOR"), g_option.text_editor.color[RULER_COLOR]);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("OPERATOR_COLOR"), g_option.text_editor.color[OPERATOR_COLOR]);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("IME_CARET_COLOR"), g_option.text_editor.color[IME_CARET_COLOR]);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("RULED_LINE_COLOR"), g_option.text_editor.color[RULED_LINE_COLOR]);

	WriteIniFileInt(_T("TEXT_EDITOR"), _T("ENABLE_WORD_WRAP"), g_option.text_editor.enable_word_wrap);
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("ENABLE_CONFINEMENT"), g_option.text_editor.enable_confinement);
	WriteIniFileString(_T("TEXT_EDITOR"), _T("FIRST_CONFINEMENT_STR"), g_option.first_confinement_str);
	WriteIniFileString(_T("TEXT_EDITOR"), _T("LAST_CONFINEMENT_STR"), g_option.last_confinement_str);

	WriteIniFileString(_T("APPLICATION"), _T("INITIAL_DIR"), g_option.initial_dir);
	WriteIniFileString(_T("APPLICATION"), _T("KB_MACRO_DIR"), g_option.kb_macro_dir);
	WriteIniFileInt(_T("APPLICATION"), _T("BOOT_ON_IME"), g_option.boot_on_ime);
	WriteIniFileInt(_T("APPLICATION"), _T("SAVE_MODIFIED"), g_option.save_modified);
	WriteIniFileInt(_T("APPLICATION"), _T("WINDOW_TITLE_IS_PATH_NAME"), g_option.window_title_is_path_name);
	WriteIniFileInt(_T("APPLICATION"), _T("SEARCH_LOOP_MSG"), g_option.search_loop_msg);
	WriteIniFileInt(_T("APPLICATION"), _T("SEARCH_DLG_CLOSE_AT_FOUND"), g_option.search_dlg_close_at_found);
	WriteIniFileInt(_T("APPLICATION"), _T("ENABLE_TAG_JUMP"), g_option.enable_tag_jump);
	WriteIniFileInt(_T("APPLICATION"), _T("MAX_MRU"), g_option.max_mru);

	WriteIniFileString(_T("APPLICATION"), _T("BROWSER_01"), g_option.browser_01);
	WriteIniFileString(_T("APPLICATION"), _T("BROWSER_02"), g_option.browser_02);
	WriteIniFileString(_T("APPLICATION"), _T("BROWSER_03"), g_option.browser_03);

	WriteIniFileInt(_T("APPLICATION"), _T("DEFAULT_KANJI_CODE"), g_option.default_kanji_code);
	WriteIniFileInt(_T("APPLICATION"), _T("DEFAULT_LINE_TYPE"), g_option.default_line_type);
	WriteIniFileString(_T("APPLICATION"), _T("DEFAULT_EDIT_MODE_STR"), g_option.default_edit_mode);
	WriteIniFileInt(_T("APPLICATION"), _T("FILE_OPEN_KANJI_CODE"), g_option.file_open_kanji_code);

#ifdef GLOBAL_H_OTBEDIT
	WriteIniFileInt(_T("APPLICATION"), _T("TAB_TITLE_IS_PATH_NAME"), g_option.tab_title_is_path_name);
	WriteIniFileInt(_T("APPLICATION"), _T("TAB_CLOSE_AT_MCLICK"), g_option.tab_close_at_mclick);
	WriteIniFileInt(_T("APPLICATION"), _T("TAB_DRAG_MOVE"), g_option.tab_drag_move);
	WriteIniFileInt(_T("APPLICATION"), _T("CLOSE_BTN_ON_TAB"), g_option.close_btn_on_tab);
	WriteIniFileInt(_T("APPLICATION"), _T("SHOW_TAB_TOOLTIP"), g_option.show_tab_tooltip);
	WriteIniFileInt(_T("APPLICATION"), _T("TAB_CREATE_AT_DBLCLICK"), g_option.tab_create_at_dblclick);
	WriteIniFileString(_T("APPLICATION"), _T("FILE_LIST_FILTER"), g_option.file_list_filter);
	WriteIniFileInt(_T("APPLICATION"), _T("DISABLE_DOUBLE_BOOT"), g_option.disable_double_boot);
	WriteIniFileInt(_T("APPLICATION"), _T("HIDE_TAB_BAR_AT_ONE"), g_option.hide_tab_bar_at_one);
	WriteIniFileInt(_T("APPLICATION"), _T("TAB_MULTI_ROW"), g_option.tab_multi_row);

	WriteIniFileInt(_T("FILETAB"), _T("TEXT_COLOR"), g_option.file_tab.color[FILETAB_TEXT_COLOR]);
	WriteIniFileInt(_T("FILETAB"), _T("ACTIVE_BK_COLOR"), g_option.file_tab.color[FILETAB_ACTIVE_BK_COLOR]);
	WriteIniFileInt(_T("FILETAB"), _T("NO_ACTIVE_BK_COLOR"), g_option.file_tab.color[FILETAB_NO_ACTIVE_BK_COLOR]);
	WriteIniFileInt(_T("FILETAB"), _T("BTN_COLOR"), g_option.file_tab.color[FILETAB_BTN_COLOR]);
#endif
	
	WriteIniFileInt(_T("OGREP"), _T("SEARCH_DIR_AS_CURRENT_DIR"), g_option.ogrep.search_dir_as_current_dir);

#ifndef GLOBAL_H_OTBEDIT
	WriteIniFileInt(_T("FULL_SCREEN"), _T("TOP_SPACE"), g_option.full_screen.top_space);
	WriteIniFileInt(_T("FULL_SCREEN"), _T("LEFT_SPACE"), g_option.full_screen.left_space);
	WriteIniFileInt(_T("FULL_SCREEN"), _T("HEIGHT"), g_option.full_screen.height);
	WriteIniFileInt(_T("FULL_SCREEN"), _T("WIDTH"), g_option.full_screen.width);
	WriteIniFileString(_T("FULL_SCREEN"), _T("BG_IMAGE_FILE"), g_option.full_screen.bg_image_file);
#endif

	SaveIniFile();
}

void COeditApp::OnOption() 
{
	COptionSheet dlg(_T("オプション"), AfxGetMainWnd());
	int		i;

	dlg.m_setup_page.m_initial_dir = g_option.initial_dir;
#ifdef GLOBAL_H_OTBEDIT
	dlg.m_setup_page.m_file_list_filter = g_option.file_list_filter;
	dlg.m_setup_page2.m_tab_title_is_path_name = g_option.tab_title_is_path_name;
	dlg.m_setup_page2.m_tab_close_at_mclick = g_option.tab_close_at_mclick;
	dlg.m_setup_page2.m_tab_drag_move = g_option.tab_drag_move;
	dlg.m_setup_page2.m_close_btn_on_tab = g_option.close_btn_on_tab;
	dlg.m_setup_page2.m_show_tab_tooltip = g_option.show_tab_tooltip;
	dlg.m_setup_page2.m_tab_create_at_dblclick = g_option.tab_create_at_dblclick;
	dlg.m_setup_page2.m_tab_multi_row = g_option.tab_multi_row;
	dlg.m_setup_page2.m_disable_double_boot = g_option.disable_double_boot;
	dlg.m_setup_page2.m_hide_tab_bar_at_one = g_option.hide_tab_bar_at_one;
#endif
	dlg.m_setup_page.m_browser_01 = g_option.browser_01;
	dlg.m_setup_page.m_browser_02 = g_option.browser_02;
	dlg.m_setup_page.m_browser_03 = g_option.browser_03;
	dlg.m_setup_page.m_edit_mode = g_option.default_edit_mode;
	dlg.m_setup_page.m_file_open_kanji_code = g_option.file_open_kanji_code;
	dlg.m_setup_page.m_line_type = g_option.default_line_type;
	dlg.m_setup_page.m_kanji_code = g_option.default_kanji_code;
	dlg.m_setup_page.m_ogrep_search_dir_as_current_dir = g_option.ogrep.search_dir_as_current_dir;

	dlg.m_setup_page2.m_boot_on_ime = g_option.boot_on_ime;
	dlg.m_setup_page2.m_save_modified = g_option.save_modified;
	dlg.m_setup_page2.m_window_title_is_path_name = g_option.window_title_is_path_name;
	dlg.m_setup_page2.m_search_loop_msg = g_option.search_loop_msg;
	dlg.m_setup_page2.m_search_dlg_close_at_found = g_option.search_dlg_close_at_found;
	dlg.m_setup_page2.m_enable_tag_jump = g_option.enable_tag_jump;
	dlg.m_setup_page2.m_max_mru = g_option.max_mru;

	dlg.m_editor_page.m_tabstop = g_option.text_editor.tabstop;
	dlg.m_editor_page.m_row_space = g_option.text_editor.row_space;
	dlg.m_editor_page.m_char_space = g_option.text_editor.char_space;
	dlg.m_editor_page.m_top_space = g_option.text_editor.top_space;
	dlg.m_editor_page.m_left_space = g_option.text_editor.left_space;
	dlg.m_editor_page.m_show_line_feed = g_option.text_editor.show_line_feed;
	dlg.m_editor_page.m_show_line_end = g_option.text_editor.show_line_end;
	dlg.m_editor_page.m_show_tab = g_option.text_editor.show_tab;
	dlg.m_editor_page.m_show_space = g_option.text_editor.show_space;
	dlg.m_editor_page.m_show_2byte_space = g_option.text_editor.show_2byte_space;
	dlg.m_editor_page.m_show_row_num = g_option.text_editor.show_row_num;
	dlg.m_editor_page.m_show_col_num = g_option.text_editor.show_col_num;
#ifdef GLOBAL_H_OEDIT
	dlg.m_editor_page.m_show_col_num_split_window = g_option.text_editor.show_col_num_split_window;
#endif
	dlg.m_editor_page.m_show_row_line = g_option.text_editor.show_row_line;
	dlg.m_editor_page.m_show_ruled_line = g_option.text_editor.show_ruled_line;
	dlg.m_editor_page.m_show_edit_row = g_option.text_editor.show_edit_row;
	dlg.m_editor_page.m_show_brackets_bold = g_option.text_editor.show_brackets_bold;
	dlg.m_editor_page.m_line_len = g_option.text_editor.line_len;
	dlg.m_editor_page.m_clickable_url = g_option.text_editor.clickable_url;
	dlg.m_editor_page.m_wheel_scroll_row = g_option.text_editor.wheel_scroll_row;
	dlg.m_editor_page2.m_indent_mode = g_option.text_editor.indent_mode;
	dlg.m_editor_page2.m_tab_as_space = g_option.text_editor.tab_as_space;
	dlg.m_editor_page2.m_drag_drop_edit = g_option.text_editor.drag_drop_edit;
#ifdef GLOBAL_H_OTBEDIT
	dlg.m_editor_page2.m_text_drop_edit = g_option.text_editor.text_drop_edit;
#endif
	dlg.m_editor_page2.m_mouse_select_copy = g_option.text_editor.mouse_select_copy;
	dlg.m_editor_page2.m_check_row_copy_at_no_sel = g_option.text_editor.row_copy_at_no_sel;
	dlg.m_editor_page2.m_clear_after_copy = g_option.text_editor.clear_after_copy;
	dlg.m_editor_page2.m_use_keyword_window = g_option.text_editor.use_keyword_window;

	dlg.m_editor_page2.m_enable_word_wrap = g_option.text_editor.enable_word_wrap;
	dlg.m_editor_page2.m_enable_confinement = g_option.text_editor.enable_confinement;
	dlg.m_editor_page2.m_first_confinement_str = g_option.first_confinement_str;
	dlg.m_editor_page2.m_last_confinement_str = g_option.last_confinement_str;
	dlg.m_editor_page2.m_brackets_blink_time = g_option.text_editor.brackets_blink_time;

	for(i = 0; i < EDIT_CTRL_COLOR_CNT; i++) {
		dlg.m_editor_page.m_color[i] = g_option.text_editor.color[i];
	}

#ifdef GLOBAL_H_OTBEDIT
	for(i = 0; i < FILETAB_CTRL_COLOR_CNT; i++) {
		dlg.m_setup_page2.m_color[i] = g_option.file_tab.color[i];
	}
#endif

#ifndef GLOBAL_H_OTBEDIT
	dlg.m_editor_full_screen.m_top_space = g_option.full_screen.top_space;
	dlg.m_editor_full_screen.m_left_space = g_option.full_screen.left_space;
	dlg.m_editor_full_screen.m_height = g_option.full_screen.height;
	dlg.m_editor_full_screen.m_width = g_option.full_screen.width;
	dlg.m_editor_full_screen.m_bg_image_file = g_option.full_screen.bg_image_file;
#endif

	if(dlg.DoModal() != IDOK) return;

	if(g_option.initial_dir.Compare(dlg.m_setup_page.m_initial_dir) != 0) {
		g_option.initial_dir = dlg.m_setup_page.m_initial_dir;
		SetCurrentDirectory(g_option.initial_dir);
	}
#ifdef GLOBAL_H_OTBEDIT
	if(g_option.tab_title_is_path_name != dlg.m_setup_page2.m_tab_title_is_path_name ||
			g_option.tab_multi_row != dlg.m_setup_page2.m_tab_multi_row ||
			g_option.hide_tab_bar_at_one != dlg.m_setup_page2.m_hide_tab_bar_at_one) {
		g_option.tab_title_is_path_name = dlg.m_setup_page2.m_tab_title_is_path_name;
		g_option.tab_multi_row = dlg.m_setup_page2.m_tab_multi_row;
		g_option.hide_tab_bar_at_one = dlg.m_setup_page2.m_hide_tab_bar_at_one;
		UpdateAllDocViews(NULL, UPD_TAB_MODE, 0);
	}
	g_option.tab_close_at_mclick = dlg.m_setup_page2.m_tab_close_at_mclick;
	g_option.tab_drag_move = dlg.m_setup_page2.m_tab_drag_move;
	g_option.close_btn_on_tab = dlg.m_setup_page2.m_close_btn_on_tab;
	g_option.show_tab_tooltip = dlg.m_setup_page2.m_show_tab_tooltip;
	g_option.tab_create_at_dblclick = dlg.m_setup_page2.m_tab_create_at_dblclick;
	g_option.disable_double_boot = dlg.m_setup_page2.m_disable_double_boot;

	if(g_option.file_list_filter != dlg.m_setup_page.m_file_list_filter) {
		g_option.file_list_filter = dlg.m_setup_page.m_file_list_filter;
		g_file_list_bar.SetFilter(g_option.file_list_filter);
	}
#endif
	g_option.browser_01 = dlg.m_setup_page.m_browser_01;
	g_option.browser_02 = dlg.m_setup_page.m_browser_02;
	g_option.browser_03 = dlg.m_setup_page.m_browser_03;
	g_option.default_kanji_code = dlg.m_setup_page.m_kanji_code;
	g_option.default_line_type = dlg.m_setup_page.m_line_type;
	g_option.default_edit_mode = dlg.m_setup_page.m_edit_mode;
	g_option.file_open_kanji_code = dlg.m_setup_page.m_file_open_kanji_code;
	g_open_kanji_code = g_option.file_open_kanji_code;
	g_option.ogrep.search_dir_as_current_dir = dlg.m_setup_page.m_ogrep_search_dir_as_current_dir;

	g_option.boot_on_ime = dlg.m_setup_page2.m_boot_on_ime;
	g_option.save_modified = dlg.m_setup_page2.m_save_modified;
	g_option.window_title_is_path_name = dlg.m_setup_page2.m_window_title_is_path_name;
	g_option.search_loop_msg = dlg.m_setup_page2.m_search_loop_msg;
	g_option.search_dlg_close_at_found = dlg.m_setup_page2.m_search_dlg_close_at_found;
	g_option.enable_tag_jump = dlg.m_setup_page2.m_enable_tag_jump;
	g_option.max_mru = dlg.m_setup_page2.m_max_mru;

	g_option.text_editor.tabstop = dlg.m_editor_page.m_tabstop;
	g_option.text_editor.row_space = dlg.m_editor_page.m_row_space;
	g_option.text_editor.char_space = dlg.m_editor_page.m_char_space;
	g_option.text_editor.top_space = dlg.m_editor_page.m_top_space;
	g_option.text_editor.left_space = dlg.m_editor_page.m_left_space;
	g_option.text_editor.show_line_feed = dlg.m_editor_page.m_show_line_feed;
	g_option.text_editor.show_line_end = dlg.m_editor_page.m_show_line_end;
	g_option.text_editor.show_tab = dlg.m_editor_page.m_show_tab;
	g_option.text_editor.show_space = dlg.m_editor_page.m_show_space;
	g_option.text_editor.show_2byte_space = dlg.m_editor_page.m_show_2byte_space;
	g_option.text_editor.show_row_num = dlg.m_editor_page.m_show_row_num;
	g_option.text_editor.show_col_num = dlg.m_editor_page.m_show_col_num;
#ifdef GLOBAL_H_OEDIT
	g_option.text_editor.show_col_num_split_window = dlg.m_editor_page.m_show_col_num_split_window;
#endif
	g_option.text_editor.show_row_line = dlg.m_editor_page.m_show_row_line;
	g_option.text_editor.show_ruled_line = dlg.m_editor_page.m_show_ruled_line;
	g_option.text_editor.show_edit_row = dlg.m_editor_page.m_show_edit_row;
	g_option.text_editor.show_brackets_bold = dlg.m_editor_page.m_show_brackets_bold;
	g_option.text_editor.line_len = dlg.m_editor_page.m_line_len;
	g_option.text_editor.clickable_url = dlg.m_editor_page.m_clickable_url;
	g_option.text_editor.wheel_scroll_row = dlg.m_editor_page.m_wheel_scroll_row;
	g_option.text_editor.indent_mode = dlg.m_editor_page2.m_indent_mode;
	g_option.text_editor.tab_as_space = dlg.m_editor_page2.m_tab_as_space;
	g_option.text_editor.drag_drop_edit = dlg.m_editor_page2.m_drag_drop_edit;
#ifdef GLOBAL_H_OTBEDIT
	g_option.text_editor.text_drop_edit = dlg.m_editor_page2.m_text_drop_edit;
#endif
	g_option.text_editor.mouse_select_copy = dlg.m_editor_page2.m_mouse_select_copy;
	g_option.text_editor.row_copy_at_no_sel = dlg.m_editor_page2.m_check_row_copy_at_no_sel;
	g_option.text_editor.clear_after_copy = dlg.m_editor_page2.m_clear_after_copy;
	g_option.text_editor.use_keyword_window = dlg.m_editor_page2.m_use_keyword_window;

	g_option.text_editor.enable_word_wrap = dlg.m_editor_page2.m_enable_word_wrap;
	g_option.text_editor.enable_confinement = dlg.m_editor_page2.m_enable_confinement;
	g_option.first_confinement_str = dlg.m_editor_page2.m_first_confinement_str;
	g_option.last_confinement_str = dlg.m_editor_page2.m_last_confinement_str;
	g_option.text_editor.brackets_blink_time = dlg.m_editor_page2.m_brackets_blink_time;

	for(i = 0; i < EDIT_CTRL_COLOR_CNT; i++) {
		g_option.text_editor.color[i] = dlg.m_editor_page.m_color[i];
	}

#ifdef GLOBAL_H_OTBEDIT
	for(i = 0; i < FILETAB_CTRL_COLOR_CNT; i++) {
		g_option.file_tab.color[i] = dlg.m_setup_page2.m_color[i];
	}
#endif

#ifndef GLOBAL_H_OTBEDIT
	g_option.full_screen.top_space = dlg.m_editor_full_screen.m_top_space;
	g_option.full_screen.left_space = dlg.m_editor_full_screen.m_left_space;
	g_option.full_screen.height = dlg.m_editor_full_screen.m_height;
	g_option.full_screen.width = dlg.m_editor_full_screen.m_width;
	g_option.full_screen.bg_image_file = dlg.m_editor_full_screen.m_bg_image_file;
#endif

	OptionChanged();
}

void COeditApp::OptionChanged()
{
	UpdateAllDocViews(NULL, UPD_EDITOR_OPTION, 0);

#ifdef GLOBAL_H_OEDIT
	RedrawEditCtrl();
#endif

	SaveOption();
}