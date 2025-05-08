/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

int print_edit_data(HDC hdc, TCHAR *doc_name, CEditData *edit_data,
	TCHAR *font_face_name, int font_point, RECT *space, BOOL print_page, 
	BOOL print_date, int line_len, int row_num_digit, TCHAR *msg_buf);
int get_print_info(HDC hdc, TCHAR *font_face_name, int font_point, RECT *space, 
	int line_len, int row_num_digit, int *rows_par_page, int *chars_par_page);
