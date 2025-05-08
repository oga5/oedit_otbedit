#include "stdafx.h"
#include "accellist.h"

#include "default_accel.h"

#define ACCEL_ALLOC_CNT		100


static const struct {
	WORD key;
	TCHAR *key_name;
} key_name_map[] = {
	{VK_LBUTTON, _T("LBUTTON")},
	{VK_RBUTTON, _T("RBUTTON")},
	{VK_CANCEL, _T("CANCEL")},
	{VK_MBUTTON, _T("MBUTTON")},
	{VK_BACK, _T("BACK")},
	{VK_TAB, _T("TAB")},
	{VK_CLEAR, _T("CLEAR")},
	{VK_RETURN, _T("ENTER")},
	{VK_PAUSE, _T("PAUSE")},
	{VK_CAPITAL, _T("CAPITAL")},
	{VK_KANA, _T("KANA")},
	{VK_JUNJA, _T("JUNJA")},
	{VK_FINAL, _T("FINAL")},
	{VK_KANJI, _T("KANJI")},
	{VK_ESCAPE, _T("ESCAPE")},
	{VK_CONVERT, _T("CONVERT")},
	{VK_NONCONVERT, _T("NONCONVERT")},
	{VK_ACCEPT, _T("ACCEPT")},
	{VK_MODECHANGE, _T("MODECHANGE")},
	{VK_SPACE, _T("SPACE")},
	{VK_PRIOR, _T("PRIOR")},
	{VK_NEXT, _T("NEXT")},
	{VK_END, _T("END")},
	{VK_HOME, _T("HOME")},
	{VK_LEFT, _T("LEFT")},
	{VK_UP, _T("UP")},
	{VK_RIGHT, _T("RIGHT")},
	{VK_DOWN, _T("DOWN")},
	{VK_SELECT, _T("SELECT")},
	{VK_PRINT, _T("PRINT")},
	{VK_EXECUTE, _T("EXECUTE")},
	{VK_SNAPSHOT, _T("SNAPSHOT")},
	{VK_INSERT, _T("INSERT")},
	{VK_DELETE, _T("DELETE")},
	{VK_HELP, _T("HELP")},
	{VK_LWIN, _T("LWIN")},
	{VK_RWIN, _T("RWIN")},
	{VK_APPS, _T("APPS")},
	{VK_NUMPAD0, _T("NUMPAD0")},
	{VK_NUMPAD1, _T("NUMPAD1")},
	{VK_NUMPAD2, _T("NUMPAD2")},
	{VK_NUMPAD3, _T("NUMPAD3")},
	{VK_NUMPAD4, _T("NUMPAD4")},
	{VK_NUMPAD5, _T("NUMPAD5")},
	{VK_NUMPAD6, _T("NUMPAD6")},
	{VK_NUMPAD7, _T("NUMPAD7")},
	{VK_NUMPAD8, _T("NUMPAD8")},
	{VK_NUMPAD9, _T("NUMPAD9")},
	{VK_MULTIPLY, _T("MULTIPLY")},
	{VK_ADD, _T("ADD")},
	{VK_SEPARATOR, _T("SEPARATOR")},
	{VK_SUBTRACT, _T("SUBTRACT")},
	{VK_DECIMAL, _T("DECIMAL")},
	{VK_DIVIDE, _T("DIVIDE")},
	{VK_F1, _T("F1")},
	{VK_F2, _T("F2")},
	{VK_F3, _T("F3")},
	{VK_F4, _T("F4")},
	{VK_F5, _T("F5")},
	{VK_F6, _T("F6")},
	{VK_F7, _T("F7")},
	{VK_F8, _T("F8")},
	{VK_F9, _T("F9")},
	{VK_F10, _T("F10")},
	{VK_F11, _T("F11")},
	{VK_F12, _T("F12")},
	{VK_F13, _T("F13")},
	{VK_F14, _T("F14")},
	{VK_F15, _T("F15")},
	{VK_F16, _T("F16")},
	{VK_F17, _T("F17")},
	{VK_F18, _T("F18")},
	{VK_F19, _T("F19")},
	{VK_F20, _T("F20")},
	{VK_F21, _T("F21")},
	{VK_F22, _T("F22")},
	{VK_F23, _T("F23")},
	{VK_F24, _T("F24")},
	{VK_NUMLOCK, _T("NUMLOCK")},
	{VK_SCROLL, _T("SCROLL")},
};

CAccelList::CAccelList()
{
	m_accel_list = NULL;
	m_accel_cnt = 0;
	m_accel_alloc_cnt = 0;
}

CAccelList::~CAccelList()
{
	free_accel();
}

BOOL CAccelList::load_accel_list()
{
	free_accel();

	int accel_cnt = AfxGetApp()->GetProfileInt(_T("ACCELERATOR"), _T("ACCEL_CNT"), -1);
	if(accel_cnt == -1) {
		return load_default_accel_list();
	}

	TCHAR cmd_name[100];
	TCHAR key_name[100];
	TCHAR virt_name[100];

	WORD key;
	WORD cmd;
	BYTE fVirt;

#ifdef _DEBUG
	DWORD tick;
	tick = GetTickCount();
#endif

	int		i;
	for(i = 0; i < accel_cnt; i++) {
		make_reg_key(i, cmd_name, key_name, virt_name);

		cmd = AfxGetApp()->GetProfileInt(_T("ACCELERATOR"), cmd_name, -1);
		key = AfxGetApp()->GetProfileInt(_T("ACCELERATOR"), key_name, -1);
		fVirt = AfxGetApp()->GetProfileInt(_T("ACCELERATOR"), virt_name, -1);

		if(cmd > 0 && key > 0 && fVirt > 0) {
			add_accel(cmd, key, fVirt);
		}
	}

#ifdef _DEBUG
	tick = GetTickCount() - tick;
	TRACE(_T("CAccelList::load_accel_list time: %d\n"), tick);
#endif

	return TRUE;
}

BOOL CAccelList::save_accel_list()
{
	CWaitCursor	wait_cursor;

	TCHAR cmd_name[100];
	TCHAR key_name[100];
	TCHAR virt_name[100];

	AfxGetApp()->WriteProfileInt(_T("ACCELERATOR"), _T("ACCEL_CNT"), m_accel_cnt);

	int		i;
	for(i = 0; i < m_accel_cnt; i++) {
		make_reg_key(i, cmd_name, key_name, virt_name);
		AfxGetApp()->WriteProfileInt(_T("ACCELERATOR"), cmd_name, m_accel_list[i].cmd);
		AfxGetApp()->WriteProfileInt(_T("ACCELERATOR"), key_name, m_accel_list[i].key);
		AfxGetApp()->WriteProfileInt(_T("ACCELERATOR"), virt_name, m_accel_list[i].fVirt);
	}

	return TRUE;
}

BOOL CAccelList::load_default_accel_list()
{
	free_accel();

	int		i;

	for(i = 0; i < sizeof(default_accel)/sizeof(default_accel[0]); i++) {
		add_accel(default_accel[i].cmd, default_accel[i].key, default_accel[i].fVirt);
	}

	return TRUE;
}

BOOL CAccelList::realloc_accel()
{
	m_accel_list = (ACCEL *)realloc(m_accel_list, 
		sizeof(ACCEL) * ((INT_PTR)m_accel_alloc_cnt + ACCEL_ALLOC_CNT));
	if(m_accel_list == NULL) return FALSE;

	m_accel_alloc_cnt += ACCEL_ALLOC_CNT;

	return TRUE;
}

void CAccelList::free_accel()
{
	if(m_accel_list != NULL) free(m_accel_list);
	m_accel_list = NULL;
	m_accel_cnt = 0;
	m_accel_alloc_cnt = 0;
}

BOOL CAccelList::add_accel(WORD cmd, WORD key, BYTE fVirt)
{
	if(m_accel_cnt == m_accel_alloc_cnt) {
		if(realloc_accel() == FALSE) return FALSE;
	}

	m_accel_list[m_accel_cnt].fVirt = fVirt;
	m_accel_list[m_accel_cnt].cmd = cmd;
	m_accel_list[m_accel_cnt].key = key;

	m_accel_cnt++;
	
	return TRUE;
}

BOOL CAccelList::delete_accel(int idx)
{
	memmove(&m_accel_list[idx], &m_accel_list[idx + 1],
		sizeof(ACCEL) * ((INT_PTR)m_accel_cnt - idx - 1));
	m_accel_cnt--;

	return TRUE;
}

BOOL CAccelList::delete_accel(WORD key, BYTE fVirt)
{
	int		idx;

	idx = search_accel_from_key(key, fVirt);
	if(idx < 0) return TRUE;

	return delete_accel(idx);
}

BOOL CAccelList::delete_accel_from_cmd(WORD cmd)
{
	int		idx;

	for(;;) {
		idx = search_accel_from_cmd(cmd, 0);
		if(idx < 0) break;

		delete_accel(idx);
	}

	return TRUE;
}

int CAccelList::search_accel_from_key(WORD key, BYTE fVirt)
{
	int		i;

	for(i = 0; i < m_accel_cnt; i++) {
		if(m_accel_list[i].key == key && m_accel_list[i].fVirt == fVirt) return i;
	}

	return -1;
}

int CAccelList::search_accel_from_cmd(WORD cmd, int pos)
{
	int		i;

	for(i = pos; i < m_accel_cnt; i++) {
		if(m_accel_list[i].cmd == cmd) return i;
	}

	return -1;
}

const TCHAR *CAccelList::get_key_name(WORD key)
{
	int		i, loop_cnt;

	loop_cnt = sizeof(key_name_map)/sizeof(key_name_map[0]);

	for(i = 0; i < loop_cnt; i++) {
		if(key_name_map[i].key == key) return key_name_map[i].key_name;
	}

	return _T("");
}

WORD CAccelList::get_key_code(const TCHAR *key_name)
{
	int		i, loop_cnt;

	loop_cnt = sizeof(key_name_map)/sizeof(key_name_map[0]);

	for(i = 0; i < loop_cnt; i++) {
		if(_tcscmp(key_name_map[i].key_name, key_name) == 0) return key_name_map[i].key;
	}

	return 0;
}

CString CAccelList::get_accel_str(WORD key, BOOL alt, BOOL ctrl, BOOL shift)
{
	CString	keyInfo = _T("");
	CString ch = _T("");

	if(alt) keyInfo += _T("Alt+");
	if(ctrl) keyInfo += _T("Ctrl+");
	if(shift) keyInfo += _T("Shift+");

	if((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9')) {
		ch.Format(_T("%c"), key);
		keyInfo += ch;
	} else {
		ch = get_key_name(key);
		if(ch == "") {
			ch.Format(_T("%c"), MapVirtualKey(key, 2));
		}
		keyInfo += ch;
	}

	return keyInfo;
}

BOOL CAccelList::get_accel_info(const TCHAR *accel_str, WORD &key, BYTE &fVirt)
{
	fVirt = FVIRTKEY | FNOINVERT;

	if(_tcsstr(accel_str, _T("Alt+")) != NULL) fVirt |= FALT;
	if(_tcsstr(accel_str, _T("Ctrl+")) != NULL) fVirt |= FCONTROL;
	if(_tcsstr(accel_str, _T("Shift+")) != NULL) fVirt |= FSHIFT;

	const TCHAR *p1 = accel_str;
	const TCHAR *p2;
	for(;;) {
		p2 = _tcschr(p1, '+');
		if(p2 == NULL) break;
		p1 = p2 + 1;
	}

	if(_tcslen(p1) == 1 && ((*p1 >= 'A' && *p1 <= 'Z') || (*p1 >= '0' && *p1 <= '9'))) {
		key = *p1;
	} else {
		key = get_key_code(p1);
		if(key == 0) {
			key = VkKeyScan(*p1);
		}
	}

	return TRUE;
}

BOOL CAccelList::add_accel(WORD cmd, const TCHAR *accel_str)
{
	WORD	key;
	BYTE	fVirt;

	get_accel_info(accel_str, key, fVirt);
	delete_accel(key, fVirt);
	return add_accel(cmd, key, fVirt);
}

BOOL CAccelList::delete_accel(const TCHAR *accel_str)
{
	WORD	key;
	BYTE	fVirt;

	get_accel_info(accel_str, key, fVirt);
	return delete_accel(key, fVirt);
}

int CAccelList::search_accel_str(WORD cmd, int pos, CString &accel_str)
{
	int idx = search_accel_from_cmd(cmd, pos);
	if(idx < 0) return idx;

	accel_str = get_accel_str(m_accel_list[idx].key, 
		m_accel_list[idx].fVirt & FALT, m_accel_list[idx].fVirt & FCONTROL,
		m_accel_list[idx].fVirt & FSHIFT);

	return idx + 1;
}

int CAccelList::search_accel_str2(WORD cmd, CString &accel_str)
{
	CString	tmp_str;

	accel_str = _T("");

	int idx = 0;
	for(;;) {
		idx = search_accel_str(cmd, idx, tmp_str);
		if(idx < 0) break;

		if(accel_str != _T("")) accel_str += _T(", ");
		accel_str += tmp_str;
	}

	if(accel_str == _T("")) return -1;
	return 0;
}

int CAccelList::search_cmd(const TCHAR *accel_str)
{
	WORD	key;
	BYTE	fVirt;

	get_accel_info(accel_str, key, fVirt);
	int idx = search_accel_from_key(key, fVirt);
	if(idx == -1) return -1;

	return m_accel_list[idx].cmd;
}

void CAccelList::operator=(CAccelList &accel_list)
{
	int		i;
	ACCEL	*list;

	free_accel();

	list = accel_list.get_accel_list();
	for(i = 0; i < accel_list.get_accel_cnt(); i++) {
		add_accel(list[i].cmd, list[i].key, list[i].fVirt);
	}
}

void CAccelList::make_no_allow_key_list(CAccelList &accel_list)
{
	int		i;
	ACCEL	*list;

	free_accel();

	list = accel_list.get_accel_list();
	for(i = 0; i < accel_list.get_accel_cnt(); i++) {
		if(list[i].key == VK_LEFT ||
			list[i].key == VK_RIGHT ||
			list[i].key == VK_UP ||
			list[i].key == VK_DOWN ||
			list[i].key == VK_HOME ||
			list[i].key == VK_END ||
			list[i].key == VK_PRIOR ||
			list[i].key == VK_NEXT ||
			list[i].key == VK_ESCAPE ||
			list[i].key == VK_DELETE ||
			list[i].key == VK_BACK ||
			list[i].key == VK_RETURN ||
			(list[i].key == VK_TAB && !(list[i].fVirt & FCONTROL))) continue;
		add_accel(list[i].cmd, list[i].key, list[i].fVirt);
	}
}

void CAccelList::make_reg_key(int idx, TCHAR *cmd_name, TCHAR *key_name, TCHAR *virt_name)
{
	_stprintf(cmd_name, _T("CMD_%.4d"), idx);
	_stprintf(key_name, _T("KEY_%.4d"), idx);
	_stprintf(virt_name, _T("VIRT_%.4d"), idx);
}

CString CAccelList::GetCommandName(UINT nID)
{
	CString msg;
	msg.LoadString(nID);

	TCHAR *p = msg.GetBuffer(0);
	p = _tcschr(p, '\n');
	if(p == NULL) {
		msg = _T("");
	}  else {
		msg = p + 1;
	}

	return msg;
}

CString CAccelList::GetCommandInfo(UINT nID)
{
	CString msg;
	msg.LoadString(nID);

	TCHAR *p = msg.GetBuffer(256);
	p = _tcschr(p, '\n');
	if(p == NULL) return msg;
	*p = '\0';
	msg.ReleaseBuffer();

	return msg;
}

void CAccelList::make_filterd_accel_list(CAccelList &accel_list, WORD *key_arr, int key_cnt)
{
	free_accel();

	int		i, j;
	ACCEL	*list = accel_list.get_accel_list();
	
	for(i = 0; i < accel_list.get_accel_cnt(); i++) {
		for(j = 0; j < key_cnt; j++) {
			if(list[i].cmd == key_arr[j]) {
				add_accel(list[i].cmd, list[i].key, list[i].fVirt);
			}
		}
	}
}
