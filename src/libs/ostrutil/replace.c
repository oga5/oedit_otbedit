/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "local.h"
#include "get_char.h"

static int check_replace_str(const TCHAR  *p)
{
	if(get_char(p) == '\\') return 1;
	if(get_char(p) != '$') return 0;
	p += get_char_len(p);
	if(*p == '&' || (*p >= '1' && *p <= '9') || *p == '+' ||
		*p == '`' || *p == '\'') return 1;
	return 0;
}

int have_replace_str(const TCHAR *p)
{
	for(; *p != '\0'; p += get_char_len(p)) {
		if(check_replace_str(p)) return 1;
	}
	return 0;
}

static TCHAR *copy_str(TCHAR *result, HREG_DATASRC data_src, 
	OREG_POINT *start_pt, INT_PTR len)
{
	int		i;
	OREG_POINT	cur_pt = *start_pt;
	const TCHAR *pbuf = get_char_buf_src(data_src, start_pt);

	if(result == NULL) return NULL;

	for(i = 0; i < len;) {
		unsigned int ch = get_char(pbuf);
		result = put_char(result, ch);
		i += get_char_len2(ch);
		pbuf = next_char_buf_src(pbuf, data_src, &cur_pt);
	}

	return result;
}

INT_PTR oreg_get_backref_str(HREG_DATASRC data_src, HREG_DATA reg_data, int idx, TCHAR *buf)
{
	BACK_REF *back_ref = &(reg_data->back_ref);
	int		  back_ref_idx = idx - 1;

	if(buf == NULL) {
		return back_ref->data[back_ref_idx].len;
	}

	buf = copy_str(buf, data_src,
		&(back_ref->data[back_ref_idx].start_pt),
		back_ref->data[back_ref_idx].len);
	*buf = '\0';

	return back_ref->data[back_ref_idx].len;
}

int oreg_get_backref_cnt(HREG_DATA reg_data)
{
	return reg_data->back_ref_cnt;
}

void oreg_get_match_data(HREG_DATA reg_data,
	OREG_POINT *pstart_pt, OREG_POINT *pend_pt)
{
	*pstart_pt = reg_data->match_start;
	*pend_pt = reg_data->match_end;
}

void oreg_get_backref_data(HREG_DATA reg_data, int idx,
	OREG_POINT *pstart_pt, OREG_POINT *pend_pt)
{
	BACK_REF *back_ref = &(reg_data->back_ref);
	*pstart_pt = back_ref->data[idx].start_pt;
	*pend_pt = back_ref->data[idx].end_pt;
}

static int find_named_back_ref_idx(HREG_DATA reg_data, const TCHAR *p,
	int *skip_len)
{
	const TCHAR *name;
	int		name_len;
	int 	len = 0;
	int		idx;

	assert(get_char(p) == '{');
	len += get_char_len(p);
	p += get_char_len(p);

	name = p;

	for(; *p;) {
		if(get_char(p) == '}') {
			len += get_char_len(p);
			break;
		}
		len += get_char_len(p);
		p += get_char_len(p);
	}

	name_len = len - 2;
	*skip_len = len;

    for(idx = 0; idx < reg_data->back_ref_cnt; idx++) {
        if(reg_data->back_ref.data[idx].len < 0) continue;
        if(_tcsncmp(name, reg_data->back_ref_name[idx], name_len) == 0) {
			if(_tcslen(reg_data->back_ref_name[idx]) == (unsigned int)name_len) return idx;
		}
    }

	return -1;
}

INT_PTR oreg_make_replace_str(HREG_DATASRC data_src,
	OREG_POINT *start_pt, OREG_POINT *end_pt,
	const TCHAR *rep_str, HREG_DATA reg_data, TCHAR  *result)
{
	int idx;
	INT_PTR cp_len;
	BACK_REF *back_ref = &(reg_data->back_ref);
	OREG_POINT	copy_pt;
	INT_PTR	len = 0;
	unsigned int ch;

	const TCHAR *p = rep_str;

	if(result != NULL) result[0] = '\0';

	for(;;) {
		if(get_char(p) == '\0') break;
		if(check_replace_str(p)) {
			p += get_char_len(p);
			ch = get_char(p);
			if((ch >= '1' && ch <= '9') || ch == '+') {
				if(ch == '+') {
					p += get_char_len(p);

					if(get_char(p) == '{') {
						int skip_len;
						idx = find_named_back_ref_idx(reg_data, p, &skip_len);
						p += skip_len;
					} else {
						idx = back_ref->last_match_idx;
					}
				} else {
					p = get_back_ref_idx(p, &idx, reg_data->back_ref_cnt);
				}
				if(idx >= 0 && idx < reg_data->back_ref_cnt && 
					back_ref->data[idx].len >= 0) {
					result = copy_str(result, data_src,
						&(back_ref->data[idx].start_pt),
						back_ref->data[idx].len);
					len += back_ref->data[idx].len;
				}
				continue;
			} else if(*p == '&') {
				cp_len = data_src->get_len(data_src->src, start_pt, end_pt);
				result = copy_str(result, data_src, start_pt, cp_len);
				len += cp_len;
			} else if(*p == '`') {
				copy_pt = *start_pt;
				copy_pt.col = 0;
				cp_len = data_src->get_len(data_src->src, &copy_pt, start_pt);
				result = copy_str(result, data_src, &copy_pt, cp_len);
				len += cp_len;
			} else if(*p == '\'') {
				const TCHAR *pbuf;
				copy_pt = *end_pt;

				pbuf = get_char_buf_src(data_src, &copy_pt);
				for(;;) {
					if(*pbuf == '\0' || *pbuf == '\n') break;
					pbuf = next_char_buf_src(pbuf, data_src, &copy_pt);
				}

				cp_len = data_src->get_len(data_src->src, end_pt, &copy_pt);
				result = copy_str(result, data_src, end_pt, cp_len);
				len += cp_len;
			} else {
				cp_len = get_char_len(p);
				if(*p == 'n') {
					if(result != NULL) *result = '\n';
				} else if(*p == 't') {
					if(result != NULL) *result = '\t';
				} else {
					if(result != NULL) _tcsncpy(result, p, cp_len);
				}
				if(result != NULL) result += cp_len;
				len += cp_len;
			}
			p += get_char_len(p);
			continue;
		}
		cp_len = get_char_len(p);
		if(result != NULL) {
			_tcsncpy(result, p, cp_len);
			result += cp_len;
		}
		len += cp_len;
		p += cp_len;
	}
	if(result != NULL) *result = '\0';

	return len + 1;
}

INT_PTR oreg_check_backref_idx(HREG_DATA reg_data, int idx)
{
	BACK_REF *back_ref = &(reg_data->back_ref);
	int		  back_ref_idx = idx - 1;

	return back_ref->data[back_ref_idx].len;
}

