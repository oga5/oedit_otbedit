/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef __COMMON_UTIL_H_INCLUDED__
#define __COMMON_UTIL_H_INCLUDED__

// defineする番号が，greputil.hと重複しないようにする
#define WM_COPY_DATA_SET_OPTION			1010

struct _set_option_st {
	TCHAR		file_name[_MAX_PATH];
	int			row;
	TCHAR		mode[10];
};

void SetBootOption(const TCHAR *file_name, int row, const TCHAR *mode);
BOOL TagJump(const TCHAR *tag_data);

#endif __COMMON_UTIL_H_INCLUDED__
