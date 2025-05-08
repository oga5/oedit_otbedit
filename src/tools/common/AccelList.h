/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(_ACCEL_LIST_INCLUDED_)
#define _ACCEL_LIST_INCLUDED_

class CAccelList
{
private:
	ACCEL	*m_accel_list;
	int		m_accel_cnt;
	int		m_accel_alloc_cnt;

	BOOL realloc_accel();
	void free_accel();

	BOOL add_accel(WORD cmd, WORD key, BYTE fVirt);
	BOOL delete_accel(int idx);
	BOOL delete_accel(WORD key, BYTE fVirt);

	int search_accel_from_key(WORD key, BYTE fVirt);
	int search_accel_from_cmd(WORD cmd, int pos);

	static const TCHAR *get_key_name(WORD key);
	static WORD get_key_code(const TCHAR *key_name);

	void make_reg_key(int idx, TCHAR *cmd_name, TCHAR *key_name, TCHAR *virt_name);

public:
	CAccelList();
	~CAccelList();

	static BOOL get_accel_info(const TCHAR *accel_str, WORD &key, BYTE &fVirt);

	int search_accel_str(WORD cmd, int pos, CString &accel_str);
	int search_accel_str2(WORD cmd, CString &accel_str);
	int search_cmd(const TCHAR *accel_str);
	BOOL add_accel(WORD cmd, const TCHAR *accel_str);
	BOOL delete_accel(const TCHAR *accel_str);
	BOOL delete_accel_from_cmd(WORD cmd);

	BOOL load_accel_list();
	BOOL load_default_accel_list();
	BOOL save_accel_list();

	ACCEL *get_accel_list() { return m_accel_list; }
	int get_accel_cnt() { return m_accel_cnt; }

	static CString get_accel_str(WORD key, BOOL alt, BOOL ctrl, BOOL shift);

	void operator=(CAccelList &accel_list);
	void make_no_allow_key_list(CAccelList &accel_list);
	void make_filterd_accel_list(CAccelList &accel_list, WORD *key_arr, int key_cnt);

	CString GetCommandName(UINT nID);
	CString GetCommandInfo(UINT nID);
};

#endif _ACCEL_LIST_INCLUDED_
