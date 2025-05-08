/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef _GRID_DATA_H_INCLUDE
#define _GRID_DATA_H_INCLUDE

#define GRID_DATA_NUMBER_TYPE	0
#define GRID_DATA_VARCHAR_TYPE	1
#define GRID_DATA_CHAR_TYPE		2

#define MAX_GRID_NOTIFY_WND_CNT	4


#include "oregexp.h"
#include "GridDispData.h"
#include "Undo.h"
#include "UnicodeArchive.h"

// 範囲選択用
struct grid_data_select_area_st
{
	int drag_flg;
	int select_mode;
	POINT start_pt;
	POINT pos1;
	POINT pos2;
};

typedef enum {
	GRID_CALC_TYPE_TOTAL,
	GRID_CALC_TYPE_AVE,
	GRID_CALC_TYPE_MAX,
	GRID_CALC_TYPE_MIN,
	GRID_CALC_TYPE_ROWS,
	GRID_CALC_TYPE_NONE,
} GRID_CALC_TYPE;

typedef enum {
	GRID_SAVE_CSV,
	GRID_SAVE_TSV,
} GRID_SAVE_FORMAT;


#define MAX_FILTER_SEARCH_TEXT_LEN (1024)
typedef struct griddata_filter_cond_st {
	size_t data_size{ 0 };
	int filter_col_no{ 0 };
	TCHAR search_text[MAX_FILTER_SEARCH_TEXT_LEN]{ _T("") };
	BOOL b_distinct_lwr_upr{ FALSE };
	BOOL b_distinct_width_ascii{ FALSE };
	BOOL b_regexp{ FALSE };
	int row_cnt{ 0 };
	int* row_idx{ NULL };
} griddata_filter_cond;

class CGridData_Filter;

class CGridData
{
public:
	CGridData();
	virtual ~CGridData();

	virtual int Get_ColCnt() { return 100; }
	virtual int Get_RowCnt() { return 1000; }
	virtual const TCHAR *Get_ColData(int row, int col) { return _T("data"); }
	virtual const TCHAR *Get_DispData(int row, int col) { return Get_ColData(row, col); }
	virtual const TCHAR *Get_EditData(int row, int col) { return Get_ColData(row, col); }
	virtual const TCHAR *Get_CopyData(int row, int col) { return Get_EditData(row, col); }
	virtual int Get_CopyDataBufSize(int row, int col, BOOL b_escape, char quote_char);

	virtual BOOL IsColDataNull(int row, int col) { return FALSE; }

	virtual const TCHAR *Get_ColName(int col) { return _T("test"); }
	virtual const TCHAR *Get_DispColName(int col) { return Get_ColName(col);}
	virtual int Get_ColMaxSize(int col) { return -1; }
	virtual int Get_ColScale(int col) { return 0; }
	virtual int Get_ColDataType(int row, int col) { return GRID_DATA_VARCHAR_TYPE; }
	virtual BOOL IsNullableCol(int col) { return TRUE; }

	virtual BOOL IsEditable() { return FALSE; }
	virtual int Get_ColLimit(int row, int col) { return 1024; }
	virtual int InsertRows(int row, int row_cnt) { return 0; }
	virtual int DeleteRows(int row1, int row2) { return 0; }
	virtual int DeleteNullRows(BOOL all_flg) { return 0; }
	virtual int UpdateCells(POINT *pos1, POINT *pos2, const TCHAR *data, int len) { return 0; }
	virtual int UpdateCell(int row, int col, const TCHAR *data, int len) { return 0; }
	virtual BOOL IsEditableCell(int row, int col) { return FALSE; }

	virtual int GetMaxColWidth(int col, int limit_width, CFontWidthHandler *dchandler,
		volatile int *cancel_flg = NULL);
	virtual int GetMaxColWidthNoLastSpace(int col, int limit_width, CFontWidthHandler *dchandler,
		volatile int *cancel_flg = NULL);
	int GetColWidth(const TCHAR *p, int limit_width, CDC *dc, CFontWidthHandler *dchandler, 
		const TCHAR *p_end);

	virtual void StartUndoSet() {}
	virtual void EndUndoSet() {}
	virtual CUndo *GetUndo() { return NULL; }

	BOOL IsValidPt(int row, int col);
	BOOL IsValidCurPt();

	virtual BOOL IsInsertRow(int row) { return FALSE; }
	virtual BOOL IsUpdateRow(int row) { return FALSE; }
	virtual BOOL IsDeleteRow(int row) { return FALSE; }
	virtual BOOL IsUpdateCell(int row, int col) { return FALSE; }
	virtual BOOL IsNullRow(int row) { return FALSE; }
	virtual BOOL CanUndo() { return FALSE; }
	virtual BOOL CanRedo() { return FALSE; }
	virtual int Undo() { return 0; }
	virtual int Redo() { return 0; }
	virtual int Paste(const TCHAR *pstr) { return 0; }

	virtual BOOL CanInsertRow() { return FALSE; }
	virtual BOOL CanDeleteRow(int row) { return FALSE; }
	virtual const TCHAR *GetOriginalText(int row, int col) { return _T(""); }

	virtual int GetRowHeaderLen() { return 6; }
	virtual const TCHAR *GetRowHeader(int row) { 
		_stprintf(m_row_header_buf, _T("%6d"), row + 1);
		return m_row_header_buf;
	}

	virtual COLORREF GetCellTextColor(int row, int col) { return m_default_text_color; }
	void SetDefaultTextColor(COLORREF color) { m_default_text_color = color; }

	virtual COLORREF GetCellHeaderTextColor(int col) { return m_default_text_color; }

	virtual BOOL IsCustomizeBkColor() { return FALSE; }
	virtual COLORREF GetCellBkColor(int row, int col) { return RGB(255, 255, 255); }

	int SearchDataRegexp(POINT start_pt, POINT *result_pt, 
		int dir, BOOL b_loop, BOOL *b_looped, BOOL b_cur_cell, 
		BOOL b_select_area, HREG_DATA reg_data);
	virtual int SearchColumnRegexp(int start_cell, int *result_col, 
		int dir, BOOL b_loop, BOOL *b_looped, BOOL b_cur_cell, HREG_DATA reg_data);

	int get_cur_col() { return m_selected_cell.x; }
	int get_cur_row() { return m_selected_cell.y; }
	POINT *get_cur_cell() { return &m_selected_cell; }
	void init_cur_cell();
	void set_cur_cell(int row, int col);
	void set_valid_cell(int row, int col);
	void clear_selected_cell();
	virtual BOOL is_selected_cell();

	virtual void SetDispColWidth(int col, int width);
	virtual int GetDispColWidth(int col);

	void SetDispFlg(int col, BOOL flg);
	BOOL GetDispFlg(int col);

	struct grid_data_select_area_st *GetSelectArea() { return &m_select_area; }
	BOOL IsInSelectedArea(int row, int col);

	virtual void RegistNotifyWnd(HWND hwnd);
	virtual void UnRegistNotifyWnd(HWND hwnd);
	void SendNotifyMessage(HWND sender, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL IsWindowActive();

	virtual CString GetLastErrorMessage() { return _T(""); }

	BOOL HaveSelected();
	BOOL HaveSelectedMultiRow();

	CString CalcSelectedData(GRID_CALC_TYPE calc_type, CPoint pt1, CPoint pt2);

	void SetDefaultColWidth();

	int ToLower();
	int ToUpper();
	int ToHankaku(BOOL b_alpha, BOOL b_kata);
	int ToZenkaku(BOOL b_alpha, BOOL b_kata);
	void InputSequence(__int64 start_num, __int64 incremental);

	virtual void SortData(int *col_no, int *order, int sort_col_cnt);

	virtual CString GetDispColType(int col) { return ""; }

	CGridDispData *GetDispData() { return m_p_disp_data; }
	void SetDispData(CGridDispData *disp_data) { m_p_disp_data = disp_data; }

	BOOL SaveFile(CUnicodeArchive &ar, GRID_SAVE_FORMAT format, 
		int put_colname, int dquote_flg, TCHAR *msg_buf);
	BOOL SaveFile(const TCHAR *file_name, int kanji_code, int line_type,
		GRID_SAVE_FORMAT format, int put_colname, int dquote_flg, TCHAR *msg_buf);

	CGridData_Filter* GetFilterGridData() { return m_grid_data_filter; }
	BOOL GetGridFilterMode() { return m_grid_filter_mode; }
	virtual int FilterData(int filter_col_no, const TCHAR* search_text, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp,
		int* find_cnt, CString* msg_str, BOOL b_err_at_no_find = TRUE);
	virtual void FilterOff();

	virtual BOOL IsSupportFilterFlg() { return FALSE; }
	virtual void UnsetFilterFlg(int row) {}
	virtual void SetFilterFlg(int row) {}
	virtual BOOL IsFilterRow(int row) { return FALSE; }

protected:
	COLORREF	m_default_text_color;
	POINT		m_selected_cell{ 0, 0 };
	struct grid_data_select_area_st m_select_area { 0, 0, { 0, 0 }, { 0, 0 }, { 0, 0 } };

	HWND		m_notify_wnd_list[MAX_GRID_NOTIFY_WND_CNT];

	TCHAR		m_row_header_buf[100]{ _T("") };

	void AllocDispInfo();
	void FreeDispInfo();
	virtual void SwapRow(int r1, int r2) { }

	int RestoreFilterData(griddata_filter_cond* d, CString* msg_str);

private:
	double CalcSelectedDataDouble(GRID_CALC_TYPE calc_type, CPoint start_pt, CPoint end_pt, POINT *result_pt,
		int *p_max_scale);
	CPoint CalcSelectedDataStr(GRID_CALC_TYPE calc_type, CPoint start_pt, CPoint end_pt);

	int SearchDataMainRegexp(int r, int rl, int c, int cl, POINT *result_pt, 
		int dir, HREG_DATA reg_data, BOOL b_select_area);

	struct disp_info_st {
		unsigned short col_size:15;
		unsigned short disp_flg:1;
	} *m_disp_info;

	int m_disp_info_cnt;

	class CGridUpdateProcess
	{
	public:
		virtual CString ProcessData(const TCHAR *text) = 0;
	};
	int ProcessUpdateCells(CGridUpdateProcess &process);

	double SortCmp(int *col_no, int *order, int sort_col_cnt, int r1, int r2);
	void SortDataMain(int *col_no, int *order, int sort_col_cnt, int left, int right);

	void SortDataComb11(int *col_no, int *order, int sort_col_cnt, int size);

	CGridDispData	m_disp_data;
	CGridDispData	*m_p_disp_data;

	virtual void FreeGridFilterData();
	BOOL		m_grid_filter_mode{ FALSE };
	CGridData_Filter* m_grid_data_filter{ NULL };
};

class CGridData_SwapRowCol : public CGridData
{
public:
	CGridData_SwapRowCol();
	virtual ~CGridData_SwapRowCol() { }

	virtual int Get_ColCnt() { return m_grid_data->Get_RowCnt(); }
	virtual int Get_RowCnt() { return m_grid_data->Get_ColCnt(); }
	virtual const TCHAR *Get_ColName(int col) {
		_stprintf(m_row_header_buf, _T("%d"), col + 1);
		return m_row_header_buf;
	}

	virtual int GetRowHeaderLen() { return m_row_header_len; }
	virtual const TCHAR *GetRowHeader(int row) { 
		return m_grid_data->Get_ColName(row);
	}
	virtual const TCHAR *Get_ColData(int row, int col) {
		return m_grid_data->Get_ColData(col, row);
	}
	virtual const TCHAR *Get_DispData(int row, int col) { return m_grid_data->Get_DispData(col, row); }
	virtual const TCHAR *Get_EditData(int row, int col) { return m_grid_data->Get_EditData(col, row); }
	virtual const TCHAR* Get_CopyData(int row, int col) { return m_grid_data->Get_CopyData(col, row); }
	virtual int Get_CopyDataBufSize(int row, int col, BOOL b_escape, char quote_char) {
		return m_grid_data->Get_CopyDataBufSize(col, row, b_escape, quote_char);
	}
	virtual int Get_ColLimit(int row, int col) { return m_grid_data->Get_ColLimit(col, row); }

	virtual BOOL IsColDataNull(int row, int col) {
		return m_grid_data->IsColDataNull(col, row);
	}
	virtual int Get_ColMaxSize(int col) { return 100; }

	virtual int Get_ColDataType(int row, int col) {
		return m_grid_data->Get_ColDataType(col, row);
	}

	virtual void SortData(int *col_no, int *order, int sort_col_cnt) { }

	virtual int SearchColumnRegexp(int start_cell, int *result_col, 
		int dir, BOOL b_loop, BOOL *b_looped, BOOL b_cur_cell, HREG_DATA reg_data);

	void SetGridData(CGridData *grid_data) { 
		m_grid_data = grid_data;
		SetDispData(grid_data->GetDispData());
	}

	void InitDispInfo(BOOL b_free_disp_info = TRUE);

	// 編集機能のサポート
	virtual BOOL IsEditable() { return m_grid_data->IsEditable(); }

	virtual BOOL IsInsertRow(int row) { return m_grid_data->IsInsertRow(row); }
	virtual BOOL IsUpdateRow(int row) { return m_grid_data->IsUpdateRow(row); }
	virtual BOOL IsDeleteRow(int row) { return m_grid_data->IsDeleteRow(row); }
	virtual BOOL IsUpdateCell(int row, int col) { return m_grid_data->IsUpdateCell(col, row); }

	virtual int InsertRows(int row, int row_cnt) { 
		SetCurCell();
		int r = m_grid_data->InsertRows(row, row_cnt);
		AllocDispInfo();
		UpdateCurCell();
		return r;
	}
	virtual int UpdateCell(int row, int col, const TCHAR *data, int len) { 
		SetCurCell();
		int r = m_grid_data->UpdateCell(col, row, data, len);
		UpdateCurCell();
		return r;
	}
	virtual int UpdateCells(POINT *pos1, POINT *pos2, const TCHAR *data, int len) {
		POINT new_pos1, new_pos2;
		new_pos1.x = pos1->y;
		new_pos1.y = pos1->x;
		new_pos2.x = pos2->y;
		new_pos2.y = pos2->x;
		SetCurCell();
		int r = m_grid_data->UpdateCells(&new_pos1, &new_pos2, data, len);
		UpdateCurCell();
		return r;
	}
	virtual int DeleteRows(int row1, int row2) {
		SetCurCell();
		int r = m_grid_data->DeleteRows(row1, row2);
		UpdateCurCell();
		return r;
	}
	virtual int DeleteNullRows(BOOL all_flg) { return m_grid_data->DeleteNullRows(all_flg); }

	virtual int Paste(const TCHAR *pstr);

	virtual CUndo *GetUndo() { return m_grid_data->GetUndo(); }
	virtual void StartUndoSet() { m_grid_data->StartUndoSet(); }
	virtual void EndUndoSet() { m_grid_data->EndUndoSet();}

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

	virtual BOOL IsEditableCell(int row, int col) { return m_grid_data->IsEditableCell(col, row); }

	virtual BOOL CanInsertRow() { return m_grid_data->CanInsertRow(); }
	virtual BOOL CanDeleteRow(int row) { return m_grid_data->CanDeleteRow(row); }
	virtual const TCHAR *GetOriginalText(int row, int col) { return m_grid_data->GetOriginalText(col, row); }

	virtual BOOL is_selected_cell();

protected:

private:
	void UpdateCurCell() {
		m_selected_cell.x = m_grid_data->get_cur_row();
		m_selected_cell.y = m_grid_data->get_cur_col();
	}
	void SetCurCell() { 
		if(m_selected_cell.x >= 0 && m_selected_cell.y >= 0) 
			m_grid_data->set_cur_cell(m_selected_cell.x, m_selected_cell.y);
	}
	int			m_row_header_len;
	CGridData	*m_grid_data;
};

#endif _GRID_DATA_H_INCLUDE
