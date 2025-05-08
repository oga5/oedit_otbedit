/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef _OSTRUTIL_BM_SEARCH_H_INCLUDED_
#define _OSTRUTIL_BM_SEARCH_H_INCLUDED_

#ifdef  __cplusplus
extern "C" {
#endif

#include "get_char.h"

typedef struct bm_search_data_st *HBM_DATA;

void bm_free(HBM_DATA data);
HBM_DATA bm_create_data(const TCHAR *pattern, int i_flg);
TCHAR *bm_search(HBM_DATA bm_data, const TCHAR *p_buf, INT_PTR len);

#ifdef  __cplusplus
}
#endif

#endif /* _OSTRUTIL_BM_SEARCH_H_INCLUDED_ */

