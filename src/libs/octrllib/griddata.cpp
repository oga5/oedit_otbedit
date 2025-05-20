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

#include "GridData.h"
#include "GridDataFilter.h"
#include "ostrutil.h"
#include "str_inline.h"
#include "mbutil.h"
#include "hankaku.h"

#include <float.h>

#define GRID_DEFAULT_COL_WIDTH	100

static CString FormatValue(double v, int max_scale, const TCHAR *str)
{
	CString result;
	CString tmp;

	if(str != NULL) {
		tmp = str;
	} else {
		tmp.Format(_T("%.*f"), max_scale, v);
	}

	int d_len = tmp.Find('.');
	if(d_len == -1) {
		d_len = tmp.GetLength();
	}
	TCHAR *p_r = result.GetBuffer(d_len * 2);
	TCHAR *p_i = tmp.GetBuffer(0);
	int i;

	if(*p_i == '-') {
		*p_r = *p_i;
		p_r++;
		p_i++;
		d_len--;
	}

	for(i = 0; i < d_len; i++) {
		if(i > 0 && (d_len - i) % 3 == 0) {
			*p_r = ',';
			p_r++;
		}
		*p_r = *p_i;
		p_r++;
		p_i++;
	}
	*p_r = '\0';
	result.ReleaseBuffer();

	if(*p_i == '.') {
		result += p_i;
	}

	return result;
}

CGridData::CGridData() : m_disp_info(NULL), m_disp_info_cnt(0), m_default_text_color(RGB(0, 0, 0))
{
	for(int i = 0; i < MAX_GRID_NOTIFY_WND_CNT; i++) {
		m_notify_wnd_list[i] = NULL;
	}

	m_p_disp_data = &m_disp_data;
}

CGridData::~CGridData()
{
	FreeDispInfo();
	FreeGridFilterData();
}

void CGridData::AllocDispInfo()
{
	if(m_disp_info_cnt >= Get_ColCnt()) return;

	m_disp_info = (struct disp_info_st *)realloc(m_disp_info,
		(sizeof(struct disp_info_st) * Get_ColCnt()));

	for(int i = m_disp_info_cnt; i < Get_ColCnt(); i++) {
		m_disp_info[i].col_size = GRID_DEFAULT_COL_WIDTH;
		m_disp_info[i].disp_flg = 1;
	}

	m_disp_info_cnt = Get_ColCnt();
}

void CGridData::FreeDispInfo()
{
	if(m_disp_info != NULL) {
		free(m_disp_info);
		m_disp_info = NULL;
	}
	m_disp_info_cnt = 0;
}

void CGridData::SetDefaultColWidth()
{	
	for(int i = 0; i < m_disp_info_cnt; i++) {
		m_disp_info[i].col_size = GRID_DEFAULT_COL_WIDTH;
	}
}

void CGridData::SetDispColWidth(int col, int width)
{
	if(col >= m_disp_info_cnt) return;

	if(width <= 0) {
		SetDispFlg(col, FALSE);
		return;
	}

	m_disp_info[col].col_size = width;
	SetDispFlg(col, TRUE);
}

int CGridData::GetDispColWidth(int col)
{
	if(col < 0 || col >= m_disp_info_cnt) return GRID_DEFAULT_COL_WIDTH;
	if(!GetDispFlg(col)) return 0;
	return m_disp_info[col].col_size;
}

void CGridData::SetDispFlg(int col, BOOL flg)
{
	if(col >= m_disp_info_cnt) return;

	m_disp_info[col].disp_flg = flg;
	if(m_disp_info[col].disp_flg && m_disp_info[col].col_size == 0) {
		m_disp_info[col].col_size = GRID_DEFAULT_COL_WIDTH;
	}
}

BOOL CGridData::GetDispFlg(int col)
{
	if(col < 0 || col >= m_disp_info_cnt) return TRUE;
	return m_disp_info[col].disp_flg;
}

void CGridData::RegistNotifyWnd(HWND hwnd)
{
	for(int i = 0; i < MAX_GRID_NOTIFY_WND_CNT; i++) {
		if(m_notify_wnd_list[i] == NULL) {
			m_notify_wnd_list[i] = hwnd;
			break;
		}
	}
}

void CGridData::UnRegistNotifyWnd(HWND hwnd)
{
	for(int i = 0; i < MAX_GRID_NOTIFY_WND_CNT; i++) {
		if(m_notify_wnd_list[i] == hwnd) {
			m_notify_wnd_list[i] = NULL;
			break;
		}
	}
}

void CGridData::SendNotifyMessage(HWND sender, UINT msg, WPARAM wParam, LPARAM lParam)
{
	for(int i = 0; i < MAX_GRID_NOTIFY_WND_CNT; i++) {
		if(m_notify_wnd_list[i] != NULL && m_notify_wnd_list[i] != sender) {
			::SendMessage(m_notify_wnd_list[i], msg, wParam, lParam);
		}
	}
}

BOOL CGridData::IsWindowActive()
{
	for(int i = 0; i < MAX_GRID_NOTIFY_WND_CNT; i++) {
		if(m_notify_wnd_list[i] != NULL) {
			if(::GetFocus() == m_notify_wnd_list[i]) return TRUE;
		}
	}
	return FALSE;
}

BOOL CGridData::IsInSelectedArea(int row, int col)
{
	if(HaveSelected() == FALSE) return FALSE;

	if(GetSelectArea()->pos1.y > row || GetSelectArea()->pos2.y < row) return FALSE;

	if(GetSelectArea()->pos1.x <= GetSelectArea()->pos2.x) {
		if(GetSelectArea()->pos1.x > col || GetSelectArea()->pos2.x < col) return FALSE;
	} else {
		if(GetSelectArea()->pos1.x < col || GetSelectArea()->pos2.x > col) return FALSE;
	}

	return TRUE;
}


int CGridData::SearchDataMainRegexp(int r, int rl, int c, int cl, POINT *result_pt, 
	int dir, HREG_DATA reg_data, BOOL b_select_area)
{
	const TCHAR	*p;
	int		i, j, cb;

	cb = c;
	for(i = 0; i < rl; i++, r = r + dir) {
		for(c = cb, j = 0; j < cl; j++, c = c + dir) {
			if(b_select_area && !IsInSelectedArea(r, c)) continue;

			OREG_DATASRC    data_src;

			p = Get_EditData(r, c);

			oreg_make_str_datasrc(&data_src, p);
			if(oreg_exec2(reg_data, &data_src) == OREGEXP_FOUND) {
				result_pt->y = r;
				result_pt->x = c;
				return 1;
			}
		}
	}
	return 0;
}

int CGridData::SearchDataRegexp(POINT start_pt, POINT *result_pt, 
	int dir, BOOL b_loop, BOOL *b_looped, BOOL b_cur_cell, 
	BOOL b_select_area, HREG_DATA reg_data)
{
	if(start_pt.x < 0 || start_pt.x > Get_ColCnt() ||
		start_pt.y < 0 || start_pt.y > Get_RowCnt()) return 0;

	if(b_looped != NULL) *b_looped = FALSE;

	if(b_cur_cell) {
		if(SearchDataMainRegexp(start_pt.y, 1, start_pt.x, 1,
			result_pt, 1, reg_data,	b_select_area) == 1) goto FOUND;
	}

	if(dir == 1) {
		// 現在行の右側
		if(SearchDataMainRegexp(start_pt.y, 1,
			start_pt.x + 1, Get_ColCnt() - (start_pt.x + 1),
			result_pt, dir, reg_data, b_select_area) == 1) goto FOUND;
		// 現在行 + 1 から末尾まで
		if(SearchDataMainRegexp(start_pt.y + 1, Get_RowCnt() - (start_pt.y + 1),
			0, Get_ColCnt(), result_pt, dir, reg_data, b_select_area) == 1) goto FOUND;
		
		if(b_loop == FALSE) goto NOT_FOUND;
		if(b_looped != NULL) *b_looped = TRUE;

		// 先頭から現在行 - 1 まで
		if(SearchDataMainRegexp(0, start_pt.y, 0, Get_ColCnt(), 
			result_pt, dir, reg_data, b_select_area) == 1) goto FOUND;
		// 現在行の左側
		if(SearchDataMainRegexp(start_pt.y, 1, 0, start_pt.x + 1, 
			result_pt, dir, reg_data, b_select_area) == 1) goto FOUND;
	} else {
		// 現在行の左側
		if(SearchDataMainRegexp(start_pt.y, 1, start_pt.x - 1, start_pt.x,
			result_pt, dir, reg_data, b_select_area) == 1) goto FOUND;
		// 現在行 - 1 から先頭まで
		if(SearchDataMainRegexp(start_pt.y - 1, start_pt.y, Get_ColCnt() - 1, Get_ColCnt(),
			result_pt, dir, reg_data, b_select_area) == 1) goto FOUND;

		if(b_loop == FALSE) goto NOT_FOUND;
		if(b_looped != NULL) *b_looped = TRUE;

		// 末尾から現在行 + 1 まで
		if(SearchDataMainRegexp(Get_RowCnt() - 1, Get_RowCnt() - (start_pt.y + 1),
			Get_ColCnt() - 1, Get_ColCnt(), 
			result_pt, dir, reg_data, b_select_area) == 1) goto FOUND;
		// 現在行の右側
		if(SearchDataMainRegexp(start_pt.y, 1,
			Get_ColCnt() - 1, Get_ColCnt() - start_pt.x,
			result_pt, dir, reg_data, b_select_area) == 1) goto FOUND;
	}

	// fall trough
NOT_FOUND:
	return 0;

FOUND:
	return 1;
}

void CGridData::init_cur_cell()
{
	if(m_selected_cell.x < 0) m_selected_cell.x = 0;
	if(m_selected_cell.y < 0) m_selected_cell.y = 0;

	if(m_selected_cell.x >= Get_ColCnt()) m_selected_cell.x = Get_ColCnt() - 1;
	if(m_selected_cell.y >= Get_RowCnt()) m_selected_cell.y = Get_RowCnt() - 1;
}

void CGridData::set_valid_cell(int row, int col)
{
	if(row < 0) row = 0;
	if(row >= Get_RowCnt()) row = Get_RowCnt() - 1;
	if(col < 0) col = 0;
	if(col >= Get_ColCnt()) col = Get_ColCnt() - 1;

	m_selected_cell.y = row;
	m_selected_cell.x = col;
}

void CGridData::set_cur_cell(int row, int col)
{
	// NOTE: 違うSQLで検索実行したとき、カラム数やレコード数が少なくなる場合、
	// このASSERTチェックにひっかかるときがある
	// ここでチェックするのは適切ではないかも
	ASSERT((row >= 0 && row < Get_RowCnt()) || (row < 0 && Get_RowCnt() <= 0));
	ASSERT((col >= 0 && col < Get_ColCnt()) || (col < 0 && Get_ColCnt() <= 0));

	set_valid_cell(row, col);
}

void CGridData::clear_selected_cell()
{ 
	m_selected_cell.y = -1;
	m_selected_cell.x = -1;
}

BOOL CGridData::is_selected_cell()
{
	// セルが選択されているか。CanCopyなどのチェックでも使われる
	// 選択範囲がないとき、y = -1になる
	return (m_selected_cell.y != -1);
}

BOOL CGridData::HaveSelected()
{
	if(GetSelectArea()->pos1.y == -1) return FALSE;

	if(GetSelectArea()->pos1.y == GetSelectArea()->pos2.y &&
		GetSelectArea()->pos1.x == GetSelectArea()->pos2.x) return FALSE;

	return TRUE;
}

BOOL CGridData::HaveSelectedMultiRow()
{
	if(!HaveSelected()) return FALSE;
	if(GetSelectArea()->pos1.y == GetSelectArea()->pos2.y) return FALSE;
	return TRUE;
}

#pragma intrinsic(strcmp)
CPoint CGridData::CalcSelectedDataStr(GRID_CALC_TYPE calc_type, CPoint start_pt, CPoint end_pt)
{
	if(calc_type != GRID_CALC_TYPE_MAX && calc_type != GRID_CALC_TYPE_MIN) return CPoint(-1, -1);

	CPoint		min_str_pt(-1, -1);
	CPoint		max_str_pt(-1, -1);
	const TCHAR *min_str;
	const TCHAR *max_str;
	const TCHAR *cur_str;
	BOOL		b_first = TRUE;

	for(int x = start_pt.x; x <= end_pt.x; x++) {
		for(int y = start_pt.y; y <= end_pt.y; y++) {
			if(IsColDataNull(y, x)) continue;
			
			cur_str = Get_DispData(y, x);

			if(b_first) {
				min_str_pt.x = x;
				min_str_pt.y = y;
				max_str_pt.x = x;
				max_str_pt.y = y;

				min_str = cur_str;
				max_str = cur_str;
				b_first = FALSE;
				continue;
			}

			if(_tcscmp(min_str, cur_str) > 0) {
				min_str_pt.x = x;
				min_str_pt.y = y;
				min_str = cur_str;
			}
			if(_tcscmp(max_str, cur_str) < 0) {
				max_str_pt.x = x;
				max_str_pt.y = y;
				max_str = cur_str;
			}
		}
	}

	switch(calc_type) {
	case GRID_CALC_TYPE_MAX:
		return max_str_pt;
	case GRID_CALC_TYPE_MIN:
		return min_str_pt;
	}

	return CPoint(-1, -1);
}
#pragma function(strcmp)

double CGridData::CalcSelectedDataDouble(GRID_CALC_TYPE calc_type, CPoint start_pt, CPoint end_pt, 
	POINT *result_pt, int *p_max_scale)
{
	if(calc_type == GRID_CALC_TYPE_ROWS) {
		return (double)end_pt.y - start_pt.y + 1;
	}

	int		data_cnt = 0;
	double	total = 0.0;
	double	min = DBL_MAX;
	double	max = DBL_MIN;
	POINT min_pt = start_pt;
	POINT max_pt = start_pt;
	const TCHAR *data;
	int 	max_scale = 0;
	int 	scale;

	for(int x = start_pt.x; x <= end_pt.x; x++) {
		for(int y = start_pt.y; y <= end_pt.y; y++) {
			if(IsColDataNull(y, x)) continue;
			if(Get_ColDataType(y, x) == GRID_DATA_NUMBER_TYPE) {
				data = Get_ColData(y, x);
				total += _ttof(data);
				scale = get_scale(data);
				if(max_scale < scale) max_scale = scale;
				if(max < _ttof(data)) {
					max = _ttof(data);
					max_pt.x = x;
					max_pt.y = y;
				}
				if(min > _ttof(data)) {
					min = _ttof(data);
					min_pt.x = x;
					min_pt.y = y;
				}
				data_cnt++;
			}
		}
	}

	if(p_max_scale) *p_max_scale = max_scale;

	if(data_cnt == 0) return DBL_MAX;

	switch(calc_type) {
	case GRID_CALC_TYPE_TOTAL:
		return total;
	case GRID_CALC_TYPE_AVE:
		return total / data_cnt;
	case GRID_CALC_TYPE_MAX:
		*result_pt = max_pt;
		return max;
	case GRID_CALC_TYPE_MIN:
		*result_pt = min_pt;
		return min;
	default:
		return 0.0;
	}
}

CString CGridData::CalcSelectedData(GRID_CALC_TYPE calc_type, CPoint pt1, CPoint pt2)
{
	CString result;

	CPoint result_pt;
	POINT	start_pt, end_pt;

	start_pt.x = min(pt1.x, pt2.x);
	start_pt.y = min(pt1.y, pt2.y);
	end_pt.x = max(pt1.x, pt2.x);
	end_pt.y = max(pt1.y, pt2.y);

	if(calc_type == GRID_CALC_TYPE_ROWS) {
		int row_cnt = (int)CalcSelectedDataDouble(calc_type, start_pt, end_pt, &result_pt, NULL);
		result.Format(_T("行数:%s"), FormatValue(row_cnt, 0, NULL).GetBuffer(0));
		return result;
	}

	BOOL	number_calc_flg = FALSE;
	int		max_scale = 0;
	for(int x = start_pt.x; x <= end_pt.x; x++) {
		if(Get_ColDataType(0, x) == GRID_DATA_NUMBER_TYPE) {
			number_calc_flg = TRUE;
			if(max_scale < Get_ColScale(x)) max_scale = Get_ColScale(x);
			if(max_scale == 129) max_scale = 0;
		}
	}
	if(!number_calc_flg) {
		result_pt = CalcSelectedDataStr(calc_type, start_pt, end_pt);
		if(result_pt.y == -1 || result_pt.x == -1) return _T("");

		CString name = _T("");
		switch(calc_type) {
		case GRID_CALC_TYPE_MAX:
			name = _T("最大値");
			break;
		case GRID_CALC_TYPE_MIN:
			name = _T("最小値");
			break;
		default:
			return _T("");
		}
		result.Format(_T("%s:%s"), name.GetBuffer(0), Get_DispData(result_pt.y, result_pt.x));
		return result;
	}

	int calc_scale = 0;
	double value = CalcSelectedDataDouble(calc_type, start_pt, end_pt, &result_pt, &calc_scale);
	if(value == DBL_MAX) return "";

	if(calc_scale > max_scale) max_scale = calc_scale;

	CString name = _T("");

	switch(calc_type) {
	case GRID_CALC_TYPE_TOTAL:
		name = _T("合計");
		result.Format(_T("%s:%s"), name.GetBuffer(0), FormatValue(value, max_scale, NULL).GetBuffer(0));
		break;
	case GRID_CALC_TYPE_AVE:
		name = _T("平均");
		max_scale++;
		result.Format(_T("%s:%s"), name.GetBuffer(0), FormatValue(value, max_scale, NULL).GetBuffer(0));
		break;
	case GRID_CALC_TYPE_MAX:
		name = _T("最大値");
		result.Format(_T("%s:%s"), name.GetBuffer(0), FormatValue(value, 0, Get_DispData(result_pt.y, result_pt.x)).GetBuffer(0));
		break;
	case GRID_CALC_TYPE_MIN:
		name = _T("最小値");
		result.Format(_T("%s:%s"), name.GetBuffer(0), FormatValue(value, 0, Get_DispData(result_pt.y, result_pt.x)).GetBuffer(0));
		break;
	default:
		return "";
	}
	return result;
}

int CGridData::SearchColumnRegexp(int start_col, int *result_col, 
	int dir, BOOL b_loop, BOOL *b_looped, BOOL b_cur_cell, HREG_DATA reg_data)
{
	int		c;
	if(b_cur_cell) {
		c = start_col;
		if(oreg_exec_str2(reg_data, Get_ColName(c))) {
			*result_col = c;
			return 1;
		}
	}

	int		col_cnt = Get_ColCnt();
	if(dir == 1) {
		for(c = start_col + 1; c < col_cnt; c++) {
			if(oreg_exec_str2(reg_data, Get_ColName(c))) {
				*result_col = c;
				return 1;
			}
		}
		for(c = 0; c < start_col; c++) {
			if(oreg_exec_str2(reg_data, Get_ColName(c))) {
				*result_col = c;
				return 1;
			}
		}
	} else {
		for(c = start_col - 1; c >= 0; c--) {
			if(oreg_exec_str2(reg_data, Get_ColName(c))) {
				*result_col = c;
				return 1;
			}
		}
		for(c = col_cnt - 1; c > start_col; c--) {
			if(oreg_exec_str2(reg_data, Get_ColName(c))) {
				*result_col = c;
				return 1;
			}
		}
	}

	return 0;
}

int CGridData::ProcessUpdateCells(CGridUpdateProcess &process)
{
	if(!IsEditable()) return 0;
	if(!IsValidCurPt()) return 0;

	int y_start, y_end, x_start, x_end;

	if(!HaveSelected()) {
		y_start = get_cur_row();
		y_end = get_cur_row();
		x_start = get_cur_col();
		x_end = get_cur_col();
	} else if(GetSelectArea()->pos1.x <= GetSelectArea()->pos2.x) {
		y_start = GetSelectArea()->pos1.y;
		y_end = GetSelectArea()->pos2.y;
		x_start = GetSelectArea()->pos1.x;
		x_end = GetSelectArea()->pos2.x;
	} else {
		y_start = GetSelectArea()->pos1.y;
		y_end = GetSelectArea()->pos2.y;
		x_start = GetSelectArea()->pos2.x;
		x_end = GetSelectArea()->pos1.x;
	}

	BOOL b_undo_start = FALSE;

	int replace_cell_cnt = 0;
	int		x, y;
	for(y = y_start; y <= y_end; y++) {
		for(x = x_start; x <= x_end; x++) {
			if(!IsEditableCell(y, x)) continue;

			const TCHAR *p = Get_EditData(y, x);

			CString new_str = process.ProcessData(p);
			if(new_str.Compare(p) == 0) continue;

			// Undo開始
			if(!b_undo_start) {
				StartUndoSet();
				b_undo_start = TRUE;
			}

			// process
			if(UpdateCell(y, x, new_str.GetBuffer(0), -1) == 0) replace_cell_cnt++;
		}
	}

	if(b_undo_start) EndUndoSet();

	return replace_cell_cnt;
}

int CGridData::ToLower()
{
	class MakeLowerProcess : public CGridUpdateProcess
	{
	public:
		virtual CString ProcessData(const TCHAR *text) {
			CString str = text;
			str.MakeLower();
			return str;
		}
	};

	MakeLowerProcess proc;
	return ProcessUpdateCells(proc);
}

int CGridData::ToUpper()
{
	class MakeUpperProcess : public CGridUpdateProcess
	{
	public:
		virtual CString ProcessData(const TCHAR *text) {
			CString str = text;
			str.MakeUpper();
			return str;
		}
	};

	MakeUpperProcess proc;
	return ProcessUpdateCells(proc);
}

int CGridData::ToHankaku(BOOL b_alpha, BOOL b_kata)
{
	class MakeToHankakuProcess : public CGridUpdateProcess
	{
	public:
		MakeToHankakuProcess(BOOL b_alpha, BOOL b_kata) : m_b_alpha(b_alpha), m_b_kata(b_kata) {}

		virtual CString ProcessData(const TCHAR *text) {
			CString str;
			int buf_len = static_cast<int>(_tcslen(text) * 2 + 1);
			ZenkakuToHankaku2(text, 
				str.GetBuffer(buf_len),
				m_b_alpha, m_b_kata);
			str.ReleaseBuffer();		
			return str;
		}
	
	private:
		BOOL m_b_alpha;
		BOOL m_b_kata;
	};

	MakeToHankakuProcess proc(b_alpha, b_kata);
	return ProcessUpdateCells(proc);
}

int CGridData::ToZenkaku(BOOL b_alpha, BOOL b_kata)
{
	class MakeToZenkakuProcess : public CGridUpdateProcess
	{
	public:
		MakeToZenkakuProcess(BOOL b_alpha, BOOL b_kata) : m_b_alpha(b_alpha), m_b_kata(b_kata) {}

		virtual CString ProcessData(const TCHAR *text) {
			CString str;
			int buf_len = static_cast<int>(_tcslen(text) * 2 + 1);
			HankakuToZenkaku2(text, 
				str.GetBuffer(buf_len),
				m_b_alpha, m_b_kata);
			str.ReleaseBuffer();		
			return str;
		}
	
	private:
		BOOL m_b_alpha;
		BOOL m_b_kata;
	};

	MakeToZenkakuProcess proc(b_alpha, b_kata);
	return ProcessUpdateCells(proc);
}

void CGridData::InputSequence(__int64 start_num, __int64 incremental)
{
	class InputSequenceProcess : public CGridUpdateProcess
	{
	public:
		InputSequenceProcess(__int64 start_num, __int64 incremental) :
		  m_start_num(start_num), m_incremental(incremental), m_first_flg(false), m_last_result(0) {}

		virtual CString ProcessData(const TCHAR *text) {
			CString str;
			if(!m_first_flg) {
				m_first_flg = true;
				m_last_result = m_start_num;
			} else {
				m_last_result = m_last_result + m_incremental;
			}
			str.Format(_T("%I64d"), m_last_result);
			return str;
		}
	private:
		__int64 m_start_num;
		__int64 m_incremental;
		__int64 m_last_result;
		bool m_first_flg;
	};

	InputSequenceProcess proc(start_num, incremental);
	ProcessUpdateCells(proc);
}

void CGridData::SortData(int *col_no, int *order, int sort_col_cnt)
{
/*
	if(Get_RowCnt() > 200) {
		int		i;
		int		dup_cnt = 0;

		for(i = 0; i < 100; i++) {
			if(SortCmp(col_no, order, sort_col_cnt, i, i + 1) == 0) dup_cnt++;
			if(SortCmp(col_no, order, sort_col_cnt, i, i + 100) == 0) dup_cnt++;
			if(dup_cnt > 10) break;
		}
		if(dup_cnt > 10) {
			SortDataComb11(col_no, order, sort_col_cnt, Get_RowCnt());
			return;
		}
	}
	SortDataMain(col_no, order, sort_col_cnt, 0, Get_RowCnt() - 1);
*/
	SortDataComb11(col_no, order, sort_col_cnt, Get_RowCnt());
}

__inline double CGridData::SortCmp(int *col_no, int *order, int sort_col_cnt, int r1, int r2)
{
	double result;

	for(int c = 0; c < sort_col_cnt; c++) {
		int c_no = col_no[c];

		if(Get_ColDataType(r1, c_no) == GRID_DATA_NUMBER_TYPE) {
			if(IsColDataNull(r1, c_no)) {
				if(IsColDataNull(r2, c_no)) {
					result = 0;
				} else {
					result = 1;
				}
			} else if(IsColDataNull(r2, c_no)) {
				result = -1;
			} else {
				const wchar_t *t1, *t2;
				t1 = Get_ColData(r1, c_no);
				t2 = Get_ColData(r2, c_no);
				if(_tcscmp(t1, t2) == 0) {
					result = 0;
				} else {
					if(_tcschr(t1, '.') == NULL && _tcschr(t2, '.') == NULL) {
						long long l1, l2;
						wchar_t *e1, *e2;
						l1 = _tcstoll(t1, &e1, 10);
						l2 = _tcstoll(t2, &e2, 10);
						if(l1 == l2) {
							result = 0;
						} else if(l1 > l2) {
							result = 1;
						} else {
							result = -1;
						}
					} else {
						result = _ttof(Get_ColData(r1, c_no)) - _ttof(Get_ColData(r2, c_no));
					}
				}
			}
		} else {
			result = _tcscmp(Get_ColData(r1, c_no), Get_ColData(r2, c_no));
		}
		if(result != 0) return result * order[c];
	}

	return 0;
}

void CGridData::SortDataMain(int *col_no, int *order, int sort_col_cnt, int left, int right)
{
	int		i, last;
	double	result;

	if(left >= right) return;

	SwapRow(left, (left + right) / 2);

	last = left;
	for(i = left + 1; i <= right; i++) {
		result = SortCmp(col_no, order, sort_col_cnt, i, left);
		if(result < 0) SwapRow(++last, i);
	}
	if(left != last) SwapRow(left, last);

	SortDataMain(col_no, order, sort_col_cnt, left, last - 1);
	SortDataMain(col_no, order, sort_col_cnt, last + 1, right);
}

BOOL CGridData::IsValidPt(int row, int col)
{
	if(col < 0 || row < 0 || col >= Get_ColCnt() || row >= Get_RowCnt()) {
		return FALSE;
	}

	return TRUE;
}

BOOL CGridData::IsValidCurPt()
{
	POINT pt = *get_cur_cell();
	return IsValidPt(pt.y, pt.x);
}

void CGridData::SortDataComb11(int *col_no, int *order, int sort_col_cnt, int size)
{
	int		i;
	int		gap = size;
	int		done = 0;

	while((gap > 1) || !done) {
		gap = (gap * 10) / 13;
		if (gap == 0) gap = 1;
		if (gap == 9 || gap == 10) gap = 11;
		
		done = 1;

		for(i = 0; i < size - gap; ++i) {
			if(SortCmp(col_no, order, sort_col_cnt, i, i + gap) > 0) {
				SwapRow(i, i + gap);
				done = 0;
			}
		}
	}
}

int CGridData::GetColWidth(const TCHAR *p, int limit_width, CDC *dc,
	CFontWidthHandler *dchandler, const TCHAR *p_end)
{
	int w = 0;
	int ch_w;

	CGridDispData *disp_data = GetDispData();

	for(;;) {
		if(*p == '\0' || p == p_end) break;
		if(*p == '\r' || *p == '\n') {
			w += disp_data->GetFontWidth(dc, L'x', dchandler) * 2;
			break;
		}

		ch_w = disp_data->GetFontWidth(dc, get_char(p), dchandler);
		ch_w += disp_data->GetCharSpace(ch_w);

		w += ch_w;
		if(w > limit_width) {
			w = limit_width;
			break;
		}
		p += get_char_len(p);
	}

	return w;
}

static int check_cancel(volatile int *cancel_flg)
{
	if(cancel_flg != NULL) {
		for(; *cancel_flg == 2;) {
			#ifdef WIN32
			Sleep(500);
			#else
			sleep(1);
			#endif
		}
		if(*cancel_flg == 1) goto CANCEL;
	}
	return 0;

CANCEL:
	return 1;
}

int CGridData::Get_CopyDataBufSize(int row, int col, BOOL b_escape, char quote_char)
{
	int result = 0;
	const TCHAR *pdata = Get_CopyData(row, col);

	if (pdata != NULL) {
		result = result + (int)_tcslen(pdata);
		if (b_escape) {
			// "は，""に変換するので，その分を計算
			result += ostr_str_cnt(pdata, quote_char);
		}
	}

	return result;
}

int CGridData::GetMaxColWidth(int col, int limit_width, CFontWidthHandler *dchandler,
	volatile int *cancel_flg)
{
	int		i;
	int		max_width = 0;
	int		row_cnt = Get_RowCnt();

	for(i = 0; i < row_cnt; i++) {
		if(check_cancel(cancel_flg)) return max_width;

		int w = GetColWidth(Get_DispData(i, col), limit_width, NULL, dchandler, NULL);
		if(w > max_width) {
			max_width = w;
			// ウィンドウ幅を超えたときは，これ以上最大幅を求めない
			if(max_width >= limit_width) break;
		}
	}

	return max_width;
}

// CHAR型データのとき、末尾のスペースを考慮せずに最大長を求める
int CGridData::GetMaxColWidthNoLastSpace(int col, int limit_width, CFontWidthHandler *dchandler,
	volatile int *cancel_flg)
{
	int		i;
	int		max_width = 0;
	int		row_cnt = Get_RowCnt();

	for(i = 0; i < row_cnt; i++) {
		if(check_cancel(cancel_flg)) return max_width;

		int len = ostr_first_line_len_no_last_space(Get_DispData(i, col));
		const TCHAR *p_end = Get_DispData(i, col) + len;
		int w = GetColWidth(Get_DispData(i, col), limit_width, NULL, dchandler, p_end);

		if(w > max_width) {
			max_width = w;
			// ウィンドウ幅を超えたときは，これ以上最大幅を求めない
			if(max_width >= limit_width) break;
		}
	}

	return max_width;
}

BOOL CGridData::SaveFile(CUnicodeArchive &ar, GRID_SAVE_FORMAT format, 
	int put_colname, int dquote_flg, TCHAR *msg_buf)
{
	int r;
	int c;
	TCHAR sepa[2];
	
	switch(format) {
	case GRID_SAVE_CSV:
		_tcscpy(sepa, _T(","));
		break;
	case GRID_SAVE_TSV:
		_tcscpy(sepa, _T("\t"));
		break;
	default:
		ASSERT(0);
		return FALSE;
	}

	int col_cnt = Get_ColCnt();
	int row_cnt = Get_RowCnt();

	if(put_colname) {
		for(c = 0; c < col_cnt; c++) {
			if(c > 0) ar.WriteString(sepa);
			ar.CsvOut(Get_ColName(c), dquote_flg);
		}
		ar.WriteNextLine();
	}

	for(r = 0; r < row_cnt; r++) {
		for(c = 0; c < col_cnt; c++) {
			if(c > 0) ar.WriteString(sepa);
			if(!IsColDataNull(r, c)) {
				ar.CsvOut(Get_ColData(r, c), dquote_flg);
			}
		}
		ar.WriteNextLine();
	}

	ar.Flush();

	return TRUE;
}

BOOL CGridData::SaveFile(const TCHAR *file_name, int kanji_code, int line_type,
	GRID_SAVE_FORMAT format, int put_colname, int dquote_flg, TCHAR *msg_buf)
{
	BOOL result;

	CFileException fe;
	CFile file;
	if(file.Open(file_name, CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive, &fe) == FALSE) {
		// FIXME: 例外から取得できるエラーメッセージ確認する
		//fe.GetErrorMessage(msg_buf, 512);
		_stprintf(msg_buf, _T("Error: File can not open (%s)"), file_name);
		return FALSE;
	}

	CArchive ar(&file, CArchive::store | CArchive::bNoFlushOnDelete);
	CUnicodeArchive uni_ar(&ar, kanji_code, line_type);

	result = SaveFile(uni_ar, format, put_colname, dquote_flg, msg_buf);

	ar.Close();
	file.Close();

	return result;
}

void CGridData::FreeGridFilterData()
{
	if(m_grid_data_filter) {
		delete m_grid_data_filter;
		m_grid_data_filter = NULL;
	}
}

int CGridData::FilterData(int filter_col_no, const TCHAR* search_text, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp,
	int* find_cnt, CString* msg_str, BOOL b_err_at_no_find)
{
	CGridData_Filter* f = m_grid_data_filter;

	if(m_grid_data_filter == NULL) {
		f = new CGridData_Filter();
		f->SetGridData(this);
	}

	int ret_v = f->DoFilterData(filter_col_no, search_text, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp, find_cnt, msg_str, b_err_at_no_find);
	if(ret_v != 0) {
		if(m_grid_data_filter == NULL) {
			delete f;
		}
		return ret_v;
	}

	m_grid_filter_mode = TRUE;
	if(m_grid_data_filter == NULL) {
		m_grid_data_filter = f;
	}

	set_cur_cell(0, filter_col_no);

	return 0;
}

void CGridData::FilterOff()
{
	if(!m_grid_filter_mode) return;

	if(m_grid_data_filter) {
		int col = m_grid_data_filter->get_cur_col();
		set_cur_cell(0, col);
	}
	m_grid_filter_mode = FALSE;
}

int CGridData::RestoreFilterData(griddata_filter_cond* d, CString* msg_str)
{
	CGridData_Filter* f = m_grid_data_filter;

	if(m_grid_data_filter == NULL) {
		f = new CGridData_Filter();
		f->SetGridData(this);
	}

	int ret_v = f->RestoreGridDataFilterCondMain(d, msg_str);
	if(ret_v != 0) {
		if(m_grid_data_filter == NULL) {
			delete f;
		}
		return ret_v;
	}

	m_grid_filter_mode = TRUE;
	if(m_grid_data_filter == NULL) {
		m_grid_data_filter = f;
	}

	set_cur_cell(0, d->filter_col_no);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
CGridData_SwapRowCol::CGridData_SwapRowCol()
{
	m_grid_data = NULL;
	m_row_header_len = 6;
}

void CGridData_SwapRowCol::InitDispInfo(BOOL b_free_disp_info)
{
	if(b_free_disp_info) {
		FreeDispInfo();
	}
	AllocDispInfo();

	m_row_header_len = 6;

	int		row;
	for(row = 0; row < Get_RowCnt(); row++) {
		// FIXME: 日本語に対応する
		int len = static_cast<int>(_tcslen(GetRowHeader(row)));
		if(len > m_row_header_len) m_row_header_len = len;
	}

	if(IsEditable()) m_row_header_len += 2;
}

int CGridData_SwapRowCol::SearchColumnRegexp(int start_cell, int *result_col, 
	int dir, BOOL b_loop, BOOL *b_looped, BOOL b_cur_cell, HREG_DATA reg_data)
{
	return m_grid_data->SearchColumnRegexp(start_cell, result_col,
		dir, b_loop, b_looped, b_cur_cell, reg_data);
}

int CGridData_SwapRowCol::Paste(const TCHAR *pstr)
{
	if(*pstr == '\0') return 0;

	int		x_start = get_cur_col();
	int		y_start = get_cur_row();
	int		x_max = Get_ColCnt() - 1;
	int		y_max = Get_RowCnt() - 1;

	BOOL	repeat_paste_mode = FALSE;

	if(HaveSelected()) {
		x_start = min(GetSelectArea()->pos1.x, GetSelectArea()->pos2.x);
		x_max = max(GetSelectArea()->pos1.x, GetSelectArea()->pos2.x);
		y_start = min(GetSelectArea()->pos1.y, GetSelectArea()->pos2.y);
		y_max = max(GetSelectArea()->pos1.y, GetSelectArea()->pos2.y);

		if(y_start != y_max && (_tcschr(pstr, '\n') == NULL && _tcschr(pstr, '\n') == NULL)) {
			repeat_paste_mode = TRUE;
		}
	}

	POINT	pt = {x_start, y_start};

	StartUndoSet();

	const int paste_buf_size = 1024 * 1024 * 4;	// 4M
	const TCHAR	*p = pstr;
	TCHAR	*buf = (TCHAR *)malloc(paste_buf_size);
	if(buf == NULL) return 0;

	for(;;) {
		p = ostr_get_tsv_data(p, buf, paste_buf_size);

		if(IsEditableCell(pt.y, pt.x)) {
			UpdateCell(pt.y, pt.x, (TCHAR *)buf, -1);
		}

		if(*p == '\t') {
			(pt.x)++;

			if(pt.x > x_max) {
				// 右端のセルにきたとき，次の行のデータまで読み飛ばす
				for(;;) {
					p = ostr_get_tsv_data(p, buf, paste_buf_size);
					if(*p != '\t') break;
					p++;
				}
			}
		}

		if(*p == '\r' || *p == '\n' || (*p == '\0' && repeat_paste_mode)) {
			pt.x = x_start;
			(pt.y)++;
			if(pt.y > y_max) break;
			//if(pt.y >= Get_RowCnt()) InsertRow(pt.y - 1);

			if(*p == '\0' && repeat_paste_mode) {
				p = pstr;
				continue;
			}

			// 最後の改行は無視する
			if(*(p + 1) == '\0') break;
		}

		if(*p == '\0') break;
		p++;
	}

	EndUndoSet();

	free(buf);
	
	return 0;
}

BOOL CGridData_SwapRowCol::is_selected_cell() {
	// セルが選択されているか。CanCopyなどのチェックでも使われる
	// 行列入れ替えて表示のときは、x = -1になる
	return (m_selected_cell.x != -1);
}

