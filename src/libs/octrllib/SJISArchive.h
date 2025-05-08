#define KANJI_CODE_SJIS		1
#define KANJI_CODE_EUC		2
#define KANJI_CODE_JIS		3
#define KANJI_CODE_ASCII	4
#define KANJI_CODE_UNICODE	5
#define KANJI_CODE_UTF8		6
#define KANJI_CODE_UNICODE_NO_SIGNATURE	7
#define KANJI_CODE_UTF8_NO_SIGNATURE	8
#define UnknownKanjiCode	9

#define LINE_TYPE_CR_LF		1
#define LINE_TYPE_LF		2
#define LINE_TYPE_CR		3

#define SJIS_ARCHIVE_BUF_SIZE		(4096 + 1)

class CSJISArchive
{
private:
	CArchive		*m_ar;
	int				m_mode;
	TCHAR			m_back_data;
	TCHAR			m_back_buf[10];
	int				m_jis_mode;
	int				m_kana_shift;

	BOOL			m_init;

	TCHAR			m_buf[SJIS_ARCHIVE_BUF_SIZE];
	TCHAR			m_conv_buf[SJIS_ARCHIVE_BUF_SIZE * 2];
	TCHAR			*m_buf_p;

	TCHAR *SJISRead();
	TCHAR *EUCRead();
	TCHAR *JISRead();
	TCHAR *UNICODERead();
	TCHAR *UTF8Read();

	void SJISWriteString(TCHAR *buf);
	void EUCWriteString(TCHAR *buf);
	void JISWriteString(TCHAR *buf);
	void UNICODEWriteString(TCHAR *buf);
	void UTF8WriteString(TCHAR *buf);

public:
	CSJISArchive(CArchive *ar, int mode);
	~CSJISArchive();

	TCHAR *Read();
	void WriteString(TCHAR *buf);

private:
	static struct kanji_code_st {
		int			kanji_code;
		const TCHAR	*kanji_name;
	} m_kanji_code_list[];

	static struct line_type_st {
		int			kanji_code;
		const TCHAR	*kanji_name;
	} m_line_type_list[];

public:
	static int CheckKanjiCode(CArchive *ar, int default_kanji_code = KANJI_CODE_SJIS);
	static int CheckLineType(CArchive *ar, int kanji_code);

	static CString MakeFileType(int kanji_code, int line_type);

	static void SetKanjiCodeCombo(CComboBox *p_combo_kanji, int kanji_code, BOOL b_add_autodetect);
	static void SetLineTypeCombo(CComboBox *p_combo_line, int line_type);
};

int check_line_type(const char *buf);

