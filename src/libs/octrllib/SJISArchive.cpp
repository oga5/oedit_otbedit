#include "stdafx.h"

#include "SjisArchive.h"
#include <locale.h>

static int check_unicode_signature(TCHAR *buf)
{
	if(buf[0] == 0xff && buf[1] == 0xfe) return 1;
	return 0;
}

static int check_utf8_signature(TCHAR *buf)
{
	if(buf[0] == 0xef && buf[1] == 0xbb && buf[2] == 0xbf) return 1;
	return 0;
}

static int get_utf8_len(TCHAR *src)
{
	if((src[0] & 0xf0) == 0xe0) {
		if((src[1] & 0xc0) == 0x80 && (src[2] & 0xc0) == 0x80) return 3;
	} else if((src[0] & 0xe0) == 0xc0) {
		if((src[1] & 0xc0) == 0x80) return 2;
	} else if((src[0] & 0x80) == 0x00) {
		return 1;
	}

	return 0;
}

static int is_utf8(TCHAR *buf, int *check_utf8)
{
	int utf8_len;
	int utf8_3_cnt = 0;

	for(;;) {
		if(*buf == '\0') break;

		utf8_len = get_utf8_len(buf);
		if(utf8_len == 0) {
			if(buf[1] == '\0' || buf[2] == '\0') break;
			*check_utf8 = 0;
			return 0;
		}
		if(utf8_len == 3) utf8_3_cnt++;
		buf += utf8_len;
	}

	if(utf8_3_cnt > 10) return 1;

	return 0;
}

static int kanji_code(TCHAR *buf, int len, int *ascii, int *check_utf8)
{
	for(; len > 0; len--, buf++) {
		if(*buf == 0x1b && *(buf + 1) == '$') return KANJI_CODE_JIS;
		if(*buf < 0x80) {
			if(len > 1 && *(buf + 1) == '\0') return KANJI_CODE_UNICODE_NO_SIGNATURE;
			continue;
		}

		*ascii = 0;
		if(*check_utf8 && get_utf8_len(buf) == 3) {
			if(is_utf8(buf, check_utf8) == 1) return KANJI_CODE_UTF8_NO_SIGNATURE;
		}

		if(*buf == 0x8e || *buf == 0x8f) continue;
		if(*buf >= 0x81 && *buf <= 0x9f) return KANJI_CODE_SJIS;
	}

	if(*ascii == 1) return KANJI_CODE_ASCII;

	return UnknownKanjiCode;
}

#define MAX_CHECK_KANJI_CODE_CNT	20
int CSJISArchive::CheckKanjiCode(CArchive *ar, int default_kanji_code)
{
	int		ret_v;
	int		len;
	int		ascii = 1;
	int		check_utf8 = 1;
	int		cnt;

	TCHAR	buf[SJIS_ARCHIVE_BUF_SIZE];
	CFile	*file = ar->GetFile();

	for(cnt = 0; cnt < MAX_CHECK_KANJI_CODE_CNT; cnt++) {
		len = file->Read(buf, sizeof(buf) - 1);
		if(len == 0) break;
		buf[len] = '\0';

		if(cnt == 0 && check_unicode_signature(buf) == 1) {
			file->SeekToBegin();
			return KANJI_CODE_UNICODE;
		}
		if(cnt == 0 && check_utf8_signature(buf) == 1) {
			file->SeekToBegin();
			return KANJI_CODE_UTF8;
		}

		ret_v = kanji_code(buf, len, &ascii, &check_utf8);
		if(ret_v != UnknownKanjiCode && ret_v != KANJI_CODE_ASCII) {
			file->SeekToBegin();
			return ret_v;
		}
	}
	file->SeekToBegin();

	if(ascii == 1) return default_kanji_code;

	return KANJI_CODE_EUC;
}

int check_line_type(const char *p, int len)
{
	// UNICODEに対応するため，*p != '\0'はチェックしない
	for(; len > 0; p++, len--) {
		if(*p == '\r' || *p == '\n') break;
	}

	if(*p == '\r') {
		if(len > 2 && *(p + 1) == '\0' && *(p + 2) == '\n') {
			return LINE_TYPE_CR_LF;
		} else if(*(p + 1) == '\n') {
			return LINE_TYPE_CR_LF;
		} else {
			return LINE_TYPE_CR;
		}
	} else if(*p == '\n') {
		return LINE_TYPE_LF;
	}

	return -1;
}

int check_line_type(const char *buf)
{
	return check_line_type(buf, strlen(buf));
}

int CSJISArchive::CheckLineType(CArchive *ar, int kanji_code)
{
	int		len;
	TCHAR	buf[SJIS_ARCHIVE_BUF_SIZE];
	CFile	*file = ar->GetFile();
	int		line_type;

	for(;;) {
		len = file->Read(buf, sizeof(buf) - 1);
		if(len == 0) break;
		buf[len] = '\0';

		line_type = check_line_type((const char *)buf, len);
		if(line_type != -1) {
			file->SeekToBegin();
			return line_type;
		}
	}
	file->SeekToBegin();

	switch(kanji_code) {
	case KANJI_CODE_JIS:
	case KANJI_CODE_EUC:
		return LINE_TYPE_LF;
		break;
	case KANJI_CODE_SJIS:
		return LINE_TYPE_CR_LF;
		break;
	}

	return LINE_TYPE_CR_LF;
}

TCHAR *euctosjis(TCHAR *src, TCHAR *dst,
	int dst_size, TCHAR *back_flg)
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

		if(*src == 0x8e) {
			src++;
			if(*src == '\0') {
				*dst = '\0';
				*back_flg = *(src - 1);
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
			*back_flg = *(src - 1);
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
	if(*(src - 1) == '\r') {
		*(dst - 1) = '\0';
		*back_flg = '\r';
		return NULL;
	}

	*dst = '\0';
	return NULL;
}

TCHAR *sjistoeuc(TCHAR *src, TCHAR *dst,
	int dst_size, TCHAR *back_flg)
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

TCHAR *jistosjis(TCHAR *src, TCHAR *dst,
	int dst_size, TCHAR *back_data, int *jis_mode, int *kana_shift)
{
	int		dst_cnt = 0;

	dst_size -= 2;
	back_data[0] = '\0';

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
				back_data[0] = *src;
				back_data[1] = '\0';
				return NULL;
			}
			if(*(src + 2) == '\0') {
				*dst = '\0';
				back_data[0] = *src;
				back_data[1] = *(src + 1);
				back_data[2] = '\0';
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
			back_data[0] = *(src - 1);
			back_data[1] = '\0';
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
	if(*(src - 1) == '\r') {
		*(dst - 1) = '\0';
		back_data[0] = '\r';
		back_data[1] = '\0';
		return NULL;
	}
	*dst = '\0';
	return NULL;
}

TCHAR *sjistojis(TCHAR *src, TCHAR *dst,
	int dst_size, TCHAR *back_flg, int *jis_mode, int *kana_shift)
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


CSJISArchive::CSJISArchive(CArchive *ar, int mode)
{
	m_ar = ar;
	m_mode = mode;
	m_back_data = '\0';
	m_buf_p = NULL;
	strcpy((char *)m_back_buf, "");
	m_jis_mode = 0;
	m_kana_shift = 0;

	m_init = FALSE;

	setlocale(LC_ALL, "Japanese");
}

CSJISArchive::~CSJISArchive()
{
}

CString CSJISArchive::MakeFileType(int kanji_code, int line_type)
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
	case KANJI_CODE_UNICODE:
		file_type = "UNICODE";
		break;
	case KANJI_CODE_UNICODE_NO_SIGNATURE:
		file_type = "UNICODE(no sig.)";
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

TCHAR *CSJISArchive::Read()
{
	TCHAR *ret;

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
	case KANJI_CODE_UNICODE:
	case KANJI_CODE_UNICODE_NO_SIGNATURE:
		ret = UNICODERead();
		break;
	case KANJI_CODE_UTF8:
	case KANJI_CODE_UTF8_NO_SIGNATURE:
		ret = UTF8Read();
		break;
	}

	if(ret != NULL && ret[0] == '\0') {
		switch(m_mode) {
		case KANJI_CODE_SJIS:
		case KANJI_CODE_EUC:
		case KANJI_CODE_UNICODE:
		case KANJI_CODE_UNICODE_NO_SIGNATURE:
		case KANJI_CODE_UTF8:
		case KANJI_CODE_UTF8_NO_SIGNATURE:
			ret[0] = m_back_data;
			ret[1] = '\0';
			m_back_data = '\0';
			break;
		case KANJI_CODE_JIS:
			strcat((char *)ret, (char *)m_back_buf);
			m_back_buf[0] = '\0';
			break;
		}
	}

	return ret;
}

TCHAR *CSJISArchive::SJISRead()
{
	int		len, read_len;
	read_len = sizeof(m_buf) - 1;

	len = m_ar->GetFile()->Read((char *)m_buf, read_len);
	if(len == 0) return NULL;
	m_buf[len] = '\0';

	if(len == read_len && m_buf[len - 1] == '\r') {
		m_buf[len - 1] = '\0';
		m_ar->GetFile()->Seek(-1, CFile::current);
	}

	return m_buf;
}

TCHAR *CSJISArchive::EUCRead()
{
	int		len;

	if(m_buf_p == NULL) {
		if(m_back_data == '\0') {
			len = m_ar->Read((char *)m_buf, sizeof(m_buf) - 1);
			if(len == 0) return NULL;
		} else {
			m_buf[0] = m_back_data;
			len = m_ar->Read(m_buf + 1, sizeof(m_buf) - 2) + 1;
		}
		m_buf[len] = '\0';

		m_buf_p = m_buf;
	}

	m_buf_p = euctosjis(m_buf_p, m_conv_buf, sizeof(m_conv_buf), &m_back_data);

	return m_conv_buf;
}

TCHAR *CSJISArchive::JISRead()
{
	int		len;
	int		back_data_len;

	if(m_buf_p == NULL) {
		if(m_back_buf[0] == '\0') {
			len = m_ar->Read((char *)m_buf, sizeof(m_buf) - 1);
			if(len == 0) return NULL;
		} else {
			back_data_len = strlen((char *)m_back_buf);
			strcpy((char *)m_buf, (char *)m_back_buf);
			len = m_ar->Read((char *)m_buf + back_data_len,
				sizeof(m_buf) - back_data_len - 1);
			len = len + back_data_len;
		}
		m_buf[len] = '\0';

		m_buf_p = m_buf;
    }
	
	m_buf_p = jistosjis(m_buf_p, m_conv_buf, sizeof(m_conv_buf), m_back_buf,
		&m_jis_mode, &m_kana_shift);

    return m_conv_buf;
}

void CSJISArchive::WriteString(TCHAR *buf)
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
	case KANJI_CODE_UNICODE:
	case KANJI_CODE_UNICODE_NO_SIGNATURE:
		UNICODEWriteString(buf);
		break;
	case KANJI_CODE_UTF8:
	case KANJI_CODE_UTF8_NO_SIGNATURE:
		UTF8WriteString(buf);
		break;
	}
}

void CSJISArchive::SJISWriteString(TCHAR *buf)
{
	m_ar->WriteString(buf);
}

void CSJISArchive::EUCWriteString(TCHAR *buf)
{
	TCHAR *p;

	for(p = buf; p != NULL;) {
		p = sjistoeuc(p, m_conv_buf, sizeof(m_conv_buf), &m_back_data);
		m_ar->WriteString(m_conv_buf);
	}
}

void CSJISArchive::JISWriteString(TCHAR *buf)
{
	TCHAR *p;

	for(p = buf; p != NULL;) {
		p = sjistojis(p, m_conv_buf, sizeof(m_conv_buf), &m_back_data, &m_jis_mode, &m_kana_shift);
		m_ar->WriteString(m_conv_buf);
	}
}

TCHAR *unicodetosjis(wchar_t *src, TCHAR *dst)
{
	int		mbsize;	

	for(;;) {
		if(*src == L'\0') break;

		mbsize = wctomb((char *)dst, *src);
		src++;
		if(mbsize == -1) continue;

		dst += mbsize;
	}

	*dst = '\0';
	return dst;
}

TCHAR *CSJISArchive::UNICODERead()
{
	int		len;

	if(m_init == FALSE) {
		// 0xff 0xfeを読み飛ばす
		len = m_ar->GetFile()->Read((char *)m_buf, 2);
		if(len == 0) return NULL;

		if(check_unicode_signature(m_buf) == 0) {
			// signature無し
			m_ar->GetFile()->SeekToBegin();
		}

		m_init = TRUE;
	}

	int		read_len = sizeof(m_buf) - 3;
	// 偶数にする
	read_len -= 2 - read_len % 2;

	len = m_ar->GetFile()->Read((char *)m_buf, read_len);
	if(len == 0) return NULL;
	m_buf[len] = '\0';
	m_buf[len + 1] = '\0';

	m_buf_p = unicodetosjis((wchar_t *)m_buf, m_conv_buf);

	// '\r'と'\n'を切り離さない
	if(len == read_len && *(m_buf_p - 1) == '\r') {
		*(m_buf_p - 1) = '\0';
		m_ar->GetFile()->Seek(-2, CFile::current);
	}

	return m_conv_buf;
}

TCHAR *sjistounicode(TCHAR *src, wchar_t *dst, int dst_size, int *dst_byte)
{
	int		mbsize;
	
	*dst_byte = 0;

	dst_size -= 2 - dst_size % 2; // 偶数にする
	dst_size -= 2;	// L'\0'の分を確保

	for(;;) {
		if(*src == '\0') break;
		if(*dst_byte >= dst_size) {
			*dst = L'\0';
			return src;
		}

		mbsize = mbtowc(dst, (char *)src, MB_CUR_MAX);
		if(mbsize == -1) {
			src++;
			continue;
		}

		src += mbsize;
		dst++;
		(*dst_byte) += 2;
	}

	*dst = L'\0';
	return NULL;
}

void CSJISArchive::UNICODEWriteString(TCHAR *buf)
{
	TCHAR *p;
	int			dst_byte;

	if(m_init == FALSE) {
		// 0xff 0xfeを書き込む
		if(m_mode == KANJI_CODE_UNICODE) {
			TCHAR uni_head[2];
			uni_head[0] = 0xff;
			uni_head[1] = 0xfe;
			m_ar->Write(uni_head, 2);
		}

		m_init = TRUE;
	}

	for(p = buf; p != NULL;) {
		p = sjistounicode(p, (wchar_t *)m_conv_buf, sizeof(m_conv_buf), &dst_byte);
		m_ar->Write(m_conv_buf, dst_byte);
	}
}

static unsigned short get_unicode_from_utf8(TCHAR *src, int utf8_len)
{
	switch(utf8_len) {
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

TCHAR *utf8tosjis(TCHAR *src, TCHAR *dst, int *back_cnt)
{
	int		mbsize;
	unsigned short unicode_char = 0;
	int		utf8_len;

	*back_cnt = 0;

	for(;;) {
		if(*src == '\0') break;

		utf8_len = get_utf8_len(src);
		if(utf8_len == 0) {
			*back_cnt = _tcslen(src);
			if(*back_cnt < 3) break;

			// error(1byte読み飛ばす)
			*back_cnt = 0;
			memcpy(dst, src, 1);
			src++;
			dst++;
			continue;
		}

		unicode_char = get_unicode_from_utf8(src, utf8_len);
	
		mbsize = wctomb((char *)dst, unicode_char);
		if(mbsize == -1) {
			TRACE(_T("wctomb error: %d:%d\n"), utf8_len, unicode_char);
			memcpy(dst, src, utf8_len);
			src += utf8_len;
			dst += utf8_len;
			continue;
		}

		src += utf8_len;
		dst += mbsize;
	}

	*dst = '\0';
	return dst;
}

TCHAR *CSJISArchive::UTF8Read()
{
	int		len;

	if(m_init == FALSE) {
		// UTF-8 signature(0xef 0xbb 0xbf)を読み飛ばす
		len = m_ar->GetFile()->Read((char *)m_buf, 3);
		if(len == 0) return NULL;

		if(check_utf8_signature(m_buf) == 0) {
			// signature無し
			m_ar->GetFile()->SeekToBegin();
		}

		m_init = TRUE;
	}

	int		read_len = sizeof(m_buf) - 1;

	len = m_ar->GetFile()->Read((char *)m_buf, read_len);
	if(len == 0) return NULL;
	m_buf[len] = '\0';

	int back_cnt;
	m_buf_p = utf8tosjis(m_buf, m_conv_buf, &back_cnt);

	if(len == read_len && back_cnt != 0) {
		m_ar->GetFile()->Seek(-back_cnt, CFile::current);
	} else if(len == read_len && *(m_buf_p - 1) == '\r') {
		// '\r'と'\n'を切り離さない
		*(m_buf_p - 1) = '\0';
		m_ar->GetFile()->Seek(-1, CFile::current);
	}

	return m_conv_buf;
}

static int get_utf8_len_from_unicode(wchar_t unicode_char)
{
	if(unicode_char <= 0x007f) return 1;
	if(unicode_char >= 0x0080 && unicode_char <= 0x07ff) return 2;
	if(unicode_char >= 0x0800 && unicode_char <= 0xffff) return 3;

	return 0;
}

static int get_utf8_from_unicode(wchar_t unicode_char, TCHAR *dst)
{
	int utf8_len = get_utf8_len_from_unicode(unicode_char);

	switch(utf8_len) {
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

TCHAR *sjistoutf8(TCHAR *src, TCHAR *dst, int dst_size)
{
	int		mbsize;
	int		dst_cnt = 0;
	int		utf8_len;
	wchar_t	unicode_char;

	// utf8データが途中で切れないようにする
	dst_size -= 4;

	for(;;) {
		if(*src == '\0') break;
		if(dst_cnt >= dst_size) {
			*dst = '\0';
			return src;
		}

		mbsize = mbtowc(&unicode_char, (char *)src, MB_CUR_MAX);
		if(mbsize == -1) {
			TRACE(_T("mbtowc error:%d:%.10s\n"), unicode_char, src);
			src++;
			continue;
		}

		utf8_len = get_utf8_from_unicode(unicode_char, dst);

		src += mbsize;
		dst += utf8_len;
		dst_cnt += utf8_len;
	}

	*dst = '\0';
	return NULL;
}

void CSJISArchive::UTF8WriteString(TCHAR *buf)
{
	if(m_init == FALSE) {
		if(m_mode == KANJI_CODE_UTF8) {
			// 0xef 0xbb 0xbfを書き込む
			TCHAR uni_head[3];
			uni_head[0] = 0xef;
			uni_head[1] = 0xbb;
			uni_head[2] = 0xbf;
			m_ar->Write(uni_head, 3);
		}

		m_init = TRUE;
	}

	TCHAR *p;

	for(p = buf; p != NULL;) {
		p = sjistoutf8(p, m_conv_buf, sizeof(m_conv_buf));
		m_ar->WriteString(m_conv_buf);
	}
}

struct CSJISArchive::kanji_code_st CSJISArchive::m_kanji_code_list[] = {
	{KANJI_CODE_SJIS, _T("SJIS")},
	{KANJI_CODE_EUC, _T("EUC")},
	{KANJI_CODE_JIS, _T("JIS")},
	{KANJI_CODE_UNICODE, _T("UNICODE")},
	{KANJI_CODE_UNICODE_NO_SIGNATURE, _T("UNICODE(no signature)")},
	{KANJI_CODE_UTF8, _T("UTF8")},
	{KANJI_CODE_UTF8_NO_SIGNATURE, _T("UTF8(no signature)")},
};

struct CSJISArchive::line_type_st CSJISArchive::m_line_type_list[] = {
	{LINE_TYPE_CR_LF, _T("CR+LF(\\r\\n)")},
	{LINE_TYPE_LF, _T("LF(\\n)")},
	{LINE_TYPE_CR, _T("CR(\\r)")},
};

void CSJISArchive::SetKanjiCodeCombo(CComboBox *p_combo_kanji, int kanji_code, BOOL b_add_autodetect)
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

void CSJISArchive::SetLineTypeCombo(CComboBox *p_combo_line, int line_type)
{
	if(p_combo_line == NULL) return;

	int		i;
	for(i = 0; i < sizeof(m_line_type_list)/sizeof(m_line_type_list[0]); i++) {
		p_combo_line->InsertString(i, m_line_type_list[i].kanji_name);
		p_combo_line->SetItemData(i, m_line_type_list[i].kanji_code);
	}

	for(i = 0; p_combo_line->GetItemData(i) != CB_ERR; i++) {
		if(p_combo_line->GetItemData(i) == (UINT)line_type) {
			p_combo_line->SetCurSel(i);
			break;
		}
	}
}

