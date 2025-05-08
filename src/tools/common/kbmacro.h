/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef _KBMACRO_H_INCLUDE
#define _KBMACRO_H_INCLUDE

BOOL RecStartKBMacro();
BOOL RecEndKBMacro();
BOOL IsRecodingKBMacro();

BOOL CanPlayKBMacro();
BOOL IsPlaingKBMacro();
void PlayKBMacro();

void ClearKBMacroBuffer();

BOOL SaveKBMacro(TCHAR *file_name);
BOOL LoadKBMacro(TCHAR *file_name);
BOOL PlayKBMacro(TCHAR *file_name);

#endif _KBMACRO_H_INCLUDE
