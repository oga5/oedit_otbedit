/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include <stdio.h>
#include <string.h>

#include "StrGridData.h"
#include "ostrutil.h"
#include "mbutil.h"

CStrGridData::CStrGridData()
{
	m_row_cnt = 0;
	m_col_cnt = 0;
	m_str = NULL;
	m_col_name = NULL;
	m_editable_col = NULL;
	m_editable = TRUE;
}

CStrGridData::~CStrGridData()
{
	FreeMem();
}

int CStrGridData::Init(int row, int col, BOOL editable)
{
	if(AllocMem(row, col, editable) != 0) return 1;

	int		i;
	for(i = 0; i < col; i++) {
		SetEditableCell(i, editable);
	}
	SetEditable(editable);

	m_row_cnt = row;
	m_col_cnt = col;

	AllocDispInfo();

	return 0;
}

int CStrGridData::AllocMem(int row, int col, BOOL editable)
{
	FreeMem();

	int str_arr_size = row * col;
	m_str = new CString[str_arr_size];
	if(m_str == NULL) goto ERR1;

	m_col_name = new CString[col];
	if(m_col_name == NULL) goto ERR1;

	m_editable_col = new BOOL[col];
	if(m_editable_col == NULL) goto ERR1;

	return 0;

ERR1:
	FreeMem();
	return 1;
}

void CStrGridData::FreeMem()
{
	if(m_str != NULL) {
		delete[] m_str;
		m_str = NULL;
	}
	if(m_col_name != NULL) {
		delete[] m_col_name;
		m_col_name = NULL;
	}
	if(m_editable_col != NULL) {
		delete[] m_editable_col;
		m_editable_col = NULL;
	}
	m_row_cnt = 0;
	m_col_cnt = 0;
}

void CStrGridData::SetColName(int col, const TCHAR *name)
{
	m_col_name[col] = name;
}

void CStrGridData::SetEditableCell(int col, BOOL editable)
{
	m_editable_col[col] = editable;
}

const TCHAR *CStrGridData::Get_ColName(int col)
{
	return m_col_name[col].GetBuffer(0);
}

int CStrGridData::Get_ColMaxSize(int col)
{
	return 100;
}

const TCHAR *CStrGridData::Get_ColData(int row, int col)
{
	return m_str[GetColIdx(row, col)].GetBuffer(0);
}

BOOL CStrGridData::IsColDataNull(int row, int col)
{
	const TCHAR *p = Get_ColData(row, col);
	if(p == NULL || _tcslen(p) == 0) return TRUE;
	return FALSE;
}

int CStrGridData::UpdateCells(POINT *pos1, POINT *pos2, const TCHAR *data, int len)
{
	int		x, y;

	for(y = pos1->y; y <= pos2->y; y++) {
		for(x = pos1->x; x <= pos2->x; x++) {
			UpdateCell(y, x, data, len);
		}
	}

	return 0;
}

int CStrGridData::UpdateCell(int row, int col, const TCHAR *data, int len)
{
	if(IsEditableCell(row, col) == FALSE) return 1;

	m_str[GetColIdx(row, col)] = data;
	return 0;
}

int CStrGridData::Paste(const TCHAR *pstr)
{
	POINT	pt = *get_cur_cell();

	return UpdateCell(pt.y, pt.x, pstr, 0);
}

void CStrGridData::SetEditable(BOOL editable)
{
	m_editable = editable;
}

BOOL CStrGridData::IsEditableCell(int row, int col)
{
	if(!m_editable) return FALSE;
	return m_editable_col[col];
}
