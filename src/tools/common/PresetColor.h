﻿/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 /*
 Solarized was created by Ethan Schoonover and you can find out more about 
 the design work that went into it on his project page. 
 https://ethanschoonover.com/solarized/
 */

#ifndef PRESET_COLOR_H_INCLUDED
#define PRESET_COLOR_H_INCLUDED

// 色プリセット構造体
struct EditorColorPreset {
	LPCTSTR name;
	COLORREF colors[EDIT_CTRL_COLOR_CNT];
};

// プリセット例（必要に応じて追加）
static const EditorColorPreset g_color_presets[] = {
	{
		_T("default"), {
			RGB(0x00, 0x00, 0x00), RGB(0x00, 0x00, 0xcd), RGB(0x00, 0x78, 0x00), RGB(0xff, 0xff, 0xff),
			RGB(0x00, 0x96, 0x96), RGB(0x00, 0x00, 0xdc), RGB(0xc8, 0xc8, 0xc8), RGB(0x00, 0x00, 0x32),
			RGB(0x00, 0x64, 0x00), RGB(0x00, 0x00, 0x80), RGB(0x00, 0x80, 0xc0), RGB(0x00, 0xa0, 0x00),
			RGB(0xb4, 0xb4, 0xb4), RGB(0x00, 0x00, 0x00), RGB(0x00, 0x00, 0x00), RGB(0x00, 0x00, 0x00),
		}
	},
	{
		_T("solarized-light"), {
			RGB(0x58, 0x6e, 0x75), RGB(0x26, 0x8b, 0xd2), RGB(0x2a, 0xa1, 0x98), RGB(0xff, 0xfd, 0xf9),
			RGB(0xcb, 0x4b, 0x16), RGB(0xdc, 0x32, 0x2f), RGB(0xc8, 0xc8, 0xc8), RGB(0x07, 0x36, 0x42),
			RGB(0x85, 0x9a, 0x00), RGB(0xd3, 0x36, 0x82), RGB(0x00, 0x80, 0xc0), RGB(0x00, 0xa0, 0x00),
			RGB(0xb4, 0xb4, 0xb4), RGB(0x00, 0x00, 0x00), RGB(0x00, 0x00, 0x00), RGB(0x00, 0x00, 0x00),
		}
	},
	{
		_T("solarized-dark"), {
			RGB(0xfd, 0xf6, 0xe3), RGB(0x26, 0x8b, 0xd2), RGB(0x2a, 0xa1, 0x98), RGB(0x07, 0x36, 0x42),
			RGB(0xcb, 0x4b, 0x16), RGB(0xdc, 0x32, 0x2f), RGB(0xc8, 0xc8, 0xc8), RGB(0xee, 0xe8, 0xd5),
			RGB(0x85, 0x9a, 0x00), RGB(0xd3, 0x36, 0x82), RGB(0x00, 0x80, 0xc0), RGB(0x00, 0xa0, 0x00),
			RGB(0xb4, 0xb4, 0xb4), RGB(0x00, 0x00, 0x00), RGB(0x00, 0x00, 0x00), RGB(0x00, 0x00, 0x00),
		}
	},
	// 他のプリセットもここに追加
};
static const int g_color_preset_count = sizeof(g_color_presets) / sizeof(g_color_presets[0]);

#endif // PRESET_COLOR_H_INCLUDED
