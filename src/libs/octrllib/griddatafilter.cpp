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

#include "GridDataFilter.h"
#include "EditableGridData.h"
#include "octrl_util.h"
#include "ostrutil.h"
#include "str_inline.h"
#include "mbutil.h"
#include "hankaku.h"

CGridData_Filter::CGridData_Filter()
{
}

CGridData_Filter::~CGridData_Filter()
{
	FreeFilterCond();
}

void CGridData_Filter::FreeFilterCond()
{
	if(m_filter_cond != NULL) {
		free(m_filter_cond);
		m_filter_cond = NULL;
	}
}

griddata_filter_cond* CGridData_Filter::AllocFilterCond(int row_cnt)
{
	FreeFilterCond();

	size_t alloc_size = sizeof(griddata_filter_cond) + sizeof(int) * row_cnt;
	griddata_filter_cond* d = (griddata_filter_cond*)malloc(alloc_size);
	if(d == NULL) return NULL;

	d->data_size = alloc_size;
	d->row_cnt = row_cnt;
	d->row_idx = (int*)((char*)d + sizeof(griddata_filter_cond));

	return d;
}

int CGridData_Filter::DoFilterData(int filter_col_no, const TCHAR* search_text, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp,
	int *find_cnt, CString *msg_str, BOOL b_err_at_no_find)
{
	*find_cnt = 0;
	msg_str->Format(_T(""));

	CString search_text_string = search_text;

	if(m_grid_data == NULL || m_grid_data->Get_RowCnt() == 0) {
		msg_str->Format(_T("データがありません"));
		return 1;
	}

	if(_tcslen(search_text) >= MAX_FILTER_SEARCH_TEXT_LEN) {
		msg_str->Format(_T("検索条件の文字列が長すぎます"));
		return 1;
	}

	CRegData	reg_data;
	if(!reg_data.Compile2(search_text, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp)) {
		msg_str->Format(_T("不正な正規表現です"));
		return 1;
	}

	int *row_idx = (int*)malloc(sizeof(int) * m_grid_data->Get_RowCnt());
	if(row_idx == NULL) {
		msg_str->Format(_T("malloc error (CGridData_Filter::FilterData)"));
		return 1;
	}

	int row_arr_idx = 0;
	int i;
	for(i = 0; i < m_grid_data->Get_RowCnt(); i++) {
		const TCHAR* p = m_grid_data->Get_ColData(i, filter_col_no);

		OREG_DATASRC    data_src;
		oreg_make_str_datasrc(&data_src, p);
		if(oreg_exec2(reg_data.GetRegData(), &data_src) == OREGEXP_FOUND) {
			row_idx[row_arr_idx] = i;
			row_arr_idx++;
		}
	}
	if(b_err_at_no_find && row_arr_idx == 0) {
		msg_str->Format(_T("フィルタの条件に一致するレコードがありません"));
		free(row_idx);
		return 1;
	}

	*find_cnt = row_arr_idx;
	m_row_cnt = row_arr_idx;

	m_filter_cond = AllocFilterCond(m_row_cnt);
	if(m_filter_cond == NULL) {
		msg_str->Format(_T("malloc error (CGridData_Filter::FilterData)"));
		return 1;
	}

	m_filter_cond->filter_col_no = filter_col_no;
	m_filter_cond->b_distinct_lwr_upr = b_distinct_lwr_upr;
	m_filter_cond->b_distinct_width_ascii = b_distinct_width_ascii;
	m_filter_cond->b_regexp = b_regexp;
	_tcscpy(m_filter_cond->search_text, search_text_string.GetBuffer(0));
	memcpy(m_filter_cond->row_idx, row_idx, (sizeof(int) * m_row_cnt));

	free(row_idx);

	m_row_idx = m_filter_cond->row_idx;
	SetFilterRowFlg();

	return 0;
}

int CGridData_Filter::RestoreGridDataFilterCondMain(griddata_filter_cond* d, CString* msg_str)
{
	// UNDO bufferから取り出したとき、row_idxのポインタの位置が正しくないので修正する
	d->row_idx = (int*)((char*)d + sizeof(griddata_filter_cond));

	m_filter_cond = AllocFilterCond(d->row_cnt);
	if(m_filter_cond == NULL) {
		msg_str->Format(_T("malloc error (CGridData_Filter::RestoreGridDataFilterCond)"));
		return 1;
	}

	m_row_cnt = d->row_cnt;

	m_filter_cond->filter_col_no = d->filter_col_no;
	m_filter_cond->b_distinct_lwr_upr = d->b_distinct_lwr_upr;
	m_filter_cond->b_distinct_width_ascii = d->b_distinct_width_ascii;
	m_filter_cond->b_regexp = d->b_regexp;
	_tcscpy(m_filter_cond->search_text, d->search_text);
	memcpy(m_filter_cond->row_idx, d->row_idx, (sizeof(int) * m_row_cnt));

	m_row_idx = m_filter_cond->row_idx;
	SetFilterRowFlg();

	return 0;
}

void CGridData_Filter::SetFilterRowFlg()
{
	if(!m_grid_data->IsSupportFilterFlg()) return;

	int i;
	for(i = 0; i < m_grid_data->Get_RowCnt(); i++) {
		m_grid_data->UnsetFilterFlg(i);
	}
	for(i = 0; i < m_row_cnt; i++) {
		m_grid_data->SetFilterFlg(GetFilterDataRow(i));
	}
}

void CGridData_Filter::RestoreRowIdxFromFilterRowFlg()
{
	if(!m_grid_data->IsSupportFilterFlg()) return;

	int r = 0;
	int i;
	for(i = 0; i < m_grid_data->Get_RowCnt(); i++) {
		if(m_grid_data->IsFilterRow(i)) {
			m_row_idx[r] = i;
			r++;
		}
	}
}

void CGridData_Filter::UpdateCurCell()
{
	// データ上の行番号から、フィルタ中の行番号にする
	// FIXME: バイナリサーチにする
	int r;
	int disp_row = 0;
	int data_row = m_grid_data->get_cur_row();
	for(r = 0; r < m_row_cnt; r++) {
		if(m_row_idx[r] == data_row) {
			disp_row = r;
			break;
		}
	}
	m_selected_cell.y = disp_row;
	m_selected_cell.x = m_grid_data->get_cur_col();
}

void CGridData_Filter::SetCurCell()
{
	if(m_selected_cell.x >= 0 && m_selected_cell.y >= 0)
		m_grid_data->set_cur_cell(GetFilterDataRow(m_selected_cell.y), m_selected_cell.x);
}

int CGridData_Filter::Paste(const TCHAR* pstr)
{
	return CEditableGridData::PasteCommon(this, pstr);
}

int CGridData_Filter::UpdateCells(POINT* pos1, POINT* pos2, const TCHAR* data, int len)
{
	return CEditableGridData::UpdateCellsCommon(this, pos1, pos2, data, len);
};

void CGridData_Filter::SortData(int* col_no, int* order, int sort_col_cnt)
{
	m_grid_data->SortData(col_no, order, sort_col_cnt);

	if(m_grid_data->IsSupportFilterFlg()) {
		RestoreRowIdxFromFilterRowFlg();
	} else {
		CString msg_str;
		int find_cnt;
		DoFilterData(m_filter_cond->filter_col_no, m_filter_cond->search_text, m_filter_cond->b_distinct_lwr_upr,
			m_filter_cond->b_distinct_width_ascii, m_filter_cond->b_regexp, &find_cnt, &msg_str, FALSE);
	}
}

const TCHAR* CGridData_Filter::GetRowHeader(int row)
{
	if(IsUpdateRow(row)) {
		_stprintf(m_row_header_buf, _T("*%6d"), row + 1);
	} else if(IsInsertRow(row)) {
		_stprintf(m_row_header_buf, _T("+%6d"), row + 1);
	} else if(IsDeleteRow(row)) {
		_stprintf(m_row_header_buf, _T("-%6d"), row + 1);
	} else {
		_stprintf(m_row_header_buf, _T("%7d"), row + 1);
	}

	return m_row_header_buf;
}
