/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef _FONT_WIDTH_H_INCLUDE
#define _FONT_WIDTH_H_INCLUDE

#include "get_char.h"

// Uniscribeのテスト
//#define USE_UNISCRIBE	1


class CFontWidthHandler {
private:
	CWnd	*m_wnd;
	CDC		*m_pdc;
	CFont	*m_font;
	CFont	*m_pOldFont;

public:
	CFontWidthHandler(CWnd *wnd, CFont *font) : 
		m_wnd(wnd), m_pdc(NULL), m_font(font), m_pOldFont(NULL) { }
	~CFontWidthHandler() {
		if(m_pdc) {
			m_pdc->SelectObject(m_pOldFont);
			m_wnd->ReleaseDC(m_pdc);
		}
	}
	CDC *GetDC() {
		if(!m_pdc) {
			m_pdc = m_wnd->GetDC();
			m_pOldFont = m_pdc->SelectObject(m_font);
		}
		return m_pdc;
	}
};

class CFontWidthData
{
private:
	int			*m_font_width;
	int			m_tabstop;
	int			m_char_space;
	int			m_full_char_width;

	int			m_full_char_adjust_width;
	BOOL		m_fixed_pitch_adjust;

#ifdef _DEBUG
	BOOL		m_init_flg;
#endif

#ifdef USE_UNISCRIBE
	static int GetUniscribeFontWidth(CDC *pdc, unsigned int ch, CFontWidthHandler *pdchandler);
#else
	static int GetFontWidthMain(CDC *pdc, unsigned int ch, CFontWidthHandler *pdchandler);
#endif

public:
	CFontWidthData();
	~CFontWidthData();

	void InitFontWidth(CDC *pdc);
	int GetFontWidth(CDC *pdc, unsigned int ch, CFontWidthHandler *pdchandler);
	int GetTabWidth() { 
		if(m_font_width == NULL) return 0;
		return m_font_width[L'x'] * m_tabstop + m_char_space * m_tabstop;
	}

	void SetTabStop(int tabstop) { m_tabstop = tabstop; }
	int GetTabStop() { return m_tabstop; }

	void SetCharSpaceSetting(int char_space) { m_char_space = char_space; }
	int GetCharSpaceSetting() { return m_char_space; }

	int GetCharSpace(int w) { 
		if(w >= m_full_char_width) {
			return GetCharSpaceSetting() * 2;
		} else {
			return GetCharSpaceSetting();
		}
	}
	BOOL IsFullWidthChar(CDC *pdc, unsigned int ch, CFontWidthHandler *pdchandler) {
		return (GetFontWidth(pdc, ch, pdchandler) >= m_full_char_width);
	}
};

__inline int CFontWidthData::GetFontWidth(CDC *pdc, unsigned int ch, CFontWidthHandler *pdchandler)
{
#ifdef _DEBUG
	ASSERT(m_init_flg);
#endif

#ifdef USE_UNISCRIBE
	if(get_char_len2(ch) == 2) {
		return GetUniscribeFontWidth(pdc, ch, pdchandler);
	}
	if(m_font_width[ch] < 0) {
		m_font_width[ch] = GetUniscribeFontWidth(pdc, ch, pdchandler);
	}
	return m_font_width[ch];
#else
	if(get_char_len2(ch) == 2) {
		return GetFontWidthMain(pdc, ch, pdchandler);
	}
	if(m_font_width[ch] < 0) {
		m_font_width[ch] = GetFontWidthMain(pdc, ch, pdchandler);
		if(m_fixed_pitch_adjust && m_font_width[ch] == m_full_char_adjust_width) {
			m_font_width[ch] = m_font_width[L'x'] * 2;
		}
	}
	return m_font_width[ch];
#endif
}


#endif _FONT_WIDTH_H_INCLUDE
