/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "LanguageSetting.h"

#include "UnicodeArchive.h"
#include "fileutil.h"
#include "ostrutil.h"

CLanguageSetting::CLanguageSetting(const TCHAR *name, const TCHAR *short_name,
	const TCHAR *config_file_name, const TCHAR *suffix, CString &base_dir) : 
	m_name(name), m_short_name(short_name),
	m_config_file_name(config_file_name), m_suffix(suffix), m_base_dir(base_dir)
{
	m_is_enable_c_style_indent = FALSE;
}

CString CLanguageSetting::GetDefaultExt()
{
	CString ext = m_suffix;

	if(ext.Find(';', 0) != -1) {
		ext = ext.Left(ext.Find(';', 0));
	}
	ext = ext.Right(ext.GetLength() - 1);

	return ext;
}

BOOL CLanguageSetting::LoadSetting(CStrToken *str_token, TCHAR *msg_buf)
{
	TCHAR	buf[2048];

	str_token->Init();
	if(m_config_file_name.GetAt(0) == '<') {
		return LoadInternalSetting(str_token, msg_buf);
	}

	CUnicodeArchive uni_ar;
	if(!uni_ar.OpenFile(m_config_file_name, _T("r"))) {
		_stprintf(msg_buf, _T("言語設定ファイルが開けません(%s)"), m_config_file_name.GetBuffer(0));
		return FALSE;
	}

	int line_no = 1;
	TCHAR	setting_name[2048];
	const TCHAR	*p;

	CString keyword_file_name;

	for(;; line_no++) {
		if(uni_ar.ReadLine(buf, sizeof(buf)) == NULL) break;
		ostr_chomp(buf, ' ');
		if(buf[0] == '\0') continue;
		if(buf[0] == '/' && buf[1] == '/') continue;	// コメント行

		p = ostr_split(buf, setting_name, ':');
		if(ostr_strcmp_nocase(setting_name, _T("KeywordNocase")) == 0) {
			str_token->SetKeywordLowerUpper((ostr_strcmp_nocase(p, _T("TRUE")) == 0));
		} else if(ostr_strcmp_nocase(setting_name, _T("KeywordFile")) == 0) {
			keyword_file_name.Format(_T("%s%s"), m_base_dir.GetBuffer(0), p);
		} else if(ostr_strcmp_nocase(setting_name, _T("OpenComment")) == 0) {
			str_token->SetOpenComment(p);
		} else if(ostr_strcmp_nocase(setting_name, _T("CloseComment")) == 0) {
			str_token->SetCloseComment(p);
		} else if(ostr_strcmp_nocase(setting_name, _T("RowComment")) == 0) {
			str_token->SetRowComment(p);
		} else if(ostr_strcmp_nocase(setting_name, _T("BreakChars")) == 0) {
			str_token->SetBreakChars(p);
		} else if(ostr_strcmp_nocase(setting_name, _T("OperatorChars")) == 0) {
			str_token->SetOperatorChars(p);
		} else if(ostr_strcmp_nocase(setting_name, _T("BracketChars")) == 0) {
			str_token->SetBracketChars(p);
		} else if(ostr_strcmp_nocase(setting_name, _T("QuoteChars")) == 0) {
			str_token->SetQuoteChars(p);
		} else if(ostr_strcmp_nocase(setting_name, _T("EscapeChar")) == 0) {
			str_token->SetEscapeChar(*p);
		} else if(ostr_strcmp_nocase(setting_name, _T("LanguageKeywordChars")) == 0) {
			str_token->SetLangKeywordChars(p);
		} else if(ostr_strcmp_nocase(setting_name, _T("CStyleIndent")) == 0) {
			m_is_enable_c_style_indent = ((ostr_strcmp_nocase(p, _T("TRUE")) == 0));
		} else if(ostr_strcmp_nocase(setting_name, _T("TagLanguageMode")) == 0) {
			str_token->SetTagMode((ostr_strcmp_nocase(p, _T("TRUE")) == 0));
		} else {
			_stprintf(msg_buf, 
				_T("言語設定ファイルの書式が不正です\n")
				_T("(ファイル名：%s, 行：%d)"), m_config_file_name.GetBuffer(0), line_no);
			return FALSE;
		}
	}

	if(str_token->initDefaultKeyword(keyword_file_name, msg_buf) != 0) {
		_stprintf(msg_buf, _T("キーワード設定ファイルが開けません(ファイル名：%s)"), p);
		return FALSE;
	}

	return TRUE;
}

BOOL CLanguageSetting::LoadInternalSetting(CStrToken *str_token, TCHAR *msg_buf)
{
	CString keyword_file_name = _T("");

	if(m_config_file_name == _T("<text>")) {
		str_token->SetKeywordLowerUpper(TRUE);
		str_token->SetBreakChars(_T(" ,.:;(){}[]'\"=><-|&*+-/%!^"));
		str_token->SetBracketChars(_T("]>)}"));
		keyword_file_name.Format(_T("%s\\text.txt"), m_base_dir.GetBuffer(0));
	}
	if(m_config_file_name == _T("<cpp>")) {
		str_token->SetKeywordLowerUpper(FALSE);
		str_token->SetOpenComment(_T("/*"));
		str_token->SetCloseComment(_T("*/"));
		str_token->SetRowComment(_T("//"));
		str_token->SetQuoteChars(_T("\"'"));
		str_token->SetBreakChars(_T(" ,.:;(){}[]'\"=><-|&*+-/%!^"));
		str_token->SetBracketChars(_T(")}]"));
		str_token->SetOperatorChars(_T("=!&|^~*/+-%<>;:.,(){}[]"));
		str_token->SetEscapeChar('\\');
		keyword_file_name.Format(_T("%s\\cpp.txt"), m_base_dir.GetBuffer(0));
	}
	if(m_config_file_name == _T("<java>")) {
		str_token->SetKeywordLowerUpper(FALSE);
		str_token->SetOpenComment(_T("/*"));
		str_token->SetCloseComment(_T("*/"));
		str_token->SetRowComment(_T("//"));
		str_token->SetQuoteChars(_T("\"'"));
		str_token->SetBreakChars(_T(" ,.:;(){}[]'\"=><-|&*+-/%!^"));
		str_token->SetBracketChars(_T(")}"));
		str_token->SetOperatorChars(_T("=!&|^~*/+-%<>;:.,(){}[]"));
		str_token->SetEscapeChar('\\');
		keyword_file_name.Format(_T("%s\\java.txt"), m_base_dir.GetBuffer(0));
	}
	if(m_config_file_name == _T("<sql>")) {
		str_token->SetKeywordLowerUpper(TRUE);
		str_token->SetOpenComment(_T("/*"));
		str_token->SetCloseComment(_T("*/"));
		str_token->SetRowComment(_T("--"));
		str_token->SetQuoteChars(_T("'"));
		str_token->SetBreakChars(_T(" ,;'\".%()=><"));
		str_token->SetBracketChars(_T(")"));
		str_token->SetOperatorChars(_T("=!*/+-%;.,(){}:"));
		keyword_file_name.Format(_T("%s\\sql.txt"), m_base_dir.GetBuffer(0));
	}
	if(m_config_file_name == _T("<perl>")) {
		str_token->SetKeywordLowerUpper(FALSE);
		str_token->SetRowComment(_T("#"));
		str_token->SetQuoteChars(_T("\"'"));
		str_token->SetBreakChars(_T(" ,.:;(){}[]'\"=><-&|*+-/%"));
		str_token->SetBracketChars(_T(")}"));
		str_token->SetOperatorChars(_T("=!&|^~*/+-%<>;:.,(){}[]"));
		str_token->SetEscapeChar('\\');
		str_token->SetLangKeywordChars(_T("$%@"));
		keyword_file_name.Format(_T("%s\\perl.txt"), m_base_dir.GetBuffer(0));
	}
	if(m_config_file_name == _T("<html>")) {
		str_token->SetKeywordLowerUpper(TRUE);
		str_token->SetOpenComment(_T("<!--"));
		str_token->SetCloseComment(_T("-->"));
		str_token->SetQuoteChars(_T("\""));
		str_token->SetBreakChars(_T(" ,.:;(){}[]\"=></"));
		str_token->SetOperatorChars(_T("=/<>"));
		str_token->SetBracketChars(_T(">)}"));
		str_token->SetTagMode(TRUE);
		keyword_file_name.Format(_T("%s\\html.txt"), m_base_dir.GetBuffer(0));
	}
	if(str_token->initDefaultKeyword(keyword_file_name.GetBuffer(0), msg_buf) != 0) {
//		sprintf(msg_buf, "キーワード設定ファイルが開けません(ファイル名：%s)", p);
//		return FALSE;
	}
	return TRUE;
}

CLanguageSettingList::CLanguageSettingList()
{
	m_setting_list = NULL;
	m_cnt = 0;
	m_base_dir = _T("");
	m_max_cnt = 6;
	m_menu_id_base = 0;
}

CLanguageSettingList::~CLanguageSettingList()
{
	FreeList();
}

BOOL CLanguageSettingList::AddSetting(int idx, int line_no, const TCHAR *data, TCHAR *msg_buf)
{
	TCHAR		split_buf[2048];
	const TCHAR		*p = data;
	CString		name;
	CString		short_name;
	CString		config_file_name;
	CString		suffix;

	p = ostr_split(p, split_buf, ',');
	if(split_buf[0] == '\0') goto ERR1;
	name = split_buf;

	p = ostr_split(p, split_buf, ',');
	if(split_buf[0] == '\0') goto ERR1;
	short_name = split_buf;

	p = ostr_split(p, split_buf, ',');
	if(split_buf[0] == '\0') goto ERR1;
	if(split_buf[0] == '<') {
		config_file_name = split_buf;
	} else {
		config_file_name.Format(_T("%s%s"), m_base_dir.GetBuffer(0), split_buf);
	}

	p = ostr_split(p, split_buf, ',');
	if(split_buf[0] == '\0') goto ERR1;
	suffix = split_buf;

	m_setting_list[idx] = new CLanguageSetting(name, short_name, 
		config_file_name, suffix, m_base_dir);

	return TRUE;

ERR1:
	_stprintf(msg_buf, _T("言語設定ファイルの書式が不正です(行：%d)"), line_no);
	return FALSE;
}

BOOL CLanguageSettingList::InitList(const TCHAR *base_dir, UINT menu_id_base, int max_cnt, TCHAR *msg_buf)
{
	if(max_cnt < 6) max_cnt = 6;

	CString	*setting_list = new CString[max_cnt];
	int		*line_no_arr = new int[max_cnt];
	int		i;

	memset(line_no_arr, 0, sizeof(line_no_arr[0]) * max_cnt);

	m_menu_id_base = menu_id_base;
	m_max_cnt = max_cnt;
	m_base_dir = base_dir;

	CString lang_file_name;
	lang_file_name.Format(_T("%s%s"), base_dir, _T("languages.txt"));
	if(!is_file_exist(lang_file_name)) {
		lang_file_name.Format(_T("%s%s"), base_dir, _T("languages_default.txt"));
		if(!is_file_exist(lang_file_name)) {
			lang_file_name = _T("Internal");
			m_cnt = 6;
			setting_list[0] = _T("Text,Txt,<text>,*.txt");
			setting_list[1] = _T("Html,Html,<html>,*.htm;*.html;*.shtml");
			setting_list[2] = _T("C++,C++,<cpp>,*.cpp;*.c;*.h");
			setting_list[3] = _T("Java,Java,<java>,*.java");
			setting_list[4] = _T("Perl,Perl,<perl>,*.cgi;*.pl");
			setting_list[5] = _T("SQL,SQL,<sql>,*.sql");
			for(i = 0; i < m_cnt; i++) line_no_arr[i] = i + 1;
		}
	}

	FreeList();

	if(lang_file_name.Compare(_T("Internal")) != 0) {
		TCHAR	buf[2048];
		CUnicodeArchive uni_ar;
		if(!uni_ar.OpenFile(lang_file_name.GetBuffer(0), _T("r"))) {
			_stprintf(msg_buf, _T("言語設定ファイルが開けません(%s)"), lang_file_name.GetBuffer(0));
			goto ERR1;
		}

		// キーワードの数を数える
		int		line_no;
		m_cnt = 0;
		for(line_no = 1;; line_no++) {
			if(uni_ar.ReadLine(buf, sizeof(buf)) == NULL) break;
			ostr_chomp(buf, ' ');
			if(buf[0] == '\0') continue;
			if(buf[0] == '/' && buf[1] == '/') continue;	// コメント行

			setting_list[m_cnt] = buf;
			line_no_arr[m_cnt] = line_no;

			m_cnt++;
			if(m_cnt > max_cnt) {
				_stprintf(msg_buf, _T("設定可能な編集モードの最大数を超えています"));
				goto ERR1;
			}
		}
	}

	// 配列のメモリを確保
	m_setting_list = new CLanguageSetting*[m_cnt];
	for(i = 0; i < m_cnt; i++) {
		m_setting_list[i] = NULL;
	}
	// 設定
	for(i = 0; i < m_cnt; i++) {
		if(!AddSetting(i, line_no_arr[i], setting_list[i], msg_buf)) goto ERR1;
	}

	if(m_cnt == 0) {
		_stprintf(msg_buf, _T("言語設定ファイルに設定がありません(%s)"), lang_file_name.GetBuffer(0));
		goto ERR1;
	}

	delete[] line_no_arr;
	delete[] setting_list;
	return TRUE;

ERR1:
	delete[] line_no_arr;
	delete[] setting_list;
	return FALSE;
}

void CLanguageSettingList::FreeList()
{
	if(m_setting_list == NULL) return;

	for(int i = 0; i < m_cnt; i++) {
		delete m_setting_list[i];
	}
	delete[] m_setting_list;
}

static const TCHAR *GetExtFromSuffixList(const TCHAR *p, TCHAR *ext)
{
	*ext = '\0';
	if(p == NULL || *p == '\0') return 0;

	if(*p == ';') p++;
	if(*p == '*') p++;

	for(; *p != ';' && *p != '\0'; p++) {
		*ext = *p;
		ext++;
	}
	*ext = '\0';

	if(*p == '\0') return NULL;
	return p;
}

int CLanguageSettingList::GetSettingIdxFromFileName(const TCHAR *file_name)
{
	TCHAR	ext[_MAX_EXT];
	TCHAR	ext2[_MAX_EXT];

	if(_tcsstr(file_name, _T("Temporary Internet Files")) != NULL) {	// for IE
		_tcscpy(ext, _T(".htm"));
	} else {
		_tsplitpath(file_name, NULL, NULL, NULL, ext);
	}

	for(int idx = 0; idx < m_cnt; idx++) {
		const TCHAR *p = m_setting_list[idx]->GetSuffix();
		for(; p != NULL;) {
			p = GetExtFromSuffixList(p, ext2);
			if(ostr_strcmp_nocase(ext, ext2) == 0) return idx;
		}
	}

	return -1;
}

int CLanguageSettingList::GetSettingIdx(const TCHAR *name)
{
	if(_tcsicmp(name, _T("txt")) == 0) return GetSettingIdx(_T("text"));
	if(_tcsicmp(name, _T("cpp")) == 0) return GetSettingIdx(_T("c++"));

	for(int i = 0; i < m_cnt; i++) {
		if(m_setting_list[i]->GetName().CollateNoCase(name) == 0) return i;
	}

	return -1;
}

CLanguageSetting *CLanguageSettingList::GetSetting(int idx)
{
	if(m_setting_list == NULL) return NULL;
	if(idx >= m_cnt) return m_setting_list[0];
	return m_setting_list[idx];
}

CString CLanguageSettingList::GetMessageString(int idx)
{
	CString s;
	s.Format(_T("編集モードを%sにする"), GetSetting(idx)->GetName().GetBuffer(0));
	return s;
}

CString CLanguageSettingList::GetCommandString(int idx)
{
	CString s;

	if(idx >= m_cnt) {
		s.Format(_T("編集モードを変更する(%d)\n編集モード(%d)"), idx, idx);
	} else {
		s.Format(_T("%s\n%s"), GetMessageString(idx).GetBuffer(0), GetSetting(idx)->GetName().GetBuffer(0));
	}
	return s;
}

void CLanguageSettingList::AddEditModeMenu(CMenu* pPopupMenu, int idx)
{
	UINT	id;
	for(id = m_menu_id_base; id <= m_menu_id_base + m_max_cnt; id++) {
		pPopupMenu->RemoveMenu(id, MF_BYCOMMAND);
	}

	int		i;
	for(i = 0; i < GetCount(); i++) {
		if(idx == i) {
			pPopupMenu->AppendMenu(MF_STRING | MF_CHECKED, (UINT_PTR)m_menu_id_base + i, GetSetting(i)->GetName());
		} else {
			pPopupMenu->AppendMenu(MF_STRING, (UINT_PTR)m_menu_id_base + i, GetSetting(i)->GetName());
		}
	}
}

void CLanguageSettingList::LangNameOut(CDC *dc, int x)
{
	CFont		font;
	LOGFONT		lf;
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = 400;
	lf.lfItalic = 0;
	lf.lfUnderline = 0;
	lf.lfStrikeOut = 0;
	lf.lfCharSet = 128;
	lf.lfOutPrecision = OUT_CHARACTER_PRECIS;
	lf.lfClipPrecision = CLIP_CHARACTER_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
	lf.lfHeight = -10;
	_tcscpy(lf.lfFaceName, _T("ＭＳ ゴシック"));
	font.CreateFontIndirect(&lf);

	dc->SetBkColor(GetSysColor(COLOR_BTNFACE));

	CFont *old_font = dc->SelectObject(&font);
	for(int lang_idx = 0; lang_idx < GetCount(); lang_idx++) {
		dc->TextOut(x, 2, GetSetting(lang_idx)->GetShortName(), 3);
		x += 16;
	}
	dc->SelectObject(old_font);
	font.DeleteObject();
}

void CLanguageSettingList::AddToolBarBtn(CToolBar *pToolBar)
{
	static const int bitmap_width = 16;
	static const int bitmap_height = 15;

	CClientDC	client_dc(pToolBar);
	CBitmap		tool_bar_bmp;

	CDC dc;
	dc.CreateCompatibleDC(&client_dc);
	int b_width = GetCount() * bitmap_width + 
		pToolBar->GetToolBarCtrl().GetButtonCount() * bitmap_width;
	int b_height = bitmap_height;
	tool_bar_bmp.CreateCompatibleBitmap(&client_dc, b_width, b_height);

	CBitmap *old_bmp = dc.SelectObject(&tool_bar_bmp);

	::FillRect(dc, CRect(0, 0, b_width, b_height), GetSysColorBrush(COLOR_BTNFACE));

	int blt_width = 0;
	LangNameOut(&dc, blt_width);
	
	dc.SelectObject(old_bmp);
	dc.DeleteDC();

	CImageList* pImgList = pToolBar->GetToolBarCtrl().GetImageList();
	pImgList->Add(&tool_bar_bmp, RGB(192,192,192));
	pToolBar->GetToolBarCtrl().SetImageList(pImgList);
	DeleteObject(tool_bar_bmp);

	TBBUTTON	tb_btn;
	int bitmap_base = pToolBar->GetToolBarCtrl().GetButtonCount();
	for(int lang_idx = 0; lang_idx < GetCount(); lang_idx++) {
		tb_btn.iBitmap = lang_idx + bitmap_base;
		tb_btn.idCommand = m_menu_id_base + lang_idx;
		tb_btn.fsState = TBSTATE_ENABLED;
		tb_btn.fsStyle = TBSTYLE_BUTTON;
		tb_btn.iString = 0;

		pToolBar->GetToolBarCtrl().InsertButton(lang_idx, &tb_btn);
	}

	// セパレータを挿入
	tb_btn.iBitmap = -1;
	tb_btn.idCommand = 0;
	tb_btn.fsState = TBSTATE_ENABLED;
	tb_btn.fsStyle = TBSTYLE_SEP;
	tb_btn.iString = 0;

	pToolBar->GetToolBarCtrl().InsertButton(GetCount(), &tb_btn);
}

static void AppendFilterSuffix2(CString &filter, OPENFILENAME& ofn,
	CString fileter_name, CString ext_name)
{
		// add to filter
	filter += fileter_name;
	ASSERT(!filter.IsEmpty());  // must have a file type name
	filter += (TCHAR)'\0';  // next string please
	filter += (TCHAR)'*';
	filter += ext_name;
	filter += (TCHAR)'\0';  // next string please
	ofn.nMaxCustFilter++;
}

CString CLanguageSettingList::GetFilterSuffix(OPENFILENAME& ofn, UINT all_filter_id)
{
	CString strFilter;

	CString allFilter;
	VERIFY(allFilter.LoadString(all_filter_id));

	AppendFilterSuffix2(strFilter, ofn, allFilter, _T("*.*"));

	for(int i = 0; i < GetCount(); i++) {
		CString name;
		name.Format(_T("%sファイル(%s)"), 
			GetSetting(i)->GetName().GetBuffer(0),
			GetSetting(i)->GetSuffix().GetBuffer(0));
		AppendFilterSuffix2(strFilter, ofn, name, GetSetting(i)->GetSuffix());
	}

	return strFilter;
}

void CLanguageSettingList::AddComboEditModeForSetup(CComboBox *pComboBox, CString cur_setting)
{
	if(pComboBox == NULL) return;

	int base = 0;
	pComboBox->InsertString(base, _T("前回起動時の設定"));
	pComboBox->SetItemData(base, -1);

	base++;
	for(int idx = 0; idx < GetCount(); idx++, base++) {
		pComboBox->InsertString(base, GetSetting(idx)->GetName());
		pComboBox->SetItemData(base, idx);

		if(cur_setting == GetSetting(idx)->GetName()) {
			pComboBox->SetCurSel(base);
		}
	}

	if(pComboBox->GetCurSel() == CB_ERR) {
		pComboBox->SetCurSel(0);
	}
}

