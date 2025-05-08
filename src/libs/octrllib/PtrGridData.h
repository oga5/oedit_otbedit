/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef _PTRGRID_DATA_H_INCLUDE
#define _PTRGRID_DATA_H_INCLUDE

#include "griddata.h"

class CPtrGridData : public CGridData
{
private:
	void QSort(int col, int left, int right);
	void QSortSwap(int r1, int r2);
	const TCHAR **m_data;
	const TCHAR **m_col_name;

protected:
	int		m_col_cnt;
	int		m_row_cnt;
	int		m_init_row_cnt;
	int		m_alloc_row_cnt;

	int		GetColIdx(int row, int col) { return row * m_col_cnt + col; }
	virtual BOOL AllocData(int row, int col);
	virtual void FreeData();

	virtual void InitRow(int row);

public:
	CPtrGridData();
	virtual ~CPtrGridData();

	virtual int Init(int col, int init_row_cnt);
	void SetData(int row, int col, const TCHAR *data);
	virtual int AddRow();
	virtual int DeleteAllRow();

	virtual int Get_ColCnt() { return m_col_cnt; }
	virtual int Get_RowCnt() { return m_row_cnt; }
	virtual const TCHAR *Get_ColData(int row, int col);

	virtual const TCHAR *Get_ColName(int col) { return m_col_name[col]; }
	void Set_ColName(int col, const TCHAR *col_name) { m_col_name[col] = col_name; }

	void Sort(int sort_col);
};

class CPtrColorGridData : public CPtrGridData
{
private:
	COLORREF *m_color_data_bk;
	COLORREF *m_color_data_text;
	TCHAR	**m_row_header;

protected:
	virtual BOOL AllocData(int row, int col);
	virtual void FreeData();
	virtual void InitRow(int row);

	TCHAR *GetRowHeaderBuf(int row);

public:
	CPtrColorGridData();
	virtual ~CPtrColorGridData();

	virtual BOOL IsCustomizeBkColor() { return TRUE; }

	void	SetData(int row, int col, TCHAR *data, COLORREF bk_color, COLORREF text_color);
	void	SetColor(int row, int col, COLORREF bk_color, COLORREF text_color);
	void	SetRowHeader(int row, const TCHAR *text);
	void	SetRowHeader(int row, int row_cnt);

	virtual const TCHAR *GetRowHeader(int row);

	virtual COLORREF GetCellTextColor(int row, int col);
	virtual COLORREF GetCellBkColor(int row, int col);
};

#endif _PTRGRID_DATA_H_INCLUDE
