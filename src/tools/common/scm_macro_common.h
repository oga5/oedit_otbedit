/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef SCM_MACRO_COMMON_H_INCLUDED
#define SCM_MACRO_COMMON_H_INCLUDED

// global
#include "oscheme.h"

void clear_scm_macro(oscheme **psc);

BOOL scm_call_event_handler(oscheme *sc, enum scm_event_handler ev);
BOOL scm_have_event_handler(oscheme *sc, enum scm_event_handler ev);
BOOL scm_apply_keyboard_hook(oscheme *sc, WPARAM keycode);

int scm_eval_str(oscheme *sc, const TCHAR *code);
int scm_eval_file(oscheme *sc, const TCHAR *file_name);

void scm_set_status_bar_flg(oscheme *sc);

// local
#include "EditCtrl.h"

struct scm_event_handler_table_st {
	enum scm_event_handler event_cd;
	TCHAR* event_name{ NULL };
	pcell	proc{ NULL };
};

struct _oscheme_user_data_st {
	int _scm_err_flg{ 0 };
	int _scm_status_bar_flg{ 0 };
	CString _scm_err_buf{ _T("") };
	CEditData* _scm_out_buf{ NULL };
	CMapPtrToPtr* _key_hook{ NULL };
	CMapWordToPtr* _timer_proc{ NULL };
	struct scm_event_handler_table_st* event_tbl{ NULL };
	int event_cnt{ 0 };
	void (*pre_proc_func)(oscheme* sc) { NULL };
	void (*post_proc_func)(oscheme* sc) { NULL };
};

struct scm_key_hook_st
{
	CString keyname{ _T("") };
	CString comment{ _T("") };
	pcell	proc{ NULL };
};

struct scm_call_back_st {
	TCHAR* callback_name{ NULL };
	oscheme_callback_func func{ NULL };
	int min_argc{ 0 };
	int max_argc{ 0 };
	TCHAR* type_chk_str{ NULL };
};

struct _oscheme_user_data_st *scm_get_user_data(oscheme *sc);

void scm_clear_err_flg(oscheme *sc);

void scm_set_status_msg(const TCHAR *msg);
void scm_start_macro_msg(oscheme *sc);
void scm_end_macro_msg(oscheme *sc);

pcell scm_set_cancel_key(oscheme *sc, pcell *argv, int argc);
pcell scm_get_all_keys(oscheme *sc, pcell *argv, int argc);
pcell scm_set_key(oscheme *sc, pcell *argv, int argc);

pcell scm_set_event_handler(oscheme *sc, pcell *argv, int argc);
pcell scm_get_event_handler(oscheme *sc, pcell *argv, int argc);
pcell scm_clear_event_handler(oscheme *sc, pcell *argv, int argc);

pcell scm_set_timer_event(oscheme *sc, pcell *argv, int argc);
pcell scm_clear_timer_event(oscheme *sc, pcell *argv, int argc);

pcell scm_get_app_dir(oscheme *sc, pcell *argv, int argc);
pcell scm_get_command_line_option(oscheme *sc, pcell *argv, int argc);

pcell scm_msg_box(oscheme *sc, pcell *argv, int argc);
pcell scm_input_box(oscheme *sc, pcell *argv, int argc);
pcell scm_status_bar_msg(oscheme *sc, pcell *argv, int argc);

void scm_port_err(oscheme *sc, const TCHAR *str);
void scm_port_out(oscheme *sc, const TCHAR *str);

void scm_disp_eval_info(oscheme *sc);

pcell scm_rgb(oscheme *sc, pcell *argv, int argc);

BOOL init_scm_macro_main(oscheme **psc, const TCHAR *ini_scm_file,
	struct scm_call_back_st *callbacks, int callback_cnt,
	struct scm_event_handler_table_st *events, int event_cnt,
	void (*pre_proc_func)(oscheme *sc), void (*post_proc_func)(oscheme *sc));

#endif  SCM_MACRO_COMMON_H_INCLUDED


