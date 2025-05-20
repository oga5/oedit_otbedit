/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "Renderer.h"
#include "get_char.h"

CRenderer::CRenderer()
{
	m_fixed_tab_width = FALSE;
}

CRenderer::~CRenderer()
{
}

//
//U+0000-007F 	Basic Latin 	基本ラテン文字 （ASCII互換）
//U+3040-309F 	Hiragana 	平仮名
//U+30A0-30FF 	Katakana 	片仮名
//U+4E00-9FFF 	CJK Unified Ideographs 	CJK統合漢字
//U+FF00-FFEF 	Halfwidth and Fullwidth Forms 	半角・全角形

static inline int _is_basic_latin(unsigned int ch) {
	return (ch <= 0x0080);
}

static inline int _is_hiragana(unsigned int ch) {
	return (ch >= 0x3040 && ch <= 0x309F);
}

static inline int _is_katakana(unsigned int ch) {
	return (ch >= 0x30A0 && ch <= 0x30FF);
}

static inline int _is_cjk(unsigned int ch) {
	return (ch >= 0x4E00 && ch <= 0x9FFF);
}

static inline int _is_half_and_full(unsigned int ch) {
	return (ch >= 0xFF00 && ch <= 0xFFEF);
}

int CRenderer::TextOut2(CDC *pdc, CDC *p_paintdc, const TCHAR *p, int len, const RECT &rect,
	CFontWidthData *font_width, int row_left, UINT textout_options, 
	int top_offset, int left_offset)
{
	if(rect.left > rect.right) return rect.left;

	int		dx_width[100];
	RECT	rect2 = rect;
	int		top = rect.top + top_offset;
	int		left = rect.left + left_offset;
	int		i;

	// dx_widthの配列の範囲を超えないようにする
	// サロゲートペアがあるため、-1する
	int dx_width_limit = (sizeof(dx_width) / sizeof(dx_width[0])) - 1;

	int tab_width = font_width->GetTabWidth();

	for(; len > 0;) {
		// タブの表示
		if(get_char(p) == '\t') {
			if(m_fixed_tab_width) {
				dx_width[0] = font_width->GetFontWidth(pdc, ' ', NULL) + font_width->GetCharSpaceSetting();
			} else {
				// row_leftはタブ位置を調整するために使用する
				dx_width[0] = tab_width - ((rect2.left - row_left) % tab_width);
			}
			rect2.left = left;
			rect2.right = rect2.left + dx_width[0];
			if(rect2.left < rect.left) {
				rect2.left = rect.left;
			}
			if(rect2.right > rect.right) {
				rect2.right = rect.right;
			}

			pdc->ExtTextOut(left, top, textout_options, &rect2, _T(" "), 1, dx_width);

			p++;
			len--;
			left = rect2.right;
			continue;
		}

		int		tmp_len = len;
		{
			int text_out_width = 0;
			const TCHAR *tmp_p = p;

			for(i = 0; i < tmp_len;) {
				unsigned int ch = get_char(tmp_p);
				// タブの手前まで表示する
				if(ch == '\t') {
					tmp_len = i;
					break;
				}
				if(i >= dx_width_limit) {
					tmp_len = i;
					break;
				}

				dx_width[i] = font_width->GetFontWidth(pdc, ch, NULL);
				dx_width[i] += font_width->GetCharSpace(dx_width[i]);
				text_out_width += dx_width[i];

				if(is_lead_byte(*tmp_p)) {
					dx_width[i + 1] = 0;
					i += 2;
					tmp_p += 2;
				} else {
					i++;
					tmp_p++;
				}
			}

			rect2.left = left;
			rect2.right = rect2.left + text_out_width;
			if(rect2.left < rect.left) {
				rect2.left = rect.left;
			}
			if(rect2.right > rect.right) {
				rect2.right = rect.right;
			}
		}
		if(rect2.right > 0 && p_paintdc->RectVisible(&rect2)) {
			pdc->ExtTextOut(left, top, textout_options, &rect2, p, tmp_len, dx_width);
		}
		p = p + tmp_len;
		len = len - tmp_len;
		left = rect2.right;
	}

	return left;
}