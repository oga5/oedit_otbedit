/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef _MBUTIL_H_INCLUDED_
#define _MBUTIL_H_INCLUDED_

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef WIN32
    #ifndef __inline
    #define __inline
    #endif
#endif

#include "get_char.h"

#define MB_NORMAL	0
#define MB_LEAD		1
#define MB_TAIL		2

void my_mbsupr(TCHAR *mbstr);
void my_mbslwr(TCHAR *mbstr);
void my_mbsupr_1byte(TCHAR *mbstr);
void my_mbslwr_1byte(TCHAR *mbstr);
TCHAR *my_mbschr(const TCHAR *mbstr, unsigned int ch);
TCHAR *my_mbsstr(const TCHAR *string1, const TCHAR *string2);

#ifdef  __cplusplus
}
#endif

#endif /* _MBUTIL_H_INCLUDE_ */
