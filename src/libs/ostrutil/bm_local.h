/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef _OSTRUTIL_BM_LOCAL_H_INCLUDED_
#define _OSTRUTIL_BM_LOCAL_H_INCLUDED_

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef _UNICODE
#define BM_SKIP_TABLE_SIZE 65536
#else
#define BM_SKIP_TABLE_SIZE 256
#endif

typedef TCHAR *(*BMSearchFunc)(HBM_DATA bm_data, const TCHAR *p_buf, size_t len);

typedef struct bm_search_data_st {
	BMSearchFunc func;
    TCHAR *pattern;
    TCHAR *lwr_buf;
	size_t	lwr_buf_len;
    size_t  pattern_len;
	size_t  skip_table[BM_SKIP_TABLE_SIZE];
} BM_DATA;

#ifdef  __cplusplus
}
#endif

#endif /* _OSTRUTIL_BM_LOCAL_H_INCLUDED_ */


