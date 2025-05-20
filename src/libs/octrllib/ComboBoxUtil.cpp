/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#include "stdafx.h"
#include "ComboBoxUtil.h"

#define COMBO_TIMER_ID 1122
#define COMBO_TIMER_ID_FOR_MOUSE_WHEEL 1123

#define COMBO_DELAY_MSEC 200

struct combo_subclass_state {
	WNDPROC	proc;
	TCHAR	buf[50];
	DWORD	tick_count;
	int		last_notice_idx;
	CComboBox *combo;
};

static BOOL Combo_check_alpha_key(int ch)
{
	// ' '(0x20) - '~'(0x7e)
	if(ch >= 0x20 && ch <= 0x7e) return TRUE;
	return FALSE;
}

static BOOL Combo_check_arrow_key(int ch)
{
	if(ch == VK_UP || ch == VK_DOWN || ch == VK_LEFT || ch == VK_RIGHT) return TRUE;
	return FALSE;
}

static void Combo_ClearKeyBuffer(struct combo_subclass_state *state)
{
	state->tick_count = 0;
	_tcscpy(state->buf, _T(""));
}

static void Combo_OnKeyUp(HWND hwnd, struct combo_subclass_state *state, int ch)
{
	CComboBox *combo = state->combo;

	// 英数字入力中は、KeyUpからの待機時間にする
	if(Combo_check_alpha_key(ch) && state->tick_count != 0) {
		state->tick_count = GetTickCount();
	}

	// 矢印キーを押しっぱなしで移動するなど、キー入力が連続して行われる
	// 可能性があるので、KeyUpで通知する
	if(state->last_notice_idx != combo->GetCurSel()) {
		// 英数字や矢印キーを連続入力されたとき、1文字入力ごとに通知しないで遅延させる
		combo->KillTimer(COMBO_TIMER_ID);
		combo->SetTimer(COMBO_TIMER_ID, COMBO_DELAY_MSEC, NULL);
	}
}

static BOOL Combo_OnKeyDown(HWND hwnd, struct combo_subclass_state *state, int ch)
{
	CComboBox *combo = state->combo;
	int item_cnt = combo->GetCount();
	int cur_idx = combo->GetCurSel();
	int new_idx = CB_ERR;

	if(item_cnt == 0) return FALSE;
	if(!Combo_check_arrow_key(ch)) return FALSE;

	if(ch == VK_DOWN || ch == VK_RIGHT) {
		if(cur_idx == CB_ERR) {
			new_idx = 0;
		} else {
			new_idx = cur_idx + 1;
		}
	}
	if(ch == VK_UP || ch == VK_LEFT) {
		if(cur_idx == CB_ERR) {
			new_idx = item_cnt - 1;
		} else {
			new_idx = cur_idx - 1;
		}
	}

	if(new_idx < 0) new_idx = item_cnt - 1;
	if(new_idx >= item_cnt) new_idx = 0;
	if(new_idx != cur_idx) {
		// 矢印で移動したとき、キー入力バッファをクリア
		Combo_ClearKeyBuffer(state);
		combo->SetCurSel(new_idx);
		combo->KillTimer(COMBO_TIMER_ID);
	}

	return TRUE;
}

static BOOL Combo_SearchNext(HWND hwnd, struct combo_subclass_state *state, int ch)
{
	CComboBox *combo = state->combo;
	int item_cnt = combo->GetCount();
	int cur_idx = combo->GetCurSel();
	int idx;
	TCHAR b[2];

	// ' '(0x20) - '~'(0x7e)
	ASSERT(Combo_check_alpha_key(ch));

	if(state->tick_count == 0 || GetTickCount() - state->tick_count > COMBO_DELAY_MSEC) {
		state->last_notice_idx = -1;
		_tcscpy(state->buf, _T(""));
	}
	state->tick_count = GetTickCount();
	_stprintf(b, _T("%c"), tolower(ch));
	_tcscat(state->buf, b);

	size_t find_len = _tcslen(state->buf);
	if(find_len > 20) {
		Combo_ClearKeyBuffer(state);
		return FALSE;
	}

	// Shiftキーが押されている場合、上に向かって検索
	int arrow = 1;
	if(GetAsyncKeyState(VK_SHIFT) < 0) arrow = -1;

	if(cur_idx == CB_ERR) {
		if(arrow > 0) {
			idx = 0;
		} else {
			idx = item_cnt - 1;
		}
	} else {
		if(find_len == 1) {
			if(arrow > 0) {
				idx = cur_idx + 1;
				if(idx >= item_cnt) idx = 0;
			} else {
				idx = cur_idx - 1;
				if(idx < 0) idx = item_cnt - 1;
			}
		} else {
			idx = cur_idx;
		}
	}

	int i;
	CString str;

	for(i = 0; i < item_cnt; i++) {
		combo->GetLBText(idx, str);
		str.MakeLower();
		
		if(_tcsncmp(str, state->buf, find_len) == 0) {
			TRACE(_T("Combo_SearchNext:%s\n"), str);
			combo->SetCurSel(idx);
			combo->KillTimer(COMBO_TIMER_ID);
			state->tick_count = GetTickCount();
			return TRUE;
		}

		idx += arrow;
		if(arrow < 0 && idx < 0) idx = item_cnt - 1;
		if(arrow > 0 && idx >= item_cnt) idx = 0;
		if(idx == cur_idx) break;
	}

	// 同じ文字の連続入力で見つからない場合、先頭文字で一致するか検索
	if(find_len == 2 && state->buf[0] == state->buf[1]) {
		Combo_ClearKeyBuffer(state);
		return Combo_SearchNext(hwnd, state, ch);
	}

	// 見つからないとき
	Combo_ClearKeyBuffer(state);
	return FALSE;
}

static void Combo_NoticeSelChange(HWND hwnd, struct combo_subclass_state *state)
{
	CComboBox *combo = state->combo;

	if(state->last_notice_idx != combo->GetCurSel()) {
		combo->GetParent()->SendMessage(WM_COMMAND, MAKEWPARAM(combo->GetDlgCtrlID(), CBN_SELCHANGE), 
			LPARAM((HWND)combo->GetSafeHwnd()));
		state->last_notice_idx = combo->GetCurSel();
	}
}

static LRESULT CALLBACK Combo_SubclassWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	struct combo_subclass_state *state = (struct combo_subclass_state *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if(state == NULL) return 0;

	WNDPROC proc = state->proc;
	CComboBox *combo = state->combo;

	switch(message) {
	case WM_KEYDOWN:
		if(Combo_OnKeyDown(hwnd, state, (int)wParam)) return 0;
		break;
	case WM_KEYUP:
		Combo_OnKeyUp(hwnd, state, (int)wParam);
		break;
	case WM_CHAR:
		if(Combo_check_alpha_key((int)wParam)) {
			Combo_SearchNext(hwnd, state, (int)wParam);
			return 0;
		}
		break;
	case WM_DESTROY:
		::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
		::SetWindowLongPtr(hwnd, GWLP_USERDATA, NULL);
		free(state);
		break;
	case WM_TIMER:
		if(wParam == COMBO_TIMER_ID || wParam == COMBO_TIMER_ID_FOR_MOUSE_WHEEL) {
			combo->KillTimer(wParam);
			Combo_NoticeSelChange(hwnd, state);
			return 0;
		}
		break;
	case WM_MOUSEWHEEL:
		TRACE(_T("Combo_SubclassWndProc: WM_MOUSEWHEEL"));
		combo->KillTimer(COMBO_TIMER_ID_FOR_MOUSE_WHEEL);
		combo->SetTimer(COMBO_TIMER_ID_FOR_MOUSE_WHEEL, COMBO_DELAY_MSEC, NULL);
		break;
	case WM_GETDLGCODE:
		// 矢印入力を受け取る
		return (CallWindowProc(proc, hwnd, message, wParam, lParam)) | DLGC_WANTARROWS;
	}

	return (CallWindowProc(proc, hwnd, message, wParam, lParam));
}

void ComboBoxKeyboardExtend(CComboBox *combo)
{
	HWND hwnd = combo->GetSafeHwnd();
	struct combo_subclass_state *state = (struct combo_subclass_state *)malloc(sizeof(struct combo_subclass_state));
	if(state == NULL) return;

	state->proc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_WNDPROC);
	_tcscpy(state->buf, _T(""));
	state->tick_count = 0;
	state->combo = combo;
	state->last_notice_idx = combo->GetCurSel();

	::SetWindowLongPtr (hwnd, GWLP_USERDATA, (LONG_PTR)state);
	// ウィンドウプロシージャを切り替える
	::SetWindowLongPtr (hwnd, GWLP_WNDPROC, (LONG_PTR)Combo_SubclassWndProc);
}

int ComboFindString(CComboBox &combo, CString &text)
{
	int		i;
	CString data;
	
	for(i = 0;; i++) {
		if(combo.GetLBTextLen(i) == CB_ERR) return CB_ERR;
		combo.GetLBText(i, data);
		if(data == text) return i;
	}
}

int ComboSelectString(CComboBox &combo, CString &text)
{
	int		idx = ComboFindString(combo, text);
	if(idx == CB_ERR) return CB_ERR;

	combo.SetCurSel(idx);
	return idx;
}

CString ComboGetCurSelString(CComboBox &combo)
{
	int		idx = combo.GetCurSel();
	if(idx == CB_ERR) return _T("");
	
	CString result;
	combo.GetLBText(idx, result);

	return result;
}

