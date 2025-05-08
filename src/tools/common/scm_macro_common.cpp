/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "scm_macro_common.h"
#include "accellist.h"
#include "fileutil.h"
#include "InputDlg.h"

#include "MainFrm.h"

static oscheme *local_sc = NULL;

struct _oscheme_user_data_st *scm_get_user_data(oscheme *sc)
{
	return (struct _oscheme_user_data_st *)oscheme_get_user_data(sc);
}

void scm_clear_err_flg(oscheme *sc)
{
	struct _oscheme_user_data_st *user_data = scm_get_user_data(sc);

	user_data->_scm_err_flg = 0;
	user_data->_scm_err_buf.Empty();
	user_data->_scm_out_buf->del_all();
}

void scm_set_status_msg(const TCHAR *msg)
{
	if(!IsWindow(AfxGetMainWnd()->GetSafeHwnd())) return;

	((CFrameWnd *)AfxGetMainWnd())->SetMessageText(msg);
}

void scm_set_status_bar_flg(oscheme *sc)
{
	if(sc == NULL) return;
	struct _oscheme_user_data_st *user_data = scm_get_user_data(sc);
	if(user_data == NULL) return;
	user_data->_scm_status_bar_flg = 1;
}

void scm_start_macro_msg(oscheme *sc)
{
	struct _oscheme_user_data_st *user_data = scm_get_user_data(sc);
	scm_set_status_msg(_T("マクロ実行中..."));
	user_data->_scm_status_bar_flg = 0;
}

void scm_end_macro_msg(oscheme *sc)
{
	struct _oscheme_user_data_st *user_data = scm_get_user_data(sc);
	if(!user_data->_scm_status_bar_flg) scm_set_status_msg(_T(""));
	user_data->_scm_status_bar_flg = 0;
}

pcell scm_set_cancel_key(oscheme *sc, pcell *argv, int argc)
{
	WORD keycode;
	BYTE fvirt;
	if(!CAccelList::get_accel_info(oscheme_get_strvalue(sc, argv[0]), keycode, fvirt)) {
		CString msg;
		msg.Format(_T("%s is invalid key"), oscheme_get_strvalue(sc, argv[0]));
		oscheme_error(sc, msg);
		return oscheme_false(sc);
	}

	oscheme_set_cancel_key(sc, fvirt, keycode);
	return oscheme_true(sc);
}

pcell scm_get_all_keys(oscheme *sc, pcell *argv, int argc)
{
	void *key;
	struct scm_key_hook_st *elem;
	struct _oscheme_user_data_st *user_data = scm_get_user_data(sc);

	oscheme_set_dont_gc_flg(sc);

	pcell	result = oscheme_nil(sc);

	POSITION pos = user_data->_key_hook->GetStartPosition();
	for(; pos != NULL;) {
		user_data->_key_hook->GetNextAssoc(pos, key, (void *&)elem);

		pcell	key = oscheme_mk_string(sc, elem->keyname.GetBuffer(0));
		pcell	comment = oscheme_mk_string(sc, elem->comment.GetBuffer(0));
		pcell	l = oscheme_nil(sc);
		l = oscheme_cons_cell(sc, comment, l);
		l = oscheme_cons_cell(sc, elem->proc, l);
		l = oscheme_cons_cell(sc, key, l);
		
		result = oscheme_cons_cell(sc, l, result);
	}

	oscheme_clear_dont_gc_flg(sc);
	return result;
}

pcell scm_set_key(oscheme *sc, pcell *argv, int argc)
{
	struct _oscheme_user_data_st *user_data = scm_get_user_data(sc);
	WORD keycode;
	BYTE fvirt;
	if(!CAccelList::get_accel_info(oscheme_get_strvalue(sc, argv[0]), keycode, fvirt)) {
		CString msg;
		msg.Format(_T("%s is invalid key"), oscheme_get_strvalue(sc, argv[0]));
		oscheme_error(sc, msg);
		return oscheme_false(sc);
	}

	if(oscheme_bound_global(sc, argv[1]) != 0) return oscheme_false(sc);

	struct scm_key_hook_st *elem = new(struct scm_key_hook_st);
	elem->keyname = oscheme_get_strvalue(sc, argv[0]);
	elem->proc = argv[1];
	if(argc == 3) {
		elem->comment = oscheme_get_strvalue(sc, argv[2]);
	}

	UINT_PTR key = (fvirt << 16) | (keycode & 0xffff);
	{
		// 既に同じキーで登録されている場合、削除する
		struct scm_key_hook_st *old_elem;
		if(user_data->_key_hook->Lookup((void *)key, (void *&)old_elem)) {
			oscheme_unbound_global(sc, old_elem->proc);
			delete old_elem;
		}
	}
	user_data->_key_hook->SetAt((void *)key, elem);

	return oscheme_true(sc);
}


static int _scm_get_event_idx(oscheme *sc, const TCHAR *event_name)
{
	struct _oscheme_user_data_st *user_data = scm_get_user_data(sc);
	int		i;

	for(i = 0; i < user_data->event_cnt; i++) {
		if(_tcscmp(event_name, user_data->event_tbl[i].event_name) == 0) {
			return i;
		}
	}
	
	return -1;
}

pcell scm_set_event_handler(oscheme *sc, pcell *argv, int argc)
{
	struct _oscheme_user_data_st *user_data = scm_get_user_data(sc);
	const TCHAR *event_name = oscheme_get_strvalue(sc, argv[0]);
	int		idx = _scm_get_event_idx(sc, event_name);

	if(idx >= 0) {
		if(oscheme_bound_global(sc, argv[1]) != 0) return oscheme_false(sc);
		if(user_data->event_tbl[idx].proc != NULL) {
			oscheme_unbound_global(sc, user_data->event_tbl[idx].proc);
		}
		user_data->event_tbl[idx].proc = argv[1];
	} else {
		CString msg;
		msg.Format(_T("%s is invalid event name"), event_name);
		oscheme_error(sc, msg);
		return oscheme_false(sc);
	}

	return oscheme_true(sc);
}

pcell scm_get_event_handler(oscheme *sc, pcell *argv, int argc)
{
	struct _oscheme_user_data_st *user_data = scm_get_user_data(sc);
	const TCHAR *event_name = oscheme_get_strvalue(sc, argv[0]);
	int		idx = _scm_get_event_idx(sc, event_name);

	pcell proc;

	if(idx >= 0) {
		proc = user_data->event_tbl[idx].proc;
	} else {
		CString msg;
		msg.Format(_T("%s is invalid event name"), event_name);
		oscheme_error(sc, msg);
		return oscheme_false(sc);
	}

	if(proc == NULL) {
		return oscheme_nil(sc);
	} else {
		return proc;
	}
}

pcell scm_clear_event_handler(oscheme *sc, pcell *argv, int argc)
{
	struct _oscheme_user_data_st *user_data = scm_get_user_data(sc);
	const TCHAR *event_name = oscheme_get_strvalue(sc, argv[0]);
	int		idx = _scm_get_event_idx(sc, event_name);

	pcell proc;

	if(idx >= 0) {
		proc = user_data->event_tbl[idx].proc;
	} else {
		CString msg;
		msg.Format(_T("%s is invalid event name"), event_name);
		oscheme_error(sc, msg);
		return oscheme_false(sc);
	}

	if(proc == NULL) {
		return oscheme_false(sc);
	} else {
		user_data->event_tbl[idx].proc = NULL;
		oscheme_unbound_global(sc, proc);
		return oscheme_true(sc);
	}
}

pcell scm_get_app_dir(oscheme *sc, pcell *argv, int argc)
{
	CString path = GetAppPath();
	return oscheme_mk_string(sc, path.GetBuffer(0));
}

pcell scm_get_command_line_option(oscheme *sc, pcell *argv, int argc)
{
	CString option = AfxGetApp()->m_lpCmdLine;
	return oscheme_mk_string(sc, option.GetBuffer(0));
}

TCHAR *dotted_cell_tostring(oscheme *sc, pcell c)
{
	TCHAR *p = oscheme_tostring(sc, c);
	int len = (int)_tcslen(p);

	if(p[len - 1] == ')') p[len - 1] = '\0';
	if(p[0] == '(') p++;

	return p;
}

pcell scm_msg_box(oscheme *sc, pcell *argv, int argc)
{
	if(!IsWindow(AfxGetMainWnd()->GetSafeHwnd())) return oscheme_false(sc);

	// ウィンドウが最小化された状態でMessageBoxを表示すると、ウィンドウを復元できなくなる
	if(AfxGetMainWnd()->IsIconic()) {
		CMainFrame *f = (CMainFrame *)AfxGetMainWnd();
		f->ActivateFrame();
	}

	AfxGetMainWnd()->MessageBox(dotted_cell_tostring(sc, argv[0]), 
		_T("scheme"), MB_ICONINFORMATION | MB_OK);
	return oscheme_true(sc);
}

pcell scm_input_box(oscheme *sc, pcell *argv, int argc)
{
	const TCHAR *title = _T("");
	const TCHAR *value = _T("");
	if(argc >= 1) title = oscheme_get_strvalue(sc, argv[0]);
	if(argc >= 2) value = oscheme_get_strvalue(sc, argv[1]);

	CInputDlg	dlg;
	dlg.CreateDlg(AfxGetMainWnd(), title, value);
	if(dlg.DoModal() != IDOK) {
		return oscheme_false(sc);
	}
	return oscheme_mk_string(sc, dlg.GetValue());
}

pcell scm_status_bar_msg(oscheme *sc, pcell *argv, int argc)
{
	if(!IsWindow(AfxGetMainWnd()->GetSafeHwnd())) return oscheme_false(sc);

	struct _oscheme_user_data_st *user_data = scm_get_user_data(sc);
	user_data->_scm_status_bar_flg = 1;
	scm_set_status_msg(dotted_cell_tostring(sc, argv[0]));

	return oscheme_true(sc);
}

void scm_port_err(oscheme *sc, const TCHAR *str)
{
	struct _oscheme_user_data_st *user_data = scm_get_user_data(sc);
	user_data->_scm_err_flg = 1;
	user_data->_scm_err_buf += str;
}

void scm_port_out(oscheme *sc, const TCHAR *str)
{
	struct _oscheme_user_data_st *user_data = scm_get_user_data(sc);
	user_data->_scm_out_buf->paste_no_undo(str);
}

void clear_scm_macro(oscheme **psc)
{
	oscheme *sc = *psc;
	if(sc == NULL) return;

	struct _oscheme_user_data_st *user_data = scm_get_user_data(sc);

	void *key;
	struct scm_key_hook_st *elem;

	POSITION pos = user_data->_key_hook->GetStartPosition();
	for(; pos != NULL;) {
		user_data->_key_hook->GetNextAssoc(pos, key, (void *&)elem);
		delete elem;
	}
	delete user_data->_scm_out_buf;
	delete user_data->_timer_proc;
	delete user_data->_key_hook;
	delete user_data;

	clear_oscheme(sc);
	*psc = NULL;
}

void scm_disp_eval_info(oscheme *sc)
{
	struct _oscheme_user_data_st *user_data = scm_get_user_data(sc);

	if(oscheme_canceled(sc)) {
		if(IsWindow(AfxGetMainWnd()->GetSafeHwnd())) {
			user_data->_scm_status_bar_flg = 1;
			scm_set_status_msg(_T("キャンセルしました"));
			return;
		}
	}

	if(user_data->_scm_err_flg) {
		AfxGetMainWnd()->MessageBox(user_data->_scm_err_buf, _T("scheme error"), MB_ICONEXCLAMATION | MB_OK);
	} else if(!user_data->_scm_out_buf->is_empty()) {
		AfxGetMainWnd()->MessageBox(user_data->_scm_out_buf->get_all_text(), _T("scheme output"), MB_ICONINFORMATION | MB_OK);
	}
}

static BOOL _scm_apply_proc(oscheme *sc, pcell proc)
{
	struct _oscheme_user_data_st *user_data = scm_get_user_data(sc);
	user_data->pre_proc_func(sc);

	scm_clear_err_flg(sc);
	scm_start_macro_msg(sc);

	pcell r = oscheme_apply(sc, proc);

	scm_disp_eval_info(sc);
	scm_end_macro_msg(sc);

	user_data->post_proc_func(sc);

	return TRUE;
}

static void CALLBACK EXPORT scm_timer_proc(HWND hWnd, UINT nMsg, UINT_PTR nIDEvent, DWORD dwTime)
{
	struct _oscheme_user_data_st *user_data = scm_get_user_data(local_sc);

	void *ptr;
	if(user_data->_timer_proc->Lookup((WORD)nIDEvent, ptr)) {
		pcell proc = (pcell)ptr;
		_scm_apply_proc(local_sc, proc);
	}
}

pcell scm_set_timer_event(oscheme *sc, pcell *argv, int argc)
{
	if(AfxGetMainWnd() == NULL) {
		return oscheme_false(sc);
	}

	struct _oscheme_user_data_st *user_data = scm_get_user_data(sc);

	int timer_id = oscheme_get_int_value(sc, argv[0]);
	int timer_msec = oscheme_get_int_value(sc, argv[1]);
	void *ptr;

	if(user_data->_timer_proc->Lookup(timer_id, ptr)) {
		oscheme_unbound_global(sc, (pcell)ptr);
	}

	if(oscheme_bound_global(sc, argv[2]) != 0) return oscheme_false(sc);
	user_data->_timer_proc->SetAt(timer_id, argv[2]);

	AfxGetMainWnd()->SetTimer(timer_id, timer_msec, scm_timer_proc);

	return oscheme_true(sc);
}

pcell scm_clear_timer_event(oscheme *sc, pcell *argv, int argc)
{
	struct _oscheme_user_data_st *user_data = scm_get_user_data(sc);

	int timer_id = oscheme_get_int_value(sc, argv[0]);
	void *ptr;

	if(user_data->_timer_proc->Lookup(timer_id, ptr)) {
		user_data->_timer_proc->RemoveKey(timer_id);
		oscheme_unbound_global(sc, (pcell)ptr);

		if(AfxGetMainWnd()) {
			AfxGetMainWnd()->KillTimer(timer_id);
		}

		return oscheme_true(sc);
	}
	return oscheme_false(sc);
}

BOOL scm_apply_keyboard_hook(oscheme *sc, WPARAM keycode)
{
	if(sc == NULL) return FALSE;

	struct _oscheme_user_data_st *user_data = scm_get_user_data(sc);
	struct scm_key_hook_st	*elem;

	UINT_PTR fVirt = FVIRTKEY | FNOINVERT;

	if(GetAsyncKeyState(VK_MENU) < 0) fVirt |= FALT;
	if(GetAsyncKeyState(VK_CONTROL) < 0) fVirt |= FCONTROL;
	if(GetAsyncKeyState(VK_SHIFT) < 0) fVirt |= FSHIFT;

	UINT_PTR key = (fVirt << 16) | (keycode & 0xffff);

	if(user_data->_key_hook->Lookup((void *)key, (void *&)elem)) {
TRACE(_T("keyboard-hook: %s\n"), CAccelList::get_accel_str((WORD)keycode, fVirt & FALT, fVirt & FCONTROL, fVirt & FSHIFT).GetBuffer(0));
		return _scm_apply_proc(sc, elem->proc);
	}
	return FALSE;
}

BOOL scm_have_event_handler(oscheme *sc, enum scm_event_handler ev)
{
	if(sc == NULL) return FALSE;

	struct _oscheme_user_data_st *user_data = scm_get_user_data(sc);
	pcell	proc = user_data->event_tbl[ev].proc;

	if(proc) return TRUE;
	return FALSE;
}

BOOL scm_call_event_handler(oscheme *sc, enum scm_event_handler ev)
{
	if(sc == NULL) return FALSE;

	struct _oscheme_user_data_st *user_data = scm_get_user_data(sc);
	pcell	proc = user_data->event_tbl[ev].proc;

	if(proc) {
		return _scm_apply_proc(sc, proc);
	}
	return FALSE;
}

int scm_eval_str(oscheme *sc, const TCHAR *code)
{
	if(sc == NULL) return FALSE;

	struct _oscheme_user_data_st *user_data = scm_get_user_data(sc);
	user_data->pre_proc_func(sc);

	scm_clear_err_flg(sc);
	scm_start_macro_msg(sc);

	pcell r = oscheme_eval_str(sc, code);

	scm_disp_eval_info(sc);
	scm_end_macro_msg(sc);

	user_data->post_proc_func(sc);

	return 0;
}

int scm_eval_file(oscheme *sc, const TCHAR *file_name)
{
	if(sc == NULL) return FALSE;

	CString		cmd;
	CString		e_fname = file_name;
	e_fname.Replace(_T("\\"), _T("\\\\"));
	cmd.Format(_T("((lambda () (load \"%s\")))"), e_fname.GetBuffer(0));
	return scm_eval_str(sc, cmd);
}

static BOOL regist_callbacks(oscheme *sc, struct scm_call_back_st *callbacks, int callback_cnt)
{
	int i;

	for(i = 0; i < callback_cnt; i++) {
		if(oscheme_regist_callback_func(sc, 
			callbacks[i].callback_name,
			callbacks[i].func,
			callbacks[i].min_argc,
			callbacks[i].max_argc,
			callbacks[i].type_chk_str) != 0) return FALSE;
	}
	return TRUE;
}

BOOL init_scm_macro_main(oscheme **psc, const TCHAR *ini_scm_file,
	struct scm_call_back_st *callbacks, int callback_cnt,
	struct scm_event_handler_table_st *events, int event_cnt,
	void (*pre_proc_func)(oscheme *sc), void (*post_proc_func)(oscheme *sc))
{
	CString	init_scm_file;
	CString	load_path, cmd;
	oscheme *sc = NULL;

	struct _oscheme_user_data_st *user_data = new struct _oscheme_user_data_st;
	if(user_data == NULL) goto ERR1;

	user_data->_key_hook = new CMapPtrToPtr;
	if(user_data->_key_hook == NULL) goto ERR1;

	user_data->_timer_proc = new CMapWordToPtr;
	if(user_data->_timer_proc == NULL) goto ERR1;

	user_data->_scm_out_buf = new CEditData;
	if(user_data->_scm_out_buf == NULL) goto ERR1;

	user_data->event_tbl = events;
	user_data->event_cnt = event_cnt;

	user_data->pre_proc_func = pre_proc_func;
	user_data->post_proc_func = post_proc_func;

	sc = init_oscheme();
	if(sc == NULL) goto ERR1;

	oscheme_set_user_data(sc, user_data);

	scm_clear_err_flg(sc);

	load_path.Format(_T("%s\\scmlib"), GetAppPath().GetBuffer(0));
	load_path.Replace(_T("\\"), _T("\\\\"));
	cmd.Format(_T("(define *load-path* '(\"%s\"))"), load_path.GetBuffer(0));
	oscheme_eval_str(sc, cmd);

	oscheme_set_out_func(sc, scm_port_out);
	oscheme_set_err_func(sc, scm_port_err);

	if(!regist_callbacks(sc, callbacks, callback_cnt)) goto ERR1;

	init_scm_file.Format(_T("%s\\scmlib\\init.scm"), GetAppPath().GetBuffer(0));
	if(is_file_exist(init_scm_file)) {
		if(oscheme_load_file(sc, init_scm_file) != 0) goto ERR1;
		if(user_data->_scm_err_flg) {
			AfxGetMainWnd()->MessageBox(user_data->_scm_err_buf, _T("scheme error"), MB_ICONEXCLAMATION | MB_OK);
		}
	}

	init_scm_file.Format(_T("%s\\scmlib\\%s"), GetAppPath().GetBuffer(0), ini_scm_file);
	if(is_file_exist(init_scm_file)) {
		if(oscheme_load_file(sc, init_scm_file) != 0) goto ERR1;
		if(user_data->_scm_err_flg) {
			AfxGetMainWnd()->MessageBox(user_data->_scm_err_buf, _T("scheme error"), MB_ICONEXCLAMATION | MB_OK);
		}
	}

	local_sc = sc;
	*psc = sc;
	return TRUE;

ERR1:
	AfxGetMainWnd()->MessageBox(_T("マクロの初期化に失敗しました。"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
	clear_oscheme(sc);
	*psc = NULL;
	return FALSE;
}

pcell scm_rgb(oscheme *sc, pcell *argv, int argc)
{
	COLORREF rgb = RGB(
		oscheme_get_ivalue(sc, argv[0]),
		oscheme_get_ivalue(sc, argv[1]),
		oscheme_get_ivalue(sc, argv[2]));

	return oscheme_mk_number(sc, rgb);
}

