/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */


#include <string.h>
#include <mbstring.h>

#include "mbutil.h"
#include "get_char.h"
#include "str_inline.h"

unsigned int get_ascii_char(unsigned int ch)
{
	static TCHAR* markSet = _T("　¥‾”’‘");
	static TCHAR* markConvTbl = _T(" \\~\"'`");

	const TCHAR *cp;

	if (ch >= L'！' && ch <= L'｝') {
		ch -= L'！' - L'!';
	}
	else if ((cp = inline_strchr(markSet, ch)) != NULL) {
		ch = markConvTbl[cp - markSet];
	}

	return ch;
}

static int __HankakuToZenkaku(const TCHAR *str, TCHAR *buf,
	int b_alpha, int b_kata)
{
	static TCHAR* convTbl = _T("。「」、・ヲァィゥェォャュョッーアイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワン゛゜");
	static TCHAR* pszDakuSet = _T("カキクケコサシスセソタチツテトハヒフヘホ");
	static TCHAR* dakuConvTbl = _T("ガギグゲゴザジズゼゾダヂヅデドバビブベボ");
	static TCHAR* pszYouSet = _T("ハヒフヘホ");
	static TCHAR* dakuYouTbl = _T("パピプペポ");

	TCHAR *pbuf = buf;
	TCHAR *cp;
	unsigned int ch;
	int	flg = 0;

	for(; *str != '\0';) {
		ch = get_char(str);

		// 半角英数
		if(b_alpha && ch >= 0x20 && ch <= 0x7E) {
			flg = 1;
			if(ch == ' ') {
				ch = L'　';
			} else if(ch == '\\') {
				ch = L'¥';
			} else if(ch == '~') {
				ch = L'‾';
			} else {
				// '!' - '}'
				ch += L'！' - L'!';
			}
		}

		// 半角カタカナ
		if(b_kata && ch >= 0xFF61 && ch <= 0xFF9F) {
			flg = 1;

			if(get_char(str + get_char_len(str)) == L'゛' && (cp = inline_strchr(pszDakuSet, *str)) != NULL) {
				ch = dakuConvTbl[cp - pszDakuSet];
				str += get_char_len(str);
			} else if(get_char(str + get_char_len(str)) == L'゜' && (cp = inline_strchr(pszYouSet, *str)) != NULL) {
				ch = dakuYouTbl[cp - pszYouSet];
				str += get_char_len(str);
			} else {
				ch = convTbl[ch - 0xFF61];
			}
		}

		pbuf = put_char(pbuf, ch);
		str += get_char_len(str);
	}

	*pbuf = '\0';
	return flg;
}

// 半角文字列を全角文字列に変換する
int HankakuToZenkaku(const TCHAR *str, TCHAR *buf)
{
	return __HankakuToZenkaku(str, buf, 1, 1);
}

int HankakuToZenkaku2(const TCHAR *str, TCHAR *buf,
	int b_alpha, int b_kata)
{
	return __HankakuToZenkaku(str, buf, b_alpha, b_kata);
}

#define GET_CHAR(p) (*p) + *(p + 1)

static int __ZenkakuToHankaku(const TCHAR *str, TCHAR *buf,
	int b_alpha, int b_kata)
{
	static TCHAR* zenKataSet = _T("。「」、・ヲァィゥェォャュョッーアイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワン゛゜");
	static TCHAR* zenKataConvTbl = _T("。「」、・ヲァィゥェォャュョッーアイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワン゛゜");

	static TCHAR* zenDakuSet = _T("ガギグゲゴザジズゼゾダヂヅデドバビブベボ");
	static TCHAR* zenDakuConvTbl = _T("カキクケコサシスセソタチツテトハヒフヘホ");

	static TCHAR* zenYouSet = _T("パピプペポ");
	static TCHAR* zenYouConvTbl = _T("ハヒフヘホ");

	static TCHAR* markSet = _T("　¥‾”’‘");
	static TCHAR* markConvTbl = _T(" \\~\"'`");

	TCHAR *pbuf = buf;
	TCHAR *cp;
	unsigned int ch;
	int	flg = 0;

	for(; *str != '\0';) {
		ch = get_char(str);

		// 全角英数
		if(b_alpha) {
			if(ch >= L'！' && ch <= L'｝') {
				flg = 1;
				ch -= L'！' - L'!';
			} else if((cp = inline_strchr(markSet, *str)) != NULL) {
				flg = 1;
				ch = markConvTbl[cp - markSet];
			}
		}

		// 全角カタカナ
		if(b_kata) {
			if((cp = inline_strchr(zenKataSet, *str)) != NULL) {
				flg = 1;
				ch = zenKataConvTbl[cp - zenKataSet];
			} else if((cp = inline_strchr(zenDakuSet, *str)) != NULL) {
				flg = 1;
				ch = zenDakuConvTbl[cp - zenDakuSet];
				pbuf = put_char(pbuf, ch);
				ch = L'゛';
			} else if((cp = inline_strchr(zenYouSet, *str)) != NULL) {
				flg = 1;
				ch = zenYouConvTbl[cp - zenYouSet];
				pbuf = put_char(pbuf, ch);
				ch = L'゜';
			}
		}

		pbuf = put_char(pbuf, ch);
		str += get_char_len(str);
	}

	*pbuf = '\0';
	return flg;
}

// 半角文字列を全角文字列に変換する
int ZenkakuToHankaku(const TCHAR *str, TCHAR *buf)
{
	return __ZenkakuToHankaku(str, buf, 1, 1);
}

int ZenkakuToHankaku2(const TCHAR *str, TCHAR *buf,
	int b_alpha, int b_kata)
{
	return __ZenkakuToHankaku(str, buf, b_alpha, b_kata);
}
