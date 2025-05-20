/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#if !defined(AFX_GRIDCTRL_H__D78AAD60_6D2E_11D4_B06E_00E018A83B1B__INCLUDED_)
#define AFX_GRIDCTRL_H__D78AAD60_6D2E_11D4_B06E_00E018A83B1B__INCLUDED_

// GridCtrl.h : ヘッダー ファイル
//
#include "GridData.h"
#include "ScrollWnd.h"

#include "octrl_msg.h"
#include "EditCtrl.h"

#define GRID_CTRL_MAX_CELL_DATA_LEN		256

/////////////////////////////////////////////////////////////////////////////
// CGridCtrl ウィンドウ

#define GRS_COL_HEADER			(0x0001 << 0)
#define GRS_ROW_HEADER			(0x0001 << 1)
#define GRS_LINE_SELECT			(0x0001 << 2)
#define GRS_MULTI_SELECT		(0x0001 << 3)
#define GRS_ON_DIALOG			(0x0001 << 4)
#define GRS_INVERT_SELECT_TEXT	(0x0001 << 5)
#define GRS_SHOW_NULL_CELL		(0x0001 << 6)
#define GRS_SHOW_CUR_ROW		(0x0001 << 7)
#define GRS_SHOW_SPACE			(0x0001 << 8)
#define GRS_SHOW_2BYTE_SPACE	(0x0001 << 9)
#define GRS_ADJUST_COL_WIDTH_NO_USE_COL_NAME	(0x0001 << 10)
#define GRS_SEARCH_SELECTED_AREA	(0x0001 << 11)
#define GRS_SHOW_LINE_FEED		(0x0001 << 12)
#define GRS_SHOW_TAB			(0x0001 << 13)
#define GRS_COPY_ESCAPE_DBL_QUOTE	(0x0001 << 14)
#define GRS_IME_CARET_COLOR		(0x0001 << 15)
#define GRS_END_KEY_LIKE_EXCEL	(0x0001 << 16)
#define GRS_HIGHLIGHT_HEADER	(0x0001 << 17)
#define GRS_COLUMN_NAME_TOOL_TIP	(0x0001 << 18)
#define GRS_SHOW_NOTNULL_COL	(0x0001 << 19)
#define GRS_SWAP_ROW_COL_MODE	(0x0001 << 20)
#define GRS_SHOW_CELL_ALWAYS_TOP	(0x0001 << 21)
#define GRS_SHOW_CELL_ALWAYS_HALF	(0x0001 << 22)
#define GRS_ADJUST_COL_WIDTH_TRIM_CHAR_DATA	(0x0001 << 23)
#define GRS_DONT_CHANGE_TOPLEFT_CELL_COLOR	(0x0001 << 24)
#define GRS_ALLOW_F2_ENTER_EDIT (0x0001 << 25)

#define GR_COPY_FORMAT_CSV			1	// CSV形式
#define GR_COPY_FORMAT_TAB			2	// TAB区切り
#define GR_COPY_FORMAT_CSV_CNAME	3	// CSV形式(カラム名付き)
#define GR_COPY_FORMAT_TAB_CNAME	4	// TAB区切り(カラム名付き)
#define GR_COPY_FORMAT_FIX_LEN		5	// 固定長(カラム名付き)
#define GR_COPY_FORMAT_FIX_LEN_CNAME	6	// 固定長(カラム名付き)
#define GR_COPY_FORMAT_COLUMN_NAME	7	// カラム名
#define GR_COPY_FORMAT_SQL			8	// SQLのinsert文用
#define GR_COPY_FORMAT_WHERE_CLAUSE 9	// SQLのwhere句用
#define GR_COPY_FORMAT_IN_CLAUSE 10		// SQLのin句用

#define GR_COPY_OPTION_QUOTED_NAME (0x0001 << 0)
#define GR_COPY_OPTION_CONVERT_CRLF (0x0001 << 1)
#define GR_COPY_OPTION_USE_NULL (0x0001 << 2)		// GR_COPY_FORMAT_SQLのとき、データがNULLの場合、''ではなくNULLを出力

#define GRIDCTRL_CLASSNAME	_T("OGAWA_GridCtrl")

#define GRID_CTRL_ADJUST_COL_WIDTH_CANCEL 1929

class CGridCtrl : public CScrollWnd
{
	DECLARE_DYNAMIC(CGridCtrl)

public:
	CGridCtrl();
	virtual ~CGridCtrl();

	void InitColWidth();

	void Update();
	void InitSelectedCell();

	void SetCellPadding(int top, int bottom, int left, int right);

	virtual void SetGridData(CGridData *griddata);
	virtual CGridData *GetGridData() { return m_grid_data; }
	virtual BOOL SetFont(CFont *font);

	void SetDispColWidth(int col, int width) { m_grid_data->SetDispColWidth(col, width); }
	int GetDispColWidth(int col) { return m_grid_data->GetDispColWidth(col); }

	int AdjustAllColWidth(BOOL use_col_name, BOOL force_window_width, 
		BOOL update_window = TRUE, volatile int *cancel_flg = NULL);
	void EqualAllColWidth();

	int GetSelectedRow() { return m_grid_data->get_cur_row(); }
	int GetSelectedCol() { return m_grid_data->get_cur_col(); }

	void Copy(int copy_format = GR_COPY_FORMAT_TAB, int copy_option = 0);
	void Copy(int copy_format, int y_start, int y_end, int x_start, int x_end, int copy_option);
	CString GetCopyString(int copy_format, int y_start, int y_end, int x_start, int x_end, int copy_option = 0);

	void Paste();
	int Paste(const TCHAR *pstr);
	void Cut();
	void Undo();
	void Redo();
	BOOL EnterEdit(BOOL b_cursor_first = FALSE, BOOL b_focus = TRUE);
	BOOL LeaveEdit();
	void InputEnter();
	BOOL IsEnterEdit() { return m_edit_cell && ::IsWindowVisible(m_edit_cell->GetSafeHwnd()); }

	int UpdateCell(int row, int col, const TCHAR *data, int len);

	BOOL CanUndo();
	BOOL CanRedo();
	BOOL CanPaste();
	BOOL CanCopy();
	BOOL CanCopyColumnName();
	BOOL CanCut();

	void SetGridStyle(DWORD dwGridStyle, BOOL b_update_window = FALSE);
	DWORD GetGridStyle() { return m_gridStyle; }
	void SetColor(int type, COLORREF color);
	COLORREF GetColor(int type);

	void SetTopLeftCellColor(COLORREF color);

	int SearchText2(const TCHAR *search_text, int dir, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii,
		BOOL b_regexp, BOOL b_loop, BOOL *b_looped, BOOL b_cur_cell = FALSE);
	int SearchNext2(const TCHAR *search_text, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp, BOOL *b_looped);
	int SearchPrev2(const TCHAR *search_text, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp, BOOL *b_looped);

	int SearchText(const TCHAR *search_text, int dir, BOOL b_distinct_lwr_upr, BOOL b_regexp,
		BOOL b_loop, BOOL *b_looped, BOOL b_cur_cell = FALSE);
	int SearchNext(const TCHAR *search_text, BOOL b_distinct_lwr_upr, BOOL b_regexp, BOOL *b_looped);
	int SearchPrev(const TCHAR *search_text, BOOL b_distinct_lwr_upr, BOOL b_regexp, BOOL *b_looped);

	int ReplaceText2(const TCHAR *search_text, const TCHAR *replace_text, int dir,
		BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp, BOOL b_all, BOOL b_selected_area,
		int *preplace_cnt, BOOL *b_looped);
	int ReplaceText(const TCHAR *search_text, const TCHAR *replace_text, int dir,
		BOOL b_distinct_lwr_upr, BOOL b_regexp, BOOL b_all, BOOL b_selected_area, 
		int *preplace_cnt, BOOL *b_looped);
	void ClearSearchText();

	int SearchUpdateCellNext(BOOL *b_looped);
	int SearchUpdateCellPrev(BOOL *b_looped);

	int SearchColumn2(const TCHAR *search_text, int dir, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp,
		BOOL b_loop, BOOL *b_looped);
	int SearchColumn(const TCHAR *search_text, int dir, BOOL b_distinct_lwr_upr, BOOL b_regexp,
		BOOL b_loop, BOOL *b_looped);

	BOOL HaveSelected();

	BOOL IsValidCurPt();
	void SelectAll();

	int DeleteRow();
	int InsertRows(int row_cnt);

	int DeleteNullRows(BOOL all_flg);

	void ClearSelected(BOOL b_notify_parent_wnd = TRUE);
	void SelectRow(int row);
	void SelectArea(CPoint sel_pt, CPoint pt1, CPoint pt2);

	void SetCell(int x, int y);
	void UnSelect();

	virtual int GetScrollLeftMargin();
	virtual int GetScrollTopMargin();

	void InvalidateRow(int row);
	void InvalidateCell(POINT *pt);

	void DocumentStart();
	void DocumentEnd();
	void LineStart();
	void LineEnd();
	void LineUp() { MoveCell(0, -1); }
	void LineDown() { MoveCell(0, 1); }
	void PageUp() { MoveCell(0, -GetShowRow()); }
	void PageDown() { MoveCell(0, GetShowRow()); }
	void ColumnStart();
	void ColumnEnd();

	void ScrollUp() { VScroll(SB_LINEUP); }
	void ScrollDown() { VScroll(SB_LINEDOWN); }
	void ScrollPageUp() { VScroll(SB_PAGEUP); }
	void ScrollPageDown() { VScroll(SB_PAGEDOWN); }
	void ScrollLeft() { HScroll(SB_LINEUP); }
	void ScrollRight() { HScroll(SB_LINEDOWN); }

	void SetColHeaderRow(int row) { m_col_header_row = row; }

	int GetRowHeight() { return m_row_height; }
	int GetCharWidth() { return m_font_width; }
	int GetColHeaderHeight() { return m_col_header_height; }
	int GetRowHeaderWidth() { return m_row_header_width; }

	void SetMinColWidth(int width) { m_min_colwidth = width; }
	int GetMinColWidth() { return m_min_colwidth; }
	int GetMaxColWidth();
	void SetAdjustMinColChars(int chars) { m_adjust_min_col_chars = chars; }

	void SetFixColMode(int start_col, int end_col);
	void ClearFixColMode();
	BOOL MakeFixColData(int &start_col, int &end_col);
	BOOL IsFixColMode() { return (m_fix_end_col > 0); }
	void GetFixColModeData(int &start_col, int &end_col);

	int HitRowHeader(CPoint point);
	int HitColHeader(CPoint point);
	BOOL HitAllSelectArea(CPoint point);

	void SetDispFlg_SelectedCol(BOOL flg);
	void SetDispAllCol();

	void SetCharSpace(int space);

	void WindowMoved();

	int ToUpper();
	int ToLower();
	int ToHankaku(BOOL b_alpha, BOOL b_kata);
	int ToZenkaku(BOOL b_alpha, BOOL b_kata);
	void InputSequence(__int64 start_num, __int64 incremental);

	BOOL GetEndKeyFlg() { return m_end_key_flg; }

	void SortData(int *col_no, int *order, int sort_col_cnt);
	void SetNullText(const TCHAR *null_text);

protected:
	void InitData();

	void MoveCell(int x, int y);
	void SelChanged(POINT *pt, BOOL b_area_select = TRUE, BOOL b_no_clear_selected_area = FALSE);
	void SetEditCellPos();
	virtual void PreEnterEdit() {}

	int HitColSeparator(CPoint point);
	void HitCell(CPoint point, POINT *pt, BOOL out_of_window = FALSE);
	BOOL HitSelectedArea(CPoint cell_point);

	void CalcShowRow();
	int GetShowCol();
	int GetShowRow();
	int GetScrSize(int old_col, int new_col);

	void InvalidateTopLeft();
	void InvalidateRowHeader(int row);
	void InvalidateCellHeader(int col);
	void InvalidateRange(POINT *pt1, POINT *pt2);

	void Update_AllWnd();
	void Invalidate_AllWnd(BOOL b_self = TRUE);
	void InvalidateRow_AllWnd(int row);
	void InvalidateCell_AllWnd(POINT *pt);
	void InvalidateRowHeader_AllWnd(int row);
	void InvalidateCellHeader_AllWnd(int col);
	void InvalidateRange_AllWnd(POINT *pt1, POINT *pt2);

	void GetCellRect(POINT *pt, RECT *rect);
	void GetCellWidth(int col, RECT *rect);

	int DrawDragRect(CPoint point, BOOL first = FALSE);

	void PreDragHeader(CPoint point, int col);
	void StartDragHeader(CPoint point);
	void EndDragHeader(CPoint point);
	void DoDragHeader(CPoint point);

	BOOL IsSelectedCell(int row, int col);
	void SelectCell(CPoint pt);
	void SelectCol(int col);

	void StartSelect(CPoint point);
	void PreDragSelected(int mode);
	void StartDragSelected();
	void DoDragSelected(CPoint point);
	void EndDragSelected();

	int CalcCopyDataSize(int copy_format, int y_start, int y_end, int x_start, int x_end, int copy_option);
	void GetCopyData(TCHAR *buf, int copy_format, int y_start, int y_end, int x_start, int x_end, int copy_option);
	void CopyToClipboard(int copy_format, int y_start, int y_end, int x_start, int x_end, int copy_option);

	void SetHeaderSize();
	void PostSetFont();

	void OnEditKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void DeleteKey();

	void ShowCell(POINT *pt);

	BOOL IsSelectedCellColor(int y, int x);
	COLORREF GetCellBkColor(int y, int x);

	void PaintMain(CDC *pdc, CDC *p_paintdc);
	void PaintData(CDC *pdc, CDC *p_paintdc, int show_row, int show_col, CPoint &scr_pt);
	void PaintCell(CDC *pdc, CDC *p_paintdc, RECT &rect, int y, int x);
	virtual void PaintCellData(CDC *pdc, CDC *p_paintdc, RECT rect, int y, int x);
	int PaintTextMain(CDC *pdc, CDC *p_paintdc, RECT rect, const TCHAR *data, int len, int left_offset = 0);
	int TextOut2(CDC *pdc, CDC *p_paintdc, const TCHAR *p, int len, RECT rect, int row_left, 
		UINT textout_options, int left_offset);

	void PaintSpaceMain(CDC *pdc, CDC *p_paintdc, RECT &rect, const TCHAR *data);
	void PaintSpace(CDC *pdc, CDC *p_paintdc, int top, int left, int right);
	void Paint2byteSpace(CDC *pdc, CDC *p_paintdc, int top, int left, int right);
	void PaintTab(CDC *pdc, CDC *p_paintdc, int top, int left, int right);
	void PaintLineFeed(CDC *pdc, CDC *p_paintdc, int top, int left);

	void PaintRowNumber(CDC *pdc, CDC *p_paintdc, int show_row, CPoint &scr_pt, CRect &winrect);
	void PaintColHeaderMain(CDC *pdc, CDC *p_paintdc, int x, CRect &rect);
	void PaintColHeader(CDC *pdc, CDC *p_paintdc, int show_col, CPoint &scr_pt, CRect &winrect);
	void PaintNoDispColLine(CDC *pdc, CDC *p_paintdc, int left, int top, int bottom);
	void PaintGrid(CDC *pdc, CDC *p_paintdc, int show_row, int show_col, CPoint &scr_pt, CRect &winrect);

	int GetFontWidth(CDC *pdc, unsigned int ch, CFontWidthHandler *pdchandler = NULL);

	int AdjustColWidth(int col, BOOL use_col_name, BOOL force_window_width,
		volatile int *cancel_flg = NULL);

	void SaveSearchData(const TCHAR *search_text, BOOL b_distinct_lwr_upr, BOOL b_regexp);
	void SaveSearchData2(const TCHAR *search_text, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp);
	int ReplaceTextAll(const TCHAR *search_text, const TCHAR *replace_text, 
		BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp, BOOL b_selected_area, int *preplace_cnt);

	BOOL IsActiveSplitter();

	const TCHAR *GetDispData(int y, int x) { return m_grid_data->Get_DispData(y, x); }
	const TCHAR *GetEditData(int y, int x) { return m_grid_data->Get_EditData(y, x); }
	const TCHAR *GetEnterEditData(int y, int x) { return m_grid_data->Get_EditData(y, x); }
	CString GetLeaveEditData() { return m_edit_data->get_all_text(); }

	BOOL HaveSelectedRow(int row);
	BOOL HaveSelectedCol(int col);

	void ClearIME();

	CString GetDispColNameWithNotNullFlg(int col);

protected:
	DWORD m_gridStyle;
	int m_row_height;
	int m_row_header_width;
	int m_col_header_height;
	int m_col_header_row;
	CFont m_font;
	
	RECT m_droprect;
	static CGridData m_null_grid_data;
	CGridData *m_grid_data;

	CEditData	*m_edit_data;
	CEditCtrl	*m_edit_cell;
	BOOL		m_edit_cell_focused;
	BOOL		m_last_active_wnd;

	BOOL		m_change_active_row_text_color;

	int		m_font_height;
	int		m_font_width;
	int		m_num_width;
	int		m_show_row;

	// カラム幅のドラッグ変更用
	struct {
		int drag_flg;
		int col;
		POINT pt;
	} m_drag_header;

	int			m_min_colwidth;
	int			m_adjust_min_col_chars;

	int			m_fix_start_col;
	int			m_fix_end_col;

	CSearchData m_search_data;

	BOOL		m_end_key_flg;

	int		m_cell_padding_top;
	int		m_cell_padding_bottom;
	int		m_cell_padding_left;
	int		m_cell_padding_right;

// オーバーライド
	// ClassWizard は仮想関数を生成しオーバーライドします。
	//{{AFX_VIRTUAL(CGridCtrl)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

	// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CGridCtrl)
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void SetConversionWindow(int x, int y);
	void ValidateEditCell();

	void NoticeChangeCellPos();
	void NoticeUpdateGridData();

	BOOL CreateEditData();
	void FreeEditData();

	void RegisterWndClass();
	int GetLineRight(CPoint &scr_pt, CRect &win_rect);
	void ShowToolTip();

	BOOL CGridCtrl::IsUpdateDispRowAndDispCol(int disp_row, int disp_col);

	CPoint	m_tool_tip_pt;
	int		m_last_tool_tip_col;

	CString		m_null_text;
	int			m_null_text_len;

protected:
	virtual int GetHScrollSizeLimit();
	virtual int GetVScrollSizeLimit();
	virtual int GetHScrollSize(int xOrig, int x);
	virtual int GetVScrollSize(int yOrig, int y);

	virtual int GetVScrollMin();
	virtual int GetVScrollMax();
	virtual int GetVScrollPage();
	virtual int GetHScrollMin();
	virtual int GetHScrollMax();
	virtual int GetHScrollPage();

	virtual void Scrolled(CSize sizeScroll, BOOL bThumbTrack);
};

/////////////////////////////////////////////////////////////////////////////
#endif // !defined(AFX_GRIDCTRL_H__D78AAD60_6D2E_11D4_B06E_00E018A83B1B__INCLUDED_)
