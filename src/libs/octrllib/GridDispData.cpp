/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "StdAfx.h"
#include "GridDispData.h"
#include "ColorUtil.h"

CGridDispData::CGridDispData()
{
	m_color[GRID_BG_COLOR] = RGB(255, 255, 255);
	m_color[GRID_HEADER_BG_COLOR] = RGB(230, 230, 255);
	m_color[GRID_LINE_COLOR] = RGB(200, 200, 250);
	m_color[GRID_SELECT_COLOR] = RGB(200, 200, 220);
	m_color[GRID_DELETE_COLOR] = RGB(150, 150, 150);
	m_color[GRID_INSERT_COLOR] = RGB(220, 220, 255);
	m_color[GRID_UPDATE_COLOR] = RGB(255, 220, 220);
	m_color[GRID_UNEDITABLE_COLOR] = RGB(240, 240, 240);
	m_color[GRID_TEXT_COLOR] = RGB(0, 0, 0);
	m_color[GRID_NULL_CELL_COLOR] = RGB(240, 240, 240);
	m_color[GRID_MARK_COLOR] = RGB(0, 150, 150);
	m_color[GRID_SELECTED_CELL_LINE_COLOR] = RGB(0, 0, 0);
	m_color[GRID_NO_DISP_CELL_LINE_COLOR] = RGB(120, 120, 120);
	m_color[GRID_SEARCH_COLOR] = RGB(255, 255, 50);
	m_color[GRID_HEADER_LINE_COLOR] = RGB(100, 100, 100);
	m_color[GRID_CUR_ROW_TEXT_COLOR] = RGB(0, 0, 0);

	m_topleft_cell_color = INVALID_COLOR;

	m_tool_tip.Create();

	m_renderer.SetFixedTabWidth(TRUE);
}

CGridDispData::~CGridDispData()
{
	m_tool_tip.Destroy();
}

void CGridDispData::SetColor(int type, COLORREF color)
{
	if(m_color[type] == color) return;

	m_color[type] = color;
}

COLORREF CGridDispData::GetTopLeftCellColor()
{
	if(m_topleft_cell_color == INVALID_COLOR) {
		return GetColor(GRID_BG_COLOR);
	}

	return m_topleft_cell_color;
}


