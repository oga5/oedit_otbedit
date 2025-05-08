/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef _STRGRID_DATA_H_INCLUDE
#define _STRGRID_DATA_H_INCLUDE

#include "griddata.h"
#include "EditableGridData.h"

class CEditableStrGridData : public CEditableGridData {
private:
	int			m_col_cnt;
	CString		*m_str;
	CString		*m_col_name;

	void FreeMem() {
		if(m_str != NULL) {
			delete[] m_str;
			m_str = NULL;
		}
		if(m_col_name != NULL) {
			delete[] m_col_name;
			m_col_name = NULL;
		}
		m_col_cnt = 0;
	}
	int GetColIdx(int row, int col) { return m_col_cnt * row + col; }

public:
	CEditableStrGridData() { m_str = NULL; m_col_name = NULL; m_col_cnt = 0; }
	virtual ~CEditableStrGridData() { FreeMem(); }

	virtual int Get_ColCnt() { return m_col_cnt; }
	virtual int GetSystemReserveColumnCnt() { return 0; }

	int Init(int row, int col) {
		m_col_cnt = col;
		int str_arr_size = row * col;
		m_str = new CString[str_arr_size];
		m_col_name = new CString[col];
		CEditableGridData::AllocDataMemory(row, col);
		return 0;
	}
	void SetColName(int col, const TCHAR *name) {
		ASSERT(m_col_name != NULL);
		if(m_col_name == NULL) return;
		m_col_name[col] = name;
	}
	void InitData(int row, int col, const TCHAR *str) {
		ASSERT(m_str != NULL);
		if(m_str == NULL) return;
		m_str[GetColIdx(row, col)] = str;
	}
	
	virtual const TCHAR *Get_ColData(int row, int col) {
		if(CEditableGridData::IsUpdateCell(row, col)) {
			return CEditableGridData::Get_ColData(row, col);
		}
		if(m_str == NULL) return _T("");
		return m_str[GetColIdx(row, col)];
	}
	virtual const TCHAR *Get_ColName(int col) {
		if(m_col_name == NULL) return _T("");
		return m_col_name[col];
	}
};

class CStrGridData : public CGridData
{
private:
	int			m_col_cnt;
	int			m_row_cnt;
	CString		*m_str;
	CString		*m_col_name;
	BOOL		*m_editable_col;
	BOOL		m_editable;

	int		AllocMem(int row, int col, BOOL editable);
	void	FreeMem();

	int		GetColIdx(int row, int col) { return m_col_cnt * row + col; }

public:
	CStrGridData();
	virtual ~CStrGridData();

	int		Init(int row, int col, BOOL editable = TRUE);
	void	SetColName(int col, const TCHAR *name);
	void	SetEditable(BOOL editable);
	void	SetEditableCell(int col, BOOL editable);

	virtual int Get_ColCnt() { return m_col_cnt; }
	virtual int Get_RowCnt() { return m_row_cnt; }

	virtual const TCHAR *Get_ColName(int col);
	virtual int Get_ColMaxSize(int col);
	virtual const TCHAR *Get_ColData(int row, int col);
	virtual int Get_ColDataType(int row, int col) { return GRID_DATA_VARCHAR_TYPE; }

	virtual BOOL IsColDataNull(int row, int col);

	virtual BOOL IsEditable() { return m_editable; }

	virtual int Get_ColLimit(int row, int col) { return 1024; }
	virtual int InsertRow(int row) { return 0; }
	virtual int InsertRows(int row, int row_cnt) { return 0; }
	virtual int DeleteRow(int row) { return 0; }
	virtual int DeleteRows(int row1, int row2) { return 0; }

	virtual int UpdateCells(POINT *pos1, POINT *pos2, const TCHAR *data, int len);
	virtual int UpdateCell(int row, int col, const TCHAR *data, int len);

	virtual BOOL IsEditableCell(int row, int col);

	virtual void StartUndoSet() {}
	virtual void EndUndoSet() {}

	virtual BOOL IsInsertRow(int row) { return FALSE; }
	virtual BOOL IsUpdateRow(int row) { return FALSE; }
	virtual BOOL IsDeleteRow(int row) { return FALSE; }
	virtual BOOL IsUpdateCell(int row, int col) { return FALSE; }
	virtual BOOL CanUndo() { return FALSE; }
	virtual BOOL CanRedo() { return FALSE; }
	virtual int Undo() { return 0; }
	virtual int Redo() { return 0; }
	virtual int Paste(const TCHAR *pstr);
};

#endif _STRGRID_DATA_H_INCLUDE
