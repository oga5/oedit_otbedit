/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "check_kanji.h"

#ifdef WIN32
#include <CRTDBG.H>
#else
#include <assert.h>
#define _ASSERT assert
#endif

void write_utf16le_sig(FILE *fp)
{
	fputc(0xff, fp);
	fputc(0xfe, fp);
}

void write_utf16be_sig(FILE *fp)
{
	fputc(0xfe, fp);
	fputc(0xff, fp);
}

int check_utf16le_signature(const BUF_BYTE *buf)
{
	if(buf[0] == 0xff && buf[1] == 0xfe) return 1;
	return 0;
}

int check_utf16be_signature(const BUF_BYTE *buf)
{
	if(buf[0] == 0xfe && buf[1] == 0xff) return 1;
	return 0;
}

int check_utf8_signature(const BUF_BYTE *buf)
{
	if(buf[0] == 0xef && buf[1] == 0xbb && buf[2] == 0xbf) return 1;
	return 0;
}

int check_utf16le_signature_fp(FILE *fp)
{
	BUF_BYTE	buf[2] = {0, 0};

	fseek(fp, 0, SEEK_SET);
	fread(buf, sizeof(BUF_BYTE), 2, fp);
	fseek(fp, 0, SEEK_SET);

	return check_utf16le_signature(buf);
}

void skip_utf16le_signature(FILE *fp)
{
	BUF_BYTE	buf[2] = {0, 0};
	fseek(fp, 0, SEEK_SET);
	fread(buf, sizeof(BUF_BYTE), 2, fp);

	_ASSERT(check_utf16le_signature(buf));
}

int check_utf16be_signature_fp(FILE *fp)
{
	BUF_BYTE	buf[2] = {0, 0};

	fseek(fp, 0, SEEK_SET);
	fread(buf, sizeof(BUF_BYTE), 2, fp);
	fseek(fp, 0, SEEK_SET);

	return check_utf16be_signature(buf);
}

void skip_utf16be_signature(FILE *fp)
{
	BUF_BYTE	buf[2] = {0, 0};
	fseek(fp, 0, SEEK_SET);
	fread(buf, sizeof(BUF_BYTE), 2, fp);

	_ASSERT(check_utf16be_signature(buf));
}

static int get_utf8_len_main(const BUF_BYTE *src, int len)
{
	if((src[0] & 0xf0) == 0xf0) {
		if(len < 3) return 0;
		if((src[1] & 0xc0) == 0x80 && (src[2] & 0xc0) == 0x80 && (src[2] & 0xc0)) return 4;
	} else if((src[0] & 0xf0) == 0xe0) {
		if(len < 3) return 0;
		if((src[1] & 0xc0) == 0x80 && (src[2] & 0xc0) == 0x80) return 3;
	} else if((src[0] & 0xe0) == 0xc0) {
		if(len < 2) return 0;
		if((src[1] & 0xc0) == 0x80) return 2;
	} else if((src[0] & 0x80) == 0x00) {
		return 1;
	}

	return 0;
}

int get_utf8_len(const BUF_BYTE *src)
{
	// バッファサイズが十分にあると仮定
	return get_utf8_len_main(src, 100);
}

struct _check_kanji_code_st {
	int		kanji_code;
	int		flg;
	int		hiragana_cnt;
	int		katakana_cnt;
	int		hankana_cnt;
	int		kanji_point;
	int		ratio;
};

struct _kanji_data_st {
	BUF_BYTE sjis[2];
	BUF_BYTE euc[2];
	BUF_BYTE utf8[3];
	int point;
};

/*
区 点 JIS  SJIS EUC  UTF-8  UTF-16 字
04 01 2421 829F A4A1 E38181 3041   ぁ
04 63 245F 82DD A4DF E381BF 307F   み
04 64 2460 82DE A4E0 E38280 3080   む
04 83 2473 82F1 A4F3 E38293 3093   ん
05 01 2521 8340 A5A1 E382A1 30A1   ァ
05 31 253F 835E A5BF E382BF 30BF   タ
05 32 2540 835F A5C0 E38380 30C0   ダ
05 86 2576 8396 A5F6 E383B6 30F6   ヶ
*/
struct _kanji_data_st _kanji[] = {
	{ { 0x81, 0x42 }, { 0xa1, 0xa3 }, { 0xe3, 0x80, 0x82 }, 3 }, // 。
	{ { 0x81, 0x41 }, { 0xa1, 0xa2 }, { 0xe3, 0x80, 0x81 }, 3 }, // 、
	{ { 0x81, 0x43 }, { 0xa1, 0xa4 }, { 0xef, 0xbc, 0x8c }, 3 }, // ，
	{ { 0x81, 0x60 }, { 0xa1, 0xc1 }, { 0xe3, 0x80, 0x9c }, 3 }, // 〜
	{ { 0x93, 0xfa }, { 0xc6, 0xfc }, { 0xe6, 0x97, 0xa5 }, 3 }, // 日
	{ { 0x90, 0x6c }, { 0xbf, 0xcd }, { 0xe4, 0xba, 0xba }, 3 }, // 人
	{ { 0x88, 0xea }, { 0xb0, 0xec }, { 0xe4, 0xb8, 0x80 }, 3 }, // 一
	{ { 0x8c, 0xa9 }, { 0xb8, 0xab }, { 0xe8, 0xa6, 0x8b }, 3 }, // 見
	{ { 0x8e, 0x96 }, { 0xbb, 0xf6 }, { 0xe4, 0xba, 0x8b }, 3 }, // 事
	{ { 0x96, 0x7b }, { 0xcb, 0xdc }, { 0xe6, 0x9c, 0xac }, 3 }, // 本
	{ { 0x94, 0x4e }, { 0xc7, 0xaf }, { 0xe5, 0xb9, 0xb4 }, 3 }, // 年
	{ { 0x8e, 0x9e }, { 0xbb, 0xfe }, { 0xe6, 0x99, 0x82 }, 3 }, // 時
	{ { 0x95, 0xaa }, { 0xca, 0xac }, { 0xe5, 0x88, 0x86 }, 3 }, // 分
	{ { 0x91, 0xe5 }, { 0xc2, 0xe7 }, { 0xe5, 0xa4, 0xa7 }, 3 }, // 大
	{ { 0x8f, 0xac }, { 0xbe, 0xae }, { 0xe5, 0xb0, 0x8f }, 3 }, // 小
	{ { 0x8f, 0xe3 }, { 0xbe, 0xe5 }, { 0xe4, 0xb8, 0x8a }, 3 }, // 上
	{ { 0x89, 0xba }, { 0xb2, 0xbc }, { 0xe4, 0xb8, 0x8b }, 3 }, // 下
	{ { 0x8d, 0x82 }, { 0xb9, 0xe2 }, { 0xe9, 0xab, 0x98 }, 3 }, // 高
	{ { 0x92, 0xe1 }, { 0xc4, 0xe3 }, { 0xe4, 0xbd, 0x8e }, 3 }, // 低
	{ { 0x00, 0x00 }, { 0x00, 0x00 }, { 0x00, 0x00, 0x00 }, 0 }  // END
};

// バッファの切れ目で、無効な文字コードに判定してしまわないように
// バッファの末尾数バイトはチェックしない
#define NO_CHECK_LAST_CHAR_CNT	4

static void _check_sjis(const BUF_BYTE *buf, int len, struct _check_kanji_code_st *k)
{
	int		i;
	int		no_hankana_flg = 0;

	k->kanji_code = KANJI_CODE_SJIS;
	k->flg = 1;

	for(; len > NO_CHECK_LAST_CHAR_CNT && *buf != '\0';) {
		BUF_BYTE c1 = *buf;
		if(c1 < 0x80) {
			buf++;
			len--;
			no_hankana_flg = 0;
			continue;
		}
		if(c1 >= 0xa0 && c1 <= 0xdf) {
			buf++;
			len--;
			if(!no_hankana_flg) k->hankana_cnt++;
			no_hankana_flg = 0;

			// EUC全角カナなどの可能性がある場合、点数を減らす
			if(c1 >= 0xa1 && c1 <= 0xa5) k->hankana_cnt--;

			continue;
		}
		if((c1 >= 0x81 && c1 <= 0x9f) || (c1 >= 0xe0 && c1 <= 0xfc)) {
			BUF_BYTE c2 = *(buf + 1);
			if(c2 == '\0') break;

			if((c2 >= 0x40 && c2 <= 0x7e) || (c2 >=0x80 && c2 <= 0xfc)) {
				no_hankana_flg = 0;
				if(c1 == 0x82 && (c2 >= 0x9f && c2 <= 0xf1)) {
					k->hiragana_cnt++;
				} else if(c1 == 0x83 && (c2 >= 0x40 && c2 <= 0x96)) {
					k->katakana_cnt++;
				} else if(c1 == 0xef && (c2 == 0xbd || c2 == 0xbe)) {
					// UTF8の半角の可能性がある場合、カウントしない
					no_hankana_flg = 1;
				} else {
					for(i = 0;; i++) {
						if(_kanji[i].sjis[0] == 0x00) {
							k->kanji_point++;
							break;
						}
						if(_kanji[i].sjis[0] == c1 && _kanji[i].sjis[1] == c2) {
							k->kanji_point += _kanji[i].point;
							break;
						}
					}
				}
				buf += 2;
				len -= 2;
				continue;
			}
		}
		goto NO_SJIS;
	}
	return;

NO_SJIS:
	k->flg = 0;
	return;
}

static void _check_utf8(const BUF_BYTE *buf, int len, struct _check_kanji_code_st *k)
{
	int utf8_len;
	int	i;

	k->kanji_code = KANJI_CODE_UTF8_NO_SIGNATURE;
	k->flg = 1;
	k->ratio = 15;

	for(; len > NO_CHECK_LAST_CHAR_CNT && *buf != '\0';) {
		utf8_len = get_utf8_len_main(buf, len);
		if(utf8_len == 0) goto NO_UTF8;

		if(utf8_len == 3) {
			BUF_BYTE c1 = *buf;
			BUF_BYTE c2 = *(buf + 1);
			BUF_BYTE c3 = *(buf + 2);
			if(c1 == 0xe3 &&
				((c2 == 0x81 && c3 >= 0x81 && c3 <= 0xbf) ||
				 (c2 == 0x82 && c3 >= 0x80 && c3 <= 0x9e))) {
					k->hiragana_cnt++;
			} else if(c1 == 0xe3 &&
				((c2 == 0x82 && c3 >= 0xa1 && c3 <= 0xbf) ||
				 (c2 == 0x83 && c3 >= 0x80 && c3 <= 0xb6))) {
					k->katakana_cnt++;
			} else if(c1 == 0xef &&
				((c2 == 0xbd && c3 >= 0xa1 && c3 <= 0xbf) ||
				 (c2 == 0xbe && c3 >= 0x80 && c3 <= 0x9f))) {
					k->hankana_cnt++;
			} else {
				for(i = 0;; i++) {
					if(_kanji[i].utf8[0] == 0x00) {
						k->kanji_point += 2;
						break;
					}
					if(_kanji[i].utf8[0] == c1 && _kanji[i].utf8[1] == c2 && _kanji[i].utf8[2] == c3) {
						k->kanji_point += _kanji[i].point;
						break;
					}
				}
			}
		}

		buf += utf8_len;
		len -= utf8_len;
	}
	return;

NO_UTF8:
	k->flg = 0;
	return;
}

static int get_euc_len(const BUF_BYTE *src, int len)
{
	if(*src <= 0x80) return 1;
	if(len == 1) return 0;

	if(*src == 0x8e) {
		if(*(src + 1) >= 0xa1 && *(src + 1) <= 0xfe) return 2;
		return 0;
	}
	if(*src == 0x8f) {
		if(*(src + 1) >= 0xa1 && *(src + 1) <= 0xfe) {
			if(len < 3) return 0;
			if(*(src + 2) >= 0xa1 && *(src + 2) <= 0xfe) return 3;
		}
		return 0;
	}
	if(*src >= 0xa1 && *src <= 0xfe) {
		if(*(src + 1) >= 0xa1 && *(src + 1) <= 0xfe) return 2;
	}
	return 0;
}

static void _check_euc(const BUF_BYTE *buf, int len, struct _check_kanji_code_st *k)
{
	int euc_len;
	int	i;

	k->kanji_code = KANJI_CODE_EUC;
	k->flg = 1;

	for(; len > NO_CHECK_LAST_CHAR_CNT && *buf != '\0';) {
		euc_len = get_euc_len(buf, len);
		if(euc_len == 0) goto NO_EUC;

		if(euc_len == 2) {
			BUF_BYTE c1 = *buf;
			BUF_BYTE c2 = *(buf + 1);

			if(c1 == 0xa4 && (c2 >= 0xa1 && c2 <= 0xf3)) {
				k->hiragana_cnt++;
			} else if(c1 == 0xa5 && (c2 >= 0xa1 && c2 <= 0xf6)) {
				k->katakana_cnt++;
			} else if(c1 == 0x8e && (c2 >= 0xa1 && c2 <= 0xfe)) {
				k->hankana_cnt++;
			} else {
				for(i = 0;; i++) {
					if(_kanji[i].euc[0] == 0x00) {
						k->kanji_point++;
						break;
					}
					if(_kanji[i].euc[0] == c1 && _kanji[i].euc[1] == c2) {
						k->kanji_point += _kanji[i].point;
						break;
					}
				}
			}
		}

		buf += euc_len;
		len -= euc_len;
	}
	return;

NO_EUC:
	k->flg = 0;
	return;
}

static int calc_code_point(struct _check_kanji_code_st *k)
{
	int p = k->hiragana_cnt * 5 + k->kanji_point * 3 + k->katakana_cnt * 2 + k->hankana_cnt;
	if(k->ratio > 0) p = p * k->ratio / 10;
	return p;
}

static int kanji_code_other(const BUF_BYTE *buf, int len)
{
	struct _check_kanji_code_st	k[3];
	int		i;
	int		max_point = -1;
	int		point;
	int ret_v = UnknownKanjiCode;

	memset(k, 0, sizeof(k));

	_check_sjis(buf, len, &k[0]);
	_check_utf8(buf, len, &k[1]);
	_check_euc(buf, len, &k[2]);

	for(i = 0; i < 3; i++) {
		if(!k[i].flg) continue;
		
		point = calc_code_point(&k[i]);
		if(point > 0 && point > max_point) {
			max_point = point;
			ret_v = k[i].kanji_code;
		}
	}

	return ret_v;
}

static void kanji_code_check_no_utf16(const BUF_BYTE *buf, int len, int *no_utf16_flg)
{
	// ASCIIが連続する場合、UTF16を否定する
	for(; len > 0; len--, buf++) {
		BUF_BYTE c1 = *buf;
		if(c1 < 0x80 && c1 != '\0') {
			if(len > 1) {
				BUF_BYTE c2 = *(buf + 1);
				if(c2 < 0x80 && c2 != '\0') {
					*no_utf16_flg = 1;
					return;
				}
			}
		}
	}
}

static int kanji_code_jis_unicode_ascii(const BUF_BYTE *buf, int len, int *all_ascii, int no_utf16_flg)
{
	// 0x80以上の場合JISを否定する
	int jis_flg = 1;
	const BUF_BYTE *buf_start = buf;
	for(; len > 0; len--, buf++) {
		if(jis_flg && *buf == 0x1b && *(buf + 1) == '$') return KANJI_CODE_JIS;

		if(*buf < 0x80) {
			if(!no_utf16_flg && len > 1 && *(buf + 1) == '\0') {
				if((buf - buf_start) % 2 == 1) {
					return KANJI_CODE_UTF16BE_NO_SIGNATURE;
				}
				return KANJI_CODE_UTF16LE_NO_SIGNATURE;
			}
		} else {
			*all_ascii = 0;
			jis_flg = 0;
		}
	}
	return UnknownKanjiCode;
}

int check_kanji_code(const BUF_BYTE *buf, int len, int *all_ascii)
{
	int		ret_v;
	int		no_utf16_flg = 0;

	kanji_code_check_no_utf16(buf, len, &no_utf16_flg);

	ret_v = kanji_code_jis_unicode_ascii(buf, len, all_ascii, no_utf16_flg);
	if(ret_v != UnknownKanjiCode) return ret_v;
	if(*all_ascii) return UnknownKanjiCode;

	ret_v = kanji_code_other(buf, len);
	if(ret_v != UnknownKanjiCode) return ret_v;

	return UnknownKanjiCode;
}

