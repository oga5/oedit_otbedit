/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef __EDITABLE_GRID_DATA_H_INCLUDED__
#define __EDITABLE_GRID_DATA_H_INCLUDED__

#include "GridData.h"
#include "MemoryPool.h"
#include "Undo.h"

#include "GappedBufferTmpl.h"

#define OPE_UPDATE_CELL_ROW		1
#define OPE_UPDATE_CELL_FIRST	2
#define OPE_UPDATE_CELL			3
#define OPE_UPDATE_CELL_TEXT	4
#define OPE_UPDATE_CELL_DATA1	8
#define OPE_UPDATE_CELL_DATA2	9
#define OPE_DELETE_ROW			10
#define OPE_DELETE_INSERTED_ROW	11
#define OPE_INSERT_ROWS			20
#define OPE_GRID_SWAP_ROW		30
#define OPE_GRID_SORT_START		31
#define OPE_GRID_SORT_END		32
#define OPE_SAVE_CURSOR			40
#define OPE_FILTER_ON			50
#define OPE_FILTER_OFF			51

#define UPDATE_FLG		(0x01 << 0)
#define DELETE_FLG		(0x01 << 1)
#define INSERT_FLG		(0x01 << 2)
#define FILTER_FLG		(0x01 << 3)

#define EDITABLE_UPDATE		(0x0001 << 0)
#define EDITABLE_DELETE		(0x0001 << 1)
#define EDITABLE_INSERT		(0x0001 << 2)


class CEditableGridData : public CGridData
{
public:
	CEditableGridData();
	virtual ~CEditableGridData();

	virtual int GetSystemReserveColumnCnt() = 0;

	virtual int Get_RowCnt() { return m_row_buffer.get_row_cnt(); }

	virtual BOOL IsEditable() { return TRUE; }

	virtual BOOL IsInsertRow(int row);
	virtual BOOL IsUpdateRow(int row);
	virtual BOOL IsDeleteRow(int row);
	virtual BOOL IsUpdateCell(int row, int col);
	virtual BOOL IsNullRow(int row);

	virtual int InsertRows(int row, int row_cnt);
	virtual int UpdateCell(int row, int col, const TCHAR *data, int len);

	static int UpdateCellsCommon(CGridData* grid_data, POINT* pos1, POINT* pos2, const TCHAR* data, int len);
	virtual int UpdateCells(POINT *pos1, POINT *pos2, const TCHAR *data, int len);

	virtual int DeleteRows(int row1, int row2);
	virtual int DeleteNullRows(BOOL all_flg);

	static int PasteCommon(CGridData *grid_data, const TCHAR* pstr);
	virtual int Paste(const TCHAR *pstr);

	virtual void StartUndoSet() { m_undo->start_undo_set(); }
	virtual void EndUndoSet() { m_undo->end_undo_set();}

	virtual BOOL CanUndo();
	virtual BOOL CanRedo();
	virtual int Undo();
	virtual int Redo();
	virtual BOOL IsModified() { 
		if(m_undo == NULL) return FALSE;
		return (m_undo->get_undo_sequence() != 0); 
	}

	virtual void SortData(int *col_no, int *order, int sort_col_cnt);

	virtual BOOL IsEditableCell(int row, int col);
	void SetEditableCell(int col, BOOL editable);

	void SetReadOnly(int col);

	void SetUseMessageBeep(BOOL b_use) { m_use_message_beep = b_use; }
	void SetMoveCellAtInsertRows(BOOL b) { m_move_cell_at_insert_rows = b; }
	void SetMoveCellAtDeleteRows(BOOL b) { m_move_cell_at_delete_rows = b; }

	virtual BOOL CanInsertRow();
	virtual BOOL CanDeleteRow(int row);

	virtual const TCHAR *Get_ColData(int row, int col);
	virtual const TCHAR *GetOriginalText(int row, int col) { return _T(""); }

	virtual int GetRowHeaderLen() { return 7; }
	virtual const TCHAR *GetRowHeader(int row);

	virtual CUndo *GetUndo() { return m_undo; }

	int *GetSortedUpdateFlgIndexArr();
	int CmpUpdateFlg(int r1, int r2);

	int GetInsertRowCnt() { return m_insert_row_cnt; }
	int GetUpdateRowCnt() { return m_update_row_cnt; }
	int GetDeleteRowCnt() { return m_delete_row_cnt; }

	virtual int FilterData(int filter_col_no, const TCHAR* search_text, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp,
		int* find_cnt, CString* msg_str, BOOL b_err_at_no_find = TRUE);
	virtual void FilterOff();

	virtual BOOL IsSupportFilterFlg() { return TRUE; }
	virtual void UnsetFilterFlg(int row);
	virtual void SetFilterFlg(int row);
	virtual BOOL IsFilterRow(int row);

protected:
	virtual void SwapRow(int r1, int r2);

	virtual int UpdateCellMain(int row, int col, const TCHAR *data, int len);
	virtual int DeleteRowMain(int row);

	int GetRowIdx(int row) { return m_row_buffer[row].row_idx; }

	const TCHAR *GetUpdatedText(int row, int col) { 
		if(m_row_buffer[row].updated_text == NULL) return NULL;
		return m_row_buffer[row].updated_text[col];
	}

	void SetEditable(int col, int editable) { m_editable[col] = editable; }
	int GetEditable(int col) { return m_editable[col]; }

	void FreeDataMemory();
	int AllocDataMemory(int row_cnt, int col_cnt);

	void *GetCellData(int row, int col);
	void SetCellData(int row, int col, void *data);
	void SetCellDataSize(int size) { m_cell_data_size = size; }

	// FIXME: privateにする
	BOOL		m_can_insert_row;
	BOOL		m_can_delete_row;

private:
	CUndo		*m_undo;
	CFixedMemoryPool	*m_update_cell_mem_pool;
	CFixedMemoryPool	*m_update_text_mem_pool;
	CFixedMemoryPool	*m_cell_data_mem_pool;

	int			*m_editable;
	BOOL		m_use_message_beep;
	BOOL		m_move_cell_at_insert_rows;
	BOOL		m_move_cell_at_delete_rows;

	size_t		m_cell_data_size;

	int			m_insert_row_cnt;
	int			m_delete_row_cnt;
	int			m_update_row_cnt;

	struct grid_row_data_st {
		int		row_idx;
		char	row_flg;
		char	*update_cell_flg;
		const TCHAR	**updated_text;
		void	*cell_data;
	};

	typedef GappedBufferTmpl<struct grid_row_data_st> CEditableGridRowBuffer;
	CEditableGridRowBuffer	m_row_buffer;

	void SetInsertFlg(int row);
	void SetUpdateFlg(int row);
	void UnSetUpdateFlg(int row);
	void SetDeleteFlg(int row);
	void UnSetDeleteFlg(int row);
	void SetUpdateCellFlg(int row, int col);
	void UnSetUpdateCellFlg(int row, int col);

	void OpeSaveCursor();

	void UnsetRowFlg(int row, char flg) { m_row_buffer[row].row_flg &= ~flg; }
	void SetRowFlg(int row, char flg) { m_row_buffer[row].row_flg |= flg; }
	BOOL CheckRowFlg(int row, char flg) { return (m_row_buffer[row].row_flg & flg); }

	char *GetUpdateCellFlg(int row) { return m_row_buffer[row].update_cell_flg; }

	int InsertRowMain(int row);
	int InsertRowsMain(int row, int row_cnt);
	int DeleteInsertedRow(int row);
	void SetUpdatedText(int row, int col, const TCHAR *p);
	void SwapRowMain(int r1, int r2);

	void InitDataMemory();

	void InitRowData(struct grid_row_data_st *p_data, int row_idx);
	void ClearRowData(struct grid_row_data_st *p_data);

	void SetCellDataMain(int row, int col, void *data);
};

#endif __EDITABLE_GRID_DATA_H_INCLUDED__
