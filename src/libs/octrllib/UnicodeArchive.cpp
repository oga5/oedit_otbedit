/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"

#include "UnicodeArchive.h"
#include "get_char.h"
#include <locale.h>

static void be_to_le(unsigned char *buf, int len)
{
	int		i;

	for(i = 0; i < len; i = i + 2) {
		char t = buf[i];
		buf[i] = buf[i + 1];
		buf[i + 1] = t;
	}
}

static int is_utf16(int kanji_code)
{
	if(kanji_code == KANJI_CODE_UTF16LE || kanji_code == KANJI_CODE_UTF16LE_NO_SIGNATURE ||
		kanji_code == KANJI_CODE_UTF16BE || kanji_code == KANJI_CODE_UTF16BE_NO_SIGNATURE) {
		return 1;
	}

	return 0;
}

static int is_utf16_be(int kanji_code)
{
	if(kanji_code == KANJI_CODE_UTF16BE || kanji_code == KANJI_CODE_UTF16BE_NO_SIGNATURE) {
		return 1;
	}

	return 0;
}

static int check_line_type_A(const BUF_BYTE *p, int len)
{
	for(; len > 0; p++, len--) {
		if(*p == '\r' || *p == '\n') break;
	}

	if(*p == '\r') {
		if(*(p + 1) == '\n') {
			return LINE_TYPE_CR_LF;
		} else {
			return LINE_TYPE_CR;
		}
	} else if(*p == '\n') {
		return LINE_TYPE_LF;
	}

	return LINE_TYPE_UNKNOWN;
}

static int check_line_type_W(const TCHAR *p, int len)
{
	for(; len > 0; p++, len--) {
		if(*p == '\r' || *p == '\n') break;
	}

	if(*p == '\r') {
		if(*(p + 1) == '\n') {
			return LINE_TYPE_CR_LF;
		} else {
			return LINE_TYPE_CR;
		}
	} else if(*p == '\n') {
		return LINE_TYPE_LF;
	}

	return LINE_TYPE_UNKNOWN;
}

int check_line_type(const TCHAR *p)
{
	return check_line_type_W(p, (int)_tcslen(p));
}

static BUF_BYTE *sjistounicode(BUF_BYTE *src, wchar_t *dst, int dst_size, BUF_BYTE *back_flg)
{
	int		mbsize;
	
	if(back_flg) *back_flg = '\0';

	dst_size -= 2 - dst_size % 2; // 偶数にする
	dst_size -= 2;	// L'\0'の分を確保

	wchar_t *dst_start = dst;
	wchar_t *dst_end = dst_start + dst_size;

	for(;;) {
		ASSERT(dst != dst_end);

		if(*src == '\0') break;

		if(*src < 0x80) {
			*dst = *src;
			src++;
			dst++;
		} else {
			// 「81～9F」 と「E0～EF」をチェック
			if(*(src + 1) == '\0') {
				if((*src >= 0x81 && *src <= 0x9F) || (*src >= 0xE0 && *src <= 0xFC)) {
					if(back_flg) *back_flg = *src;
					break;
				}
			}

			mbsize = mbtowc(dst, (char *)src, MB_CUR_MAX);
			if(mbsize == -1) {
				src++;
				continue;
			}
			src += mbsize;
			dst++;
		}
	}

	if(dst_start != dst && *(src - 1) == '\r' && back_flg) {
		*(dst - 1) = L'\0';
		*back_flg = '\r';
		return NULL;
	}

	*dst = L'\0';
	return NULL;
}

#define CHECK_KANJI_CODE_BUF_SIZE	(1024 * 64 + 1)

int CUnicodeArchive::CheckKanjiCode(CArchive *ar, int default_kanji_code)
{
	int		ret_v = default_kanji_code;
	int		len;
	int		all_ascii = 1;

	BUF_BYTE	*buf;
	int buf_size = CHECK_KANJI_CODE_BUF_SIZE;

	buf = (BUF_BYTE *)malloc(buf_size * sizeof(BUF_BYTE));
	if(buf == NULL) return default_kanji_code;

	CFile	*file = ar->GetFile();

	// NOTE: バッファの切れ目でマルチバイト文字が分割された場合
	// 正しく検出できないので、大きなバッファに一度に読む
	len = file->Read(buf, buf_size - 1);
	if(len == 0) goto END;
	buf[len] = '\0';

	if(check_utf16le_signature(buf) == 1) {
		ret_v = KANJI_CODE_UTF16LE;
		goto END;
	}
	if(check_utf16be_signature(buf) == 1) {
		ret_v = KANJI_CODE_UTF16BE;
		goto END;
	}
	if(check_utf8_signature(buf) == 1) {
		ret_v = KANJI_CODE_UTF8;
		goto END;
	}

	ret_v = check_kanji_code(buf, len, &all_ascii);
	if(ret_v != UnknownKanjiCode) goto END;

	if(all_ascii == 1) {
		if(is_utf16(default_kanji_code)) {
			ret_v = KANJI_CODE_SJIS;
			goto END;
		} else {
			ret_v = default_kanji_code;
			goto END;
		}
	}

	ret_v = KANJI_CODE_EUC;

END:
	file->SeekToBegin();
	if(buf != NULL) free(buf);
	return ret_v;
}

int CUnicodeArchive::CheckLineTypeA()
{
	BUF_BYTE	buf[UNICODE_ARCHIVE_BUF_SIZE];
	BUF_BYTE	back_buf = '\0';
	CFile	*file = m_ar->GetFile();
	int		line_type = LINE_TYPE_UNKNOWN;

	for(;;) {
		int len;
		if(back_buf) {
			buf[0] = back_buf;
			back_buf = '\0';
			len = file->Read(buf + 1, sizeof(buf) - (sizeof(BUF_BYTE) * 2)) + sizeof(BUF_BYTE);
		} else {
			len = file->Read(buf, sizeof(buf) - sizeof(BUF_BYTE));
		}

		if(len > 1 && buf[len - 1] == '\r') {
			back_buf = '\r';
			len -= 1;
		}
		buf[len] = '\0';

		if(len == 0) break;

		line_type = check_line_type_A(buf, len);
		if(line_type != LINE_TYPE_UNKNOWN) break;
	}
	file->SeekToBegin();
	return line_type;
}

int CUnicodeArchive::CheckLineTypeW()
{
	TCHAR	buf[UNICODE_ARCHIVE_BUF_SIZE];
	TCHAR	back_buf = '\0';
	CFile	*file = m_ar->GetFile();
	int		line_type = LINE_TYPE_UNKNOWN;

	for(;;) {
		int read_len;
		int str_len;
		if(back_buf) {
			buf[0] = back_buf;
			back_buf = '\0';
			read_len = file->Read(buf + 1, sizeof(buf) - (sizeof(TCHAR) * 2)) + sizeof(TCHAR);
		} else {
			read_len = file->Read(buf, sizeof(buf) - sizeof(TCHAR));
		}
		if(is_utf16_be(m_mode)) {
			be_to_le((unsigned char *)buf, read_len);
		}
		str_len = read_len / sizeof(TCHAR);

		if(str_len > 1 && buf[str_len - 1] == '\r') {
			back_buf = '\r';
			str_len -= 1;
		}
		buf[str_len] = '\0';

		if(str_len == 0) break;

		line_type = check_line_type_W(buf, str_len);
		if(line_type != LINE_TYPE_UNKNOWN) break;
	}
	file->SeekToBegin();
	return line_type;
}

int CUnicodeArchive::CheckLineType()
{
	// Unicode用とMBCS用で関数を分ける
	if(is_utf16(m_mode)) {
		m_line_type = CheckLineTypeW();
	} else {
		m_line_type = CheckLineTypeA();
	}

	if(m_line_type == LINE_TYPE_UNKNOWN) {
		switch(m_mode) {
		case KANJI_CODE_JIS:
		case KANJI_CODE_EUC:
			m_line_type = LINE_TYPE_LF;
			break;
		case KANJI_CODE_SJIS:
			m_line_type = LINE_TYPE_CR_LF;
			break;
		default:
			m_line_type = LINE_TYPE_CR_LF;
		}
	}

	return m_line_type;
}

BUF_BYTE *euctosjis(BUF_BYTE *src, BUF_BYTE *dst, int dst_size, BUF_BYTE *back_flg)
{
	int		dst_cnt = 0;

	dst_size -= 2;
	if(back_flg) *back_flg = '\0';

	for(; *src != '\0'; ) {
		if(dst_cnt >= dst_size) {
			*dst = '\0';
			return src;
		}
		if(*src < 0x80) {
			*dst = *src;
			src++;
			dst++; dst_cnt++;
			continue;
		}

		if(*src == 0x8e) {
			src++;
			if(*src == '\0') {
				*dst = '\0';
				if(back_flg) *back_flg = *(src - 1);
				return NULL;
			}
			*dst = *src;
			src++;
			dst++; dst_cnt++;
			continue;
		}

		if(*src < 0xdf) {
			*dst = ((*src) + 1) / 2 + 0x30;
		} else {
			*dst = ((*src) + 1) / 2 + 0x70;
		}
		src++;
		if(*src == '\0') {
			*dst = '\0';
			if(back_flg) *back_flg = *(src - 1);
			return NULL;
		}
		dst++; dst_cnt++;

		if(*(src - 1) % 2 == 0) {
			*dst = *src - 0x02;
		} else {
			*dst = *src - 0x61;
			if(*dst > 0x7e) (*dst)++;
		}
		src++;
		dst++; dst_cnt++;
	}
	if(*(src - 1) == '\r' && back_flg) {
		*(dst - 1) = '\0';
		*back_flg = '\r';
		return NULL;
	}

	*dst = '\0';
	return NULL;
}

BUF_BYTE *sjistoeuc(BUF_BYTE *src, BUF_BYTE *dst, int dst_size, BUF_BYTE *back_flg)
{
	int		dst_cnt = 0;

	dst_size -= 2;
	*back_flg = '\0';

	for(; *src != '\0'; ) {
		if(dst_cnt >= dst_size) {
			*dst = '\0';
			return src;
		}
		if(*src < 0x80) {
			*dst = *src;
			src++;
			dst++; dst_cnt++;
			continue;
		}

		if(*src >= 0xa0 && *src <= 0xdf) {
			*dst = 0x8e;
			dst++; dst_cnt++;
			*dst = *src;
			src++;
			dst++; dst_cnt++;
			continue;
		}

		if(*(src + 1) < 0x9f) {
			if(*src < 0xa0) {
				*dst = (*src - 0x81) * 2 + 0xa1;
			} else {
				*dst = (*src - 0xe0) * 2 + 0xdf;
			}
			src++;
			if(*src == '\0') {
				*dst = '\0';
				*back_flg = *(src - 1);
				return NULL;
			}
			dst++; dst_cnt++;
			if(*src > 0x7f) {
				*dst = *src + 0x60;
			} else {
				*dst = *src + 0x61;
			}
		} else {
			if(*src < 0xa0) {
				*dst = (*src - 0x81) * 2 + 0xa2;
			} else {
				*dst = (*src - 0xe0) * 2 + 0xe0;
			}
			src++;
			if(*src == '\0') {
				*dst = '\0';
				*back_flg = *(src - 1);
				return NULL;
			}
			dst++; dst_cnt++;
			*dst = *src + 0x02;
		}
		src++;
		dst++; dst_cnt++;
	}
	if(*(src - 1) == '\r' && dst_cnt > 1) {
		*(dst - 1) = '\0';
		*back_flg = '\r';
		return NULL;
	}

	*dst = '\0';
	return NULL;
}


#define JIS_MODE_1BYTE	1
#define JIS_MODE_2BYTE	2
#define JIS_MODE_KANA	3

BUF_BYTE *jistosjis(BUF_BYTE *src, BUF_BYTE *dst,
	int dst_size, BUF_BYTE *back_data, int *jis_mode, int *kana_shift)
{
	int		dst_cnt = 0;

	dst_size -= 2;
	if(back_data) back_data[0] = '\0';

	if(*jis_mode == 0) {
		*jis_mode = JIS_MODE_1BYTE;
		*kana_shift = 0;
	}

	for(; *src != '\0'; ) {
		if(dst_cnt >= dst_size) {
			*dst = '\0';
			return src;
		}
		if(*src == 0x0e) {
			*kana_shift = 1;
			src++;
			continue;
		}
		if(*src == 0x0f) {
			*kana_shift = 0;
			src++;
			continue;
		}
		if(*src == 0x1b) {
			if(*(src + 1) == '\0') {
				*dst = '\0';
				if(back_data) {
					back_data[0] = *src;
					back_data[1] = '\0';
				}
				return NULL;
			}
			if(*(src + 2) == '\0') {
				*dst = '\0';
				if(back_data) {
					back_data[0] = *src;
					back_data[1] = *(src + 1);
					back_data[2] = '\0';
				}
				return NULL;
			}
			src++;
			if(*src == '$') {
				src++;
				*jis_mode = JIS_MODE_2BYTE;
			} else if(*src == '(') {
				src++;
				if(*src == 'I') {
					*jis_mode = JIS_MODE_KANA;
				} else {
					*jis_mode = JIS_MODE_1BYTE;
				}
			}
			src++;
			continue;
		}
		if(*kana_shift == 1 || *jis_mode == JIS_MODE_KANA) {
			*dst = *src + 0x80;
			src++;
			dst++; dst_cnt++;
			continue;
		}
		if(*jis_mode == JIS_MODE_1BYTE) {
			*dst = *src;
			src++;
			dst++; dst_cnt++;
			continue;
		}

		if(*src < 0x5f) {
			*dst = (int)(*src + 1) / 2 + 0x70;
		} else {
			*dst = (int)(*src + 1) / 2 + 0xb0;
		}
		src++;
		if(*src == '\0') {
			*dst = '\0';
			if(back_data) {
				back_data[0] = *(src - 1);
				back_data[1] = '\0';
			}
			return NULL;
		}
		dst++; dst_cnt++;

		if(*(src - 1) % 2 == 0) {
			*dst = *src + 0x7d;
		} else {
			*dst = *src + 0x1f;
		}
		if(*dst > 0x7e) (*dst)++;

		src++;
		dst++; dst_cnt++;
	}
	if(*(src - 1) == '\r' && back_data) {
		*(dst - 1) = '\0';
		back_data[0] = '\r';
		back_data[1] = '\0';
		return NULL;
	}
	*dst = '\0';
	return NULL;
}

BUF_BYTE *sjistojis(BUF_BYTE *src, BUF_BYTE *dst,
	int dst_size, BUF_BYTE *back_flg, int *jis_mode, int *kana_shift)
{
	int		dst_cnt = 0;

	dst_size -= 7;
	*back_flg = '\0';

	if(*jis_mode == 0) {
		*jis_mode = JIS_MODE_1BYTE;
		*kana_shift = 0;
	}

	for(; *src != '\0'; ) {
		if(dst_cnt >= dst_size) {
			*dst = '\0';
			return src;
		}
		if(*src < 0x80) {
			if(*kana_shift == 1) {
				*dst = 0x0f;
				dst++; dst_cnt++;
				*jis_mode = JIS_MODE_1BYTE;
				*kana_shift = 0;
			} else if(*jis_mode != JIS_MODE_1BYTE) {
				*dst = 0x1b;
				dst++; dst_cnt++;
				*dst = 0x28;
				dst++; dst_cnt++;
				*dst = 0x4a;
				dst++; dst_cnt++;
				*jis_mode = JIS_MODE_1BYTE;
				*kana_shift = 0;
			}
			*dst = *src;
			src++;
			dst++; dst_cnt++;
			continue;
		}
		if(*src >= 0xa0 && *src <= 0xdf) {
			if(*kana_shift != 1) {
				if(*jis_mode == JIS_MODE_2BYTE) {
					*dst = 0x1b;
					dst++; dst_cnt++;
					*dst = 0x28;
					dst++; dst_cnt++;
					*dst = 0x4a;
					dst++; dst_cnt++;
					*jis_mode = JIS_MODE_1BYTE;
				}
				*dst = 0x0e;
				dst++; dst_cnt++;
				*kana_shift = 1;
			}
			*dst = *src - 0x80;
			src++;
			dst++; dst_cnt++;
			continue;
		}
		if(*kana_shift == 1) {
			*dst = 0x0f;
			dst++; dst_cnt++;
			*jis_mode = JIS_MODE_1BYTE;
			*kana_shift = 0;
		}
		if(*jis_mode != JIS_MODE_2BYTE) {
			*dst = 0x1b;
			dst++; dst_cnt++;
			*dst = 0x24;
			dst++; dst_cnt++;
			*dst = 0x42;
			dst++; dst_cnt++;
			*jis_mode = JIS_MODE_2BYTE;
			*kana_shift = 0;
		}
		if(*(src + 1) < 0x9f) {
			if(*src < 0xa0) {
				*dst = (*src - 0x81) * 2 + 0x21;
			} else {
				*dst = (*src - 0xe0) * 2 + 0x5f;
			}
			src++;
			if(*src == '\0') {
				*dst = '\0';
				*back_flg = *(src - 1);
				return NULL;
			}
			dst++; dst_cnt++;
			if(*src > 0x7f) {
				*dst = *src - 0x20;
			} else {
				*dst = *src - 0x1f;
			}
		} else {
			if(*src < 0xa0) {
				*dst = (*src - 0x81) * 2 + 0x22;
			} else {
				*dst = (*src - 0xe0) * 2 + 0x60;
			}
			src++;
			if(*src == '\0') {
				*dst = '\0';
				*back_flg = *(src - 1);
				return NULL;
			}
			dst++; dst_cnt++;
			*dst = *src - 0x7e;
		}
		src++;
		dst++; dst_cnt++;
	}
	if(*(src - 1) == '\r' && dst_cnt > 1) {
		*(dst - 1) = '\0';
		*back_flg = '\r';
		return NULL;
	}

	*dst = '\0';
	return NULL;
}

void CUnicodeArchive::Init(CArchive *ar, int mode, int line_type)
{
	m_ar = ar;
	m_mode = mode;
	m_back_data = '\0';
	m_buf_p = NULL;
	m_line_p = NULL;
	m_back_buf[0] = '\0';
	m_jis_mode = 0;
	m_kana_shift = 0;
	m_line_type = line_type;

	m_init = FALSE;

	setlocale(LC_ALL, "Japanese");

	switch(line_type) {
	case LINE_TYPE_CR_LF:
		_tcscpy(m_line_sep, _T("\r\n"));
		break;
	case LINE_TYPE_LF:
		_tcscpy(m_line_sep, _T("\n"));
		break;
	case LINE_TYPE_CR:
		_tcscpy(m_line_sep, _T("\r"));
		break;
	default:
		_tcscpy(m_line_sep, _T("\r\n"));
		break;
	}
}

void CUnicodeArchive::WriteNextLine()
{
	this->WriteString(m_line_sep);
}

BOOL CUnicodeArchive::ReOpenFile()
{
	CloseAll();
	return OpenFile(m_file_name, m_open_mode);
}

BOOL CUnicodeArchive::OpenFile(const TCHAR *file_name, const TCHAR *open_mode,
	int mode, int line_type)
{
	m_file = new CFile();

	if(*open_mode == L'r') {
		if(!m_file->Open(file_name, CFile::modeRead|CFile::shareDenyNone)) {
			delete m_file;
			m_file = NULL;
			return FALSE;
		}
		m_file_ar = new CArchive(m_file, CArchive::load);
		if(mode == UnknownKanjiCode) {
			mode = CUnicodeArchive::CheckKanjiCode(m_file_ar);
		}
	} else if(*open_mode == L'w') {
		if(!m_file->Open(file_name, CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive)) {
			delete m_file;
			m_file = NULL;
			return FALSE;
		}
		m_file_ar = new CArchive(m_file, CArchive::store | CArchive::bNoFlushOnDelete);
	}

	Init(m_file_ar, mode, line_type);

	m_file_name = file_name;
	m_open_mode = open_mode;

	return TRUE;
}

CUnicodeArchive::CUnicodeArchive()
{
	m_ar = NULL;
	m_file = NULL;
	m_file_ar = NULL;
	Init(NULL, 0, LINE_TYPE_UNKNOWN);
}

CUnicodeArchive::CUnicodeArchive(CArchive *ar, int mode, int line_type)
{
	m_ar = ar;
	m_file = NULL;
	m_file_ar = NULL;
	Init(ar, mode, line_type);
}

CUnicodeArchive::~CUnicodeArchive()
{
	CloseAll();
}

void CUnicodeArchive::CloseAll()
{
	if(m_file_ar) {
		m_file_ar->Close();
		delete m_file_ar;
		m_file_ar = NULL;
	}
	if(m_file) {
		m_file->Close();
		delete m_file;
		m_file = NULL;
	}
}

CString CUnicodeArchive::MakeFileType(int kanji_code, int line_type)
{
	CString file_type;

	switch(kanji_code) {
	case KANJI_CODE_SJIS:
		file_type = "SJIS";
		break;
	case KANJI_CODE_EUC:
		file_type = "EUC";
		break;
	case KANJI_CODE_JIS:
		file_type = "JIS";
		break;
	case KANJI_CODE_UTF16LE:
		file_type = "UNICODE";
		break;
	case KANJI_CODE_UTF16LE_NO_SIGNATURE:
		file_type = "UNICODE(no sig.)";
		break;
	case KANJI_CODE_UTF16BE:
		file_type = "UTF16BE";
		break;
	case KANJI_CODE_UTF16BE_NO_SIGNATURE:
		file_type = "UTF16BE(no sig.)";
		break;
	case KANJI_CODE_UTF8:
		file_type = "UTF8";
		break;
	case KANJI_CODE_UTF8_NO_SIGNATURE:
		file_type = "UTF8(no sig.)";
		break;
	}

	switch(line_type) {
	case LINE_TYPE_CR_LF:
		file_type += ", CRLF(\\r\\n)";
		break;
	case LINE_TYPE_LF:
		file_type += ", LF(\\n)";
		break;
	case LINE_TYPE_CR:
		file_type += ", CR(\\r)";
		break;
	}

	return file_type;
}

TCHAR *CUnicodeArchive::Read()
{
	TCHAR *ret = NULL;

	switch(m_mode) {
	case KANJI_CODE_SJIS:
		ret = SJISRead();
		break;
	case KANJI_CODE_EUC:
		ret = EUCRead();
		break;
	case KANJI_CODE_JIS:
		ret = JISRead();
		break;
	case KANJI_CODE_UTF16LE:
	case KANJI_CODE_UTF16LE_NO_SIGNATURE:
		ret = UNICODERead();
		break;
	case KANJI_CODE_UTF16BE:
	case KANJI_CODE_UTF16BE_NO_SIGNATURE:
		ret = UTF16BERead();
		break;
	case KANJI_CODE_UTF8:
	case KANJI_CODE_UTF8_NO_SIGNATURE:
		ret = UTF8Read();
		break;
	}

	return ret;
}

TCHAR *CUnicodeArchive::SJISRead()
{
	BUF_BYTE *back_data = &m_back_data;

	if(m_buf_p == NULL) {
		int		len;

		if(m_back_data == '\0') {
			len = m_ar->Read(m_buf, sizeof(m_buf) - 1);
			if(len == 0) return NULL;
		} else {
			m_buf[0] = m_back_data;
			len = m_ar->Read(m_buf + 1, sizeof(m_buf) - 2) + 1;
			if(len == 1) {
				m_back_data = '\0';
				back_data = NULL;
			}
		}
		m_buf[len] = '\0';

		m_buf_p = m_buf;
	}

	m_buf_p = sjistounicode(m_buf, (TCHAR *)m_conv_buf, sizeof(m_conv_buf), back_data);

	return (TCHAR *)m_conv_buf;
}

TCHAR *CUnicodeArchive::EUCRead()
{
	BUF_BYTE *back_data = &m_back_data;

	if(m_buf_p == NULL) {
		int		len;

		if(m_back_data == '\0') {
			len = m_ar->Read(m_buf, sizeof(m_buf) - 1);
			if(len == 0) return NULL;
		} else {
			m_buf[0] = m_back_data;
			len = m_ar->Read(m_buf + 1, sizeof(m_buf) - 2) + 1;
			if(len == 1) {
				m_back_data = '\0';
				back_data = NULL;
			}
		}
		m_buf[len] = '\0';

		m_buf_p = m_buf;
	}

	m_buf_p = euctosjis(m_buf_p, m_conv_buf, sizeof(m_conv_buf), back_data);

	sjistounicode(m_conv_buf, (TCHAR *)m_conv_buf2, sizeof(m_conv_buf2), NULL);

	return (TCHAR *)m_conv_buf2;
}

TCHAR *CUnicodeArchive::JISRead()
{
	unsigned char *back_buf = m_back_buf;

	if(m_buf_p == NULL) {
		int len;

		if(m_back_buf[0] == '\0') {
			len = m_ar->Read(m_buf, sizeof(m_buf) - 1);
			if(len == 0) return NULL;
		} else {
			int back_data_len = (int)strlen((char *)m_back_buf);
			strcpy((char *)m_buf, (char *)m_back_buf);
			m_back_buf[0] = '\0';
			len = m_ar->Read((char *)m_buf + back_data_len,
				sizeof(m_buf) - back_data_len - 1) + back_data_len;
			if(len == back_data_len) back_buf = NULL;
		}
		m_buf[len] = '\0';

		m_buf_p = m_buf;
    }
	
	m_buf_p = jistosjis(m_buf_p, m_conv_buf, sizeof(m_conv_buf), back_buf,
		&m_jis_mode, &m_kana_shift);

	sjistounicode(m_conv_buf, (TCHAR *)m_conv_buf2, sizeof(m_conv_buf2), NULL);

	return (TCHAR *)m_conv_buf2;
}

void CUnicodeArchive::WriteString(const TCHAR *buf)
{
	switch(m_mode) {
	case KANJI_CODE_SJIS:
		SJISWriteString(buf);
		break;
	case KANJI_CODE_EUC:
		EUCWriteString(buf);
		break;
	case KANJI_CODE_JIS:
		JISWriteString(buf);
		break;
	case KANJI_CODE_UTF16LE:
	case KANJI_CODE_UTF16LE_NO_SIGNATURE:
		UNICODEWriteString(buf);
		break;
	case KANJI_CODE_UTF16BE:
	case KANJI_CODE_UTF16BE_NO_SIGNATURE:
		UTF16BEWriteString(buf);
		break;
	case KANJI_CODE_UTF8:
	case KANJI_CODE_UTF8_NO_SIGNATURE:
		UTF8WriteString(buf);
		break;
	}
}

static const wchar_t *unicodetosjis(const wchar_t *src, BUF_BYTE *dst, int dst_size, int *len)
{
	int		mbsize;
	*len = 0;

	for(;;) {
		if(*src == L'\0') break;
		if(dst_size < 10) return src;

		mbsize = wctomb((char *)dst, *src);
		src += get_char_len(src);
		if(mbsize == -1) {
			*dst = '?';
			mbsize = 1;
		}

		dst += mbsize;
		dst_size -= mbsize;
		*len += mbsize;
	}

	return NULL;
}

void CUnicodeArchive::SJISWriteString(const TCHAR *buf)
{
	const TCHAR *p;
	int len;

	for(p = buf; p != NULL;) {
		p = unicodetosjis(p, m_conv_buf, sizeof(m_conv_buf), &len);
		m_ar->Write(m_conv_buf, len);
	}
}

void CUnicodeArchive::EUCWriteString(const TCHAR *buf)
{
	const TCHAR *p;
	BUF_BYTE *p_sjis;
	int len;

	for(p = buf; p != NULL;) {
		p = unicodetosjis(p, m_conv_buf, sizeof(m_conv_buf), &len);
		m_conv_buf[len] = '\0';
		for(p_sjis = m_conv_buf; p_sjis != NULL; ) {
			p_sjis = sjistoeuc(p_sjis, m_conv_buf2, sizeof(m_conv_buf2), &m_back_data);
			m_ar->Write(m_conv_buf2, (int)strlen((char *)m_conv_buf2));
		}
	}
}

void CUnicodeArchive::JISWriteString(const TCHAR *buf)
{
	const TCHAR *p;
	BUF_BYTE *p_sjis;
	int len;

	for(p = buf; p != NULL;) {
		p = unicodetosjis(p, m_conv_buf, sizeof(m_conv_buf), &len);
		m_conv_buf[len] = '\0';
		for(p_sjis = m_conv_buf; p_sjis != NULL; ) {
			p_sjis = sjistojis(p_sjis, m_conv_buf2, sizeof(m_conv_buf2), &m_back_data, &m_jis_mode, &m_kana_shift);
			m_ar->Write(m_conv_buf2, (int)strlen((char *)m_conv_buf2));
		}
	}
}

TCHAR *CUnicodeArchive::UNICODERead()
{
	int		len;
	CFile *file = m_ar->GetFile();

	if(m_init == FALSE) {
		// 0xff 0xfeを読み飛ばす
		len = file->Read((char *)m_buf, 2);
		if(len == 0) return NULL;

		if(check_utf16le_signature(m_buf) == 0) {
			// signature無し
			file->SeekToBegin();
		}

		m_back_buf[0] = '\0';
		m_back_buf[1] = '\0';
		m_init = TRUE;
	}

	int		read_buf_size = sizeof(m_buf) - 5;
	// 偶数にする
	read_buf_size -= 2 - read_buf_size % 2;

	if(m_back_buf[0] != '\0') {
		int l = 2;
		m_buf[0] = m_back_buf[0];
		m_buf[1] = m_back_buf[1];
		m_back_buf[0] = '\0';
		m_back_buf[1] = '\0';
		len = file->Read((char *)m_buf + l, read_buf_size - l) + l;
	} else {
		len = file->Read((char *)m_buf, read_buf_size);
	}
	if(len == 0) return NULL;
	m_buf[len] = '\0';
	m_buf[len + 1] = '\0';

	// バイナリファイルなどをUTF16に誤検出した場合、バッファを越えないようにNULL文字を増やす
	m_buf[len + 2] = '\0';
	m_buf[len + 3] = '\0';

	// '\r'と'\n'を切り離さない
	if(len > 2 && m_buf[len - 2] == '\r' && m_buf[len - 1] == '\0') {
		m_back_buf[0] = '\r';
		m_back_buf[1] = '\0';
		m_buf[len - 1] = '\0';
		m_buf[len - 2] = '\0';
	}

	// サロゲートペアを分離しない
	if(len > 2 && is_high_surrogate_ch(m_buf[len - 1])) {
		m_back_buf[0] = m_buf[len - 2];
		m_back_buf[1] = m_buf[len - 1];
		m_buf[len - 1] = '\0';
		m_buf[len - 2] = '\0';
	}

	return (TCHAR *)m_buf;
}

void CUnicodeArchive::UNICODEWriteString(const TCHAR *buf)
{
	if(m_init == FALSE) {
		// 0xff 0xfeを書き込む
		if(m_mode == KANJI_CODE_UTF16LE) {
			BUF_BYTE uni_head[2];
			uni_head[0] = 0xff;
			uni_head[1] = 0xfe;
			m_ar->Write(uni_head, 2);
		}

		m_init = TRUE;
	}

	m_ar->WriteString(buf);
}

TCHAR *CUnicodeArchive::UTF16BERead()
{
	int		len;
	CFile *file = m_ar->GetFile();

	if(m_init == FALSE) {
		// 0xfe 0xffを読み飛ばす
		len = file->Read((char *)m_buf, 2);
		if(len == 0) return NULL;

		if(check_utf16be_signature(m_buf) == 0) {
			// signature無し
			file->SeekToBegin();
		}

		m_back_buf[0] = '\0';
		m_back_buf[1] = '\0';
		m_init = TRUE;
	}

	int		read_buf_size = sizeof(m_buf) - 5;
	// 偶数にする
	read_buf_size -= 2 - read_buf_size % 2;

	if(m_back_buf[0] != '\0') {
		int l = 2;
		m_buf[0] = m_back_buf[0];
		m_buf[1] = m_back_buf[1];
		m_back_buf[0] = '\0';
		m_back_buf[1] = '\0';
		len = file->Read((char *)m_buf + l, read_buf_size - l) + l;
	} else {
		len = file->Read((char *)m_buf, read_buf_size);
	}
	if(len == 0) return NULL;
	be_to_le(m_buf, len);
	m_buf[len] = '\0';
	m_buf[len + 1] = '\0';

	// バイナリファイルなどをUTF16に誤検出した場合、バッファを越えないようにNULL文字を増やす
	m_buf[len + 2] = '\0';
	m_buf[len + 3] = '\0';

	// '\r'と'\n'を切り離さない
	if(len > 2 && m_buf[len - 2] == '\r' && m_buf[len - 1] == '\0') {
		m_back_buf[0] = '\r';
		m_back_buf[1] = '\0';
		m_buf[len - 1] = '\0';
		m_buf[len - 2] = '\0';
	}

	// サロゲートペアを分離しない
	if(len > 2 && is_high_surrogate_ch(m_buf[len - 1])) {
		m_back_buf[0] = m_buf[len - 2];
		m_back_buf[1] = m_buf[len - 1];
		m_buf[len - 1] = '\0';
		m_buf[len - 2] = '\0';
	}

	return (TCHAR *)m_buf;
}

void CUnicodeArchive::UTF16BEWriteString(const TCHAR *buf)
{
	if(m_init == FALSE) {
		// 0xff 0xfeを書き込む
		if(m_mode == KANJI_CODE_UTF16BE) {
			BUF_BYTE uni_head[2];
			uni_head[0] = 0xfe;
			uni_head[1] = 0xff;
			m_ar->Write(uni_head, 2);
		}

		m_init = TRUE;
	}

	_tcscpy((TCHAR *)m_conv_buf, buf);
	be_to_le(m_conv_buf, (int)_tcslen((TCHAR *)m_conv_buf) * sizeof(TCHAR));

	m_ar->WriteString((TCHAR *)m_conv_buf);
}

static unsigned int get_unicode_from_utf8(BUF_BYTE *src, int utf8_len)
{
	switch(utf8_len) {
	case 4:
		return ((src[0] & 0x07) << 18) |
			((src[1] & 0x3f) << 12) |
			((src[2] & 0x3f) << 6) |
			((src[3] & 0x3f));
	case 3:
		return ((((src[0] & 0x0f) << 4) | ((src[1] & 0x3c) >> 2)) << 8) |
			((src[1] & 0x03) << 6) | (src[2] & 0x3f);
	case 2:
		return  (((src[0] & 0x1c) >> 2) << 8) | ((src[0] & 0x03) << 6) | (src[1] & 0x3f);
	case 1:
		return src[0];
	}

	// error
	return 0xffff;
}

static BUF_BYTE *utf8tounicode(BUF_BYTE *src, wchar_t *dst, BUF_BYTE *back_buf)
{
	unsigned int ch = '\0';
	int		utf8_len;

	for(;;) {
		if(*src == '\0') break;

		utf8_len = get_utf8_len(src);
		if(utf8_len == 0) {
			int l = (int)strlen((char *)src);
			if(back_buf && l < 3) {
				strcpy((char *)back_buf, (char *)src);
				break;
			}

			// error(1byte読み飛ばす)
			dst = put_char(dst, '?');
			src++;
			continue;
		}

		ch = get_unicode_from_utf8(src, utf8_len);
	
		src += utf8_len;
		dst = put_char(dst, ch);
	}

	// '\r'と'\n'を切り離さない
	if(ch == '\r' && back_buf) {
		*(dst - 1) = '\0';
		strcpy((char *)back_buf, "\r");
	}

	*dst = '\0';
	return (BUF_BYTE *)dst;
}

TCHAR *CUnicodeArchive::UTF8Read()
{
	int		len;
	CFile *file = m_ar->GetFile();

	if(m_init == FALSE) {
		// UTF-8 signature(0xef 0xbb 0xbf)を読み飛ばす
		len = file->Read((char *)m_buf, 3);
		if(len == 0) return NULL;

		if(check_utf8_signature(m_buf) == 0) {
			// signature無し
			file->SeekToBegin();
		}

		m_back_buf[0] = '\0';
		m_init = TRUE;
	}

	unsigned char *back_buf = m_back_buf;

	if(m_back_buf[0] != '\0') {
		int l = (int)strlen((char *)m_back_buf);
		strcpy((char *)m_buf, (char *)m_back_buf);
		m_back_buf[0] = '\0';
		len = file->Read((char *)m_buf + l, sizeof(m_buf) - 1 - l) + l;
		if(len == l) back_buf = NULL;
	} else {
		len = file->Read((char *)m_buf, sizeof(m_buf) - 1);
	}
	if(len == 0) return NULL;
	m_buf[len] = '\0';

	utf8tounicode(m_buf, (wchar_t *)m_conv_buf, back_buf);

	return (TCHAR *)m_conv_buf;
}

static int get_utf8_len_from_unicode(unsigned int unicode_char)
{
	if(unicode_char < 0x0080) return 1;
	if(unicode_char < 0x0800) return 2;
	if(unicode_char < 0x10000) return 3;
	if(unicode_char < 0x200000) return 4;

	return 0;
}

static int put_utf8_from_unicode(unsigned int unicode_char, BUF_BYTE *dst)
{
	int utf8_len = get_utf8_len_from_unicode(unicode_char);

	switch(utf8_len) {
	case 4:
		dst[0] = ((unicode_char & 0x100000) >> 18) | ((unicode_char & 0xc0000) >> 18) | 0xf0;
		dst[1] = ((unicode_char & 0x30000) >> 12) | ((unicode_char & 0xf000) >> 12) | 0x80;
		dst[2] = ((unicode_char & 0x0f00) >> 6) | ((unicode_char & 0x00c0) >> 6) | 0x80;
		dst[3] = (unicode_char & 0x003f) | 0x80;
		break;
	case 3:
		dst[0] = ((unicode_char & 0xf000) >> 12) | 0xe0;
		dst[1] = ((unicode_char & 0x0f00) >> 6) | ((unicode_char & 0x00c0) >> 6) | 0x80;
		dst[2] = (unicode_char & 0x003f) | 0x80;
		break;
	case 2:
		dst[0] = ((unicode_char & 0x0700) >> 6) | ((unicode_char & 0x00c0) >> 6) | 0xc0;
		dst[1] = (unicode_char & 0x003f) | 0x80;
		break;
	case 1:
		dst[0] = (unicode_char & 0x00ff);
		break;
	}

	return utf8_len;
}

const TCHAR *unicodetoutf8(const TCHAR *src, BUF_BYTE *dst, int dst_size, int *len)
{
	int		utf8_len;
	unsigned int ch;
	BUF_BYTE *dst_start = dst;

	// utf8データが途中で切れないようにする
	dst_size -= 4;

	*len = 0;

	for(;;) {
		if(*src == '\0') break;
		if(*len >= dst_size) {
			*dst = '\0';
			return src;
		}

		ch = get_char(src);
		utf8_len = put_utf8_from_unicode(ch, dst);

		src += get_char_len(src);
		dst += utf8_len;
		*len += utf8_len;
	}

	*dst = '\0';
	return NULL;
}

void CUnicodeArchive::UTF8WriteString(const TCHAR *buf)
{
	if(m_init == FALSE) {
		if(m_mode == KANJI_CODE_UTF8) {
			// 0xef 0xbb 0xbfを書き込む
			BUF_BYTE uni_head[3];
			uni_head[0] = 0xef;
			uni_head[1] = 0xbb;
			uni_head[2] = 0xbf;
			m_ar->Write(uni_head, 3);
		}

		m_init = TRUE;
	}

	const TCHAR *p;
	int len;

	for(p = buf; p != NULL;) {
		p = unicodetoutf8(p, m_conv_buf, sizeof(m_conv_buf), &len);
		m_ar->Write(m_conv_buf, len);
	}
}

struct CUnicodeArchive::kanji_code_st CUnicodeArchive::m_kanji_code_list[] = {
	{KANJI_CODE_SJIS, _T("SJIS")},
	{KANJI_CODE_EUC, _T("EUC")},
	{KANJI_CODE_JIS, _T("JIS")},
	{KANJI_CODE_UTF16LE, _T("UNICODE")},
	{KANJI_CODE_UTF16LE_NO_SIGNATURE, _T("UNICODE(no signature)")},
	{KANJI_CODE_UTF16BE, _T("UTF16BE")},
	{KANJI_CODE_UTF16BE_NO_SIGNATURE, _T("UTF16BE(no signature)")},
	{KANJI_CODE_UTF8, _T("UTF8")},
	{KANJI_CODE_UTF8_NO_SIGNATURE, _T("UTF8(no signature)")},
};

struct CUnicodeArchive::line_type_st CUnicodeArchive::m_line_type_list[] = {
	{LINE_TYPE_CR_LF, _T("CR+LF(\\r\\n)")},
	{LINE_TYPE_LF, _T("LF(\\n)")},
	{LINE_TYPE_CR, _T("CR(\\r)")},
};

void CUnicodeArchive::SetKanjiCodeCombo(CComboBox *p_combo_kanji, int kanji_code, BOOL b_add_autodetect)
{
	if(p_combo_kanji == NULL) return;

	int base = 0;
	if(b_add_autodetect) {
		p_combo_kanji->InsertString(0, _T("自動判別"));
		p_combo_kanji->SetItemData(0, UnknownKanjiCode);
		base = 1;
	}

	int		i;
	for(i = 0; i < sizeof(m_kanji_code_list)/sizeof(m_kanji_code_list[0]); i++) {
		p_combo_kanji->InsertString(base + i, m_kanji_code_list[i].kanji_name);
		p_combo_kanji->SetItemData(base + i, m_kanji_code_list[i].kanji_code);
	}

	for(i = 0; p_combo_kanji->GetItemData(i) != CB_ERR; i++) {
		if(p_combo_kanji->GetItemData(i) == (UINT)kanji_code) {
			p_combo_kanji->SetCurSel(i);
			break;
		}
	}
}

void CUnicodeArchive::SetLineTypeCombo(CComboBox *p_combo_line, int line_type)
{
	if(p_combo_line == NULL) return;

	int		i;
	for(i = 0; i < sizeof(m_line_type_list)/sizeof(m_line_type_list[0]); i++) {
		p_combo_line->InsertString(i, m_line_type_list[i].line_type_name);
		p_combo_line->SetItemData(i, m_line_type_list[i].line_type);
	}

	for(i = 0; p_combo_line->GetItemData(i) != CB_ERR; i++) {
		if(p_combo_line->GetItemData(i) == (UINT)line_type) {
			p_combo_line->SetCurSel(i);
			break;
		}
	}
}

TCHAR *CUnicodeArchive::ReadLine(TCHAR *buf, int buf_size)
{
	TCHAR *p = buf;

	for(;;) {
		if(m_line_p == NULL || *m_line_p == L'\0') {
			m_line_p = Read();
			if(m_line_p == NULL || *m_line_p == L'\0') return NULL;
		}

		*p = *m_line_p;
		p++;
		m_line_p++;

		if(*(m_line_p - 1) == L'\r' && *m_line_p != L'\n') break;
		if(*(m_line_p - 1) == L'\n') break;
	}

	*p = L'\0';
	return buf;
}

void CUnicodeArchive::Flush()
{
	 m_ar->Flush();
}

void CUnicodeArchive::CsvOut(const TCHAR *string, int dquote_flg)
{
	ASSERT(string != NULL);

	if(dquote_flg) {
		WriteString(_T("\""));

		if(_tcsstr(string, _T("\"")) == NULL) {
			WriteString(string);
		} else {
			CString new_str = string;
			new_str.Replace(_T("\""), _T("\"\""));
			WriteString(new_str);
		}

		WriteString(_T("\""));
	} else {
		WriteString(string);
	}

	return;
}

ULONGLONG CUnicodeArchive::GetPosition()
{
	return m_ar->GetFile()->GetPosition();
}

int get_kanji_code_from_str(const TCHAR *str)
{
	int kanji_code = KANJI_CODE_SJIS;
	if(_tcscmp(str, _T("")) == 0 || _tcscmp(str, _T("SJIS")) == 0) {
		kanji_code = KANJI_CODE_SJIS;
	} else if(_tcscmp(str, _T("UNICODE")) == 0) {
		kanji_code = KANJI_CODE_UTF16LE;
	} else if(_tcscmp(str, _T("UTF16BE")) == 0) {
		kanji_code = KANJI_CODE_UTF16BE;
	} else if(_tcscmp(str, _T("UTF8")) == 0) {
		kanji_code = KANJI_CODE_UTF8;
	} else if(_tcscmp(str, _T("UTF8N")) == 0) {
		kanji_code = KANJI_CODE_UTF8_NO_SIGNATURE;
	} else {
		kanji_code = UnknownKanjiCode;
	}
	return kanji_code;
}

int get_line_type_from_str(const TCHAR *str)
{
	int line_type = LINE_TYPE_CR_LF;
	if(_tcscmp(str, _T("")) == 0 || _tcscmp(str, _T("CRLF")) == 0) {
		line_type = LINE_TYPE_CR_LF;
	} else if(_tcscmp(str, _T("CR")) == 0) {
		line_type = LINE_TYPE_CR;
	} else if(_tcscmp(str, _T("LF")) == 0) {
		line_type = LINE_TYPE_LF;
	} else {
		line_type = LINE_TYPE_UNKNOWN;
	}
	return line_type;
}


