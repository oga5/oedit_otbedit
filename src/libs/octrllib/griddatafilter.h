/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef _GRID_DATA_FILTER_H_INCLUDE
#define _GRID_DATA_FILTER_H_INCLUDE

#include "griddata.h"

class CGridData_Filter : public CGridData
{
public:
	CGridData_Filter();
	virtual ~CGridData_Filter();

	virtual int Get_ColCnt() { return m_grid_data->Get_ColCnt(); }
	virtual int Get_RowCnt() { return m_row_cnt; }
	virtual const TCHAR* Get_ColName(int col) { return m_grid_data->Get_ColName(col); }

	virtual const TCHAR* Get_ColData(int row, int col) { return m_grid_data->Get_ColData(GetFilterDataRow(row), col); }
	virtual const TCHAR* Get_DispData(int row, int col) { return m_grid_data->Get_DispData(GetFilterDataRow(row), col); }
	virtual const TCHAR* Get_EditData(int row, int col) { return m_grid_data->Get_EditData(GetFilterDataRow(row), col); }
	virtual int Get_ColLimit(int row, int col) { return m_grid_data->Get_ColLimit(GetFilterDataRow(row), col); }

	virtual BOOL IsColDataNull(int row, int col) { return m_grid_data->IsColDataNull(GetFilterDataRow(row), col); }

	virtual int Get_ColMaxSize(int col) { return m_grid_data->Get_ColMaxSize(col); }
	virtual int Get_ColScale(int col) { return m_grid_data->Get_ColScale(col); }
	virtual int Get_ColDataType(int row, int col) { return m_grid_data->Get_ColDataType(GetFilterDataRow(row), col); }

	virtual void SetDispColWidth(int col, int width) { m_grid_data->SetDispColWidth(col, width); }
	virtual int GetDispColWidth(int col) { return m_grid_data->GetDispColWidth(col); }

	void SetGridData(CGridData* grid_data) {
		m_grid_data = grid_data;
		SetDispData(grid_data->GetDispData());
	}

	// 編集機能のサポート
	virtual BOOL IsEditable() { return m_grid_data->IsEditable(); }

	virtual BOOL IsInsertRow(int row) { return m_grid_data->IsInsertRow(GetFilterDataRow(row)); }
	virtual BOOL IsUpdateRow(int row) { return m_grid_data->IsUpdateRow(GetFilterDataRow(row)); }
	virtual BOOL IsDeleteRow(int row) { return m_grid_data->IsDeleteRow(GetFilterDataRow(row)); }
	virtual BOOL IsUpdateCell(int row, int col) { return m_grid_data->IsUpdateCell(GetFilterDataRow(row), col); }

	virtual int InsertRows(int row, int row_cnt) {
/*
		SetCurCell();
		int r = m_grid_data->InsertRows(row, row_cnt);
		UpdateCurCell();
		return r;
*/
		return 1;
	}
	virtual int UpdateCell(int row, int col, const TCHAR* data, int len) {
		int r = m_grid_data->UpdateCell(GetFilterDataRow(row), col, data, len);
		return r;
	}
	virtual int UpdateCells(POINT* pos1, POINT* pos2, const TCHAR* data, int len);

	virtual int DeleteRows(int row1, int row2) {
/*
		SetCurCell();
		int r = m_grid_data->DeleteRows(row1, row2);
		UpdateCurCell();
		return r;
*/
		return 1;
	}
	virtual int DeleteNullRows(BOOL all_flg) { 
//		return m_grid_data->DeleteNullRows(all_flg); 
		return 1;
	}

	virtual int Paste(const TCHAR* pstr);

	virtual CUndo* GetUndo() { return m_grid_data->GetUndo(); }
	virtual void StartUndoSet() { m_grid_data->StartUndoSet(); }
	virtual void EndUndoSet() { m_grid_data->EndUndoSet(); }

	virtual BOOL CanUndo() { return m_grid_data->CanUndo(); }
	virtual BOOL CanRedo() { return m_grid_data->CanRedo(); }
	virtual int Undo() {
		SetCurCell();
		BOOL r = m_grid_data->Undo();
		UpdateCurCell();
		return r;
	}
	virtual int Redo() {
		SetCurCell();
		BOOL r = m_grid_data->Redo();
		UpdateCurCell();
		return r;
	}

	virtual void SortData(int* col_no, int* order, int sort_col_cnt);

	virtual BOOL IsEditableCell(int row, int col) { return m_grid_data->IsEditableCell(GetFilterDataRow(row), col); }

	virtual BOOL CanInsertRow() {
		return FALSE;
//		return m_grid_data->CanInsertRow();
	}
	virtual BOOL CanDeleteRow(int row) {
		return FALSE;
//		return m_grid_data->CanDeleteRow(GetFilterDataRow(row));
	}
	virtual const TCHAR* GetOriginalText(int row, int col) { return m_grid_data->GetOriginalText(GetFilterDataRow(row), col); }
	
	virtual int GetRowHeaderLen() { return 7; }
	virtual const TCHAR* GetRowHeader(int row);

	// フィルタ機能
	virtual int FilterData(int filter_col_no, const TCHAR* search_text, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp,
		int* find_cnt, CString* msg_str) {
		ASSERT(0);
		msg_str->Format(_T("DO NOT USE CGridData_Filter::FilterData"));
		return 1;
	};

	int DoFilterData(int filter_col_no, const TCHAR* search_text, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp,
		int* find_cnt, CString* msg_str, BOOL b_err_at_no_find);

	size_t GetGridDataFilterCondSize() { 
		if(m_filter_cond == NULL) return 0;
		return m_filter_cond->data_size;
	}
	griddata_filter_cond* GetGridDataFilterCond() { return m_filter_cond; }
	int RestoreGridDataFilterCondMain(griddata_filter_cond* d, CString* msg_str);
	void RestoreRowIdxFromFilterRowFlg();

	POINT get_dataset_point(POINT* pt) {
		POINT dataset_pt;
		dataset_pt.x = pt->x;
		dataset_pt.y = GetFilterDataRow(pt->y);
		return dataset_pt;
	}

protected:

private:
	CGridData* m_grid_data{ NULL };
	int m_row_cnt{ 0 };
	int* m_row_idx{ NULL };
	griddata_filter_cond* m_filter_cond{ NULL };

	void FreeFilterCond();
	griddata_filter_cond* AllocFilterCond(int row_cnt);

	int GetFilterDataRow(int row) {
		if(m_row_idx == NULL || row > m_row_cnt) return 0;
		return m_row_idx[row];
	}

	void SetFilterRowFlg();

private:
	void UpdateCurCell();
	void SetCurCell();
};

#endif _GRID_DATA_FILTER_H_INCLUDE
