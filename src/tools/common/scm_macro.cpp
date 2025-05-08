/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "EditCtrl.h"
#include "oscheme.h"
#include "scm_macro.h"
#include "fileutil.h"
#include "accellist.h"

#include "MainFrm.h"
#include "oeditview.h"

struct scm_event_handler_table_st event_handler_table[] = {
#define _SCM_DEF(EVT_CD,EVT_NAME) { EVT_CD, EVT_NAME, NULL },
	_SCM_DEFINE_TABLE_
#undef _SCM_DEF
};

static int _doc_line_type = LINE_TYPE_LF;

void scm_set_doc_line_type(int line_type)
{
	_doc_line_type = line_type;
}

CMainFrame *get_main_frame()
{
	if(!IsWindow(AfxGetMainWnd()->GetSafeHwnd())) return NULL;
	if(!AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMainFrame))) return NULL;
	return (CMainFrame *)AfxGetMainWnd();
}

COeditDoc *get_cur_document()
{
	CMainFrame *frm = get_main_frame();
	if(frm == NULL) return NULL;
	return (COeditDoc *)(frm->GetActiveDocument());
}

COeditView *get_cur_oedit_view()
{
	CDocument *doc = get_cur_document();
	if(doc == NULL) return NULL;

	POSITION pos = doc->GetFirstViewPosition();
	CView *pview = NULL;
	for(; pos != NULL; ) {
		pview = doc->GetNextView(pos);
		if(pview->IsKindOf(RUNTIME_CLASS(COeditView))) {
			COeditView *v = (COeditView *)pview;
			return v;
		}
	}
	
	return NULL;
}

CEditCtrl *get_cur_edit_ctrl()
{
	COeditView *pview = get_cur_oedit_view();
	if(pview == NULL) return NULL;

	CEditCtrl *edit_ctrl = pview->GetEditCtrl();
	if(edit_ctrl == NULL) return NULL;
	if(!IsWindow(edit_ctrl->GetSafeHwnd())) return NULL;

	return edit_ctrl;
}

static pcell _scm_search_str_main(oscheme *sc, pcell *argv, BOOL ignore_case, BOOL ignore_width_ascii)
{
	COeditView *v = get_cur_oedit_view();
	if(v == NULL) return oscheme_false(sc);

	v->SetSearchText(oscheme_get_strvalue(sc, argv[0]), !ignore_case, !ignore_width_ascii);

	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl == NULL) return oscheme_false(sc);

	int result = edit_ctrl->SearchText(oscheme_get_strvalue(sc, argv[0]), 
		1, !ignore_case, TRUE, NULL);
	return oscheme_mk_number(sc, result);
}

pcell scm_search_str(oscheme *sc, pcell *argv, int argc)
{
	return _scm_search_str_main(sc, argv, FALSE, FALSE);
}

pcell scm_search_str_ci(oscheme *sc, pcell *argv, int argc)
{
	return _scm_search_str_main(sc, argv, TRUE, FALSE);
}

static pcell _scm_replace_str_main(oscheme *sc, pcell *argv, BOOL ignore_case, BOOL selected_area)
{
	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl == NULL) return oscheme_false(sc);

	int replace_cnt;

	edit_ctrl->ReplaceText(
		oscheme_get_strvalue(sc, argv[0]), 
		oscheme_get_strvalue(sc, argv[1]), 
		1, !ignore_case, TRUE, TRUE, selected_area, &replace_cnt, NULL);
	return oscheme_mk_number(sc, replace_cnt);
}

pcell scm_replace_str(oscheme *sc, pcell *argv, int argc)
{
	return _scm_replace_str_main(sc, argv, FALSE, FALSE);
}

pcell scm_replace_str_ci(oscheme *sc, pcell *argv, int argc)
{
	return _scm_replace_str_main(sc, argv, TRUE, FALSE);
}

pcell scm_replace_selected_str(oscheme *sc, pcell *argv, int argc)
{
	return _scm_replace_str_main(sc, argv, FALSE, TRUE);
}

pcell scm_replace_selected_str_ci(oscheme *sc, pcell *argv, int argc)
{
	return _scm_replace_str_main(sc, argv, TRUE, TRUE);
}

pcell scm_paste_str(oscheme *sc, pcell *argv, int argc)
{
	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl == NULL) return oscheme_false(sc);

	edit_ctrl->Paste(oscheme_get_strvalue(sc, argv[0]));
	return oscheme_true(sc);
}

pcell scm_delete_str(oscheme *sc, pcell *argv, int argc)
{
	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl == NULL) return oscheme_false(sc);

	edit_ctrl->Delete();
	return oscheme_true(sc);
}

pcell scm_get_file_name(oscheme *sc, pcell *argv, int argc)
{
	CDocument *doc = get_cur_document();
	if(doc == NULL) return oscheme_false(sc);

	const TCHAR *name = doc->GetPathName();
	return oscheme_mk_string(sc, name);
}

pcell scm_get_cur_row(oscheme *sc, pcell *argv, int argc)
{
	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl == NULL) return oscheme_false(sc);

	return oscheme_mk_number(sc, edit_ctrl->GetEditData()->get_cur_row());
}

pcell scm_get_cur_col(oscheme *sc, pcell *argv, int argc)
{
	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl == NULL) return oscheme_false(sc);

	return oscheme_mk_number(sc, edit_ctrl->GetEditData()->get_cur_col());
}

pcell scm_get_row_cnt(oscheme *sc, pcell *argv, int argc)
{
	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl == NULL) return oscheme_false(sc);

	return oscheme_mk_number(sc, edit_ctrl->GetEditData()->get_row_cnt());
}

pcell scm_get_caret_col(oscheme *sc, pcell *argv, int argc)
{
	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl == NULL) return oscheme_false(sc);

	POINT pt = { (LONG)oscheme_get_ivalue(sc, argv[1]), (LONG)oscheme_get_ivalue(sc, argv[0])};
	int c = edit_ctrl->GetCaretCol(pt);
	return oscheme_mk_number(sc, c);
}

pcell scm_get_col_from_caret_col(oscheme *sc, pcell *argv, int argc)
{
	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl == NULL) return oscheme_false(sc);

	POINT pt = { oscheme_get_long_value(sc, argv[1]), oscheme_get_long_value(sc, argv[0])};
	int c = edit_ctrl->GetColFromCaretCol(pt);
	return oscheme_mk_number(sc, c);
}

pcell scm_set_row_col(oscheme *sc, pcell *argv, int argc)
{
	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl == NULL) return oscheme_false(sc);

	POINT pt = {oscheme_get_long_value(sc, argv[1]), oscheme_get_long_value(sc, argv[0])};
	edit_ctrl->ClearSelected();
	edit_ctrl->GetEditData()->set_valid_point(&pt);
	edit_ctrl->GetEditData()->set_cur(pt.y, pt.x);
	edit_ctrl->SetCaret(TRUE, 1);
	return oscheme_true(sc);
}

pcell scm_get_selected_area(oscheme *sc, pcell *argv, int argc)
{
	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl == NULL) return oscheme_false(sc);
	if(!edit_ctrl->HaveSelected()) return oscheme_false(sc);

	int		pos[4];
	pos[0] = edit_ctrl->GetEditData()->get_disp_data()->GetSelectArea()->pos1.y;
	pos[1] = edit_ctrl->GetEditData()->get_disp_data()->GetSelectArea()->pos1.x;
	pos[2] = edit_ctrl->GetEditData()->get_disp_data()->GetSelectArea()->pos2.y;
	pos[3] = edit_ctrl->GetEditData()->get_disp_data()->GetSelectArea()->pos2.x;

	oscheme_set_dont_gc_flg(sc);

	pcell	result = oscheme_nil(sc);
	for(int i = 3; i >= 0; i--) {
		pcell num = oscheme_mk_number(sc, pos[i]);
		result = oscheme_cons_cell(sc, num, result);
	}

	oscheme_clear_dont_gc_flg(sc);
	return result;
}

pcell scm_is_box_select(oscheme *sc, pcell *argv, int argc)
{
	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl == NULL) return oscheme_false(sc);
	if(!edit_ctrl->HaveSelected()) return oscheme_false(sc);

	if(edit_ctrl->GetEditData()->get_disp_data()->GetSelectArea()->box_select) {
		return oscheme_true(sc);
	}
	return oscheme_false(sc);
}

pcell scm_set_select_area(oscheme *sc, pcell *argv, int argc)
{
	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl == NULL) return oscheme_false(sc);

	POINT pt1 = {oscheme_get_long_value(sc, argv[1]), oscheme_get_long_value(sc, argv[0])};
	POINT pt2 = {oscheme_get_long_value(sc, argv[3]), oscheme_get_long_value(sc, argv[2])};

	edit_ctrl->GetEditData()->set_valid_point(&pt1);
	edit_ctrl->GetEditData()->set_valid_point(&pt2);

	edit_ctrl->SetSelectedPoint(pt1, pt2, edit_ctrl->GetNextBoxSelect());
	edit_ctrl->SetCaret(TRUE, 1);
	return oscheme_true(sc);
}

pcell scm_get_selected_str(oscheme *sc, pcell *argv, int argc)
{
	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl == NULL) return oscheme_false(sc);

	CCopyLineModeSetting l(edit_ctrl->GetEditData(), LINE_TYPE_LF);
	CString str = edit_ctrl->GetSelectedText();
	return oscheme_mk_string(sc, str.GetBuffer(0));
}

pcell scm_get_selected_str_bytes(oscheme *sc, pcell *argv, int argc)
{
	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl == NULL) return oscheme_false(sc);
	if(!edit_ctrl->HaveSelected()) return oscheme_mk_number(sc, 0);

	CCopyLineModeSetting l(edit_ctrl->GetEditData(), _doc_line_type);

	int buf_size = edit_ctrl->GetSelectedBytes();

	return oscheme_mk_number(sc, buf_size - sizeof(TCHAR));
}

pcell scm_get_selected_str_chars(oscheme *sc, pcell *argv, int argc)
{
	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl == NULL) return oscheme_false(sc);
	if(!edit_ctrl->HaveSelected()) return oscheme_mk_number(sc, 0);

	CCopyLineModeSetting l(edit_ctrl->GetEditData(), _doc_line_type);

	int chars = edit_ctrl->GetSelectedChars();

	return oscheme_mk_number(sc, chars);
}

pcell editor_move_col(oscheme *sc, int cnt, BOOL extend)
{
	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl == NULL) return oscheme_false(sc);

	int	i;
	if(cnt > 0) {
		for(i = 0; i < cnt; i++) {
			edit_ctrl->CharRight(extend);
		}
	} else {
		for(i = 0; i > cnt; i--) {
			edit_ctrl->CharLeft(extend);
		}
	}
	return oscheme_true(sc);
}

pcell editor_move_row(oscheme *sc, int row, BOOL extend)
{
	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl == NULL) return oscheme_false(sc);
	edit_ctrl->MoveCaretRow(row, extend);
	return oscheme_true(sc);
}

pcell scm_forward_char(oscheme *sc, pcell *argv, int argc)
{
	int		cnt = 1;
	BOOL	extend = FALSE;
	if(argc >= 1) cnt = oscheme_get_int_value(sc, argv[0]);
	if(argc >= 2 && !oscheme_is_false(sc, argv[1])) extend = TRUE;
	return editor_move_col(sc, cnt, extend);
}

pcell scm_backward_char(oscheme *sc, pcell *argv, int argc)
{
	int		cnt = 1;
	BOOL	extend = FALSE;
	if(argc >= 1) cnt = oscheme_get_int_value(sc, argv[0]);
	if(argc >= 2 && !oscheme_is_false(sc, argv[1])) extend = TRUE;
	return editor_move_col(sc, -cnt, extend);
}

pcell scm_next_line(oscheme *sc, pcell *argv, int argc)
{
	int		cnt = 1;
	BOOL	extend = FALSE;
	if(argc >= 1) cnt = oscheme_get_int_value(sc, argv[0]);
	if(argc >= 2 && !oscheme_is_false(sc, argv[1])) extend = TRUE;
	return editor_move_row(sc, cnt, extend);
}

pcell scm_prev_line(oscheme *sc, pcell *argv, int argc)
{
	int		cnt = 1;
	BOOL	extend = FALSE;
	if(argc >= 1) cnt = oscheme_get_int_value(sc, argv[0]);
	if(argc >= 2 && !oscheme_is_false(sc, argv[1])) extend = TRUE;
	return editor_move_row(sc, -cnt, extend);
}

pcell scm_box_select_mode(oscheme *sc, pcell *argv, int argc)
{
	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl == NULL) return oscheme_false(sc);
	if(edit_ctrl->HaveSelected()) return oscheme_false(sc);
	edit_ctrl->NextBoxSelect();
	return oscheme_true(sc);
}

pcell scm_get_all_str(oscheme *sc, pcell *argv, int argc)
{
	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl == NULL) return oscheme_false(sc);

	CCopyLineModeSetting l(edit_ctrl->GetEditData(), LINE_TYPE_LF);
	CString str = edit_ctrl->GetEditData()->get_all_text();
	return oscheme_mk_string(sc, str.GetBuffer(0));
}

pcell scm_get_row_str(oscheme *sc, pcell *argv, int argc)
{
	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl == NULL) return oscheme_false(sc);

	CString str = edit_ctrl->GetEditData()->get_row_buf(oscheme_get_int_value(sc, argv[0]));
	return oscheme_mk_string(sc, str.GetBuffer(0));
}

pcell scm_get_row_col_char(oscheme *sc, pcell *argv, int argc)
{
	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl == NULL) return oscheme_false(sc);

	CEditData *edit_data = edit_ctrl->GetEditData();

	int row = oscheme_get_int_value(sc, argv[0]);
	int col = oscheme_get_int_value(sc, argv[1]);

	POINT pt = {col, row};
	edit_data->set_valid_point(&pt);
	TCHAR *p = edit_data ->get_row_buf(pt.y) + pt.x;
	if(*p == '\0') {
		if(row == edit_data->get_row_cnt() - 1) {
			return oscheme_mk_eof(sc);
		}
		return oscheme_mk_char(sc, '\n');
	} else {
		unsigned int ch = get_char(p);
		return oscheme_mk_char(sc, ch);
	}
}

struct scm_call_back_st callback_table[] = {
	{_T("app-get-tool-dir"), scm_get_app_dir, 0, 0, _T("")},
	{_T("app-get-command-line-option"), scm_get_command_line_option, 0, 0, _T("")},
	{_T("app-msg-box"), scm_msg_box, 1, 1, _T("D")},
	{_T("app-status-bar-msg"), scm_status_bar_msg, 1, 1, _T("D")},
	{_T("app-input-box"), scm_input_box, 0, 2, _T("SS")},
	{_T("app-set-key"), scm_set_key, 2, 3, _T("ScS")},
	{_T("app-get-all-keys"), scm_get_all_keys, 0, 0, _T("")},
	{_T("app-set-cancel-key"), scm_set_cancel_key, 1, 1, _T("S")},
	{_T("app-set-event-handler"), scm_set_event_handler, 2, 2, _T("Sc")},
	{_T("app-get-event-handler"), scm_get_event_handler, 1, 1, _T("S")},
	{_T("app-clear-event-handler"), scm_clear_event_handler, 1, 1, _T("S")},
	{_T("app-set-timer"), scm_set_timer_event, 3, 3, _T("NNc")},
	{_T("app-clear-timer"), scm_clear_timer_event, 1, 1, _T("N")},
	{_T("editor-get-filename"), scm_get_file_name, 0, 0, _T("")},
	{_T("editor-get-row-cnt"),  scm_get_row_cnt, 0, 0, _T("")},
	{_T("editor-get-cur-row"), scm_get_cur_row, 0, 0, _T("")},
	{_T("editor-get-cur-col"), scm_get_cur_col, 0, 0, _T("")},
	{_T("editor-get-caret-col"), scm_get_caret_col, 2, 2, _T("NN")},
	{_T("editor-get-col-from-caret-col"), scm_get_col_from_caret_col, 2, 2, _T("NN")},
	{_T("editor-set-row-col"), scm_set_row_col, 2, 2, _T("NN")},
	{_T("editor-set-select-area"), scm_set_select_area, 4, 4, _T("NNNN")},
	{_T("editor-paste-string"), scm_paste_str, 1, 1, _T("S")},
	{_T("editor-delete-selected-string"), scm_delete_str, 0, 0, _T("")},
	{_T("editor-search-string"), scm_search_str, 1, 1, _T("S")},
	{_T("editor-search-string-ci"), scm_search_str_ci, 1, 1, _T("S")},
	{_T("editor-replace-string"), scm_replace_str, 2, 2, _T("SS")},
	{_T("editor-replace-string-ci"), scm_replace_str_ci, 2, 2, _T("SS")},
	{_T("editor-replace-selected-string"), scm_replace_selected_str, 2, 2, _T("SS")},
	{_T("editor-replace-selected-string-ci"), scm_replace_selected_str_ci, 2, 2, _T("SS")},
	{_T("editor-get-selected-string"), scm_get_selected_str, 0, 0, _T("")},
	{_T("editor-get-all-string"), scm_get_all_str, 0, 0, _T("")},
	{_T("editor-get-row-string"), scm_get_row_str, 1, 1, _T("N")},
	{_T("editor-get-row-col-char"), scm_get_row_col_char, 2, 2, _T("NN")},
	{_T("editor-get-selected-string-bytes"), scm_get_selected_str_bytes, 0, 0, _T("")},
	{_T("editor-get-selected-string-chars"), scm_get_selected_str_chars, 0, 0, _T("")},
	{_T("editor-get-selected-area"), scm_get_selected_area, 0, 0, _T("")},
	{_T("editor-forward-char"), scm_forward_char, 0, 2, _T("NA")},
	{_T("editor-backward-char"), scm_backward_char, 0, 2, _T("NA")},
	{_T("editor-next-line"), scm_next_line, 0, 2, _T("NA")},
	{_T("editor-previous-line"), scm_prev_line, 0, 2, _T("NA")},
	{_T("editor-set-box-select"), scm_box_select_mode, 0, 0, _T("")},
	{_T("editor-box-select?"), scm_is_box_select, 0, 0, _T("")},
};

void scm_pre_proc_func(oscheme *sc)
{
	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl != NULL) {
		edit_ctrl->GetEditData()->get_undo()->start_undo_set(TRUE);
	}
}

void scm_post_proc_func(oscheme *sc)
{
	CEditCtrl *edit_ctrl = get_cur_edit_ctrl();
	if(edit_ctrl != NULL) edit_ctrl->GetEditData()->get_undo()->end_undo_set();
	COeditDoc *doc = get_cur_document();
	if(doc != NULL) doc->CheckModified();
}

BOOL init_scm_macro(oscheme **psc, const TCHAR *ini_scm_file)
{
	int callback_cnt = sizeof(callback_table)/sizeof(callback_table[0]);

	return init_scm_macro_main(psc, ini_scm_file, callback_table, callback_cnt, 
		event_handler_table, MAX_SCM_EVENT, 
		scm_pre_proc_func, scm_post_proc_func);
}

