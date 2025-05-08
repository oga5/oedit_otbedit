/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef __LANGUAGE_SETTING_H_INCLUDED__
#define __LANGUAGE_SETTING_H_INCLUDED__

#include "strtoken.h"

class CLanguageSetting
{
public:
	CLanguageSetting(const TCHAR *name, const TCHAR *short_name,
		const TCHAR *config_file_name, const TCHAR *suffix, CString &base_dir);

	BOOL LoadSetting(CStrToken *str_token, TCHAR *msg_buf);

	CString &GetName() { return m_name; }
	CString &GetShortName() { return m_short_name; }
	CString &GetSuffix() { return m_suffix; }

	BOOL IsEnableCStyleIndent() { return m_is_enable_c_style_indent; }

	CString GetDefaultExt();

private:
	BOOL LoadInternalSetting(CStrToken *str_token, TCHAR *msg_buf);

	CString		m_name;
	CString		m_short_name;
	CString		m_config_file_name;
	CString		m_suffix;
	CString		m_base_dir;

	BOOL		m_is_enable_c_style_indent;
};

class CLanguageSettingList
{
public:
	CLanguageSettingList();
	~CLanguageSettingList();

	BOOL InitList(const TCHAR *base_dir, UINT menu_id_base, int max_cnt, TCHAR *msg_buf);

	int GetCount() { return m_cnt; }
	CLanguageSetting *GetSetting(int idx);

	int GetSettingIdxFromFileName(const TCHAR *file_name);
	int GetSettingIdx(const TCHAR *name);

	CString GetMessageString(int idx);
	CString GetCommandString(int idx);
	void AddEditModeMenu(CMenu* pPopupMenu, int idx);
	void AddToolBarBtn(CToolBar *pToolBar);
	CString GetFilterSuffix(OPENFILENAME& ofn, UINT all_filter_id);
	void AddComboEditModeForSetup(CComboBox *pComboBox, CString cur_setting);

	static const TCHAR *GetNameFromOldIdx(int idx);

private:
	void FreeList();
	BOOL AddSetting(int idx, int line_no, const TCHAR *data, TCHAR *msg_buf);

	void LangNameOut(CDC *dc, int x);

	CLanguageSetting	**m_setting_list;
	CString				m_base_dir;
	int					m_cnt;

	UINT		m_menu_id_base;
	int			m_max_cnt;
};

#endif __LANGUAGE_SETTING_H_INCLUDED__
