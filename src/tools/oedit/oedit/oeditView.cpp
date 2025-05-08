/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // oeditView.cpp : COeditView クラスの動作の定義を行います。
//

#include "stdafx.h"
#include "oedit.h"

#include "oeditDoc.h"
#include "oeditView.h"

#include "printeditdata.h"

#include "linejumpdlg.h"

#include "fileutil.h"
#include "file_winutil.h"
#include "CommonUtil.h"

#include "searchDlg.h"
#include "replaceDlg.h"
#include "filterDlg.h"
#include "SelectColorDlg.h"

#include "owinutil.h"
#include "scm_macro.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static const char *THIS_FILE = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// COeditView

IMPLEMENT_DYNCREATE(COeditView, CScrollView)

BEGIN_MESSAGE_MAP(COeditView, CScrollView)
	//{{AFX_MSG_MAP(COeditView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_SELECT_ALL, OnSelectAll)
	ON_COMMAND(ID_SEARCH, OnSearch)
	ON_COMMAND(ID_SEARCH_NEXT, OnSearchNext)
	ON_COMMAND(ID_SEARCH_PREV, OnSearchPrev)
	ON_COMMAND(ID_REPLACE, OnReplace)
	ON_COMMAND(ID_EDIT_INDENT, OnEditIndent)
	ON_COMMAND(ID_EDIT_INDENT_REV, OnEditIndentRev)
	ON_COMMAND(ID_EDIT_SORT_ROWS_ASC, OnEditSortRowsAsc)
	ON_COMMAND(ID_EDIT_SORT_ROWS_DESC, OnEditSortRowsDesc)
	ON_COMMAND(ID_TO_LOWER, OnToLower)
	ON_COMMAND(ID_TO_UPPER, OnToUpper)
	ON_COMMAND(ID_EDIT_REVERSE_ROWS, OnEditReverseRows)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_DELETE_SPACE, OnDeleteSpace)
	ON_COMMAND(ID_INSERT_GT, OnInsertGt)
	ON_COMMAND(ID_LINE_MODE_LEN, OnLineModeLen)
	ON_COMMAND(ID_LINE_MODE_NORMAL, OnLineModeNormal)
	ON_COMMAND(ID_LINE_MODE_RIGHT, OnLineModeRight)
	ON_UPDATE_COMMAND_UI(ID_LINE_MODE_LEN, OnUpdateLineModeLen)
	ON_UPDATE_COMMAND_UI(ID_LINE_MODE_NORMAL, OnUpdateLineModeNormal)
	ON_UPDATE_COMMAND_UI(ID_LINE_MODE_RIGHT, OnUpdateLineModeRight)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_COMMAND(ID_COLOR_INPUT, OnColorInput)
	ON_COMMAND(ID_VIEW_BROWSER_03, OnViewBrowser03)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BROWSER_03, OnUpdateViewBrowser03)
	ON_UPDATE_COMMAND_UI(ID_COLOR_INPUT, OnUpdateColorInput)
	ON_COMMAND(ID_CLEAR_SEARCH_TEXT, OnClearSearchText)
	ON_WM_COPYDATA()
	ON_COMMAND(ID_INSERT_COMMENT, OnInsertComment)
	ON_COMMAND(ID_DELETE_COMMENT, OnDeleteComment)
	ON_COMMAND(ID_DELETE_GT, OnDeleteGt)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_LINE_JUMP, OnLineJump)
	ON_COMMAND(ID_SEARCH_FILE, OnSearchFile)
	ON_COMMAND(ID_CHAR_LEFT, OnCharLeft)
	ON_COMMAND(ID_CHAR_LEFT_EXTEND, OnCharLeftExtend)
	ON_COMMAND(ID_CHAR_RIGHT, OnCharRight)
	ON_COMMAND(ID_CHAR_RIGHT_EXTEND, OnCharRightExtend)
	ON_COMMAND(ID_LINE_UP, OnLineUp)
	ON_COMMAND(ID_LINE_UP_EXTEND, OnLineUpExtend)
	ON_COMMAND(ID_LINE_DOWN, OnLineDown)
	ON_COMMAND(ID_LINE_DOWN_EXTEND, OnLineDownExtend)
	ON_COMMAND(ID_WORD_LEFT, OnWordLeft)
	ON_COMMAND(ID_WORD_LEFT_EXTEND, OnWordLeftExtend)
	ON_COMMAND(ID_WORD_RIGHT, OnWordRight)
	ON_COMMAND(ID_WORD_RIGHT_EXTEND, OnWordRightExtend)
	ON_COMMAND(ID_LINE_START, OnLineStart)
	ON_COMMAND(ID_LINE_START_EXTEND, OnLineStartExtend)
	ON_COMMAND(ID_LINE_END, OnLineEnd)
	ON_COMMAND(ID_LINE_END_EXTEND, OnLineEndExtend)
	ON_COMMAND(ID_DOCUMENT_END, OnDocumentEnd)
	ON_COMMAND(ID_DOCUMENT_END_EXTEND, OnDocumentEndExtend)
	ON_COMMAND(ID_DOCUMENT_START, OnDocumentStart)
	ON_COMMAND(ID_DOCUMENT_START_EXTEND, OnDocumentStartExtend)
	ON_COMMAND(ID_EDIT_BACKSPACE, OnEditBackspace)
	ON_COMMAND(ID_KEYWORD_COMPLETION, OnKeywordCompletion)
	ON_COMMAND(ID_KEYWORD_COMPLETION_REV, OnKeywordCompletionRev)
	ON_COMMAND(ID_SCROLL_UP, OnScrollUp)
	ON_COMMAND(ID_SCROLL_DOWN, OnScrollDown)
	ON_COMMAND(ID_SCROLL_PAGE_UP, OnScrollPageUp)
	ON_COMMAND(ID_SCROLL_PAGE_DOWN, OnScrollPageDown)
	ON_COMMAND(ID_PAGE_UP, OnPageUp)
	ON_COMMAND(ID_PAGE_UP_EXTEND, OnPageUpExtend)
	ON_COMMAND(ID_PAGE_DOWN, OnPageDown)
	ON_COMMAND(ID_PAGE_DOWN_EXTEND, OnPageDownExtend)
	ON_COMMAND(ID_SELECT_ROW, OnSelectRow)
	ON_COMMAND(ID_BOX_SELECT, OnBoxSelect)
	ON_COMMAND(ID_SET_BOOK_MARK, OnSetBookMark)
	ON_COMMAND(ID_JUMP_BOOK_MARK, OnJumpBookMark)
	ON_COMMAND(ID_JUMP_BOOK_MARK_BACK, OnJumpBookMarkBack)
	ON_COMMAND(ID_CLEAR_ALL_BOOK_MARK, OnClearAllBookMark)
	ON_COMMAND(ID_TOGGLE_OVERWRITE, OnToggleOverwrite)
	ON_COMMAND(ID_SPACE_TO_TAB, OnSpaceToTab)
	ON_COMMAND(ID_TAB_TO_SPACE, OnTabToSpace)
	ON_COMMAND(ID_JUMP_BOOK_MARK_BACK_EXTEND, OnJumpBookMarkBackExtend)
	ON_COMMAND(ID_JUMP_BOOK_MARK_EXTEND, OnJumpBookMarkExtend)
	ON_COMMAND(ID_VIEW_BROWSER_01, OnViewBrowser01)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BROWSER_01, OnUpdateViewBrowser01)
	ON_COMMAND(ID_COPY_CLICKABLE_URL, OnCopyClickableUrl)
	ON_COMMAND(ID_EDIT_DELETE_ROW, OnEditDeleteRow)
	ON_COMMAND(ID_EDIT_DELETE_AFTER_CURSOR, OnEditDeleteAfterCursor)
	ON_COMMAND(ID_LAST_EDIT_POS, OnLastEditPos)
	ON_COMMAND(ID_EDIT_CUT_ROW, OnEditCutRow)
	ON_COMMAND(ID_EDIT_COPY_ROW, OnEditCopyRow)
	ON_COMMAND(ID_EDIT_PASTE_ROW, OnEditPasteRow)
	ON_COMMAND(ID_EDIT_PASTE_ROW_UP, OnEditPasteRowUp)
	ON_COMMAND(ID_SPLIT_START, OnSplitStart)
	ON_COMMAND(ID_SPLIT_START_EXTEND, OnSplitStartExtend)
	ON_COMMAND(ID_SPLIT_END, OnSplitEnd)
	ON_COMMAND(ID_SPLIT_END_EXTEND, OnSplitEndExtend)
	ON_COMMAND(ID_SELECT_WORD, OnSelectWord)
	ON_COMMAND(ID_VIEW_CLICKABLE_URL_01, OnViewClickableUrl01)
	ON_COMMAND(ID_VIEW_CLICKABLE_URL_03, OnViewClickableUrl03)
	ON_COMMAND(ID_VIEW_CLICKABLE_URL_01_NEW_WINDOW, OnViewClickableUrl01NewWindow)
	ON_COMMAND(ID_VIEW_CLICKABLE_URL_03_NEW_WINDOW, OnViewClickableUrl03NewWindow)
	ON_COMMAND(ID_DO_CLICKABLE_URL, OnDoClickableUrl)
	ON_COMMAND(ID_EDIT_JOIN_ROW, OnEditJoinRow)
	ON_COMMAND(ID_SEARCH_SELECTED, OnSearchSelected)
	ON_COMMAND(ID_TAG_JUMP, OnTagJump)
	ON_COMMAND(ID_DELETE_SPACE_END, OnDeleteSpaceEnd)
	ON_COMMAND(ID_INSERT_DATE_TIME, OnInsertDateTime)
	ON_COMMAND(ID_HANKAKU_TO_ZENKAKU, OnHankakuToZenkaku)
	ON_COMMAND(ID_ZENKAKU_TO_HANKAKU, OnZenkakuToHankaku)
	ON_COMMAND(ID_HANKAKU_TO_ZENKAKU_ALPHA, OnHankakuToZenkakuAlpha)
	ON_COMMAND(ID_HANKAKU_TO_ZENKAKU_KATA, OnHankakuToZenkakuKata)
	ON_COMMAND(ID_ZENKAKU_TO_HANKAKU_ALPHA, OnZenkakuToHankakuAlpha)
	ON_COMMAND(ID_ZENKAKU_TO_HANKAKU_KATA, OnZenkakuToHankakuKata)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_SPLIT_ROWS, OnSplitRows)
	ON_COMMAND(ID_JOIN_ROWS, OnJoinRows)
	ON_WM_DESTROY()
	ON_COMMAND(ID_INPUT_RETURN, OnInputReturn)
	ON_COMMAND(ID_EDIT_DELETE_BEFORE_CURSOR, OnEditDeleteBeforeCursor)
	ON_COMMAND(ID_INPUT_BR, OnInputBr)
	ON_COMMAND(ID_EDIT_DELETE_WORD_AFTER_CURSOR, OnEditDeleteWordAfterCursor)
	ON_COMMAND(ID_EDIT_DELETE_WORD_BEFORE_CURSOR, OnEditDeleteWordBeforeCursor)
	ON_COMMAND(ID_INVERT_BRACKETS, OnInvertBrackets)
	ON_COMMAND(ID_GO_MATCH_BRACKETS, OnGoMatchBrackets)
	ON_COMMAND(ID_EDIT_INDENT_SPACE, OnEditIndentSpace)
	ON_COMMAND(ID_EDIT_INDENT_SPACE_REV, OnEditIndentSpaceRev)
	ON_COMMAND(ID_SPACE_LEFT, OnSpaceLeft)
	ON_COMMAND(ID_SPACE_LEFT_EXTEND, OnSpaceLeftExtend)
	ON_COMMAND(ID_SPACE_RIGHT, OnSpaceRight)
	ON_COMMAND(ID_SPACE_RIGHT_EXTEND, OnSpaceRightExtend)
	ON_COMMAND(ID_EDIT_DEL_DUP_ROWS, OnEditDelDupRows)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DEL_DUP_ROWS, OnUpdateEditDelDupRows)
	ON_COMMAND(ID_SCM_EVAL, OnScmEval)
	ON_UPDATE_COMMAND_UI(ID_SCM_EVAL, OnUpdateScmEval)
	ON_COMMAND(ID_SEARCH_SELECTED_PREV, OnSearchSelectedPrev)
	ON_COMMAND(ID_CHAR_LEFT_EXTEND_BOX, OnCharLeftExtendBox)
	ON_COMMAND(ID_CHAR_RIGHT_EXTEND_BOX, OnCharRightExtendBox)
	ON_COMMAND(ID_LINE_DOWN_EXTEND_BOX, OnLineDownExtendBox)
	ON_COMMAND(ID_LINE_UP_EXTEND_BOX, OnLineUpExtendBox)
	ON_COMMAND(ID_BOX_PASTE, OnBoxPaste)
	ON_UPDATE_COMMAND_UI(ID_BOX_PASTE, OnUpdateBoxPaste)
	ON_COMMAND(ID_FILTER, OnFilter)
	ON_COMMAND(ID_VIEW_BROWSER_02, OnViewBrowser02)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BROWSER_02, OnUpdateViewBrowser02)
	ON_COMMAND(ID_VIEW_CLICKABLE_URL_02, OnViewClickableUrl02)
	ON_COMMAND(ID_WORD_LEFT_STOP_WB, OnWordLeftStopWb)
	ON_COMMAND(ID_WORD_RIGHT_STOP_WB, OnWordRightStopWb)
	ON_COMMAND(ID_WORD_LEFT_STOP_WB_EXTEND, OnWordLeftStopWbExtend)
	ON_COMMAND(ID_WORD_RIGHT_STOP_WB_EXTEND, OnWordRightStopWbExtend)
	//}}AFX_MSG_MAP
	// 標準印刷コマンド
	ON_UPDATE_COMMAND_UI_RANGE(ID_TO_LOWER, ID_TO_UPPER, OnUpdateSelected)
	ON_UPDATE_COMMAND_UI(ID_SPLIT_ROWS, OnUpdateSelected)
	ON_UPDATE_COMMAND_UI(ID_JOIN_ROWS, OnUpdateMultiLineEdit)
	ON_UPDATE_COMMAND_UI_RANGE(ID_EDIT_REVERSE_ROWS, ID_EDIT_SORT_ROWS_DESC, OnUpdateMultiLineEdit)
	ON_UPDATE_COMMAND_UI_RANGE(ID_INSERT_COMMENT, ID_DELETE_COMMENT, OnUpdateCommentEdit)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_OVR, OnUpdateOverwrite)
	ON_UPDATE_COMMAND_UI_RANGE(ID_COPY_CLICKABLE_URL, ID_VIEW_CLICKABLE_URL_03_NEW_WINDOW, OnUpdateClickableURL)
	ON_UPDATE_COMMAND_UI_RANGE(ID_HANKAKU_TO_ZENKAKU, ID_ZENKAKU_TO_HANKAKU_ALPHA, OnUpdateSelected)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COeditView クラスの構築/消滅


COeditView::COeditView()
{
	redraw_scr_pt.x = 0;
	redraw_scr_pt.y = 0;
}

COeditView::~COeditView()
{
	CPrintDlg::DeleteBackupData();
}


/////////////////////////////////////////////////////////////////////////////
// COeditView クラスの描画

void COeditView::OnDraw(CDC* pDC)
{
	COeditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: この場所にネイティブ データ用の描画コードを追加します。
}

/////////////////////////////////////////////////////////////////////////////
// COeditView クラスの診断

#ifdef _DEBUG
void COeditView::AssertValid() const
{
	CScrollView::AssertValid();
}

void COeditView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

COeditDoc* COeditView::GetDocument() // 非デバッグ バージョンはインラインです。
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(COeditDoc)));
	return (COeditDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// COeditView クラスのメッセージ ハンドラ

int COeditView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_edit_ctrl.SetEditData(GetDocument()->GetEditData());
	m_edit_ctrl.SetSplitterMode(TRUE);
/*
	// 縦スクロールバーは常に表示
	m_edit_ctrl.SetScrollStyle(m_edit_ctrl.GetScrollStyle() | VSCROLL_ALWAYS_ON);

	// for TrackPoint (WS_VSCROLL | WS_HSCROLL)
	m_edit_ctrl.Create(NULL, NULL,
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL, 
		CRect(0, 0, lpCreateStruct->cx, lpCreateStruct->cy), this, NULL);
*/
	// for TrackPoint (WS_VSCROLL | WS_HSCROLL)
	m_edit_ctrl.Create(NULL, NULL,
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL, 
		CRect(0, 0, lpCreateStruct->cx, lpCreateStruct->cy), this, NULL);

	m_edit_ctrl.SetCodeAssistWnd_InvertSelectText(TRUE);

	m_edit_ctrl.SetFont(&g_font);
	m_edit_ctrl.SetLineMode(g_option.text_editor.line_mode, g_option.text_editor.line_len, FALSE);
	SetEditorOption();

	SetFont(&g_font);

	return 0;
}

void COeditView::OnSize(UINT nType, int cx, int cy) 
{
	CScrollView::OnSize(nType, cx, cy);

	// for TrackPoint
	if(m_edit_ctrl.IsSplitterMode()) {
		// TrackPointのスクロールを有効にするため，スクロールバーをViewの外側に表示する
		int cx2 = cx += GetSystemMetrics(SM_CXVSCROLL);
		int cy2 = cy += GetSystemMetrics(SM_CYHSCROLL);
		m_edit_ctrl.MoveWindow(0, 0, cx2, cy2);
	} else {
		m_edit_ctrl.MoveWindow(0, 0, cx, cy);
	}
}

void COeditView::OnSetFocus(CWnd* pOldWnd) 
{
	CScrollView::OnSetFocus(pOldWnd);
	
	m_edit_ctrl.SetFocus();	
}

void COeditView::OnEditCopy() 
{
	m_edit_ctrl.Copy();
}

void COeditView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_edit_ctrl.CanCopy());
}

void COeditView::OnEditCut() 
{
	m_edit_ctrl.Cut();
}

void COeditView::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_edit_ctrl.CanCut());
}

void COeditView::OnEditPaste() 
{
	m_edit_ctrl.Paste();
}

void COeditView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_edit_ctrl.CanPaste());
}

void COeditView::OnBoxPaste() 
{
	m_edit_ctrl.Paste(FALSE, TRUE);
}

void COeditView::OnUpdateBoxPaste(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_edit_ctrl.CanPaste());
}

void COeditView::OnEditRedo() 
{
	m_edit_ctrl.Redo();
}

void COeditView::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_edit_ctrl.CanRedo());
}

void COeditView::OnEditUndo() 
{
	m_edit_ctrl.Undo();
}

void COeditView::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_edit_ctrl.CanUndo());
}

void COeditView::OnSelectAll() 
{
	m_edit_ctrl.SelectAll();
}

void COeditView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	switch(lHint) {
	case UPD_NEW_DOCUMENT:
		m_edit_ctrl.ClearAll();
		break;
	case UPD_INVALIDATE:
		m_edit_ctrl.Invalidate();
		break;
	case UPD_EDIT_MODE:
		SetEditorOption();
		break;
	case UPD_EDITOR_OPTION:
		SetEditorOption();
		GetDocument()->SetDocTitle();
		break;
	case UPD_FONT:
		SetFont((CFont *)pHint);
		m_edit_ctrl.SetFont((CFont *)pHint);
		break;
	case UPD_DELETE_PANE:
		SetEditorOption();
		m_edit_ctrl.Redraw();
		break;
	case UPD_WINDOW_MOVED:
		m_edit_ctrl.SetCaret(FALSE, 1);
		break;
	case UPD_SET_FOCUS:
		m_edit_ctrl.SetFocus();
		break;
	case UPD_PRE_RELOAD_FILE:
		redraw_scr_pt.y = m_edit_ctrl.GetScrollPos(SB_VERT);
		redraw_scr_pt.x = m_edit_ctrl.GetScrollPos(SB_HORZ);
		break;
	case UPD_POST_RELOAD_FILE:
		{
			if(redraw_scr_pt.y > m_edit_ctrl.GetScrollLimit(SB_VERT)) redraw_scr_pt.y = m_edit_ctrl.GetScrollLimit(SB_VERT);
			if(redraw_scr_pt.x > m_edit_ctrl.GetScrollLimit(SB_HORZ)) redraw_scr_pt.x = m_edit_ctrl.GetScrollLimit(SB_HORZ);
			m_edit_ctrl.SetScrollPos(SB_VERT, redraw_scr_pt.y, TRUE);
			m_edit_ctrl.SetScrollPos(SB_HORZ, redraw_scr_pt.x, TRUE);
			m_edit_ctrl.Redraw(TRUE);
		}
		break;
	}
}

void COeditView::SetEditorOption()
{
	GetDocument()->SetEditDataOption();

	QWORD option = ECS_INVERT_BRACKETS | ECS_DISABLE_KEY_DOWN | ECS_V_SCROLL_EX | 
		ECS_IME_CARET_COLOR | ECS_WORD_SELECT_MODE;
	int scr_style = m_edit_ctrl.GetScrollStyle() | KEEP_WS_VH_SCROLL_STYLE;

	if(m_edit_ctrl.GetLineMode() == EC_LINE_MODE_LEN) {
		m_edit_ctrl.SetLineMode(EC_LINE_MODE_LEN, g_option.text_editor.line_len, FALSE);
	}

	if(g_full_screen_mode) {
		option |= ECS_FULL_SCREEN_MODE;
		scr_style &= ~VSCROLL_ALWAYS_ON;
		scr_style |= (NO_HSCROLL_BAR | NO_VSCROLL_BAR);
		if(m_edit_ctrl.GetStyle() & WS_VSCROLL) m_edit_ctrl.ModifyStyle(WS_VSCROLL, 0);
		if(m_edit_ctrl.GetStyle() & WS_HSCROLL) m_edit_ctrl.ModifyStyle(WS_HSCROLL, 0);

		m_edit_ctrl.SetSpaces(g_option.text_editor.row_space, 
			g_option.text_editor.char_space,
			g_option.full_screen.top_space, g_option.full_screen.left_space, 
			g_option.full_screen.width, g_option.full_screen.height);
	} else {
		scr_style |= VSCROLL_ALWAYS_ON;
		scr_style &= ~(NO_HSCROLL_BAR | NO_VSCROLL_BAR);
		if(!(m_edit_ctrl.GetStyle() & WS_VSCROLL)) m_edit_ctrl.ModifyStyle(0, WS_VSCROLL);
		if(!(m_edit_ctrl.GetStyle() & WS_HSCROLL)) m_edit_ctrl.ModifyStyle(0, WS_HSCROLL);

		if(m_edit_ctrl.GetRowSpace() != g_option.text_editor.row_space ||
			m_edit_ctrl.GetCharSpaceSetting() != g_option.text_editor.char_space ||
			m_edit_ctrl.GetTopSpace() != g_option.text_editor.top_space ||
			m_edit_ctrl.GetLeftSpace() != g_option.text_editor.left_space) {
			m_edit_ctrl.SetSpaces(g_option.text_editor.row_space, 
				g_option.text_editor.char_space,
				g_option.text_editor.top_space,
				g_option.text_editor.left_space);
		}
	}
	if(g_option.text_editor.show_line_feed) {
		option |= ECS_SHOW_LINE_FEED;
	}
	if(g_option.text_editor.show_line_end) {
		option |= ECS_SHOW_LINE_END;
	}
	if(g_option.text_editor.show_tab) {
		option |= ECS_SHOW_TAB;
	}

	if(g_option.text_editor.show_row_num) {
		option |= ECS_SHOW_ROW_NUM;
	}
	if(g_option.text_editor.show_col_num) {
		if(GetParent()->IsKindOf(RUNTIME_CLASS(CSplitterWnd))) {
			CSplitterWnd *sp = (CSplitterWnd *)GetParent();
			if(sp->GetPane(0, 0) == this || g_option.text_editor.show_col_num_split_window) {
				option |= ECS_SHOW_COL_NUM;
			}
		} else {
			option |= ECS_SHOW_COL_NUM;
		}
	}

	if(g_option.text_editor.show_space) {
		option |= ECS_SHOW_SPACE;
	}
	if(g_option.text_editor.show_2byte_space) {
		option |= ECS_SHOW_2BYTE_SPACE;
	}
	if(g_option.text_editor.show_row_line) {
		option |= ECS_SHOW_ROW_LINE;
	}
	if(g_option.text_editor.show_ruled_line) {
		option |= ECS_SHOW_RULED_LINE;
	}
	if(g_option.text_editor.show_edit_row) {
		option |= ECS_SHOW_EDIT_ROW;
	}
	if(g_option.text_editor.show_brackets_bold) {
		option |= ECS_SHOW_BRACKETS_BOLD;
	}
	if(g_option.text_editor.clickable_url) {
		option |= ECS_CLICKABLE_URL;
	}
	if(g_option.text_editor.drag_drop_edit) {
		option |= ECS_DRAG_DROP_EDIT;
	}

	if(g_option.search_loop_msg == TRUE) {
		option |= ECS_SEARCH_LOOP_MSG;
	}
	if(g_option.text_editor.mouse_select_copy == TRUE) {
		option |= ECS_DRAG_SELECT_COPY;
	}

	if(g_str_token.HaveComment() == FALSE) {
		option |= ECS_NO_COMMENT_CHECK;
	}
	if(g_str_token.GetTagMode() == TRUE) {
		option |= ECS_HTML_MODE;
	}
	if(g_option.text_editor.row_copy_at_no_sel) {
		option |= ECS_ROW_COPY_AT_NO_SEL;
	}
	if(g_option.text_editor.clear_after_copy) {
		option |= ECS_CLEAR_AFTER_COPY;
	}

	m_edit_ctrl.SetBracketsBlinkTime(g_option.text_editor.brackets_blink_time);
	m_edit_ctrl.SetUseKeywordWindow(g_option.text_editor.use_keyword_window);
	m_edit_ctrl.SetWheelScrollRate(120.0 / (double)g_option.text_editor.wheel_scroll_row);

	if(scr_style != m_edit_ctrl.GetScrollStyle()) {
		m_edit_ctrl.SetScrollStyle(scr_style);
	}
	m_edit_ctrl.SetExStyle2(option);

	for(int i = 0; i < EDIT_CTRL_COLOR_CNT; i++) {
		m_edit_ctrl.SetColor(i, g_option.text_editor.color[i]);
	}
	m_edit_ctrl.ResetCaret();

	m_edit_ctrl.SetCodeAssistWndColor(GRID_BG_COLOR, g_option.text_editor.color[BG_COLOR]);
	m_edit_ctrl.SetCodeAssistWndColor(GRID_TEXT_COLOR, g_option.text_editor.color[TEXT_COLOR]);
	m_edit_ctrl.SetCodeAssistWndColor(GRID_LINE_COLOR, g_option.text_editor.color[PEN_COLOR]);
	m_edit_ctrl.SetCodeAssistWndColor(GRID_SELECT_COLOR, g_option.text_editor.color[SELECTED_COLOR]);

	if(g_full_screen_mode) {
		m_edit_ctrl.SetBackGroundImage(g_option.full_screen.bg_image_file, TRUE);
	} else {
		m_edit_ctrl.SetBackGroundImage(NULL, FALSE);
	}
}

void COeditView::SearchMsg(int ret_v, int dir, BOOL b_looped)
{
	if(!::IsWindow(GetSafeHwnd())) return;
	CString msg = MakeSearchMsg(ret_v, dir, b_looped);
	if(!msg.IsEmpty()) GetParentFrame()->SetMessageText(msg);
}

LRESULT COeditView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message) {
	case EC_WM_CHANGE_CARET_POS:
		{
            g_cur_pos.x = static_cast<LONG>(wParam) + 1;
			g_cur_pos.y = static_cast<LONG>(lParam) + 1;
			GetDocument()->CheckModified();

			CSplitterWnd *sp = (CSplitterWnd *)GetParent();
			if(sp->GetPane(0, 0) == this) {
				scm_call_event_handler(g_sc, SCM_EVENT_ON_CURSOR_MOVED);
			}
		}
		break;
	case EC_WM_LBUTTONDBLCLK:
		if(g_option.enable_tag_jump) OnTagJump();
		break;
	case WM_SEARCH_TEXT:
		{
			BOOL b_looped;
			GetParentFrame()->SetMessageText(_T(""));
			int ret_v = m_edit_ctrl.SearchText2(g_search_data.m_search_text.GetBuffer(0),
				g_search_data.m_dir, g_search_data.m_distinct_lwr_upr, g_search_data.m_distinct_width_ascii,
				g_search_data.m_regexp, &b_looped);
			SearchMsg(ret_v, g_search_data.m_dir, b_looped);
			return ret_v;
		}
		break;
	case WM_REPLACE_TEXT:
		{
			int replace_cnt;
			BOOL b_looped;
			GetParentFrame()->SetMessageText(_T(""));
			int ret_v = m_edit_ctrl.ReplaceText2(g_search_data.m_search_text.GetBuffer(0),
				g_search_data.m_replace_text.GetBuffer(0),
				g_search_data.m_dir, g_search_data.m_distinct_lwr_upr, g_search_data.m_distinct_width_ascii,
				g_search_data.m_regexp, g_search_data.m_all, 
				g_search_data.m_replace_selected_area, &replace_cnt, &b_looped);
			GetDocument()->CheckModified();

			if(g_search_data.m_all) {
				CString msg;
				msg.Format(_T("%d件置換しました"), replace_cnt);
				GetParentFrame()->SetMessageText(msg);
			} else {
				SearchMsg(ret_v, g_search_data.m_dir, b_looped);
			}
		}
		return 0;
		break;
	case WM_FILTER_TEXT:
		{
			int replace_cnt;
			int ret_v = m_edit_ctrl.FilterText2(
				g_search_data.m_search_text.GetBuffer(0),
				g_search_data.m_distinct_lwr_upr,
				g_search_data.m_distinct_width_ascii,
				g_search_data.m_regexp, 
				m_edit_ctrl.HaveSelectedMultiLine(),
				&replace_cnt, g_search_data.m_dir);
			GetDocument()->CheckModified();

			if(ret_v == 0) {
				CString msg;
				msg.Format(_T("%d行削除しました"), replace_cnt);
				SearchMsg(ret_v, g_search_data.m_dir, FALSE);
			}
		}
		return 0;
		break;
	case WM_PASTE_COLOR:
		m_edit_ctrl.Paste((TCHAR *)wParam);
		return 0;
		break;
	case EC_WM_CHANGE_SELECTED_TEXT:
		scm_call_event_handler(g_sc, SCM_EVENT_ON_SELECTION_CHANGED);
		break;
	}

	return CScrollView::WindowProc(message, wParam, lParam);
}

void COeditView::OnSearch() 
{
	HideSearchDlgs();

	if(m_edit_ctrl.HaveSelected() && m_edit_ctrl.HaveSelectedMultiLine() == FALSE) {
		g_search_data.m_search_text = m_edit_ctrl.GetSelectedText();
	}

	CSearchDlgSingleton::GetDlg().ShowDialog(this, WM_SEARCH_TEXT, &g_search_data, g_option.search_dlg_close_at_found);
}

void COeditView::SetSearchText(const TCHAR *search_text, BOOL distinct_lwr_upr, BOOL distinct_width_ascii)
{
	g_search_data.m_dir = 1;
	g_search_data.m_distinct_lwr_upr = distinct_lwr_upr;
	g_search_data.m_distinct_width_ascii = distinct_width_ascii;
	g_search_data.m_regexp = TRUE;
	g_search_data.m_search_text = search_text;
	g_search_data.m_all = FALSE;
	g_search_data.m_replace_text = "";
	g_search_data.m_replace_selected_area = FALSE;
}

void COeditView::SetSearchSelected()
{
	if(m_edit_ctrl.HaveSelected()) {
		g_search_data.m_dir = 1;
		g_search_data.m_distinct_lwr_upr = FALSE;
		g_search_data.m_distinct_width_ascii = TRUE;
		g_search_data.m_regexp = FALSE;
		g_search_data.m_search_text = m_edit_ctrl.GetSelectedText();
	}
}

void COeditView::OnSearchSelected() 
{
	SetSearchSelected();
	OnSearchNext();
}

void COeditView::OnSearchSelectedPrev() 
{
	SetSearchSelected();
	OnSearchPrev();
}

void COeditView::OnSearchNext() 
{
	BOOL b_looped;
	GetParentFrame()->SetMessageText(_T(""));
	int ret_v = m_edit_ctrl.SearchNext2(g_search_data.m_search_text.GetBuffer(0),
		g_search_data.m_distinct_lwr_upr, g_search_data.m_distinct_width_ascii, g_search_data.m_regexp, &b_looped);
	SearchMsg(ret_v, 1, b_looped);
}

void COeditView::OnSearchPrev() 
{
	BOOL b_looped;
	GetParentFrame()->SetMessageText(_T(""));
	int ret_v = m_edit_ctrl.SearchPrev2(g_search_data.m_search_text.GetBuffer(0),
		g_search_data.m_distinct_lwr_upr, g_search_data.m_distinct_width_ascii, g_search_data.m_regexp, &b_looped);
	SearchMsg(ret_v, -1, b_looped);
}

void COeditView::OnReplace() 
{
	HideSearchDlgs();

	if(m_edit_ctrl.HaveSelected() && m_edit_ctrl.HaveSelectedMultiLine() == FALSE) {
		g_search_data.m_search_text = m_edit_ctrl.GetSelectedText();
	}

	g_search_data.m_replace_selected_area = m_edit_ctrl.HaveSelectedMultiLine();

	CReplaceDlgSingleton::GetDlg().ShowDialog(this, WM_SEARCH_TEXT, WM_REPLACE_TEXT, &g_search_data);
}

void COeditView::OnUpdateSelected(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_edit_ctrl.HaveSelected());
}

void COeditView::OnUpdateCommentEdit(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(g_str_token.HaveRowComment());
}

void COeditView::OnUpdateMultiLineEdit(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_edit_ctrl.HaveSelectedMultiLine());
}

void COeditView::OnEditIndent() 
{
	m_edit_ctrl.InsertTab(FALSE);
}

void COeditView::OnEditIndentRev() 
{
	m_edit_ctrl.InsertTab(TRUE);
}

void COeditView::OnEditIndentSpace() 
{
	m_edit_ctrl.IndentSpace(FALSE);
}

void COeditView::OnEditIndentSpaceRev() 
{
	m_edit_ctrl.IndentSpace(TRUE);
}

void COeditView::OnEditReverseRows() 
{
	m_edit_ctrl.ReverseSelectedRows();
}

void COeditView::OnEditSortRowsAsc() 
{
	m_edit_ctrl.SortSelectedRows(FALSE);
}

void COeditView::OnEditSortRowsDesc() 
{
	m_edit_ctrl.SortSelectedRows(TRUE);
}

void COeditView::OnToLower() 
{
	m_edit_ctrl.ToLower();
}

void COeditView::OnToUpper() 
{
	m_edit_ctrl.ToUpper();
}

void COeditView::OnFilePrint() 
{
	CWaitCursor	wait_cursor;
	CPrintDlg	dlg;
	TCHAR		msg_buf[1024];

	dlg.m_line_mode = g_option.text_editor.line_mode;
	dlg.m_line_len = g_option.text_editor.line_len;
	dlg.m_row_num_digit = m_edit_ctrl.GetRowNumDigit();

	if(dlg.DoModal() != IDOK) {
		return;
	}

	CString	title;
	if(dlg.m_print_title) {
		title = GetDocument()->GetTitle();

		// タイトルの" *"を取り除く
		if(title.Find(_T(" *"), 0) != -1) {
			TCHAR *p;
			p = title.GetBuffer(0);
			p[_tcslen(p) - 2] = '\0';
			title.ReleaseBuffer();
		}
	} else {
		title = _T("");
	}

	int line_len = -1;
	if(g_option.text_editor.line_mode == EC_LINE_MODE_LEN) line_len = g_option.text_editor.line_len;
	
	CRect	space(dlg.m_space_left, dlg.m_space_top, dlg.m_space_right, dlg.m_space_bottom);

	CEditData *edit_data = GetDocument()->GetEditData();
	if(dlg.m_print_selected_area) {
		if(!m_edit_ctrl.HaveSelected()) {
			MessageBox(_T("選択範囲がありません"), _T("印刷"), MB_ICONERROR | MB_OK);
			return;
		}

		edit_data = new CEditData();
		GetDocument()->SetEditDataOption(edit_data);

		edit_data->paste(GetDocument()->GetEditData()->get_selected_text());
	}

	print_edit_data(dlg.printer_dc, title.GetBuffer(0), edit_data,
		dlg.m_font_face_name.GetBuffer(0), dlg.m_font_point,
		space, dlg.m_print_page, dlg.m_print_date, line_len, dlg.m_row_num_digit, msg_buf);

	if(dlg.m_print_selected_area) {
		delete edit_data;
	}

	MessageBox(_T("印刷しました"), _T("印刷"), MB_ICONINFORMATION | MB_OK);
}

void COeditView::ReplaceText(TCHAR *search_text, TCHAR *replace_text, BOOL bInfrateSelectedArea)
{
	POINT cur_pt = GetDocument()->GetEditData()->get_cur_pt();
	int replace_cnt;
	BOOL haveSelected = m_edit_ctrl.HaveSelected();

	if(bInfrateSelectedArea) m_edit_ctrl.InfrateSelectedArea();

	m_edit_ctrl.ReplaceText(search_text, replace_text, 1, TRUE, TRUE, TRUE, TRUE, &replace_cnt, NULL);

	if(!haveSelected) {
		m_edit_ctrl.ClearSelected();
		if(replace_cnt == 0) m_edit_ctrl.MoveCaretPos(cur_pt.y, cur_pt.x);
	}
}

void COeditView::OnDeleteSpace() 
{
	ReplaceText(_T("^[\\s　]+"), _T(""));
}

void COeditView::OnDeleteSpaceEnd() 
{
	ReplaceText(_T("[\\s　]+$"), _T(""));
}

void COeditView::OnInsertGt() 
{
	ReplaceText(_T("^"), _T("> "));
}

void COeditView::OnDeleteGt() 
{
	ReplaceText(_T("^>\\s?"), _T(""));
}

void COeditView::OnLineModeLen() 
{
	m_edit_ctrl.SetLineMode(EC_LINE_MODE_LEN, g_option.text_editor.line_len);
	g_option.text_editor.line_mode = EC_LINE_MODE_LEN;
}

void COeditView::OnLineModeNormal() 
{
	m_edit_ctrl.SetLineMode(EC_LINE_MODE_NORMAL);
	g_option.text_editor.line_mode = EC_LINE_MODE_NORMAL;
}

void COeditView::OnLineModeRight() 
{
	m_edit_ctrl.SetLineMode(EC_LINE_MODE_RIGHT);
	g_option.text_editor.line_mode = EC_LINE_MODE_RIGHT;
}

void COeditView::OnUpdateLineModeLen(CCmdUI* pCmdUI) 
{
	if(m_edit_ctrl.GetLineMode() == EC_LINE_MODE_LEN) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
	}
}

void COeditView::OnUpdateLineModeNormal(CCmdUI* pCmdUI) 
{
	if(m_edit_ctrl.GetLineMode() == EC_LINE_MODE_NORMAL) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
	}
}

void COeditView::OnUpdateLineModeRight(CCmdUI* pCmdUI) 
{
	if(m_edit_ctrl.GetLineMode() == EC_LINE_MODE_RIGHT) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
	}
}

void COeditView::OnEditDelete() 
{
	m_edit_ctrl.DeleteKey();
}

void COeditView::OnColorInput() 
{
	if(CSelectColorDlgSingleton::IsVisible()) {
		CSelectColorDlgSingleton::GetDlg().ShowWindow(SW_HIDE);
	}
	CSelectColorDlgSingleton::GetDlg().ShowDialog(this, WM_PASTE_COLOR);
}

void COeditView::OnUpdateColorInput(CCmdUI* pCmdUI) 
{
	// モードに関係なく利用できるようにする
	pCmdUI->Enable(TRUE);
}

void COeditView::OnClearSearchText() 
{
	m_edit_ctrl.ClearSearchText();
}

BOOL COeditView::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct) 
{
	switch(pCopyDataStruct->dwData) {
	case WM_COPY_DATA_GET_FILE_NAME:
		{
			TCHAR	*file_name;
			file_name = (TCHAR *)pCopyDataStruct->lpData;

			if(_tcscmp(file_name, GetDocument()->GetPathName()) == 0) return TRUE;

			return FALSE;
		}
	case WM_COPY_DATA_SET_SEARCH_DATA:
		{
			struct _set_search_data_st *search_data = (struct _set_search_data_st *)pCopyDataStruct->lpData;

			m_edit_ctrl.SaveSearchData2(search_data->search_text,
				search_data->distinct_lwr_upr, search_data->distinct_width_ascii, search_data->regexp);
			GetDocument()->GetEditData()->set_cur(search_data->row, 0);
			m_edit_ctrl.SetCaret(TRUE, 1);

			g_search_data.m_search_text = search_data->search_text;
			g_search_data.m_distinct_lwr_upr = search_data->distinct_lwr_upr;
			g_search_data.m_distinct_width_ascii = search_data->distinct_width_ascii;
			g_search_data.m_regexp = search_data->regexp;

			if(AfxGetMainWnd()->IsIconic()) {
				AfxGetMainWnd()->ShowWindow(SW_SHOWNORMAL);
			}

			return TRUE;
		}
	case WM_COPY_DATA_SET_OPTION:
		{
			struct _set_option_st *option_data = (struct _set_option_st *)pCopyDataStruct->lpData;

			if(option_data->row >= 0 && option_data->row < GetDocument()->GetEditData()->get_row_cnt()) {
				GetDocument()->GetEditData()->set_cur(option_data->row, 0);
				m_edit_ctrl.SetCaret(TRUE, 1);
			}
			if(_tcslen(option_data->mode) > 0) {
				((COeditApp *)AfxGetApp)->SetEditMode(g_lang_setting_list.GetSettingIdx(option_data->mode));
				GetDocument()->UpdateAllViews(NULL, UPD_EDIT_MODE);
				GetDocument()->RedrawEditCtrl();
				AfxGetApp()->OnIdle(0);
			}

			return TRUE;
		}
	}

	return CScrollView::OnCopyData(pWnd, pCopyDataStruct);
}

void COeditView::OnInsertComment() 
{
	if(g_str_token.HaveRowComment() == FALSE) return;
	ReplaceText(_T("^"), g_str_token.GetRowComment());
}

void COeditView::OnDeleteComment() 
{
	if(g_str_token.HaveRowComment() == FALSE) return;
	CString str;
	str.Format(_T("^(%s)"), g_str_token.GetRowComment());
	ReplaceText(str.GetBuffer(0), _T(""));
}

static CString LoadMenuString(UINT nID)
{
	CString menu_str;
	if(menu_str.LoadString(nID) == FALSE) return _T("");

	int n = menu_str.Find('\n', 0);
	if(n == -1) return menu_str;

	menu_str = menu_str.Right(menu_str.GetLength() - n - 1);
	return menu_str;
}

void COeditView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CMenu	menu;
	menu.LoadMenu(IDR_EDIT_MENU);
	CMenu	url_menu;

	if(m_edit_ctrl.HitTestClickableURL()) {
		url_menu.LoadMenu(IDR_URL_MENU);
		CString		menu_str;
		menu.GetSubMenu(0)->InsertMenu(10, MF_BYPOSITION | MF_SEPARATOR, 0, _T(""));
		menu_str = LoadMenuString(ID_DO_CLICKABLE_URL);
		menu.GetSubMenu(0)->InsertMenu(11, MF_BYPOSITION | MF_POPUP, (UINT_PTR)url_menu.m_hMenu, menu_str);
		menu_str = LoadMenuString(ID_COPY_CLICKABLE_URL);
		menu.GetSubMenu(0)->InsertMenu(12, MF_BYPOSITION, ID_COPY_CLICKABLE_URL, menu_str);
	}

	UINT flags = TPM_LEFTALIGN | TPM_RIGHTBUTTON;

	// キーボード操作のときは、キャレット付近にメニューを表示する
	if(point.y < 0) {
		CPoint caret_pos = m_edit_ctrl.GetCaretPos();
		ClientToScreen(&caret_pos);

		point.x = caret_pos.x;

		if(caret_pos.y + GetPopupMenuHeight(menu.GetSubMenu(0)->GetSafeHmenu()) > ::GetSystemMetrics(SM_CYSCREEN)) {
			point.y = caret_pos.y;
			flags |= TPM_BOTTOMALIGN;
		} else {
			point.y = caret_pos.y + m_edit_ctrl.GetRowHeight();
		}
	}

	// CMainFrameを親ウィンドウにすると、メニューの有効無効を、メインメニューと同じにできる
	menu.GetSubMenu(0)->TrackPopupMenu(flags, point.x, point.y, GetParentFrame());

	// メニュー削除
	menu.DestroyMenu();
}

void COeditView::OnLineJump() 
{
	CLineJumpDlg	dlg;

	if(dlg.DoModal() != IDOK) {
		return;
	}

	m_edit_ctrl.MoveCaretPos(dlg.m_line_no - 1, 0);
}

void COeditView::OnSearchFile() 
{
	CString					cmd;
	PROCESS_INFORMATION		pi;
	STARTUPINFO				si;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	CString opt = _T("");
	if(g_option.ogrep.search_dir_as_current_dir) {
		TCHAR cur_dir[_MAX_PATH];
		GetCurrentDirectory(_MAX_PATH, cur_dir);
		opt.Format(_T("/d \"%s\""), cur_dir);
	}

	CString str = m_edit_ctrl.GetSelectedText();
	if(str != _T("") && str.Find('\n') == -1) {
		cmd.Format(_T("%sogrep.exe \"%s\" %s"), GetAppPath().GetBuffer(0), str.GetBuffer(0), opt.GetBuffer(0));
	} else {
		cmd.Format(_T("%sogrep.exe %s"), GetAppPath().GetBuffer(0), opt.GetBuffer(0));
	}

	if(CreateProcess(NULL, cmd.GetBuffer(0), NULL, NULL, FALSE,
		0, NULL, NULL, &si, &pi) == FALSE) {
		AfxMessageBox(_T("起動に失敗しました"), MB_OK);
		return;
	}

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}

void COeditView::OnCharLeft() { m_edit_ctrl.CharLeft(FALSE); }
void COeditView::OnCharLeftExtend() { m_edit_ctrl.CharLeft(TRUE); }
void COeditView::OnCharRight() { m_edit_ctrl.CharRight(FALSE); }
void COeditView::OnCharRightExtend() { m_edit_ctrl.CharRight(TRUE); }
void COeditView::OnLineDown() { m_edit_ctrl.LineDown(FALSE); }
void COeditView::OnLineDownExtend() { m_edit_ctrl.LineDown(TRUE); }
void COeditView::OnLineUp() { m_edit_ctrl.LineUp(FALSE); }
void COeditView::OnLineUpExtend() { m_edit_ctrl.LineUp(TRUE); }

void COeditView::OnCharLeftExtendBox() 
{
	if(!m_edit_ctrl.HaveSelected()) m_edit_ctrl.NextBoxSelect();
	m_edit_ctrl.CharLeft(TRUE);
}

void COeditView::OnCharRightExtendBox() 
{
	if(!m_edit_ctrl.HaveSelected()) m_edit_ctrl.NextBoxSelect();
	m_edit_ctrl.CharRight(TRUE);
}

void COeditView::OnLineDownExtendBox() 
{
	if(!m_edit_ctrl.HaveSelected()) m_edit_ctrl.NextBoxSelect();
	m_edit_ctrl.LineDown(TRUE);
}

void COeditView::OnLineUpExtendBox() 
{
	if(!m_edit_ctrl.HaveSelected()) m_edit_ctrl.NextBoxSelect();
	m_edit_ctrl.LineUp(TRUE);
}

void COeditView::OnWordLeft() { m_edit_ctrl.WordLeft(FALSE); }
void COeditView::OnWordLeftExtend() { m_edit_ctrl.WordLeft(TRUE); }
void COeditView::OnWordRight() { m_edit_ctrl.WordRight(FALSE); }
void COeditView::OnWordRightExtend() { m_edit_ctrl.WordRight(TRUE); }

void COeditView::OnLineStart() { m_edit_ctrl.LineStart(FALSE); }
void COeditView::OnLineStartExtend() { m_edit_ctrl.LineStart(TRUE); }
void COeditView::OnLineEnd() { m_edit_ctrl.LineEnd(FALSE); }
void COeditView::OnLineEndExtend() { m_edit_ctrl.LineEnd(TRUE); }

void COeditView::OnSplitStart() { m_edit_ctrl.SplitStart(FALSE); }
void COeditView::OnSplitStartExtend() { m_edit_ctrl.SplitStart(TRUE); }
void COeditView::OnSplitEnd() { m_edit_ctrl.SplitEnd(FALSE); }
void COeditView::OnSplitEndExtend() { m_edit_ctrl.SplitEnd(TRUE); }

void COeditView::OnDocumentStart() { m_edit_ctrl.DocumentStart(FALSE); }
void COeditView::OnDocumentStartExtend() { m_edit_ctrl.DocumentStart(TRUE); }
void COeditView::OnDocumentEnd() { m_edit_ctrl.DocumentEnd(FALSE); }
void COeditView::OnDocumentEndExtend() { m_edit_ctrl.DocumentEnd(TRUE); }

void COeditView::OnEditBackspace() { m_edit_ctrl.BackSpace(); }
void COeditView::OnKeywordCompletion() { m_edit_ctrl.KeywordCompletion(FALSE); }
void COeditView::OnKeywordCompletionRev() { m_edit_ctrl.KeywordCompletion(TRUE); }

void COeditView::OnScrollUp() { m_edit_ctrl.ScrollUp(); }
void COeditView::OnScrollDown() { m_edit_ctrl.ScrollDown(); }
void COeditView::OnScrollPageUp() { m_edit_ctrl.ScrollPageUp(); }
void COeditView::OnScrollPageDown() { m_edit_ctrl.ScrollPageDown(); }

void COeditView::OnPageUp() { m_edit_ctrl.PageUp(FALSE); }
void COeditView::OnPageUpExtend() { m_edit_ctrl.PageUp(TRUE); }
void COeditView::OnPageDown() { m_edit_ctrl.PageDown(FALSE); }
void COeditView::OnPageDownExtend() { m_edit_ctrl.PageDown(TRUE); }

void COeditView::OnSelectRow() { m_edit_ctrl.SelectRow(); }
void COeditView::OnSelectWord() { m_edit_ctrl.SelectWord(); }

void COeditView::OnBoxSelect() { m_edit_ctrl.NextBoxSelect(); }


DROPEFFECT COeditView::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	return CScrollView::OnDragEnter(pDataObject, dwKeyState, point);
}

void COeditView::OnDragLeave() 
{
	CScrollView::OnDragLeave();
}

DROPEFFECT COeditView::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	return CScrollView::OnDragOver(pDataObject, dwKeyState, point);
}

BOOL COeditView::OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) 
{
	return CScrollView::OnDrop(pDataObject, dropEffect, point);
}

void COeditView::OnSetBookMark() { m_edit_ctrl.SetBookMark(); }
void COeditView::OnJumpBookMark() { m_edit_ctrl.JumpBookMark(FALSE, FALSE); }
void COeditView::OnJumpBookMarkBack() { m_edit_ctrl.JumpBookMark(TRUE, FALSE); }
void COeditView::OnJumpBookMarkExtend() { m_edit_ctrl.JumpBookMark(FALSE, TRUE); }
void COeditView::OnJumpBookMarkBackExtend() { m_edit_ctrl.JumpBookMark(TRUE, TRUE); }
void COeditView::OnClearAllBookMark() { m_edit_ctrl.ClearAllBookMark(); }

void COeditView::OnToggleOverwrite() { m_edit_ctrl.ToggleOverwrite(); }
void COeditView::OnUpdateOverwrite(CCmdUI* pCmdUI) { pCmdUI->Enable(m_edit_ctrl.GetOverwrite()); }

void COeditView::OnSpaceToTab() 
{
	BOOL haveSelected = m_edit_ctrl.HaveSelected();

	m_edit_ctrl.InfrateSelectedArea();
	m_edit_ctrl.SpaceToTab();

	if(!haveSelected) m_edit_ctrl.ClearSelected();
}

void COeditView::OnTabToSpace() 
{
	BOOL haveSelected = m_edit_ctrl.HaveSelected();

	m_edit_ctrl.InfrateSelectedArea();
	m_edit_ctrl.TabToSpace();

	if(!haveSelected) m_edit_ctrl.ClearSelected();
}

void COeditView::OnViewBrowser03() 
{
	if(GetDocument()->DoFileSave() == FALSE) return;
	CString filename = GetDocument()->GetPathName();
	ViewBrowser(_T("Browser03"), g_option.browser_03.GetBuffer(0), filename.GetBuffer(0), FALSE);
}

void COeditView::OnUpdateViewBrowser03(CCmdUI* pCmdUI) 
{
//	pCmdUI->Enable((g_option.edit_mode == EDIT_MODE_HTML));
	pCmdUI->Enable(TRUE);
}

void COeditView::OnViewBrowser02() 
{
	if(GetDocument()->DoFileSave() == FALSE) return;
	CString filename = GetDocument()->GetPathName();
	ViewBrowser(_T("Browser02"), g_option.browser_02.GetBuffer(0), filename.GetBuffer(0), FALSE);
}

void COeditView::OnUpdateViewBrowser02(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(TRUE);
}

void COeditView::OnViewBrowser01() 
{
	if(GetDocument()->DoFileSave() == FALSE) return;
	CString filename = GetDocument()->GetPathName();
	ViewBrowser(_T("Browser01"), g_option.browser_01.GetBuffer(0), filename.GetBuffer(0), FALSE);
}

void COeditView::OnUpdateViewBrowser01(CCmdUI* pCmdUI) 
{
//	pCmdUI->Enable((g_option.edit_mode == EDIT_MODE_HTML));
	pCmdUI->Enable(TRUE);
}

void COeditView::OnCopyClickableUrl() 
{
	m_edit_ctrl.CopyClickable();
}

void COeditView::OnUpdateClickableURL(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_edit_ctrl.HitTestClickableURL());
}

void COeditView::OnEditDeleteRow() 
{
	m_edit_ctrl.DeleteRow();
}

void COeditView::OnEditDeleteAfterCursor() 
{
	m_edit_ctrl.DeleteAfterCaret();
}

void COeditView::OnLastEditPos() 
{
	m_edit_ctrl.MoveLastEditPos();
}

void COeditView::OnEditCutRow() 
{
	m_edit_ctrl.CutRow();
}

void COeditView::OnEditCopyRow() 
{
	m_edit_ctrl.CopyRow();
}

void COeditView::OnEditPasteRow() 
{
	m_edit_ctrl.PasteRow(FALSE);
}

void COeditView::OnEditPasteRowUp() 
{
	m_edit_ctrl.PasteRow(TRUE);
}


void COeditView::OnViewClickableUrl02() 
{
	CString url = m_edit_ctrl.GetClickableURL();
	if(url == _T("")) return;
	ViewBrowser(_T("Browser02"), g_option.browser_02.GetBuffer(0), url.GetBuffer(0), FALSE);
}

void COeditView::OnViewClickableUrl01() 
{
	CString url = m_edit_ctrl.GetClickableURL();
	if(url == _T("")) return;
	ViewBrowser(_T("Browser01"), g_option.browser_01.GetBuffer(0), url.GetBuffer(0), FALSE);
}

void COeditView::OnViewClickableUrl03() 
{
	CString url = m_edit_ctrl.GetClickableURL();
	if(url == _T("")) return;
	ViewBrowser(_T("Browser03"), g_option.browser_03.GetBuffer(0), url.GetBuffer(0), FALSE);
}

void COeditView::OnViewClickableUrl01NewWindow() 
{
	CString url = m_edit_ctrl.GetClickableURL();
	if(url == _T("")) return;
	ViewBrowser(_T("Browser01"), g_option.browser_01.GetBuffer(0), url.GetBuffer(0), TRUE);
}

void COeditView::OnViewClickableUrl03NewWindow() 
{
	CString url = m_edit_ctrl.GetClickableURL();
	if(url == _T("")) return;
	ViewBrowser(_T("Browser03"), g_option.browser_03.GetBuffer(0), url.GetBuffer(0), TRUE);
}

void COeditView::OnDoClickableUrl() 
{
	m_edit_ctrl.DoClickable();	
}

void COeditView::OnEditJoinRow() 
{
	m_edit_ctrl.JoinRow();	
}

void COeditView::OnTagJump() 
{
	int row = GetDocument()->GetEditData()->get_cur_row();
	if(TagJump(GetDocument()->GetEditData()->get_row_buf(row)) != FALSE) {
		m_edit_ctrl.ClearSelected();
	}
}

void COeditView::OnInitialUpdate() 
{
	CScrollView::OnInitialUpdate();

	SetScrollSizes(MM_TEXT, CSize(INT_MAX, INT_MAX));

	m_edit_ctrl.CheckScrollBars();
}

void COeditView::OnInsertDateTime() 
{
	m_edit_ctrl.InsertDateTime();	
}

void COeditView::OnHankakuToZenkaku() 
{
	m_edit_ctrl.HankakuToZenkaku(TRUE, TRUE);
}

void COeditView::OnHankakuToZenkakuAlpha() 
{
	m_edit_ctrl.HankakuToZenkaku(TRUE, FALSE);
}

void COeditView::OnHankakuToZenkakuKata() 
{
	m_edit_ctrl.HankakuToZenkaku(FALSE, TRUE);
}

void COeditView::OnZenkakuToHankaku() 
{
	m_edit_ctrl.ZenkakuToHankaku(TRUE, TRUE);
}

void COeditView::OnZenkakuToHankakuAlpha() 
{
	m_edit_ctrl.ZenkakuToHankaku(TRUE, FALSE);
}

void COeditView::OnZenkakuToHankakuKata() 
{
	m_edit_ctrl.ZenkakuToHankaku(FALSE, TRUE);
}


BOOL COeditView::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll) 
{
	//return CScrollView::OnScroll(nScrollCode, nPos, bDoScroll);
	//return m_edit_ctrl.OnScroll(nScrollCode, nPos, bDoScroll);
	return TRUE;
}

void COeditView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	m_edit_ctrl.HScroll(nSBCode);
}

void COeditView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	m_edit_ctrl.VScroll(nSBCode);
}

BOOL COeditView::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;
}

void COeditView::OnSplitRows() 
{
	m_edit_ctrl.SplitSelectedRows();	
}

void COeditView::OnJoinRows() 
{
	ReplaceText(_T("\n"), _T(""), FALSE);
}

void COeditView::OnDestroy() 
{
	CScrollView::OnDestroy();
	
	// Note: 2022/07/11
	// ここで以下の処理を実行すると、アプリケーションの終了時にPreTranslateMessageでエラーになる
//	if(CSearchDlgSingleton::IsVisible() && CSearchDlgSingleton::GetDlg().GetWnd() == this) {
//		CSearchDlgSingleton::GetDlg().ShowWindow(SW_HIDE);
//	}
//	if(CReplaceDlgSingleton::IsVisible() && CReplaceDlgSingleton::GetDlg().GetWnd() == this) {
//		CReplaceDlgSingleton::GetDlg().ShowWindow(SW_HIDE);
//	}
//	if(CSelectColorDlgSingleton::IsVisible() && CSelectColorDlgSingleton::GetDlg().GetWnd() == this) {
//		CSelectColorDlgSingleton::GetDlg().ShowWindow(SW_HIDE);
//	}
}

void COeditView::OnInputReturn() 
{
	m_edit_ctrl.InputChar(VK_RETURN);	
}

void COeditView::OnEditDeleteBeforeCursor() 
{
	m_edit_ctrl.DeleteBeforeCaret();	
}

void COeditView::OnInputBr() 
{
	m_edit_ctrl.Paste(_T("<br>\n"));
}

void COeditView::OnEditDeleteWordAfterCursor() 
{
	m_edit_ctrl.DeleteWordAfterCaret();
}

void COeditView::OnEditDeleteWordBeforeCursor() 
{
	m_edit_ctrl.DeleteWordBeforeCaret();
}

void COeditView::OnInvertBrackets() 
{
	m_edit_ctrl.InvertBrackets();
}

void COeditView::OnGoMatchBrackets() 
{
	m_edit_ctrl.GoMatchBrackets();
}

void COeditView::OnSpaceLeft() { m_edit_ctrl.SpaceLeft(FALSE); }
void COeditView::OnSpaceLeftExtend() { m_edit_ctrl.SpaceLeft(TRUE); }
void COeditView::OnSpaceRight() { m_edit_ctrl.SpaceRight(FALSE); }
void COeditView::OnSpaceRightExtend() { m_edit_ctrl.SpaceRight(TRUE); }

void COeditView::OnEditDelDupRows() 
{
	int replace_cnt;
	m_edit_ctrl.ReplaceText(_T("^(.*)$\\n(?:\\1$(\\n?))+"), _T("\\1\\2"),
		1, TRUE, TRUE, TRUE, TRUE, &replace_cnt, NULL);
}

void COeditView::OnUpdateEditDelDupRows(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_edit_ctrl.HaveSelectedMultiLine());
}

void COeditView::OnScmEval() 
{
	CCopyLineModeSetting l(m_edit_ctrl.GetEditData(), LINE_TYPE_LF);
	CString code = m_edit_ctrl.GetSelectedText();
	scm_eval_str(g_sc, code);
}

void COeditView::OnUpdateScmEval(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_edit_ctrl.HaveSelected());
}


void COeditView::OnFilter() 
{
	HideSearchDlgs();

	if(m_edit_ctrl.HaveSelected() && m_edit_ctrl.HaveSelectedMultiLine() == FALSE) {
		g_search_data.m_search_text = m_edit_ctrl.GetSelectedText();
	}

	CFilterDlgSingleton::GetDlg().ShowDialog(this, WM_SEARCH_TEXT, WM_FILTER_TEXT, &g_search_data);
}

void COeditView::HideSearchDlgs()
{
	if(CReplaceDlgSingleton::IsVisible()) {
		CReplaceDlgSingleton::GetDlg().ShowWindow(SW_HIDE);
	}
	if(CSearchDlgSingleton::IsVisible()) {
		CSearchDlgSingleton::GetDlg().ShowWindow(SW_HIDE);
	}
	if(CFilterDlgSingleton::IsVisible()) {
		CFilterDlgSingleton::GetDlg().ShowWindow(SW_HIDE);
	}
}

void COeditView::OnWordLeftStopWb() { m_edit_ctrl.WordLeftStopWB(FALSE); }
void COeditView::OnWordRightStopWb() { m_edit_ctrl.WordRightStopWB(FALSE); }
void COeditView::OnWordLeftStopWbExtend() { m_edit_ctrl.WordLeftStopWB(TRUE); }
void COeditView::OnWordRightStopWbExtend() { m_edit_ctrl.WordRightStopWB(TRUE); }
