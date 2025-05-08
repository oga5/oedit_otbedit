/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef _RENDERER_H_INCLUDE
#define _RENDERER_H_INCLUDE

#include "FontWidth.h"

class CRenderer
{
private:
	BOOL m_fixed_tab_width;

public:
	CRenderer();
	~CRenderer();

	void SetFixedTabWidth(BOOL b) { m_fixed_tab_width = b; }

	int TextOut2(CDC *pdc, CDC *p_paintdc, const TCHAR *p, int len, const RECT &rect,
		CFontWidthData *font_width, int row_left, UINT textout_options = ETO_OPAQUE,
		int top_offset = 0, int left_offset = 0);
};

#endif _RENDERER_H_INCLUDE
