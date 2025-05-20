/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef SCM_MACRO_H_INCLUDED
#define SCM_MACRO_H_INCLUDED

#include "global.h"
#include "scm_macro_common.h"

// 実行頻度の高いイベントを上にする
#ifdef GLOBAL_H_OTBEDIT
	#define _SCM_DEFINE_TABLE_ \
		_SCM_DEF(SCM_EVENT_ON_CURSOR_MOVED,_T("on-cursor-moved")) \
		_SCM_DEF(SCM_EVENT_ON_SELECTION_CHANGED,_T("on-selection-changed")) \
		_SCM_DEF(SCM_EVENT_ON_ACTIVATE_APP,_T("on-activate-app")) \
		_SCM_DEF(SCM_EVENT_ON_CHANGE_TAB,_T("on-tab-changed")) \
		_SCM_DEF(SCM_EVENT_ON_OPEN_FILE,_T("on-open-file")) \
		_SCM_DEF(SCM_EVENT_ON_CLOSE_FILE,_T("on-close-file")) \
		_SCM_DEF(SCM_EVENT_ON_FILE_SAVED,_T("on-file-saved"))
#else
	#define _SCM_DEFINE_TABLE_ \
		_SCM_DEF(SCM_EVENT_ON_CURSOR_MOVED,_T("on-cursor-moved")) \
		_SCM_DEF(SCM_EVENT_ON_SELECTION_CHANGED,_T("on-selection-changed")) \
		_SCM_DEF(SCM_EVENT_ON_ACTIVATE_APP,_T("on-activate-app")) \
		_SCM_DEF(SCM_EVENT_ON_OPEN_FILE,_T("on-open-file")) \
		_SCM_DEF(SCM_EVENT_ON_CLOSE_FILE,_T("on-close-file")) \
		_SCM_DEF(SCM_EVENT_ON_FILE_SAVED,_T("on-file-saved"))
#endif

enum scm_event_handler {
#define _SCM_DEF(EVT_CD,EVT_NAME) EVT_CD,
	_SCM_DEFINE_TABLE_
	MAX_SCM_EVENT
#undef _SCM_DEF
};

BOOL init_scm_macro(oscheme **psc, const TCHAR *ini_scm_file);

void scm_set_doc_line_type(int line_type);

class CCopyLineModeSetting
{
public:
	CCopyLineModeSetting(CEditData *edit_data, int copy_line_type) : m_edit_data(edit_data) {
		back_copy_line_type = m_edit_data->get_copy_line_type();
		m_edit_data->set_copy_line_type(copy_line_type);
	}
	~CCopyLineModeSetting() {
		m_edit_data->set_copy_line_type(back_copy_line_type);
	}
private:
	int			back_copy_line_type;
	CEditData	*m_edit_data;
};


#endif  SCM_MACRO_H_INCLUDED


