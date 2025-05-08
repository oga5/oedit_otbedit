/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"

#include "EditableGridData.h"
#include "hankaku.h"
#include "ostrutil.h"
#include "griddatafilter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static const char *THIS_FILE = __FILE__;
#endif

CEditableGridData::CEditableGridData()
{
	m_use_message_beep = FALSE;

	m_can_insert_row = FALSE;
	m_can_delete_row = FALSE;

	m_move_cell_at_insert_rows = TRUE;
	m_move_cell_at_delete_rows = TRUE;

	m_cell_data_size = 0;

	InitDataMemory();
}

CEditableGridData::~CEditableGridData()
{
	FreeDataMemory();
}

BOOL CEditableGridData::IsInsertRow(int row)
{ 
	return (CheckRowFlg(row, INSERT_FLG) && !CheckRowFlg(row, DELETE_FLG));
}

BOOL CEditableGridData::IsUpdateRow(int row)
{
	return (CheckRowFlg(row, UPDATE_FLG) && !CheckRowFlg(row, INSERT_FLG | DELETE_FLG));
}

BOOL CEditableGridData::IsDeleteRow(int row)
{
	return (CheckRowFlg(row, DELETE_FLG));
}

BOOL CEditableGridData::IsUpdateCell(int row, int col)
{
	char *p = GetUpdateCellFlg(row);
	p += col / 8;
	return (*p & (0x01 << (col % 8)));
}

void CEditableGridData::SetInsertFlg(int row)
{
	SetRowFlg(row, INSERT_FLG);
}

void CEditableGridData::SetUpdateFlg(int row)
{
	if (IsUpdateRow(row)) return;

	SetRowFlg(row, UPDATE_FLG);

	if (!IsDeleteRow(row) && !IsInsertRow(row)) {
		m_update_row_cnt++;
	}
}

void CEditableGridData::UnSetUpdateFlg(int row)
{
	if (!IsUpdateRow(row)) return;

	UnsetRowFlg(row, UPDATE_FLG);

	if (!IsDeleteRow(row) && !IsInsertRow(row)) {
		m_update_row_cnt--;
	}
}

void CEditableGridData::SetDeleteFlg(int row)
{
	if (IsDeleteRow(row)) return;

	m_delete_row_cnt++;
	if (IsUpdateRow(row)) {
		m_update_row_cnt--;
	}

	SetRowFlg(row, DELETE_FLG);
}

void CEditableGridData::UnSetDeleteFlg(int row)
{
	if (!IsDeleteRow(row)) return;

	UnsetRowFlg(row, DELETE_FLG);

	m_delete_row_cnt--;
	if(IsUpdateRow(row)) {
		m_update_row_cnt++;
	}
}

void CEditableGridData::SetUpdateCellFlg(int row, int col)
{
	char *p = GetUpdateCellFlg(row);
	p += col / 8;
	*p |= 0x01 << (col % 8);
}

void CEditableGridData::UnSetUpdateCellFlg(int row, int col)
{
	char *p = GetUpdateCellFlg(row);
	p += col / 8;
	*p &= ~(0x01 << (col % 8));
}

void CEditableGridData::SortData(int *col_no, int *order, int sort_col_cnt)
{
	m_undo->start_undo_set();
	m_undo->next_undo(OPE_GRID_SORT_START);
	CGridData::SortData(col_no, order, sort_col_cnt);
	m_undo->next_undo(OPE_GRID_SORT_END);
	m_undo->end_undo_set();
}

BOOL CEditableGridData::CanUndo()
{
	if(m_undo == NULL) return FALSE;
	return m_undo->can_undo();
}

BOOL CEditableGridData::CanRedo()
{
	if(m_undo == NULL) return FALSE;
	return m_undo->can_redo();
}

BOOL CEditableGridData::IsNullRow(int row)
{
	// 全てのカラムがNULLか調べる
	int		col;
	int		col_cnt = Get_ColCnt();

	for(col = 0; col < col_cnt; col++) {
		if(!IsColDataNull(row, col)) return FALSE;
	}

	return TRUE;
}

void CEditableGridData::InitRowData(struct grid_row_data_st *p_data, int row_idx)
{
	p_data->row_idx = row_idx;
	p_data->row_flg = 0;

	p_data->update_cell_flg = (char *)m_update_cell_mem_pool->calloc();
	p_data->updated_text = NULL;

	if(m_cell_data_size > 0) {
		p_data->cell_data = (BYTE *)m_cell_data_mem_pool->calloc();
	} else {
		p_data->cell_data = NULL;
	}
}

void CEditableGridData::ClearRowData(struct grid_row_data_st *p_data)
{
	if(p_data->update_cell_flg != NULL) {
		m_update_cell_mem_pool->free(p_data->update_cell_flg);
		p_data->update_cell_flg = NULL;
	}
	if(p_data->updated_text != NULL) {
		m_update_text_mem_pool->free(p_data->updated_text);
		p_data->updated_text = NULL;
	}
	if(p_data->cell_data != NULL) {
		m_cell_data_mem_pool->free(p_data->cell_data);
		p_data->cell_data = NULL;
	}
}

const TCHAR *CEditableGridData::Get_ColData(int row, int col)
{
	if(GetUpdatedText(row, col) != NULL) {
		return GetUpdatedText(row, col);
	}
	if(IsInsertRow(row)) return _T("");
	return GetOriginalText(row, col);
}

int CEditableGridData::PasteCommon(CGridData* grid_data, const TCHAR* pstr)
{
//	if(*pstr == '\0') return 0;

	int		x_start = grid_data->get_cur_col();
	int		y_start = grid_data->get_cur_row();
	int		x_max = grid_data->Get_ColCnt() - 1;
	int		y_max = grid_data->Get_RowCnt() - 1;

	BOOL	repeat_paste_mode = FALSE;
	BOOL	single_col_mode = FALSE;

	if(grid_data->HaveSelected()) {
		int paste_col_cnt = ostr_get_tsv_col_cnt(pstr);

		x_start = min(grid_data->GetSelectArea()->pos1.x, grid_data->GetSelectArea()->pos2.x);
		x_max = max(grid_data->GetSelectArea()->pos1.x, grid_data->GetSelectArea()->pos2.x);
		y_start = min(grid_data->GetSelectArea()->pos1.y, grid_data->GetSelectArea()->pos2.y);
		y_max = max(grid_data->GetSelectArea()->pos1.y, grid_data->GetSelectArea()->pos2.y);

		if(x_max - x_start < paste_col_cnt - 1) {
			// (x方向で)選択範囲よりコピーするデータの範囲の方が広い場合、選択範囲を広くする
			x_max = min(x_start + paste_col_cnt - 1, grid_data->Get_ColCnt() - 1);
			if(grid_data->GetSelectArea()->pos1.x < grid_data->GetSelectArea()->pos2.x) {
				grid_data->GetSelectArea()->pos2.x = x_max;
			} else {
				grid_data->GetSelectArea()->pos1.x = x_max;
			}
		}

		// 選択範囲が複数行で、コピーするテキストが1行だけの場合、繰り返しペーストする
		if(y_start != y_max && ostr_is_tsv_single_row(pstr)) {
			repeat_paste_mode = TRUE;
		}

		// 選択範囲が複数セルで、コピーするテキストが1セル分の場合、右のセルにも繰り返しペーストする
		if (x_start != x_max && ostr_is_tsv_single_col(pstr)) {
			single_col_mode = TRUE;
		}
	}

	POINT	pt = { x_start, y_start };

	grid_data->StartUndoSet();

	const int paste_buf_size = 1024 * 1024 * 4;	// 4M
	const TCHAR* p = pstr;
	TCHAR* buf = (TCHAR*)malloc(paste_buf_size);
	if(buf == NULL) return 0;

	for(;;) {
		if (!single_col_mode || pt.x == x_start) {
			p = ostr_get_tsv_data(p, buf, paste_buf_size);
		}

		if(grid_data->IsEditableCell(pt.y, pt.x)) {
			grid_data->UpdateCell(pt.y, pt.x, buf, -1);
		}

		if (single_col_mode) {
			(pt.x)++;
			if (pt.x <= x_max) continue;
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

			// ペーストするテキストがなくなったら終了
			if (*p == '\0') break;

			// 最後の改行は無視する
			if((*p == '\r' || *p == '\n') && *(p + 1) == '\0') break;
		}

		if(*p == '\0') break;
		p++;
	}

	grid_data->EndUndoSet();

	free(buf);

	return 0;
}

int CEditableGridData::Paste(const TCHAR *pstr)
{
	return PasteCommon(this, pstr);
}

void CEditableGridData::SwapRowMain(int r1, int r2)
{
	m_row_buffer.swap_row(r1, r2);
}

void CEditableGridData::SwapRow(int r1, int r2)
{
	m_undo->next_undo(OPE_GRID_SWAP_ROW);
	m_undo->get_cur_undo_data()->set_row(r1);
	m_undo->get_cur_undo_data()->set_col(r2);

	SwapRowMain(r1, r2);
}

void CEditableGridData::OpeSaveCursor()
{
	m_undo->next_undo(OPE_SAVE_CURSOR);
	m_undo->get_cur_undo_data()->set_row(get_cur_row());
	m_undo->get_cur_undo_data()->set_col(get_cur_col());
}

int CEditableGridData::UpdateCellsCommon(CGridData* grid_data, POINT* pos1, POINT* pos2, const TCHAR* data, int len)
{
	int		row, col;
	int		x1, x2;
	BOOL	update = FALSE;

	if(pos1->x <= pos2->x) {
		x1 = pos1->x;
		x2 = pos2->x;
	} else {
		x1 = pos2->x;
		x2 = pos1->x;
	}

	if(len == -1) len = (int)_tcslen(data);

	for(row = pos1->y; row <= pos2->y; row++) {
		for(col = x1; col <= x2; col++) {
			if(grid_data->IsEditableCell(row, col) &&
				(_tcslen(grid_data->Get_ColData(row, col)) != (UINT)len ||
					_tcsnccmp(grid_data->Get_ColData(row, col), data, len) != 0)) {
				update = TRUE;
				break;
			}
		}
	}
	if(update == FALSE) return 0;	// 変更無し

	grid_data->StartUndoSet();

	for(row = pos1->y; row <= pos2->y; row++) {
		for(col = x1; col <= x2; col++) {
			if(grid_data->IsEditableCell(row, col)) {
				grid_data->UpdateCell(row, col, data, len);
			}
		}
	}

	grid_data->EndUndoSet();

	return 0;
}

int CEditableGridData::UpdateCells(POINT *pos1, POINT *pos2, const TCHAR *data, int len)
{
	return UpdateCellsCommon(this, pos1, pos2, data, len);
}

int CEditableGridData::UpdateCellMain(int row, int col, const TCHAR *data, int len)
{
	ASSERT(row >= 0 && row < Get_RowCnt());
	ASSERT(col >= 0 && col < Get_ColCnt());

	if(len == -1) len = (int)_tcslen(data);

	if(_tcslen(Get_ColData(row, col)) == (UINT)len &&
		_tcsncmp(Get_ColData(row, col), data, len) == 0) return 0;

	if(len > Get_ColLimit(row, col)) {
		if(m_use_message_beep) MessageBeep(MB_ICONEXCLAMATION);
		return 1;
	}

	if(IsUpdateRow(row)) {
		if(IsUpdateCell(row, col)) {
			m_undo->next_undo(OPE_UPDATE_CELL);
		} else {
			m_undo->next_undo(OPE_UPDATE_CELL_FIRST);
		}
	} else {
		m_undo->next_undo(OPE_UPDATE_CELL_ROW);
	}
	m_undo->get_cur_undo_data()->set_row(row);
	m_undo->get_cur_undo_data()->set_col(col);

	// 変更前データのポインタを保存
	const TCHAR *p = Get_ColData(row, col);
	m_undo->get_cur_undo_data()->set_data((INT_PTR)p);

	// 変更後データを保存
	m_undo->next_undo(OPE_UPDATE_CELL_TEXT, TRUE);
	m_undo->get_cur_undo_data()->set_row(row);
	m_undo->get_cur_undo_data()->set_col(col);

	m_undo->add_undo_chars(data, len);
	m_undo->add_undo_char('\0');

	SetUpdatedText(row, col, m_undo->get_cur_undo_data()->get_char_buf());

	SetUpdateFlg(row);
	SetUpdateCellFlg(row, col);

	return 0;
}

int CEditableGridData::UpdateCell(int row, int col, const TCHAR *data, int len)
{
	ASSERT(IsEditableCell(row, col));

	return UpdateCellMain(row, col, data, len);
}

int CEditableGridData::DeleteInsertedRow(int row)
{
	ClearRowData(&m_row_buffer[row]);
	m_row_buffer.delete_row(row);

	m_insert_row_cnt--;

	if(m_move_cell_at_delete_rows) m_selected_cell.y = row - 1;

	return 0;
}

int CEditableGridData::DeleteRowMain(int row)
{
	ASSERT(row >= 0 && row < Get_RowCnt());

	if(m_row_buffer.get_row_cnt() <= 0) return 0;

	if(IsInsertRow(row)) {
		int		i;
		for(i = 0; i < Get_ColCnt(); i++) {
			UpdateCellMain(row, i, _T(""), 0);
		}
		m_undo->next_undo(OPE_DELETE_INSERTED_ROW);
		m_undo->get_cur_undo_data()->set_row(row);
		m_undo->get_cur_undo_data()->set_col(get_cur_col());
		DeleteInsertedRow(row);
	} else {
		if(IsDeleteRow(row)) {
			UnSetDeleteFlg(row);
		} else {
			SetDeleteFlg(row);
		}
		m_undo->next_undo(OPE_DELETE_ROW);
		m_undo->get_cur_undo_data()->set_row(row);
		m_undo->get_cur_undo_data()->set_col(get_cur_col());
	}

	return 0;
}

int CEditableGridData::DeleteRows(int row1, int row2)
{
	ASSERT(row1 >= 0 && row1 < Get_RowCnt());
	ASSERT(row2 >= 0 && row2 < Get_RowCnt());

	int		r;

	if(m_row_buffer.get_row_cnt() <= 0) return 0;

	m_undo->start_undo_set();

	OpeSaveCursor();
	
	for(r = row2; r >= row1; r--) {
		DeleteRowMain(r);
	}

	OpeSaveCursor();

	m_undo->end_undo_set();

	return 0;
}

int CEditableGridData::DeleteNullRows(BOOL all_flg)
{
	int		r;
	int		del_row_cnt = 0;

	if(m_row_buffer.get_row_cnt() <= 0) return 0;

	m_undo->start_undo_set();

	OpeSaveCursor();
	
	for(r = Get_RowCnt() - 1; r >= 0; r--) {
		if(IsDeleteRow(r)) continue;

		// 挿入されたレコードか調べる
		if(!all_flg && !IsInsertRow(r)) continue;

		if(IsNullRow(r)) {
			DeleteRowMain(r);
			del_row_cnt++;
		}
	}

	OpeSaveCursor();

	m_undo->end_undo_set();

	return del_row_cnt;
}

int CEditableGridData::InsertRowMain(int row)
{
	ASSERT(row >= -1 && row < Get_RowCnt());

	row++;

	m_row_buffer.insert_row(row);
	m_insert_row_cnt++;

	// フラグを初期化
	InitRowData(&m_row_buffer[row], -1);
	SetInsertFlg(row);

	if(m_move_cell_at_insert_rows) m_selected_cell.y = row;

	return 0;
}

int CEditableGridData::InsertRowsMain(int row, int row_cnt)
{
	ASSERT(row >= -1 && row < Get_RowCnt());

	for(int r = 0; r < row_cnt; r++) {
		if(InsertRowMain(row) != 0) return 1;
		row++;
	}

	return 0;
}

int CEditableGridData::InsertRows(int row, int row_cnt)
{
	ASSERT(row >= -1 && row < Get_RowCnt());

	int cur_row = get_cur_row();

	m_undo->start_undo_set();

	if(InsertRowsMain(row, row_cnt) != 0) return 1;

	m_undo->next_undo(OPE_INSERT_ROWS);
	m_undo->get_cur_undo_data()->set_row(cur_row);
	m_undo->get_cur_undo_data()->set_col(get_cur_col());
	m_undo->get_cur_undo_data()->set_data(row_cnt);

	m_undo->end_undo_set();

	return 0;
}

int CEditableGridData::Undo()
{
	CUndoData	*undo_data;

	m_undo->clear_cur_operation();

	if(m_undo->get_cur_undo_data() == NULL) return 1;

	for(undo_data = m_undo->get_cur_undo_data()->get_last(); undo_data != NULL;
		undo_data = undo_data->get_prev()) {
		switch(undo_data->get_operation()) {
		case 0:		// dummy
			break;
		case OPE_UPDATE_CELL_TEXT:
			break;
		case OPE_UPDATE_CELL_ROW:
			UnSetUpdateFlg(undo_data->get_row());
		case OPE_UPDATE_CELL_FIRST:
			UnSetUpdateCellFlg(undo_data->get_row(), undo_data->get_col());
		case OPE_UPDATE_CELL:
			m_selected_cell.y = undo_data->get_row();
			m_selected_cell.x = undo_data->get_col();
			SetUpdatedText(undo_data->get_row(), undo_data->get_col(), (const TCHAR *)undo_data->get_data());
			break;
		case OPE_INSERT_ROWS:
			m_selected_cell.y = undo_data->get_row();
			m_selected_cell.x = undo_data->get_col();
			{
				INT_PTR	r;
				INT_PTR	row_cnt = undo_data->get_data();
				int		row = undo_data->get_row() + 1;
				for(r = 0; r < row_cnt; r++) {
					DeleteInsertedRow(row);
				}
			}
			break;
		case OPE_DELETE_ROW:
			m_selected_cell.y = undo_data->get_row();
			m_selected_cell.x = undo_data->get_col();
			if(IsDeleteRow(undo_data->get_row())) {
				UnSetDeleteFlg(undo_data->get_row());
			} else {
				SetDeleteFlg(undo_data->get_row());
			}
			break;
		case OPE_DELETE_INSERTED_ROW:
			m_selected_cell.y = undo_data->get_row();
			m_selected_cell.x = undo_data->get_col();
			InsertRowMain(undo_data->get_row() - 1);
			break;
		case OPE_SAVE_CURSOR:
			m_selected_cell.y = undo_data->get_row();
			m_selected_cell.x = undo_data->get_col();
			break;
		case OPE_GRID_SWAP_ROW:
			SwapRowMain(undo_data->get_row(), undo_data->get_col());
			break;
		case OPE_UPDATE_CELL_DATA1:
			m_selected_cell.y = undo_data->get_row();
			m_selected_cell.x = undo_data->get_col();
			SetCellDataMain(m_selected_cell.y, m_selected_cell.x, undo_data->get_buf_data());
			break;
		case OPE_UPDATE_CELL_DATA2:
			break;
		case OPE_FILTER_ON:
			CGridData::FilterOff();
			break;
		case OPE_FILTER_OFF:
			{
				CString msg_str;
				griddata_filter_cond* filter_cond = (griddata_filter_cond*)undo_data->get_buf_data();
				RestoreFilterData(filter_cond, &msg_str);
			}
			break;
		case OPE_GRID_SORT_START:
			if(GetGridFilterMode()) {
				GetFilterGridData()->RestoreRowIdxFromFilterRowFlg();
			}
			break;
		}
	}
	m_undo->decr_undo_cnt();

	return 0;
}

int CEditableGridData::Redo()
{
	CUndoData	*undo_data;

	m_undo->clear_cur_operation();

	m_undo->incr_undo_cnt();
	if(m_undo->get_cur_undo_data() == NULL) {
		m_undo->decr_undo_cnt();
		return 1;
	}

	for(undo_data = m_undo->get_cur_undo_data()->get_first(); undo_data != NULL; 
		undo_data = undo_data->get_next()) {
		switch(undo_data->get_operation()) {
		case 0:		// dummy
			break;
		case OPE_UPDATE_CELL_ROW:
			SetUpdateFlg(undo_data->get_row());
		case OPE_UPDATE_CELL_FIRST:
			SetUpdateCellFlg(undo_data->get_row(), undo_data->get_col());
		case OPE_UPDATE_CELL:
			undo_data = undo_data->get_next();
			ASSERT(undo_data->get_operation() == OPE_UPDATE_CELL_TEXT);
			m_selected_cell.y = undo_data->get_row();
			m_selected_cell.x = undo_data->get_col();
			SetUpdatedText(undo_data->get_row(), undo_data->get_col(), undo_data->get_char_buf());
			break;
		case OPE_DELETE_INSERTED_ROW:
			m_selected_cell.y = undo_data->get_row();
			m_selected_cell.x = undo_data->get_col();

			DeleteInsertedRow(undo_data->get_row());
			break;
		case OPE_DELETE_ROW:
			m_selected_cell.y = undo_data->get_row();
			m_selected_cell.x = undo_data->get_col();
			if(IsDeleteRow(undo_data->get_row())) {
				UnSetDeleteFlg(undo_data->get_row());
			} else {
				SetDeleteFlg(undo_data->get_row());
			}
			break;
		case OPE_INSERT_ROWS:
			m_selected_cell.y = undo_data->get_row();
			m_selected_cell.x = undo_data->get_col();
			InsertRowsMain(undo_data->get_row(), (int)undo_data->get_data());
			break;
		case OPE_SAVE_CURSOR:
			m_selected_cell.y = undo_data->get_row();
			m_selected_cell.x = undo_data->get_col();
			break;
		case OPE_GRID_SWAP_ROW:
			SwapRowMain(undo_data->get_row(), undo_data->get_col());
			break;
		case OPE_UPDATE_CELL_DATA1:
			break;
		case OPE_UPDATE_CELL_DATA2:
			m_selected_cell.y = undo_data->get_row();
			m_selected_cell.x = undo_data->get_col();
			SetCellDataMain(m_selected_cell.y, m_selected_cell.x, undo_data->get_buf_data());
			break;
		case OPE_FILTER_ON:
			{
				CString msg_str;
				griddata_filter_cond* filter_cond = (griddata_filter_cond*)undo_data->get_buf_data();
				RestoreFilterData(filter_cond, &msg_str);
			}
			break;
		case OPE_FILTER_OFF:
			CGridData::FilterOff();
			break;
		case OPE_GRID_SORT_END:
			if(GetGridFilterMode()) {
				GetFilterGridData()->RestoreRowIdxFromFilterRowFlg();
			}
			break;
		}
	}

	return 0;
}

void CEditableGridData::SetEditableCell(int col, BOOL editable)
{
	if(editable) {
		m_editable[col] |= EDITABLE_UPDATE;
	} else {
		m_editable[col] &= ~EDITABLE_UPDATE;
	}
}

void CEditableGridData::SetReadOnly(int col)
{
	SetEditable(col, 0);
}

BOOL CEditableGridData::IsEditableCell(int row, int col)
{
	if(m_editable[col] & EDITABLE_UPDATE) return TRUE;
	if(m_editable[col] & EDITABLE_INSERT && IsInsertRow(row)) return TRUE;
	return FALSE;
}

int CEditableGridData::AllocDataMemory(int row_cnt, int col_cnt)
{
	int		i;

	FreeDataMemory();
	m_undo = new CUndo(INT_MAX);

	m_row_buffer.init_buffer(row_cnt);

	if(col_cnt > 0) {
		m_update_cell_mem_pool = new CFixedMemoryPool((col_cnt + 7) / 8, 100);
		m_update_text_mem_pool = new CFixedMemoryPool(sizeof(void *) * col_cnt, 100);
		if(m_cell_data_size > 0) {
			m_cell_data_mem_pool = new CFixedMemoryPool((size_t)m_cell_data_size * col_cnt, 100);
		}
	}

	if(col_cnt > 0) {
		m_editable = (BOOL*)malloc(col_cnt * sizeof(int));
		if(m_editable == NULL) return 1;
		for(i = 0; i < col_cnt; i++) {
			m_editable[i] = EDITABLE_UPDATE | EDITABLE_INSERT | EDITABLE_DELETE;
		}
	}

	for(i = 0; i < m_row_buffer.get_row_cnt(); i++) {
		// フラグを初期化
		InitRowData(&m_row_buffer[i], i);
	}

	AllocDispInfo();

	return 0;
}

void CEditableGridData::InitDataMemory()
{
	m_undo = NULL;
	m_editable = NULL;

	m_update_cell_mem_pool = NULL;
	m_update_text_mem_pool = NULL;
	m_cell_data_mem_pool = NULL;

	m_insert_row_cnt = 0;
	m_delete_row_cnt = 0;
	m_update_row_cnt = 0;
}

void CEditableGridData::FreeDataMemory()
{
	m_row_buffer.free_data();

	if(m_editable != NULL) {
		free(m_editable);
		m_editable = NULL;
	}
	if(m_undo != NULL) {
		delete m_undo;
		m_undo = NULL;
	}

	if(m_update_cell_mem_pool != NULL) {
		delete m_update_cell_mem_pool;
		m_update_cell_mem_pool = NULL;
	}
	if(m_update_text_mem_pool != NULL) {
		delete m_update_text_mem_pool;
		m_update_text_mem_pool = NULL;
	}
	if(m_cell_data_mem_pool != NULL) {
		delete m_cell_data_mem_pool;
		m_cell_data_mem_pool = NULL;
	}

	InitDataMemory();
}

BOOL CEditableGridData::CanInsertRow()
{
	return m_can_insert_row;
}

BOOL CEditableGridData::CanDeleteRow(int row)
{
	if(row < 0 || row >= Get_RowCnt()) return FALSE;

	if(IsInsertRow(row)) return TRUE;
	return m_can_delete_row;
}

const TCHAR *CEditableGridData::GetRowHeader(int row)
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

void CEditableGridData::SetUpdatedText(int row, int col, const TCHAR *p)
{
	if(m_row_buffer[row].updated_text == NULL) {
		m_row_buffer[row].updated_text = (const TCHAR **)m_update_text_mem_pool->calloc();
	}
	m_row_buffer[row].updated_text[col] = p;
}

void *CEditableGridData::GetCellData(int row, int col)
{
	if(m_row_buffer[row].cell_data == NULL) {
		return NULL;
	}

	BYTE *p = (BYTE *)m_row_buffer[row].cell_data;
	p += col * m_cell_data_size;

	return (void *)p;
}

void CEditableGridData::SetCellDataMain(int row, int col, void *data)
{
	void *p = GetCellData(row, col);
	if(p == NULL) return;

	memcpy(p, data, m_cell_data_size);
}

void CEditableGridData::SetCellData(int row, int col, void *data)
{
	// 変更前データを保存
	m_undo->next_undo(OPE_UPDATE_CELL_DATA1);
	m_undo->get_cur_undo_data()->set_row(get_cur_row());
	m_undo->get_cur_undo_data()->set_col(get_cur_col());
	m_undo->get_cur_undo_data()->set_buf_data(GetCellData(row, col), m_cell_data_size);

	// 変更後データを保存
	m_undo->next_undo(OPE_UPDATE_CELL_DATA2, TRUE);
	m_undo->get_cur_undo_data()->set_row(get_cur_row());
	m_undo->get_cur_undo_data()->set_col(get_cur_col());
	m_undo->get_cur_undo_data()->set_buf_data(data, m_cell_data_size);

	SetCellDataMain(row, col, data);
}

int CEditableGridData::CmpUpdateFlg(int r1, int r2)
{
	int flg_size = (Get_ColCnt() + 7) / 8;
	char *p1 = GetUpdateCellFlg(r1);
	char *p2 = GetUpdateCellFlg(r2);

	return memcmp(p1, p2, flg_size);
}

int *CEditableGridData::GetSortedUpdateFlgIndexArr()
{
	const int row_cnt = Get_RowCnt();
	if(row_cnt == 0) return NULL;

	// NOTE: C6385/C6386の警告を回避するために少し大き目に確保 (本来は+11は不要)
	int	*idx_arr = (int *)malloc((row_cnt + 11) * sizeof(int));
	int row;

	if(idx_arr == NULL) return NULL;

	// 初期化
	for(row = 0; row < row_cnt; row++) {
		idx_arr[row] = row;
	}

	// ソート (comb sort)
	{
		int	gap = row_cnt;
		int done = 0;
		int i;

		while((gap > 1) || !done) {
			gap = (gap * 10) / 13;
			if (gap == 0) gap = 1;
			if (gap == 9 || gap == 10) gap = 11;
			
			done = 1;

			for(i = 0; i < row_cnt - gap; ++i) {
				if(CmpUpdateFlg(idx_arr[i], idx_arr[i + gap]) > 0) {
					done = 0;

					// swap
					int tmp_idx = idx_arr[i + gap];
					idx_arr[i + gap] = idx_arr[i];
					idx_arr[i] = tmp_idx;
				}
			}
		}
	}

	return idx_arr;
}

/*
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
*/

int CEditableGridData::FilterData(int filter_col_no, const TCHAR* search_text, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp,
	int* find_cnt, CString* msg_str, BOOL b_err_at_no_find)
{
	m_undo->start_undo_set();

	// フィルタ中に再フィルタする場合、一旦フィルタをOFFにして現在の状態をUNDOに登録する
	if(GetGridFilterMode()) {
		FilterOff();
	}

	int ret_v = CGridData::FilterData(filter_col_no, search_text, b_distinct_lwr_upr, b_distinct_width_ascii, b_regexp,
		find_cnt, msg_str, b_err_at_no_find);

	if(ret_v == 0) {
		m_undo->next_undo(OPE_FILTER_ON);
		size_t filter_cond_size = GetFilterGridData()->GetGridDataFilterCondSize();
		griddata_filter_cond* filter_cond = GetFilterGridData()->GetGridDataFilterCond();
		m_undo->get_cur_undo_data()->set_buf_data(filter_cond, filter_cond_size);
	}

	m_undo->end_undo_set();

	if(ret_v != 0) {
		Undo();
	}

	return ret_v;
}

void CEditableGridData::FilterOff()
{
	if(!GetGridFilterMode()) return;

	m_undo->next_undo(OPE_FILTER_OFF);
	size_t filter_cond_size = GetFilterGridData()->GetGridDataFilterCondSize();
	griddata_filter_cond* filter_cond = GetFilterGridData()->GetGridDataFilterCond();
	m_undo->get_cur_undo_data()->set_buf_data(filter_cond, filter_cond_size);

	CGridData::FilterOff();
}

void CEditableGridData::UnsetFilterFlg(int row)
{
	UnsetRowFlg(row, FILTER_FLG);
}

void CEditableGridData::SetFilterFlg(int row)
{
	SetRowFlg(row, FILTER_FLG);
}

BOOL CEditableGridData::IsFilterRow(int row)
{
	return CheckRowFlg(row, FILTER_FLG);
}
