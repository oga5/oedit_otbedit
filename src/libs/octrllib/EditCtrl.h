/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#if !defined(AFX_EDITCTRL_H__D78AAD60_6D2E_11D4_B06E_00E018A83B1B__INCLUDED_)
#define AFX_EDITCTRL_H__D78AAD60_6D2E_11D4_B06E_00E018A83B1B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// EditCtrl.h : ヘッダー ファイル
//
#include "editdata.h"
#include "scrollwnd.h"
#include "DropTargetTmpl.h"
#include "DispColorData.h"

#include "octrl_util.h"
#include "octrl_msg.h"

/////////////////////////////////////////////////////////////////////////////
// CEditCtrl ウィンドウ

#define EDIT_CTRL_MAX_WORD_LEN	100

class CSearchData {
public: 
	CSearchData();
	~CSearchData();

	BOOL GetDispSearch() { return m_b_disp_search; }
	void SetDispSearch(BOOL b_disp_search) { m_b_disp_search = b_disp_search; }

	TCHAR *GetTextBuf() { return m_text_buf; }
	BOOL AllocTextBuf(int size);
	void MakeTextBuf(const TCHAR *p);

	const CString &GetSearchText() { return m_search_text; }
	BOOL GetRegExp() { return m_b_regexp; }
	BOOL GetDistinctLwrUpr() { return m_b_distinct_lwr_upr; }

	HREG_DATA GetRegData() { return m_reg_data.GetRegData(); }
	HREG_DATA MakeRegData(const CString &search_text, BOOL b_distinct_lwr_upr, BOOL b_regexp);
	HREG_DATA MakeRegData2(const CString &search_text, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp);

private:
	BOOL		m_b_disp_search;	// 検索結果をカラー表示する
	CString		m_search_text;
	BOOL		m_b_distinct_lwr_upr;
	BOOL		m_b_distinct_width_ascii;
	BOOL		m_b_regexp;
	TCHAR		*m_text_buf;
	int			m_text_buf_size;

	CRegData	m_reg_data;
};

class CEditCtrl : public CScrollWnd
{
	DECLARE_DYNAMIC(CEditCtrl)

// アトリビュート
protected:
	CEditData	*m_edit_data;
	unsigned int m_no_quote_color_char;

private:
	DropTargetTmpl<CEditCtrl>	m_droptarget;
	UINT		m_cf_is_box;

	CSearchData m_search_data;

	int			m_font_height;
	int			m_char_width;
	int			m_num_width;

	int			m_row_height;
	int			m_tt_flg;

	int			m_row_header_width;
	int			m_col_header_height;

	CFont		m_font;
	CFont		m_clickable_font;
	CFont		m_bold_font;

	BOOL		m_read_only;

	int			m_show_row;
	POINT		m_double_click_pos;

	QWORD		m_ex_style;

	// キーワード補完用
	struct {
		TCHAR	org_str[EDIT_CTRL_MAX_WORD_LEN];
		int		cnt;
	} m_completion_data;

	int m_row_num_digit;

	int m_row_space;	// 行間
	int m_row_space_top;	// 行間(上)

	int m_top_space;	// 上のスペース
	int m_left_space;	// 左のスペース
	int m_bottom_space;
	int m_right_space;

	RECT	m_old_dragrect;
	POINT	m_old_drag_pt;

	BOOL	m_overwrite;

	RECT	m_ime_rect;

	int		m_caret_width;
	static UINT		m_caret_blink_time;
	int		m_brackets_blink_time;

	UINT	m_dlg_code;

	COLORREF m_modified_bk_color;
	COLORREF m_original_bk_color;

	// フルスクリーン表示用
	POINT	m_null_cursor_pt;
	int		m_null_cursor_cnt;
	long m_bk_width_hi, m_bk_height_hi;
	IPicture *m_iPicture;
	BOOL m_bg_screen_size;
	CDC bk_dc;
	CBitmap bk_bmp;
	SIZE bk_dc_size;
	//

	int GetDispOffset(int idx, int *psplit_idx = NULL) { return m_edit_data->get_disp_offset(idx, psplit_idx); }
	int GetDispRow(int idx) { return m_edit_data->get_disp_row(idx); }
	struct CEditDispData::edit_data_select_area_st *GetSelectArea() { return m_edit_data->get_disp_data()->GetSelectArea(); }

	void RegisterWndClass();

	int GetFontWidth(CDC *pdc, unsigned int ch, CFontWidthHandler *pdchandler = NULL);

protected:
	void GetDispDataPoint(CPoint data_pt, POINT *disp_pt, BOOL b_line_end = FALSE);
	void GetDispCaretPoint(CPoint disp_pt, POINT *caret_pt);

// オペレーション
private:
	void MoveCaret(int row, int col, BOOL extend, BOOL show_scroll = TRUE);
	void PreMoveCaret(BOOL extends);
	void PostMoveCaret(BOOL extends, BOOL show_scroll, BOOL set_caret = TRUE);
	void MoveWord(int allow, BOOL extend, BOOL skip_space = TRUE);
	void MoveNextSpace(int allow, BOOL extend);
	CPoint GetNextWordPt(int allow);

	void DeleteAndSaveCursor(POINT pt1, POINT pt2);
	void DeleteMain(POINT *pt1, POINT *pt2, BOOL box_select, BOOL b_set_caret);
	void CopyMain();
	void CopyMain(POINT *pt1, POINT *pt2, BOOL box);
	void CopyRowMain();
	void DeleteRowMain();

	BOOL GetClickablePT(POINT *pt1, POINT *pt2);
	BOOL HitTestClickable(CPoint point);
	BOOL IsURL(const TCHAR *p);

	void SetSelectInfo(POINT start_pt);
	void StartSelect(CPoint point);
	void EndSelect(CPoint point, BOOL b_move_caret = TRUE);
	BOOL HitTestSelectedArea(CPoint point);
	BOOL DragStart();
	void ValidateSelectedArea();

	void DropPaste(POINT &pt1, POINT &pt2, TCHAR *pstr, BOOL is_box);

	void CalcShowRow();
	int GetShowCol();

	void InvalidateRow(int row);
	void InvalidateRowHeader(int idx);
	void InvalidateColHeader(int x);
	void InvalidateRows(int row1, int row2);
	void InvalidateEditData(int old_row_cnt, int old_char_cnt, int old_split_cnt, 
		int start_row, BOOL b_make_edit_data);

	void Redraw2(BOOL b_show_caret, BOOL b_make_disp_data);
	void Redraw_AllWnd(BOOL b_show_caret = FALSE);
	void Invalidate_AllWnd(BOOL bErase = TRUE);
	void InvalidateRow_AllWnd(int row);
	void InvalidateRowHeader_AllWnd(int idx);
	void InvalidateColHeader_AllWnd(int x);
	void InvalidateRows_AllWnd(int row1, int row2);
	void InvalidateEditData_AllWnd(int old_row_cnt, int old_char_cnt, int old_split_cnt, int start_row = 0);

	void CreateCaret(BOOL destroy);
	void PostSetFont();
	void SetConversionWindow(int x, int y);

	void SelectText(CPoint point, BOOL mouse_select);

	void PreDragSelected(BOOL word_select = FALSE);
	void StartDragSelected();
	void DoDragSelected(CPoint point, BOOL show_scroll = FALSE);
	void EndDragSelected();

	void ClearBackGround();
	void PaintBackGround(CDC *pdc, int width, int height);
	void FillBackGroundRect(CDC *pdc, RECT *rect, COLORREF cr);

	void PaintMain(CDC *pdc, CDC *p_paintdc);
	void PaintColNum(CDC *pdc, CDC *p_paintdc);
	int TextOut(CDispColorData *color_data, CDC *pdc, CDC *p_paintdc,
		const TCHAR *p, int len, RECT &rect, int &line_len, int row, 
		int &split,	int win_bottom, int h_scr_pos);
	int TextOut2(CDC *pdc, CDC *p_paintdc, const TCHAR *p, int len, RECT rect, int top_space = 0);
	int TextOutAA(CDC *pdc, CDC *p_paintdc, const TCHAR *p, int len, RECT rect);
	void PaintText(CDC *pdc, CDC *p_paintdc, int h_scr_pos, int rect_top, int row, int *bottom);
	void PaintTextMain(CDispColorData *color_data, CDC *pdc, CDC *p_paintdc, 
		int h_scr_pos, int rect_top, int row, const TCHAR *p);
	void PaintTextSpace(CDispColorData *color_data, CDC *pdc, CDC *p_paintdc, 
		int row, int split, int top, int left);
	void PaintLineFeed(CDC *pdc, const RECT &client_rect, int top, int left);
	void PaintLineEnd(CDC *pdc, const RECT &client_rect, int top, int left);
	void PaintEOF(CDC *pdc, const RECT &client_rect, int top, int left);
	void PaintTab(CDC *pdc, const RECT &client_rect, int top, int left);
	void Paint2byteSpace(CDC *pdc, const RECT &client_rect, int top, int left);
	void PaintSpace(CDC *pdc, const RECT &client_rect, int top, int left);
	void PaintRowLine(CDC *pdc, CDC *p_paintdc);
	BOOL RectVisibleMark(const RECT &client_rect, const RECT &mark_rect);
	BOOL VisiblePoint(const POINT *pt);

	void SetDispColor(CDispColorData *color_data, COLORREF color, int pos, int len);
	void MakeDispColorData(CDispColorData *color_data, int row, const TCHAR *p);
	void MakeDispSelectedData(CDispColorData *color_data, int row, const TCHAR *p);
	void SetSearchData(CDispColorData *color_data, int pos, int len);
	void MakeDispSearchData(CDispColorData *color_data, int row, const TCHAR *p);
	void MakeDispMarkData(CDispColorData *color_data, const TCHAR *p);
	void SetDispClickable(CDispColorData *color_data, int pos, int len);
	void MakeDispClickableDataMain(CDispColorData *color_data, const TCHAR *p, TCHAR *clickable_text);
	void MakeDispClickableDataMail(CDispColorData *color_data, const TCHAR *p);
	void MakeDispClickableData(CDispColorData *color_data, const TCHAR *p);
	void MakeDispBracketsData(CDispColorData *color_data, int row, const TCHAR *p);

	int GetDispRowTop();
	int GetDispRowBottom();
	int GetDispRowPos(int row);
	int GetDispRowSplitCnt(int row) { return m_edit_data->get_split_cnt(row); }
	int GetMaxDispCol();

	void SetLineModeRight(int win_width);

	BOOL IsCheckCommentChar(unsigned int ch);
	BOOL IsSelectedRow(int row);

	int CalcSplitCnt(int row);
	void MakeDispData(BOOL all_recalc_split = TRUE, int start_row = 0, int recalc_split_cnt = 0);
	void SetHeaderSize();

	void ClearKeywordCompletion();
	int PostSearchText(int len, POINT *pt);

	void ReplaceStrBox(const TCHAR *pstr);
	int ReplaceTextAll(const TCHAR *search_text, const TCHAR *replace_text, 
		BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp, BOOL b_selected_area,
		int *preplace_cnt);

	void InputMain(UINT nChar);
	void IndentMain(BOOL del, BOOL space_flg, int space_cnt, UINT input_char);

	void SetBracketsPt();
	void InvertBrackets(UINT nChar);
	BOOL SearchMatchBrackets(UINT nChar, POINT *pt);

	void ClearHighlightChar();
	void InvertPt(CDC *pdc, POINT char_pt);

	void ConvertMousePtToCharPt(CPoint mouse_pt, POINT *char_pt, BOOL enable_minus = FALSE);
	void GetDataPoint(CPoint point, POINT *pt, BOOL enable_minus = FALSE);

	void SetCaretPoint(CPoint point, BOOL extend);
	int HitTestRowHeader(CPoint point);
	BOOL HitDispData(CPoint point);
	void RefleshCursor();

	void ShowPoint(CPoint point);

	BOOL JumpBookMarkLoop(int start, int end, int allow, BOOL extend);

	void SetImeRect();
	BOOL CompleteComposition();

	void ZenkakuHankakuConvert(BOOL b_alpha, BOOL b_kata, BOOL b_hankaku_to_zenkaku);

	void CreateFonts(LOGFONT *lf);

	virtual void CaretMoved() {}

	QWORD SetBracketColorStyle(QWORD ex_style);

// オペレーション
protected:

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CEditCtrl)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	// コンストラクション
	CEditCtrl();
	virtual ~CEditCtrl();

// オペレーション
public:
	void ClearAll();
	void SetEditData(CEditData *arg);
	CEditData *GetEditData() { return m_edit_data; }
	void SetReadOnly(BOOL arg) { m_read_only = arg; }
	BOOL IsReadOnly() { return m_read_only; }

	virtual void SetFont(CFont *font);

	void SetCaret(BOOL show_scroll, int focus_mode = 0, BOOL b_line_end = FALSE);
	void MoveCaretPos(int row, int col, BOOL extend = FALSE);
	void MoveCaretRow(int row, BOOL extend, BOOL page_scr = FALSE);

	void ClearSelected(BOOL redraw = TRUE);
	BOOL HaveSelected();
	BOOL HaveSelectedMultiLine();
	BOOL HaveSelectedFullOneLine();
	void SelectAll();
	void SelectRow(int row, BOOL b_move_caret = TRUE);
	void SelectRow();
	void SelectWord(POINT pt);
	void SelectWord();
	void GetSelectedPoint(POINT *pt1, POINT *pt2);
	void SetSelectedPoint(POINT pt1, POINT pt2, BOOL box_select = FALSE);
	CString GetSelectedText();
	void InfrateSelectedArea();
	void NextBoxSelect() { m_edit_data->get_disp_data()->GetSelectArea()->next_box_select = TRUE; }
	BOOL GetNextBoxSelect() { return m_edit_data->get_disp_data()->GetSelectArea()->next_box_select; }

	int GetSelectedBytes();
	int GetSelectedChars();

	void ResetDispInfo();

	int GetShowRow();
	int GetRowHeight() { return m_row_height; }
	int GetCharWidth() { return m_char_width; }
	int GetColHeaderHeight() { return m_col_header_height; }
	int GetRowHeaderWidth() { return m_row_header_width; }
	
	void SetExStyle(DWORD ex_style);
	void SetExStyle2(QWORD ex_style);
	void OnExStyle(QWORD flg) { m_ex_style |= flg; }
	void OffExStyle(QWORD flg) { m_ex_style &= (~flg); }

	void SetBracketsBlinkTime(int t) { m_brackets_blink_time = t; }

	void Invalidate(BOOL bErase = TRUE);
	void CheckScrollBars();
	void Redraw(BOOL b_show_caret = FALSE);
	void CheckCommentRow(int start_row = -1, int end_row = -1);

	BOOL CanUndo();
	BOOL CanRedo();
	BOOL CanPaste();
	BOOL CanCopy();
	BOOL CanCut();
	BOOL CanDelete();

	void InputChar(UINT nChar);

	void Redo();
	void Undo();
	void Paste(BOOL data_only = FALSE, BOOL box_paste = FALSE);
	void Paste(const TCHAR *pstr, BOOL is_box = FALSE, BOOL set_caret_flg = TRUE);
	void Copy();
	void Cut();
	void Delete();

	void DeleteRow();
	void DeleteAfterCaret();
	void DeleteBeforeCaret();
	void DeleteWordAfterCaret();
	void DeleteWordBeforeCaret();
	void MoveLastEditPos();
	void CutRow();
	void CopyRow();
	void PasteRow(BOOL b_up_flg);
	void JoinRow();

	void ToUpper();
	void ToLower();
	void ToLowerWithOutCommentAndLiteral();
	void ToUpperWithOutCommentAndLiteral();
	void HankakuToZenkaku(BOOL b_alpha, BOOL b_kata);
	void ZenkakuToHankaku(BOOL b_alpha, BOOL b_kata);
	void InsertTab(BOOL del);
	void IndentSpace(BOOL del);

	int SearchText2(const TCHAR *search_text, int dir, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, 
		BOOL b_regexp, BOOL *b_looped, BOOL enable_loop = TRUE);
	int SearchNext2(const TCHAR *search_text, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii,
		BOOL b_regexp, BOOL *b_looped, BOOL enable_loop = TRUE);
	int SearchPrev2(const TCHAR *search_text, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii,
		BOOL b_regexp, BOOL *b_looped, BOOL enable_loop = TRUE);
	void SaveSearchData2(const TCHAR *search_text, BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp);

	int SearchText(const TCHAR *search_text, int dir, BOOL b_distinct_lwr_upr, 
		BOOL b_regexp, BOOL *b_looped, BOOL enable_loop = TRUE);
	int SearchNext(const TCHAR *search_text, BOOL b_distinct_lwr_upr, 
		BOOL b_regexp, BOOL *b_looped, BOOL enable_loop = TRUE);
	int SearchPrev(const TCHAR *search_text, BOOL b_distinct_lwr_upr, 
		BOOL b_regexp, BOOL *b_looped, BOOL enable_loop = TRUE);
	void SaveSearchData(const TCHAR *search_text, BOOL b_distinct_lwr_upr, BOOL b_regexp);

	int ReplaceText2(const TCHAR *search_text, const TCHAR *replace_text, int dir,
		BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp, BOOL b_all, BOOL b_selected_area,
		int *preplace_cnt, BOOL *b_looped);
	int ReplaceText(const TCHAR *search_text, const TCHAR *replace_text, int dir,
		BOOL b_distinct_lwr_upr, BOOL b_regexp, BOOL b_all, BOOL b_selected_area, 
		int *preplace_cnt, BOOL *b_looped);

	int FilterText2(const TCHAR *search_text,
		BOOL b_distinct_lwr_upr, BOOL b_distinct_width_ascii, BOOL b_regexp, BOOL b_selected_area,
		int *preplace_cnt, BOOL b_del_flg);
	int FilterText(const TCHAR *search_text,
		BOOL b_distinct_lwr_upr, BOOL b_regexp, BOOL b_selected_area, 
		int *preplace_cnt, BOOL b_del_flg);
	void ClearSearchText();

	virtual void SetColor(int type, COLORREF color);
	COLORREF GetColor(int type) { return m_edit_data->get_disp_data()->GetColor(type); }

	void SetModifiedBkColor(COLORREF color);

	virtual int GetScrollLeftMargin();
	virtual int GetScrollTopMargin();
	virtual int GetScrollRightMargin();
	virtual int GetScrollBottomMargin();

	void ReverseSelectedRows();
	void SortSelectedRows(BOOL desc);
	void SplitSelectedRows();

	int GetRowSpace() { return m_row_space; }
	int GetCharSpaceSetting() { return m_edit_data->get_disp_data()->GetCharSpaceSetting(); }
	int GetTopSpace() { return m_top_space; }
	int GetLeftSpace() { return m_left_space; }
	void SetSpaces(int row_space, int char_space, int top_space, int left_space,
		int width = -1, int height = -1);

	void SetLineMode(int mode, int line_len = 80, BOOL redraw = TRUE);
	int GetLineMode() { return m_edit_data->get_disp_data()->GetLineMode(); }

	void CharLeft(BOOL extend);
	void CharRight(BOOL extend);
	void LineUp(BOOL extend) { MoveCaretRow(-1, extend); }
	void LineDown(BOOL extend) { MoveCaretRow(1, extend); }

	void WordLeft(BOOL extend) { MoveWord(-1, extend); }
	void WordRight(BOOL extend) { MoveWord(1, extend); }

	void WordLeftStopWB(BOOL extend) { MoveWord(-1, extend, FALSE); }
	void WordRightStopWB(BOOL extend) { MoveWord(1, extend, FALSE); }

	void SpaceLeft(BOOL extend) { MoveNextSpace(-1, extend); }
	void SpaceRight(BOOL extend) { MoveNextSpace(1, extend); }
	
	void PageUp(BOOL extend);
	void PageDown(BOOL extend);

	void LineStart(BOOL extend, BOOL start_flg = FALSE);
	void LineEnd(BOOL extend);
	void SplitStart(BOOL extend);
	void SplitEnd(BOOL extend);
	void DocumentStart(BOOL extend);
	void DocumentEnd(BOOL extend);

	void ScrollUp() { VScroll(SB_LINEUP); }
	void ScrollDown() { VScroll(SB_LINEDOWN); }
	void ScrollPageUp() { VScroll(SB_PAGEUP); }
	void ScrollPageDown() { VScroll(SB_PAGEDOWN); }

	void BackSpace();
	void DeleteKey();
	void KeywordCompletion(BOOL reverse);

	void InsertDateTime();

	static HCURSOR		m_link_cursor;

	void DrawDragRect(CPoint point, BOOL first);
	DROPEFFECT DragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	DROPEFFECT DragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	BOOL Drop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	void DragLeave();

	void SetBookMark();
	void JumpBookMark(BOOL reverse, BOOL extend);
	void ClearAllBookMark();

	void SetBreakPoint();

	void ToggleOverwrite();
	BOOL GetOverwrite() { return m_overwrite; }
	int GetRowNumDigit() { return m_row_num_digit; }

	void ResetCaret();

	int TabToSpace();
	int SpaceToTab();

	BOOL HitTestClickableURL();
	BOOL DoClickable();
	BOOL CopyClickable();
	CString GetClickableURL();

	void *IMEReconvert(void *param);

	void SetDlgCode(UINT dlg_code) { m_dlg_code = dlg_code; }

	void InvertBrackets();
	void GoMatchBrackets();

	int GetCaretCol(POINT pt);
	int GetColFromCaretCol(POINT pt);

	BOOL SetBackGroundImage(const TCHAR *file_name, BOOL screen_size_flg);
	void NoticeUpdate();

	int GetDispWidthStr(const TCHAR *str);

	// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CEditCtrl)
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

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

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_EDITCTRL_H__D78AAD60_6D2E_11D4_B06E_00E018A83B1B__INCLUDED_)
