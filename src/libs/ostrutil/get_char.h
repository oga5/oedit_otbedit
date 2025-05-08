/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef _OSTRUTIL_GET_CHAR_H_INCLUDED_
#define _OSTRUTIL_GET_CHAR_H_INCLUDED_

#ifdef WIN32
#include <basetsd.h>
#endif

#ifdef LINUX
typedef unsigned char TCHAR;
typedef int INT_PTR;
typedef unsigned int UINT_PTR;
#endif

#ifdef OSTRUTIL_EUC
#include "get_char_euc.h"
#endif /* OSTRUTIL_EUC */

#ifdef _UNICODE
#include "get_char_unicode.h"
#endif

#endif /* _OSTRUTIL_GET_CHAR_H_INCLUDED_ */

