/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#if !defined(_INDENTER_H_INCLUDED_)
#define _INDENTER_H_INCLUDED_

#include "editdata.h"

class CIndenter
{
private:
	int indent_char[256];

	unsigned int get_last_char(int row);
	unsigned int get_first_char(int row);
	int get_first_char_pos(int row);

	CEditData	*m_edit_data;

	void ltrim(int row);
	void indent(int sample_row, INDENT_MODE indent_mode, int add = 0, BOOL b_ltrim = TRUE);

	BOOL is_null_line(int row);
	BOOL is_space_only(int row);
	int search_row_top(int row);
	int search_prev_row(int row);

	void set_cur_col(int x);

	BOOL is_space_char(unsigned int ch);

public:
	CIndenter();

	BOOL is_indent_char(unsigned int ch) {
		if(ch > 0xff) return FALSE;
		return indent_char[ch];
	}
	void DoIndent(CEditData *edit_data, unsigned char ch);
	void AutoIndent(CEditData *edit_data);
};

#endif _INDENTER_H_INCLUDED_
