/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef __CHECK_KANJI_H_INCLUDED__
#define __CHECK_KANJI_H_INCLUDED__

#ifdef  __cplusplus
extern "C" {
#endif

typedef unsigned char BUF_BYTE;

#define KANJI_CODE_SJIS		1
#define KANJI_CODE_EUC		2
#define KANJI_CODE_JIS		3
#define KANJI_CODE_ASCII	4
#define KANJI_CODE_UTF16LE	5
#define KANJI_CODE_UTF8		6
#define KANJI_CODE_UTF16LE_NO_SIGNATURE	7
#define KANJI_CODE_UTF8_NO_SIGNATURE	8
#define UnknownKanjiCode	9
#define KANJI_CODE_UTF16BE				10
#define KANJI_CODE_UTF16BE_NO_SIGNATURE	11

int check_kanji_code(const BUF_BYTE *buf, int len, int *all_ascii);

int check_utf16le_signature(const BUF_BYTE *buf);
int check_utf16le_signature_fp(FILE *fp);
int check_utf16be_signature(const BUF_BYTE *buf);
int check_utf16be_signature_fp(FILE *fp);
int check_utf8_signature(const BUF_BYTE *buf);

void write_utf16le_sig(FILE *fp);
void skip_utf16le_signature(FILE *fp);

void write_utf16be_sig(FILE *fp);
void skip_utf16be_signature(FILE *fp);

int get_utf8_len(const BUF_BYTE *src);

#ifdef  __cplusplus
}
#endif

#endif /* __CHECK_KANJI_H_INCLUDED__ */
