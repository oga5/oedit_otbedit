/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef UNICODE_ARCHIVE_H_INCLUDED
#define UNICODE_ARCHIVE_H_INCLUDED

#include "check_kanji.h"

#define LINE_TYPE_UNKNOWN	0
#define LINE_TYPE_CR_LF		1
#define LINE_TYPE_LF		2
#define LINE_TYPE_CR		3

#define UNICODE_ARCHIVE_BUF_SIZE		(4096 + 1)

// FIXME: ƒeƒXƒg’†
//#define UNICODE_ARCHIVE_BUF_SIZE		(33)

class CUnicodeArchive
{
private:
	CArchive		*m_ar;

	CString			m_open_mode;
	CString			m_file_name;

	int				m_mode;
	BUF_BYTE		m_back_data;
	BUF_BYTE		m_back_buf[10];
	int				m_jis_mode;
	int				m_kana_shift;

	BOOL			m_init;

	BUF_BYTE		m_buf[UNICODE_ARCHIVE_BUF_SIZE];
	BUF_BYTE		m_conv_buf[UNICODE_ARCHIVE_BUF_SIZE * 4];
	BUF_BYTE		m_conv_buf2[UNICODE_ARCHIVE_BUF_SIZE * 4];
	BUF_BYTE		*m_buf_p;

	CFile			*m_file;
	CArchive		*m_file_ar;
	TCHAR			*m_line_p;

	TCHAR *SJISRead();
	TCHAR *EUCRead();
	TCHAR *JISRead();
	TCHAR *UNICODERead();
	TCHAR *UTF16BERead();
	TCHAR *UTF8Read();

	void SJISWriteString(const TCHAR *buf);
	void EUCWriteString(const TCHAR *buf);
	void JISWriteString(const TCHAR *buf);
	void UNICODEWriteString(const TCHAR *buf);
	void UTF16BEWriteString(const TCHAR *buf);
	void UTF8WriteString(const TCHAR *buf);

	void Init(CArchive *ar, int mode, int line_type);

	void CloseAll();

	int CheckLineTypeA();
	int CheckLineTypeW();

public:
	CUnicodeArchive();
	CUnicodeArchive(CArchive *ar, int mode, int line_type = LINE_TYPE_UNKNOWN);
	~CUnicodeArchive();

	BOOL OpenFile(const TCHAR *file_name, const TCHAR *open_mode,
		int mode = UnknownKanjiCode, int line_type = LINE_TYPE_UNKNOWN);
	BOOL ReOpenFile();

	TCHAR *Read();
	TCHAR *ReadLine(TCHAR *buf, int buf_size);

	void WriteString(const TCHAR *buf);
	void WriteNextLine();
	void CsvOut(const TCHAR *string, int dquote_flg);

	int CheckLineType();

	ULONGLONG GetPosition();
	void Flush();

	const TCHAR *GetFileName() { return m_file_name; }

private:
	static struct kanji_code_st {
		int			kanji_code;
		const TCHAR	*kanji_name;
	} m_kanji_code_list[];

	static struct line_type_st {
		int			line_type;
		const TCHAR	*line_type_name;
	} m_line_type_list[];

	int m_line_type;
	TCHAR m_line_sep[3];

public:
	static int CheckKanjiCode(CArchive *ar, int default_kanji_code = KANJI_CODE_SJIS);

	static CString MakeFileType(int kanji_code, int line_type);

	static void SetKanjiCodeCombo(CComboBox *p_combo_kanji, int kanji_code, BOOL b_add_autodetect);
	static void SetLineTypeCombo(CComboBox *p_combo_line, int line_type);
};

int check_line_type(const TCHAR *buf);

int get_kanji_code_from_str(const TCHAR *str);
int get_line_type_from_str(const TCHAR *str);

#endif /* UNICODE_ARCHIVE_H_INCLUDED */
