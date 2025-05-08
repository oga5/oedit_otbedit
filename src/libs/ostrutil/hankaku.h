/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef __HANKAKU_H_INCLUDED__
#define __HANKAKU_H_INCLUDED__

#ifdef  __cplusplus
extern "C" {
#endif

int HankakuToZenkaku(const TCHAR *str, TCHAR *buf);
int HankakuToZenkaku2(const TCHAR *str, TCHAR *buf, int b_alpha, int b_kata);

int ZenkakuToHankaku(const TCHAR *str, TCHAR *buf);
int ZenkakuToHankaku2(const TCHAR *str, TCHAR *buf, int b_alpha, int b_kata);

#ifdef  __cplusplus
}
#endif

#endif __HANKAKU_H_INCLUDED__
