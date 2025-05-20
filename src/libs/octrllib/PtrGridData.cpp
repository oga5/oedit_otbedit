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

#include "PtrGridData.h"
#include "ostrutil.h"
#include "mbutil.h"

#define MAX_ROW_HEADER_SIZE		10

CPtrGridData::CPtrGridData()
{
	m_row_cnt = 0;
	m_col_cnt = 0;
	m_alloc_row_cnt = 0;
	m_data = NULL;
	m_col_name = NULL;
	m_init_row_cnt = 0;
}

CPtrGridData::~CPtrGridData()
{
	FreeData();
}

BOOL CPtrGridData::AllocData(int row, int col)
{
	m_data = (const TCHAR **)realloc(m_data, sizeof(TCHAR *) * row * col);
	if(m_data == NULL) return FALSE;

	m_alloc_row_cnt = row;

	return TRUE;
}

void CPtrGridData::FreeData()
{
	if(m_data != NULL) {
		free(m_data);
		m_data = 0;
	}
	if(m_col_name != NULL) {
		free(m_col_name);
		m_col_name = 0;
	}
	m_row_cnt = 0;
	m_col_cnt = 0;
}

int CPtrGridData::Init(int col, int init_row_cnt)
{
	static const TCHAR *default_col_name = _T("test");

	FreeData();
	if(AllocData(init_row_cnt, col) == FALSE) return 1;

	m_col_name = (const TCHAR **)malloc(sizeof(TCHAR *) * col);
	if(m_col_name == NULL) return 1;

	for(int c = 0; c < col; c++) {
		m_col_name[c] = default_col_name;
	}

	m_col_cnt = col;
	m_row_cnt = 0;
	m_init_row_cnt = init_row_cnt;

	AllocDispInfo();

	return 0;
}

void CPtrGridData::SetData(int row, int col, const TCHAR *data)
{
	m_data[GetColIdx(row, col)] = data;
}

int CPtrGridData::DeleteAllRow()
{
	m_row_cnt = 0;
	return 0;
}

int CPtrGridData::AddRow()
{
	if(m_alloc_row_cnt == m_row_cnt) {
		if(AllocData(m_row_cnt * 2, m_col_cnt) == FALSE) return 1;
	}
	InitRow(m_row_cnt);
	m_row_cnt++;

	return 0;
}

void CPtrGridData::InitRow(int row)
{
	static const TCHAR *null_str = _T("");

	for(int c = 0; c < m_col_cnt; c++) {
		SetData(row, c, null_str);
	}
}

const TCHAR *CPtrGridData::Get_ColData(int row, int col)
{
	return m_data[GetColIdx(row, col)];
}

/*----------------------------------------------------------------------
  キーワードをクイックソートする(行の入れ替え)
----------------------------------------------------------------------*/
__inline void CPtrGridData::QSortSwap(int r1, int r2) 
{
	const TCHAR *tmp;

	for(int c = 0; c < m_col_cnt; c++) {
		tmp = Get_ColData(r1, c);
		SetData(r1, c, Get_ColData(r2, c));
		SetData(r2, c, tmp);
	}
}

/*----------------------------------------------------------------------
  キーワードをクイックソートする(メインの再帰関数)
  昇順ソート
----------------------------------------------------------------------*/
void CPtrGridData::QSort(int col, int left, int right)
{
	int		i, last;

	if(left >= right) return;

	QSortSwap(left, (left + right) / 2);

	last = left;
	for(i = left + 1; i <= right; i++) {
		if(ostr_strcmp_nocase(Get_ColData(i, col), Get_ColData(left, col)) < 0) {
			QSortSwap(++last, i);
		}
	}
	QSortSwap(left, last);

	QSort(col, left, last - 1);
	QSort(col, last + 1, right);
}


void CPtrGridData::Sort(int sort_col)
{
	QSort(sort_col, 0, Get_RowCnt() - 1);
}




CPtrColorGridData::CPtrColorGridData()
{
	m_color_data_bk = NULL;
	m_color_data_text = NULL;

	m_row_header = NULL;
}

CPtrColorGridData::~CPtrColorGridData()
{
	FreeData();
}

BOOL CPtrColorGridData::AllocData(int row, int col)
{
	if(!CPtrGridData::AllocData(row, col)) return FALSE;

	m_color_data_bk = (COLORREF *)realloc(m_color_data_bk, sizeof(COLORREF) * row * col);
	if(m_color_data_bk == NULL) return FALSE;
	m_color_data_text = (COLORREF *)realloc(m_color_data_text, sizeof(COLORREF) * row * col);
	if(m_color_data_text == NULL) return FALSE;

	m_row_header = (TCHAR **)realloc(m_row_header, (sizeof(TCHAR *) * MAX_ROW_HEADER_SIZE) * row * col);
	if(m_color_data_text == NULL) return FALSE;

	return TRUE;
}

void CPtrColorGridData::FreeData()
{
	CPtrGridData::FreeData();

	if(m_color_data_bk != NULL) {
		free(m_color_data_bk);
		m_color_data_bk = NULL;
	}
	if(m_color_data_text != NULL) {
		free(m_color_data_text);
		m_color_data_text = NULL;
	}
	if(m_row_header != NULL) {
		free(m_row_header);
		m_row_header = NULL;
	}
}

TCHAR *CPtrColorGridData::GetRowHeaderBuf(int row)
{
	int buff_offset = row * MAX_ROW_HEADER_SIZE;
	return m_row_header[0] + buff_offset;
}

const TCHAR *CPtrColorGridData::GetRowHeader(int row)
{
	_tcscpy(m_row_header_buf, GetRowHeaderBuf(row));
	return m_row_header_buf;
}

void CPtrColorGridData::SetRowHeader(int row, const TCHAR *text)
{
	if(_tcslen(text) >= MAX_ROW_HEADER_SIZE) {
		_tcscpy(m_row_header[row], _T(""));
		return;
	}
	_tcscpy(GetRowHeaderBuf(row), text);
}

void CPtrColorGridData::SetRowHeader(int row, int row_cnt)
{
	TCHAR	buf[20];
	_stprintf(buf, _T("%6d"), row_cnt + 1);
	SetRowHeader(row, buf);
}

void CPtrColorGridData::InitRow(int row)
{
	static const TCHAR *null_str = _T("");

	SetRowHeader(row, row + 1);
	for(int c = 0; c < m_col_cnt; c++) {
		SetData(row, c, (TCHAR *)null_str, RGB(0xff, 0xff, 0xff), RGB(0x00, 0x00, 0x00));
	}
}

void CPtrColorGridData::SetColor(int row, int col, COLORREF bk_color, COLORREF text_color)
{
	m_color_data_bk[GetColIdx(row, col)] = bk_color;
	m_color_data_text[GetColIdx(row, col)] = text_color;
}

void CPtrColorGridData::SetData(int row, int col, TCHAR *data, COLORREF bk_color, COLORREF text_color)
{
	CPtrGridData::SetData(row, col, data);
	SetColor(row, col, bk_color, text_color);
}

COLORREF CPtrColorGridData::GetCellBkColor(int row, int col)
{
	return m_color_data_bk[GetColIdx(row, col)];
}

COLORREF CPtrColorGridData::GetCellTextColor(int row, int col)
{
	return m_color_data_text[GetColIdx(row, col)];
}

