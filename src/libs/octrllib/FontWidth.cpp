/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "FontWidth.h"
#include "get_char.h"

#ifdef USE_UNISCRIBE
#include <usp10.h>
#pragma comment(lib, "usp10.lib")
#endif

//@{ ゼロ埋め作業 //@}
inline void mem00( void* ptrv, int siz )
	{ BYTE* ptr = (BYTE*)ptrv;
	  for(;siz>3;siz-=4,ptr+=4) *(DWORD*)ptr = 0x00000000;
	  for(;siz;--siz,++ptr) *ptr = 0x00; }

//@{ FF埋め作業 //@}
inline void memFF( void* ptrv, int siz )
	{ BYTE* ptr = (BYTE*)ptrv;
	  for(;siz>3;siz-=4,ptr+=4) *(DWORD*)ptr = 0xffffffff;
	  for(;siz;--siz,++ptr) *ptr = 0xff; }

void CFontWidthData::InitFontWidth(CDC *pdc)
{
#ifdef _DEBUG
	m_init_flg = TRUE;
#endif

	if(m_font_width == NULL) {
		m_font_width = new int[65536];
	}

	// 文字幅テーブル初期化（ASCII範囲の文字以外は遅延処理）
	// 0xffで初期化
	memFF( m_font_width, 65536 * sizeof(int) );
	// 下位サロゲートは文字幅ゼロ
	mem00( m_font_width + 0xDC00, (0xE000 - 0xDC00) * sizeof(int) );

	// ASCIIデータを初期化
	m_font_width[L'\0'] = 0;
	GetCharWidth32( pdc->GetSafeHdc(), L' ', L'~', m_font_width + L' ' );

	// Tab幅を設定
	m_font_width[L'\t'] = m_font_width[L'x'];

	TEXTMETRIC tm;
	pdc->GetTextMetrics(&tm);

	m_fixed_pitch_adjust = FALSE;

	m_full_char_width = GetFontWidth(pdc, L'あ', NULL);

	// TMPF_FIXED_PITCHがセットされていると、固定ピッチではない
	// (名前と意味が逆になってるので注意)
	if(!(tm.tmPitchAndFamily & TMPF_FIXED_PITCH)) {
		// 固定ピッチのときの処理
		//
		// Windows2000で固定ピッチフォントで、全角文字の幅が半角文字の2倍にならない
		// ケースを回避
		// http://support.microsoft.com/kb/417434/ja
		m_full_char_adjust_width = (m_font_width[L'x'] * 2) - 1;
		if(m_full_char_width == m_full_char_adjust_width) {
			m_fixed_pitch_adjust = TRUE;
			m_font_width[L'あ'] = -1;
		}
	}
}

#ifdef USE_UNISCRIBE
int CFontWidthData::GetUniscribeFontWidth(CDC *pdc, unsigned int ch, CFontWidthHandler *pdchandler)
{
	if(pdc == NULL) {
		if(pdchandler == NULL) {
			ASSERT(0);
			return 0;
		}
		pdc = pdchandler->GetDC();
	}
	HDC hdc = pdc->GetSafeHdc();

	WCHAR	buf[3];
	WCHAR	*p;
	p = put_char(buf, ch);
	*p = L'\0';

	SCRIPT_CONTROL			scriptControl = { 0 };
	SCRIPT_STATE			scriptState   = { 0 };
	SCRIPT_STRING_ANALYSIS	ssa;
	int		width[2] = { 0, 0 };
	int		wlen = get_char_len2(ch);

	HRESULT r = ScriptStringAnalyse(hdc, buf, wlen, wlen * 2,
		-1, SSA_GLYPHS, 0, &scriptControl, &scriptState, 0, 0, 0, &ssa);
	HRESULT r2 = ScriptStringGetLogicalWidths(ssa, width);
	HRESULT r3 = ScriptStringFree(&ssa);

	if(wlen == 2) return width[0] + width[1];
	return width[0];
}
#else
int CFontWidthData::GetFontWidthMain(CDC *pdc, unsigned int ch, CFontWidthHandler *pdchandler)
{
	if(pdc == NULL) {
		if(pdchandler == NULL) {
			ASSERT(0);
			return 0;
		}
		pdc = pdchandler->GetDC();
	}
	HDC hdc = pdc->GetSafeHdc();

	WCHAR	buf[3];
	WCHAR	*p;
	p = put_char(buf, ch);
	*p = L'\0';
	SIZE	sz[1];
	int		wlen = get_char_len2(ch);

	GetTextExtentPoint32(hdc, buf, wlen, sz);

	return sz[0].cx;
}
#endif

CFontWidthData::CFontWidthData()
{
	m_char_space = 0;
	m_tabstop = 4;
	m_font_width = NULL;
	m_fixed_pitch_adjust = FALSE;
	m_full_char_adjust_width = 0;
	m_full_char_width = 0;

#ifdef _DEBUG
	m_init_flg = FALSE;
#endif
}

CFontWidthData::~CFontWidthData()
{
	if(m_font_width) {
		delete m_font_width;
	}
}
