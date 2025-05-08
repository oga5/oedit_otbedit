/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"

#include <stdio.h>
#include "UnicodeArchive.h"
#include "kbmacro.h"


struct event_list {
	EVENTMSG			msg;
	struct event_list	*next;
};

struct event_list *g_start = NULL;
struct event_list *g_cur = NULL;

BOOL g_delete_at_end_play = FALSE;
struct event_list *g_saved_start = NULL;


int rec_status = HC_ACTION;
int play_status = HC_GETNEXT;

HHOOK rec_hook = NULL;
HHOOK play_hook = NULL;

void ClearKBMacroBuffer()
{
	if(rec_hook != NULL) RecEndKBMacro();
	struct event_list *next;
	struct event_list *cur;
	
	for(cur = g_start; cur != NULL;) {
		next = cur->next;
		free(cur);
		cur = next;
	}

	g_start = NULL;
	g_cur = NULL;
}

BOOL InitKBMacroBuffer()
{
	if(g_start != NULL) ClearKBMacroBuffer();

	g_start = (struct event_list *)malloc(sizeof(struct event_list));
	if(g_start == NULL) return FALSE;
	g_start->next = NULL;
	g_cur = g_start;

	rec_status = HC_ACTION;

	return TRUE;
}

static struct event_list *add_event_list(struct event_list *cur, 
	EVENTMSG *msg)
{
	cur->msg = *msg;
	cur->next = (struct event_list *)malloc(sizeof(struct event_list));
	if(cur->next == NULL) {
		return NULL;
	}
	cur = cur->next;
	cur->next = NULL;

	return cur;
}

LRESULT CALLBACK JournalRecordProc(int code,// フックコード
	WPARAM wParam,// 未定義
	LPARAM lParam// 処理されたメッセージへのポインタ
)
{
	EVENTMSG	*msg = (EVENTMSG *)lParam;

	if(code == HC_SYSMODALOFF) rec_status = HC_SYSMODALOFF;
	if(code == HC_SYSMODALON) rec_status = HC_SYSMODALON;

	if(rec_status != HC_SYSMODALON && code == HC_ACTION) {
		if(g_cur == NULL) goto EXIT;

		// マウスメッセージは保存しない
		if(msg->message < WM_KEYFIRST || msg->message > WM_KEYLAST) goto EXIT;

		g_cur = add_event_list(g_cur, msg);
		if(g_cur == NULL) {
			// FIXME: エラーメッセージ
			goto EXIT;
		}
	}

EXIT:
	return CallNextHookEx(rec_hook, code, wParam, lParam);
}

//const int WH_KEYBOARD_LL = 13;
//#define WH_KEYBOARD_LL 13
LRESULT CALLBACK LowLevelKeyboardProc(
	int nCode,// フックコード
	WPARAM wParam,// メッセージ
	LPARAM lParam// メッセージデータの構造体へのポインタ
)
{
	TRACE(_T("%d:%d:%d\n"), nCode, wParam, lParam);
	return CallNextHookEx(rec_hook, nCode, wParam, lParam);
}

BOOL RecStartKBMacro()
{
	if(InitKBMacroBuffer() == FALSE) return FALSE;

	// Ctrlキー，ShiftキーをOFFにする
	if(GetAsyncKeyState(VK_CONTROL) < 0 || GetAsyncKeyState(VK_SHIFT) < 0) {
		BYTE	key_state[256];
		GetKeyboardState(key_state);
		key_state[VK_CONTROL] = 0;
		key_state[VK_SHIFT] = 0;
		SetKeyboardState(key_state);
	}

//	rec_hook = SetWindowsHookEx(WH_JOURNALRECORD, JournalRecordProc,
//		::AfxGetInstanceHandle(), NULL);
	rec_hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc,
		::AfxGetInstanceHandle(), NULL);
	if(rec_hook == NULL) return FALSE;

	return TRUE;
}

BOOL RecEndKBMacro()
{
	if(rec_hook == NULL) return FALSE;

	BOOL ret = UnhookWindowsHookEx(rec_hook);
	rec_hook = NULL;
	return ret;
}

void PlayEndKBMacro()
{
	if(play_hook == NULL) return;

	UnhookWindowsHookEx(play_hook);
	play_hook = NULL;

	if(g_delete_at_end_play == TRUE) {
		ClearKBMacroBuffer();
		g_start = g_saved_start;
		g_delete_at_end_play = FALSE;
		g_saved_start = NULL;
	}
}

LRESULT CALLBACK JournalPlaybackProc(
	int code,// フックコード
	WPARAM wParam,// 未定義
	LPARAM lParam// 処理されるメッセージへのポインタ
)
{
	if(code == HC_SYSMODALOFF) play_status = HC_SYSMODALOFF;
	if(code == HC_SYSMODALON) play_status = HC_SYSMODALON;

	if(play_status != HC_SYSMODALON && code == HC_GETNEXT) {
		EVENTMSG	*msg = (EVENTMSG *)lParam;
		*msg = g_cur->msg;
		if(msg->message == WM_KEYDOWN || msg->message == WM_KEYUP) {
			msg->paramL = LOBYTE(msg->paramL);
		}
	}
	if(code == HC_SKIP) {
		g_cur = g_cur->next;
		if(g_cur == NULL) {
			PlayEndKBMacro();
		}
	}

	return CallNextHookEx(play_hook, code, wParam, lParam);
}

void PlayKBMacro()
{
	if(g_start == NULL) return;
	if(rec_hook != NULL) RecEndKBMacro();
	if(play_hook != NULL) PlayEndKBMacro();

	// Ctrlキー，ShiftキーをOFFにする
	BYTE	key_state[256];
	GetKeyboardState(key_state);
	key_state[VK_CONTROL] = 0;
	key_state[VK_SHIFT] = 0;
	SetKeyboardState(key_state);

	play_status = HC_GETNEXT;
	g_cur = g_start;

	play_hook = SetWindowsHookEx(WH_JOURNALPLAYBACK, JournalPlaybackProc,
		::AfxGetInstanceHandle(), NULL);
}

BOOL IsPlaingKBMacro()
{
	if(play_hook == NULL) return FALSE;
	return TRUE;
}

BOOL IsRecodingKBMacro()
{
	if(rec_hook == NULL) return FALSE;
	return TRUE;
}

BOOL CanPlayKBMacro()
{
	if(IsRecodingKBMacro() || IsPlaingKBMacro() || g_start == NULL) return FALSE;
	return TRUE;
}

#define KB_MACRO_FILE_HEADER _T("// oedit keyboard macro file\n")

BOOL SaveKBMacro(TCHAR *file_name)
{
	if(g_start == NULL) return FALSE;
	FILE *fp = NULL;

	fp = _tfopen(file_name, _T("wb"));
	if(fp == NULL) {
		// FIXME: エラーメッセージ
		return FALSE;
	}

	_ftprintf(fp, KB_MACRO_FILE_HEADER);
	_ftprintf(fp, _T("// DO NOT EDIT THIS FILE\n"));

	struct event_list *cur;
	for(cur = g_start; cur != NULL; cur = cur->next) {
		_ftprintf(fp, _T("%u:%u:%u:%u\n"), cur->msg.message, cur->msg.paramH, 
			cur->msg.paramL, cur->msg.time);
	}

	fclose(fp);

	return TRUE;
}

BOOL LoadKBMacro(TCHAR *file_name)
{
	if(InitKBMacroBuffer() == FALSE) return FALSE;

	CUnicodeArchive uni_ar;
	if(!uni_ar.OpenFile(file_name, _T("r"))) return FALSE;

	EVENTMSG	msg;
	TCHAR		buf[1024];
	struct event_list *cur = g_start;

	if(uni_ar.ReadLine(buf, sizeof(buf)) == NULL || _tcsncmp(buf, KB_MACRO_FILE_HEADER, _tcslen(KB_MACRO_FILE_HEADER) - 1) != 0) {
		ClearKBMacroBuffer();
		return FALSE;
	}

	for(;;) {
		if(uni_ar.ReadLine(buf, sizeof(buf)) == NULL) break;
		
		if(buf[0] == '/' && buf[1] == '/') continue;	// コメント行

		_stscanf(buf, _T("%u:%u:%u:%u"), &(msg.message), &(msg.paramH), 
			&(msg.paramL), &(msg.time));

		cur = add_event_list(cur, &msg);
		if(cur == NULL) {
			ClearKBMacroBuffer();
			return FALSE;
		}
	}

	return TRUE;
}

BOOL PlayKBMacro(TCHAR *file_name)
{
	g_delete_at_end_play = TRUE;
	g_saved_start = g_start;
	g_start = NULL;

	if(LoadKBMacro(file_name) == FALSE) {
		g_start = g_saved_start;
		g_delete_at_end_play = FALSE;
		g_saved_start = NULL;
		return FALSE;
	}

	PlayKBMacro();

	return TRUE;
}
