/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef __GRID_DISP_DATA_H_INCLUDED__
#define __GRID_DISP_DATA_H_INCLUDED__

enum {
	GRID_BG_COLOR,
	GRID_HEADER_BG_COLOR,
	GRID_LINE_COLOR,
	GRID_SELECT_COLOR,
	GRID_DELETE_COLOR,
	GRID_INSERT_COLOR,
	GRID_UPDATE_COLOR,
	GRID_UNEDITABLE_COLOR,
	GRID_TEXT_COLOR,
	GRID_NULL_CELL_COLOR,
	GRID_MARK_COLOR,
	GRID_HEADER_LINE_COLOR,
	GRID_SELECTED_CELL_LINE_COLOR,
	GRID_NO_DISP_CELL_LINE_COLOR,
	GRID_SEARCH_COLOR,
	GRID_CUR_ROW_TEXT_COLOR,
	GRID_CTRL_COLOR_CNT,
};

#include "ToolTipEx.h"
#include "FontWidth.h"
#include "Renderer.h"
#include "ColorUtil.h"

class CGridDispData
{
public:
	CGridDispData();
	~CGridDispData();

	void SetColor(int type, COLORREF color);
	COLORREF GetColor(int type) { return m_color[type]; }

	CToolTipEx *GetToolTip() { return &m_tool_tip; }

	void SetCharSpaceSetting(int char_space) { m_font_width_data.SetCharSpaceSetting(char_space); }

	void InitFontWidth(CDC *pdc) { m_font_width_data.InitFontWidth(pdc); }
	int GetFontWidth(CDC *pdc, unsigned int ch, CFontWidthHandler *pdchandler) { 
		return m_font_width_data.GetFontWidth(pdc, ch, pdchandler);
	}
	int GetCharSpace(int w) { return m_font_width_data.GetCharSpace(w); }
	int GetTabWidth() { return 0; }

	CFontWidthData *GetFontData() { return &m_font_width_data; }
	CRenderer *GetRenderer() { return &m_renderer; }

	COLORREF GetTopLeftCellColor();
	void SetTopLeftCellColor(COLORREF cr) { m_topleft_cell_color = cr; }

private:
	COLORREF	m_color[GRID_CTRL_COLOR_CNT];
	COLORREF	m_topleft_cell_color;

	CToolTipEx	m_tool_tip;

	CFontWidthData	m_font_width_data;
	CRenderer		m_renderer;
};

#endif __GRID_DISP_DATA_H_INCLUDED__
