#ifndef __EditDocData_INCLUDE__
#define __EditDocData_INCLUDE__

#include "EditData.h"
/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "CodeAssistEditCtrl.h"

class CDocData {
public:
	CEditData	edit_data;
	CCodeAssistEditCtrl	edit_ctrl;
	int			edit_mode;
	POINT		cur_pt;
	POINT		scr_pt;
	CString		file_name;
	int			kanji_code;
	int			line_type;
	BOOL		is_new_document;
	int			seq_no;
	CDocData	*next;

	CDocData(int seq, CString file_name_) {
		file_name = file_name_;
		edit_mode = 0;
		kanji_code = KANJI_CODE_SJIS;
		line_type = LINE_TYPE_CR_LF;
		
		is_new_document = FALSE;

		seq_no = seq;
		next = NULL;

		cur_pt.x = 0;
		cur_pt.y = 0;
		scr_pt.x = 0;
		scr_pt.y = 0;
	}

	~CDocData() {
		if(edit_ctrl.GetSafeHwnd() != NULL) {
			edit_ctrl.ShowWindow(SW_HIDE);
			edit_ctrl.DestroyWindow();
		}
	}
};

class CDocDataArray {
private:
	CDocData	*m_doc_data;
	CDocData	*m_cur_data;
	int			m_doc_data_seq;

	CDocData *GetTail() {
		CDocData	*data;
		for(data = m_doc_data; data != NULL;) {
			if(data->next == NULL) return data;
			data = data->next;
		}
		return NULL;
	}

	CDocData *FindData(int seq) {
		CDocData	*data;
		for(data = m_doc_data; data != NULL;) {
			if(data->seq_no == seq) return data;
			data = data->next;
		}
		return NULL;
	}

	CDocData *GetPrev() {
		CDocData	*data;
		for(data = m_doc_data; data != NULL;) {
			if(data->next == m_cur_data) {
				return data;
			}
			data = data->next;
		}
		return NULL;
	}
	CDocData *GetNext() {
		return m_cur_data->next;
	}

public:
	CDocDataArray() {
		m_doc_data_seq = 0;
		m_doc_data = new CDocData(m_doc_data_seq, "-");
		m_doc_data_seq++;

		m_doc_data->edit_ctrl.SetReadOnly(TRUE);
		m_doc_data->file_name = "-";
		m_cur_data = m_doc_data;
	}

	~CDocDataArray() {
		freeArray();
	}

	void freeArray() {
		CDocData	*data, *next;

		for(data = m_doc_data; data != NULL;) {
			next = data->next;
			delete data;
			data = next;
		}
	}

	int AddDocData(CString file_name) {
		CDocData	*data = new CDocData(m_doc_data_seq, file_name);
		m_doc_data_seq++;
		GetTail()->next = data;
		return data->seq_no;
	}

	void DeleteDocData() {
		CDocData	*data = GetPrev();
		if(data == NULL) return;

		data->next = m_cur_data->next;
		delete m_cur_data;
		m_cur_data = data;
		if(m_cur_data->seq_no == 0 && m_cur_data->next != NULL) m_cur_data = m_cur_data->next;
	}

	void ChangeCurrentIdx(int idx) {
		CDocData	*data = FindData(idx);
		if(data == NULL) return;

		// 同じEditCtrlを表示中に，SW_HIDE->SW_SHOWするとキャレットが消えてしまう
		if(m_cur_data == data && m_cur_data->edit_ctrl.IsWindowVisible()) {
			return;
		}

		if(m_cur_data->edit_ctrl.GetSafeHwnd() != NULL) {
			m_cur_data->edit_ctrl.ShowWindow(SW_HIDE);
		}
		m_cur_data = data;
		if(IsEmpty() == FALSE && m_cur_data->edit_ctrl.GetSafeHwnd() != NULL) {
			m_cur_data->edit_ctrl.ShowWindow(SW_SHOW);
		}
	}

	int FindData(CString file_name) {
		CDocData	*data;
		for(data = m_doc_data; data != NULL;) {
			if(data->file_name == file_name) return data->seq_no;
			data = data->next;
		}
		return -1;
	}

	CDocData *GetCurrentDocData() { return m_cur_data; }
	int GetCurrentDocDataId() { return m_cur_data->seq_no; }

	BOOL IsEmpty() {
		if(m_doc_data->next == NULL) return TRUE;
		return FALSE;
	}

	CDocData *GetDocData(int idx) { return FindData(idx); }
};

#endif __EditDocData_INCLUDE__
