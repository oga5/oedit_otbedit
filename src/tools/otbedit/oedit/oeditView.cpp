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

#include "owinutil.h"
#include "scm_macro.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static const char *THIS_FILE = __FILE__;
#endif

#define CLOSE_BUTTON_WIDTH		20
//#define CLOSE_BUTTON_ID			201
#define TAB_CTRL_ID				202

/////////////////////////////////////////////////////////////////////////////
// COeditView

IMPLEMENT_DYNCREATE(COeditView, CView)

BEGIN_MESSAGE_MAP(COeditView, CView)
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
	ON_WM_PAINT()
	ON_COMMAND(ID_NEXT_WINDOW, OnNextWindow)
	ON_COMMAND(ID_PREV_WINDOW, OnPrevWindow)
	ON_COMMAND(ID_TAB_MOVE_LEFT, OnTabMoveLeft)
	ON_COMMAND(ID_TAB_MOVE_RIGHT, OnTabMoveRight)
	ON_UPDATE_COMMAND_UI(ID_TAB_MOVE_LEFT, OnUpdateTabMoveLeft)
	ON_UPDATE_COMMAND_UI(ID_TAB_MOVE_RIGHT, OnUpdateTabMoveRight)
	ON_COMMAND(ID_TAB_MOVE_TOP, OnTabMoveTop)
	ON_COMMAND(ID_TAB_MOVE_LAST, OnTabMoveLast)
	ON_UPDATE_COMMAND_UI(ID_TAB_MOVE_LAST, OnUpdateTabMoveLast)
	ON_UPDATE_COMMAND_UI(ID_TAB_MOVE_TOP, OnUpdateTabMoveTop)
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
	ON_COMMAND(ID_EDIT_COPY_ROW, OnEditCopyRow)
	ON_COMMAND(ID_EDIT_CUT_ROW, OnEditCutRow)
	ON_COMMAND(ID_EDIT_PASTE_ROW, OnEditPasteRow)
	ON_COMMAND(ID_EDIT_PASTE_ROW_UP, OnEditPasteRowUp)
	ON_COMMAND(ID_SPLIT_START, OnSplitStart)
	ON_COMMAND(ID_SPLIT_START_EXTEND, OnSplitStartExtend)
	ON_COMMAND(ID_SPLIT_END, OnSplitEnd)
	ON_COMMAND(ID_SPLIT_END_EXTEND, OnSplitEndExtend)
	ON_COMMAND(ID_SELECT_WORD, OnSelectWord)
	ON_COMMAND(ID_SORT_TAB, OnSortTab)
	ON_UPDATE_COMMAND_UI(ID_SORT_TAB, OnUpdateSortTab)
	ON_COMMAND(ID_VIEW_CLICKABLE_URL_01, OnViewClickableUrl01)
	ON_COMMAND(ID_VIEW_CLICKABLE_URL_01_NEW_WINDOW, OnViewClickableUrl01NewWindow)
	ON_COMMAND(ID_VIEW_CLICKABLE_URL_03, OnViewClickableUrl03)
	ON_COMMAND(ID_VIEW_CLICKABLE_URL_03_NEW_WINDOW, OnViewClickableUrl03NewWindow)
	ON_COMMAND(ID_EDIT_JOIN_ROW, OnEditJoinRow)
	ON_COMMAND(ID_REDRAW, OnRedraw)
	ON_COMMAND(ID_SEARCH_SELECTED, OnSearchSelected)
	ON_COMMAND(ID_TAG_JUMP, OnTagJump)
	ON_COMMAND(ID_DELETE_SPACE_END, OnDeleteSpaceEnd)
	ON_COMMAND(ID_INSERT_DATE_TIME, OnInsertDateTime)
	ON_COMMAND(ID_HANKAKU_TO_ZENKAKU, OnHankakuToZenkaku)
	ON_COMMAND(ID_HANKAKU_TO_ZENKAKU_ALPHA, OnHankakuToZenkakuAlpha)
	ON_COMMAND(ID_HANKAKU_TO_ZENKAKU_KATA, OnHankakuToZenkakuKata)
	ON_COMMAND(ID_ZENKAKU_TO_HANKAKU, OnZenkakuToHankaku)
	ON_COMMAND(ID_ZENKAKU_TO_HANKAKU_ALPHA, OnZenkakuToHankakuAlpha)
	ON_COMMAND(ID_ZENKAKU_TO_HANKAKU_KATA, OnZenkakuToHankakuKata)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_SPLIT_ROWS, OnSplitRows)
	ON_COMMAND(ID_JOIN_ROWS, OnJoinRows)
	ON_COMMAND(ID_INPUT_RETURN, OnInputReturn)
	ON_COMMAND(ID_FILTER, OnFilter)
	ON_UPDATE_COMMAND_UI(ID_NEXT_WINDOW, OnUpdateNextWindow)
	ON_UPDATE_COMMAND_UI(ID_PREV_WINDOW, OnUpdatePrevWindow)
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
	ON_COMMAND(ID_SEARCH_SELECTED_PREV, OnSearchSelectedPrev)
	ON_COMMAND(ID_CHAR_LEFT_EXTEND_BOX, OnCharLeftExtendBox)
	ON_COMMAND(ID_CHAR_RIGHT_EXTEND_BOX, OnCharRightExtendBox)
	ON_COMMAND(ID_LINE_DOWN_EXTEND_BOX, OnLineDownExtendBox)
	ON_COMMAND(ID_LINE_UP_EXTEND_BOX, OnLineUpExtendBox)
	ON_COMMAND(ID_SCM_EVAL, OnScmEval)
	ON_UPDATE_COMMAND_UI(ID_SCM_EVAL, OnUpdateScmEval)
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_BOX_PASTE, OnBoxPaste)
	ON_UPDATE_COMMAND_UI(ID_BOX_PASTE, OnUpdateBoxPaste)
	ON_COMMAND(ID_TAB_CLOSE_LEFT, OnTabCloseLeft)
	ON_UPDATE_COMMAND_UI(ID_TAB_CLOSE_LEFT, OnUpdateTabCloseLeft)
	ON_COMMAND(ID_TAB_CLOSE_OTHER, OnTabCloseOther)
	ON_UPDATE_COMMAND_UI(ID_TAB_CLOSE_OTHER, OnUpdateTabCloseOther)
	ON_COMMAND(ID_TAB_CLOSE_RIGHT, OnTabCloseRight)
	ON_UPDATE_COMMAND_UI(ID_TAB_CLOSE_RIGHT, OnUpdateTabCloseRight)
	ON_COMMAND(ID_VIEW_BROWSER_02, OnViewBrowser02)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BROWSER_02, OnUpdateViewBrowser02)
	ON_COMMAND(ID_VIEW_CLICKABLE_URL_02, OnViewClickableUrl02)
	ON_COMMAND(ID_WORD_LEFT_STOP_WB, OnWordLeftStopWb)
	ON_COMMAND(ID_WORD_LEFT_STOP_WB_EXTEND, OnWordLeftStopWbExtend)
	ON_COMMAND(ID_WORD_RIGHT_STOP_WB, OnWordRightStopWb)
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
	m_tab_height = 20;
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
	CView::AssertValid();
}

void COeditView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
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
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetWindowText(OTBEDIT_VIEW_WINDOWNAME);

	DWORD tab_ext_opt;
	if(g_option.tab_multi_row) {
		tab_ext_opt = TCS_MULTILINE;
	} else {
		tab_ext_opt = TCS_SINGLELINE;
	}
	m_tab_ctrl.Create(WS_VISIBLE | WS_CHILD | TCS_FOCUSNEVER | TCS_OWNERDRAWFIXED | tab_ext_opt,
		CRect(0, 0, lpCreateStruct->cx, m_tab_height), this, TAB_CTRL_ID);

	m_close_btn.Create(this, ID_FILE_CLOSE);

	GetDocument()->GetEditCtrl()->SetEditData(GetDocument()->GetEditData());
	GetDocument()->GetEditCtrl()->Create(NULL, NULL,
		WS_CHILD | WS_VSCROLL, 
		CRect(0, m_tab_height, lpCreateStruct->cx, lpCreateStruct->cy - m_tab_height), this, NULL);

	SetFont();

	m_search_dlg.Create(IDD_SEARCH_DLG, this);
	m_replace_dlg.Create(IDD_REPLACE_DLG, this);
	m_filter_dlg.Create(IDD_FILTER_DLG, this);
	m_color_dlg.Create(IDD_SELECT_COLOR_DLG, this);

	return 0;
}

void COeditView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	RecalcLayout();
}

void COeditView::OnSetFocus(CWnd* pOldWnd) 
{
	CView::OnSetFocus(pOldWnd);
	
	GetDocument()->GetEditCtrl()->SetFocus();	
}

void COeditView::OnEditCopy() 
{
	GetDocument()->GetEditCtrl()->Copy();
}

void COeditView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetEditCtrl()->CanCopy());
}

void COeditView::OnEditCut() 
{
	GetDocument()->GetEditCtrl()->Cut();
}

void COeditView::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetEditCtrl()->CanCut());
}

void COeditView::OnEditPaste() 
{
	GetDocument()->GetEditCtrl()->Paste();
}

void COeditView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetEditCtrl()->CanPaste());
}

void COeditView::OnBoxPaste() 
{
	GetDocument()->GetEditCtrl()->Paste(FALSE, TRUE);
}

void COeditView::OnUpdateBoxPaste(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetEditCtrl()->CanPaste());
}

void COeditView::OnEditRedo() 
{
	GetDocument()->GetEditCtrl()->Redo();
}

void COeditView::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetEditCtrl()->CanRedo());
}

void COeditView::OnEditUndo() 
{
	GetDocument()->GetEditCtrl()->Undo();
}

void COeditView::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetEditCtrl()->CanUndo());
}

void COeditView::OnSelectAll() 
{
	GetDocument()->GetEditCtrl()->SelectAll();
}

CString COeditView::GetTabName(CString path_name)
{
	TCHAR	*file_name = path_name.GetBuffer(0);
	TCHAR	*p;
	INT_PTR	last_sepa_pos = 0;
	INT_PTR	pre_last_sepa_pos = 0;

	/* '\'はSJISの2バイト目でも使われるので，文字列の前から調べていく */
	for(p = file_name; p[0] != '\0'; p++) {
		/* 2バイト文字はスキップ */
		if(is_lead_byte(*p) == 1) {
			p++;
			continue;
		}

		if(p[0] == '\\') {
			pre_last_sepa_pos = last_sepa_pos;
			last_sepa_pos = p - file_name;
		}
	}

	CString tab_name;
	TCHAR *name = tab_name.GetBuffer(_MAX_PATH);

	if(last_sepa_pos == 0) {
		_tcscpy(name, file_name);
	} else if(pre_last_sepa_pos == 0 || g_option.tab_title_is_path_name == FALSE) {
		_stprintf(name, _T("%s"), file_name + last_sepa_pos + 1);
	} else {
		_stprintf(name, _T("%s"), file_name + pre_last_sepa_pos + 1);
	}

	tab_name.ReleaseBuffer();
	return tab_name;
}

void COeditView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	switch(lHint) {
	case UPD_PRE_CHANGE_EDIT_DATA:
		PreChangeEditData();
		break;
	case UPD_POST_CHANGE_EDIT_DATA:
		{
			INT_PTR int_val = reinterpret_cast<INT_PTR>(pHint);
			PostChangeEditData((int)int_val);
		}
		break;
	case UPD_NEW_DOCUMENT:
		{
			CString file_name = GetTabName(GetDocument()->GetPathName());
			
			INT_PTR int_val = reinterpret_cast<INT_PTR>(pHint);
			int idx;
			idx = m_tab_ctrl.InsertItem((int)int_val, file_name, (int)int_val);
			m_tab_ctrl.SetCurSel(idx);

			CRect	win_rect;
			GetClientRect(&win_rect);

			GetDocument()->GetEditCtrl()->SetEditData(GetDocument()->GetEditData());
			GetDocument()->GetEditCtrl()->Create(NULL, NULL,
				WS_VISIBLE | WS_CHILD | WS_VSCROLL, 
				CRect(0, m_tab_height, win_rect.Width(), win_rect.Height()), this, NULL);

			GetDocument()->GetEditCtrl()->SetCodeAssistWnd_InvertSelectText(TRUE);

			GetDocument()->GetEditCtrl()->SetFont(&g_font);
			GetDocument()->GetEditCtrl()->SetLineMode(g_option.text_editor.line_mode, g_option.text_editor.line_len, FALSE);
			SetEditorOption();

			CheckBtn();
			RecalcLayout();
		}
//		GetDocument()->GetEditCtrl()->SetEditData(GetDocument()->GetEditData());
//		GetDocument()->GetEditCtrl()->Redraw();
		break;
	case UPD_CLOSE_DOCUMENT:
		{
			int		tab_idx = m_tab_ctrl.GetCurSel();
			if(tab_idx != -1) m_tab_ctrl.DeleteItem((int)tab_idx);
			Invalidate();

			CheckBtn();
			RecalcLayout();
		}
		break;
	case UPD_CHANGE_MODIFIED:
		{
			CString file_name = GetTabName(GetDocument()->GetPathName());
			if(GetDocument()->IsModified()) {
				file_name += _T(" *");
			}
			int		tab_idx = m_tab_ctrl.GetCurSel();
			if(tab_idx != -1) {
				TCHAR	title[1024] = _T("");
				TC_ITEM	get_item;
				get_item.mask = TCIF_TEXT;
				get_item.pszText = title;
				get_item.cchTextMax = sizeof(title);
				m_tab_ctrl.GetItem(tab_idx, &get_item);
				
				if(file_name.Compare(title) != 0) {
					m_tab_ctrl.SetItemText(tab_idx, file_name);
					RecalcLayout();
				}
			}
		}
		break;
	case UPD_REDRAW:
		GetDocument()->GetEditCtrl()->Redraw();
		break;
	case UPD_INVALIDATE:
		GetDocument()->GetEditCtrl()->Invalidate();
		break;
	case UPD_EDIT_MODE:
		// FIXME: 起動時にエラーになるのを回避
		if(GetDocument()->GetEditCtrl()->GetSafeHwnd() != NULL) {
			//GetDocument()->GetEditCtrl()->CheckCommentRow();
			SetEditorOption();
			GetDocument()->SaveEditMode();
		}
		break;
	case UPD_EDITOR_OPTION:
		SetEditorOption();
		GetDocument()->SetDocTitle();
		break;
	case UPD_FONT:
		SetFont();
		break;
	case UPD_TAB_MODE:
		ChangeTabMode();
		break;
	case UPD_WINDOW_MOVED:
		GetDocument()->GetEditCtrl()->SetCaret(FALSE, 1);
		break;
	case UPD_SAVE_ALL:
		SaveAllDocument();
		break;
	}	
}

void COeditView::SetFont()
{
	CFont *font = &g_font;
	CView::SetFont(font);
	m_tab_ctrl.SetFont(&g_tab_font);
	GetDocument()->GetEditCtrl()->SetFont(font);

	TEXTMETRIC tm;
	CDC *pdc = GetDC();
	pdc->SetMapMode(MM_TEXT);
	CFont *pOldFont = pdc->SelectObject(font);
	pdc->GetTextMetrics(&tm);
	pdc->SelectObject(pOldFont);
	ReleaseDC(pdc);

	RecalcLayout();
}

int COeditView::GetTabHeight()
{
	if(g_option.hide_tab_bar_at_one && m_tab_ctrl.GetItemCount() == 1 ||
		m_tab_ctrl.GetItemCount() == 0) {
		return 0;
	}

	CRect item_rect;
	m_tab_ctrl.GetItemRect(0, item_rect);

	return item_rect.Height() * m_tab_ctrl.GetRowCount() + 4;
}

void COeditView::RecalcLayout()
{
	CRect	rect;
	GetClientRect(rect);

	// m_tab_heightが0のとき，タブの高さが正しく計算できないのを回避
	if(m_tab_height == 0) m_tab_height = 20;

	int tab_width = rect.Width();
	if(!g_option.close_btn_on_tab) tab_width -= CLOSE_BUTTON_WIDTH;

	// 横幅を調整する
	m_tab_ctrl.MoveWindow(0, 0, tab_width, m_tab_height);
	// タブの高さを計算
	m_tab_height = GetTabHeight();
	// タブの高さを反映する
	m_tab_ctrl.MoveWindow(0, 0, tab_width, m_tab_height);

	GetDocument()->GetEditCtrl()->MoveWindow(0, m_tab_height, rect.Width(), rect.Height() - m_tab_height);

	RECT	btn_rect;
	CalcCloseButtonRect(&btn_rect);
	m_close_btn.MoveWindow(&btn_rect);

	// FIXME: タブに閉じるボタンを表示中は、ボタンを隠す
	if(m_tab_height == 0 || g_option.close_btn_on_tab) {
		m_close_btn.ShowWindow(SW_HIDE);
	} else {
		m_close_btn.ShowWindow(SW_SHOW);
	}

	Invalidate();
}

void COeditView::SetEditorOption()
{
	int i;

	GetDocument()->SetEditDataOption();

	if(GetDocument()->GetEditCtrl()->GetSafeHwnd() == NULL) return;

	QWORD		option = ECS_INVERT_BRACKETS | ECS_DISABLE_KEY_DOWN | ECS_V_SCROLL_EX | 
		ECS_IME_CARET_COLOR | ECS_WORD_SELECT_MODE;

	if(GetDocument()->GetEditCtrl()->GetRowSpace() != g_option.text_editor.row_space ||
		GetDocument()->GetEditCtrl()->GetCharSpaceSetting() != g_option.text_editor.char_space ||
		GetDocument()->GetEditCtrl()->GetTopSpace() != g_option.text_editor.top_space ||
		GetDocument()->GetEditCtrl()->GetLeftSpace() != g_option.text_editor.left_space) {
		GetDocument()->GetEditCtrl()->SetSpaces(g_option.text_editor.row_space, 
			g_option.text_editor.char_space,
			g_option.text_editor.top_space,
			g_option.text_editor.left_space);
	}

	if(GetDocument()->GetEditCtrl()->GetLineMode() == EC_LINE_MODE_LEN) {
		GetDocument()->GetEditCtrl()->SetLineMode(EC_LINE_MODE_LEN, g_option.text_editor.line_len, FALSE);
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
		option |= ECS_SHOW_COL_NUM;
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
	if(g_option.text_editor.text_drop_edit) {
		option |= ECS_TEXT_DROP_EDIT;
	}
	if(g_option.text_editor.mouse_select_copy) {
		option |= ECS_DRAG_SELECT_COPY;
	}
	if(g_option.text_editor.row_copy_at_no_sel) {
		option |= ECS_ROW_COPY_AT_NO_SEL;
	}
	if(g_option.text_editor.clear_after_copy) {
		option |= ECS_CLEAR_AFTER_COPY;
	}

	if(g_option.search_loop_msg == TRUE) {
		option |= ECS_SEARCH_LOOP_MSG;
	}

	if(g_str_token.HaveComment() == FALSE) {
		option |= ECS_NO_COMMENT_CHECK;
	}
	if(g_str_token.GetTagMode() == TRUE) {
		option |= ECS_HTML_MODE;
	}

	GetDocument()->GetEditCtrl()->SetBracketsBlinkTime(g_option.text_editor.brackets_blink_time);
	GetDocument()->GetEditCtrl()->SetUseKeywordWindow(g_option.text_editor.use_keyword_window);
	GetDocument()->GetEditCtrl()->SetWheelScrollRate(120.0 / g_option.text_editor.wheel_scroll_row);

	GetDocument()->GetEditCtrl()->SetExStyle2(option);

	for(i = 0; i < EDIT_CTRL_COLOR_CNT; i++) {
		GetDocument()->GetEditCtrl()->SetColor(i, g_option.text_editor.color[i]);
	}
	GetDocument()->GetEditCtrl()->ResetCaret();

	GetDocument()->GetEditCtrl()->SetCodeAssistWndColor(GRID_BG_COLOR, g_option.text_editor.color[BG_COLOR]);
	GetDocument()->GetEditCtrl()->SetCodeAssistWndColor(GRID_TEXT_COLOR, g_option.text_editor.color[TEXT_COLOR]);
	GetDocument()->GetEditCtrl()->SetCodeAssistWndColor(GRID_LINE_COLOR, g_option.text_editor.color[PEN_COLOR]);
	GetDocument()->GetEditCtrl()->SetCodeAssistWndColor(GRID_SELECT_COLOR, g_option.text_editor.color[SELECTED_COLOR]);

	GetDocument()->GetEditCtrl()->Redraw();

	{
		BOOL need_set_color = FALSE;
		for(i = 0; i < FILETAB_CTRL_COLOR_CNT; i++) {
			if(g_option.file_tab.color[i] != m_tab_ctrl.GetColor(i)) {
				need_set_color = TRUE;
				break;
			}
		}
		if(need_set_color) {
			for(i = 0; i < FILETAB_CTRL_COLOR_CNT; i++) {
				m_tab_ctrl.SetColor(i, g_option.file_tab.color[i]);
			}
			m_tab_ctrl.RedrawWindow();
		}

		DWORD tab_style = 0;
		if(g_option.tab_drag_move) tab_style |= FTC_TAB_DRAG_MOVE;
		if(g_option.close_btn_on_tab) tab_style |= FTC_CLOSE_BTN_ON_ALL_TAB;
		if(g_option.show_tab_tooltip) tab_style |= FTC_SHOW_TAB_TOOLTIP;
		m_tab_ctrl.SetExStyle(tab_style);
	}

	RecalcLayout();
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
			scm_call_event_handler(g_sc, SCM_EVENT_ON_CURSOR_MOVED);
		}
		break;
	case EC_WM_LBUTTONDBLCLK:
		if(g_option.enable_tag_jump) OnTagJump();
		break;
	case WM_SEARCH_TEXT:
		{
			BOOL b_looped;
			GetParentFrame()->SetMessageText(_T(""));
			int ret_v = GetDocument()->GetEditCtrl()->SearchText2(
				m_search_data.m_search_text.GetBuffer(0),
				m_search_data.m_dir, m_search_data.m_distinct_lwr_upr, m_search_data.m_distinct_width_ascii,
				m_search_data.m_regexp, &b_looped);
			SearchMsg(ret_v, m_search_data.m_dir, b_looped);
			return ret_v;
		}
		break;
	case WM_REPLACE_TEXT:
		{
			int replace_cnt;
			BOOL b_looped;
			GetParentFrame()->SetMessageText(_T(""));
			int ret_v = GetDocument()->GetEditCtrl()->ReplaceText2(
				m_search_data.m_search_text.GetBuffer(0),
				m_search_data.m_replace_text.GetBuffer(0),
				m_search_data.m_dir, m_search_data.m_distinct_lwr_upr, m_search_data.m_distinct_width_ascii,
				m_search_data.m_regexp, m_search_data.m_all, 
				m_search_data.m_replace_selected_area, &replace_cnt, &b_looped);
			GetDocument()->CheckModified();

			if(m_search_data.m_all) {
				CString msg;
				msg.Format(_T("%d件置換しました"), replace_cnt);
				GetParentFrame()->SetMessageText(msg);
			} else {
				SearchMsg(ret_v, m_search_data.m_dir, b_looped);
			}
		}
		return 0;
		break;
	case WM_FILTER_TEXT:
		{
			int replace_cnt;
			int ret_v = GetDocument()->GetEditCtrl()->FilterText2(
				m_search_data.m_search_text.GetBuffer(0),
				m_search_data.m_distinct_lwr_upr,
				m_search_data.m_distinct_width_ascii,
				m_search_data.m_regexp, 
				GetDocument()->GetEditCtrl()->HaveSelectedMultiLine(),
				&replace_cnt, m_search_data.m_dir);
			GetDocument()->CheckModified();

			if(ret_v == 0) {
				CString msg;
				msg.Format(_T("%d行削除しました"), replace_cnt);
				GetParentFrame()->SetMessageText(msg);
			}
		}
		return 0;
		break;
	case WM_PASTE_COLOR:
		GetDocument()->GetEditCtrl()->Paste((TCHAR *)wParam);
		return 0;
		break;
	case EC_WM_CHANGE_SELECTED_TEXT:
		scm_call_event_handler(g_sc, SCM_EVENT_ON_SELECTION_CHANGED);
		break;
	}

	return CView::WindowProc(message, wParam, lParam);
}

void COeditView::OnSearch() 
{
	if(m_replace_dlg.IsWindowVisible()) m_replace_dlg.ShowWindow(SW_HIDE);
	if(m_filter_dlg.IsWindowVisible()) m_filter_dlg.ShowWindow(SW_HIDE);

	if(m_search_dlg.IsWindowVisible()) {
		m_search_dlg.SetSearchText(GetDocument()->GetEditCtrl()->GetSelectedText());
		m_search_dlg.m_combo_search_text.SetFocus();
		return;
	}

	if(GetDocument()->GetEditCtrl()->HaveSelected() && GetDocument()->GetEditCtrl()->HaveSelectedMultiLine() == FALSE) {
		m_search_data.m_search_text = GetDocument()->GetEditCtrl()->GetSelectedText();
	}

	m_search_dlg.ShowDialog(this, WM_SEARCH_TEXT, &m_search_data, g_option.search_dlg_close_at_found);
}

void COeditView::OnFilter() 
{
	if(m_search_dlg.IsWindowVisible()) m_search_dlg.ShowWindow(SW_HIDE);
	if(m_replace_dlg.IsWindowVisible()) m_replace_dlg.ShowWindow(SW_HIDE);
	
	if(m_filter_dlg.IsWindowVisible()) {
		m_filter_dlg.m_combo_search_text.SetFocus();
		return;
	}

	if(GetDocument()->GetEditCtrl()->HaveSelected() && GetDocument()->GetEditCtrl()->HaveSelectedMultiLine() == FALSE) {
		m_search_data.m_search_text = GetDocument()->GetEditCtrl()->GetSelectedText();
	}

	m_filter_dlg.ShowDialog(this, WM_SEARCH_TEXT, WM_FILTER_TEXT, &m_search_data);
}

void COeditView::SetSearchText(const TCHAR *search_text, BOOL distinct_lwr_upr, BOOL distinct_width_ascii)
{
	m_search_data.m_dir = 1;
	m_search_data.m_distinct_lwr_upr = distinct_lwr_upr;
	m_search_data.m_distinct_width_ascii = distinct_width_ascii;
	m_search_data.m_regexp = TRUE;
	m_search_data.m_search_text = search_text;
	m_search_data.m_all = FALSE;
	m_search_data.m_replace_text = "";
	m_search_data.m_replace_selected_area = FALSE;
}

void COeditView::SetSearchSelected()
{
	if(GetDocument()->GetEditCtrl()->HaveSelected()) {
		m_search_data.m_dir = 1;
		m_search_data.m_distinct_lwr_upr = FALSE;
		m_search_data.m_distinct_width_ascii = FALSE;
		m_search_data.m_regexp = FALSE;
		m_search_data.m_search_text = GetDocument()->GetEditCtrl()->GetSelectedText();
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
	int ret_v = GetDocument()->GetEditCtrl()->SearchNext2(
		m_search_data.m_search_text.GetBuffer(0),
		m_search_data.m_distinct_lwr_upr, m_search_data.m_distinct_width_ascii, m_search_data.m_regexp, &b_looped);
	SearchMsg(ret_v, 1, b_looped);
}

void COeditView::OnSearchPrev() 
{
	BOOL b_looped;
	GetParentFrame()->SetMessageText(_T(""));
	int ret_v = GetDocument()->GetEditCtrl()->SearchPrev2(
		m_search_data.m_search_text.GetBuffer(0),
		m_search_data.m_distinct_lwr_upr, m_search_data.m_distinct_width_ascii, m_search_data.m_regexp, &b_looped);
	SearchMsg(ret_v, -1, b_looped);
}

void COeditView::OnReplace() 
{
	if(m_search_dlg.IsWindowVisible()) m_search_dlg.ShowWindow(SW_HIDE);
	if(m_filter_dlg.IsWindowVisible()) m_filter_dlg.ShowWindow(SW_HIDE);

	if(m_replace_dlg.IsWindowVisible()) {
		m_replace_dlg.m_combo_search_text.SetFocus();
		return;
	}

	if(GetDocument()->GetEditCtrl()->HaveSelected() && GetDocument()->GetEditCtrl()->HaveSelectedMultiLine() == FALSE) {
		m_search_data.m_search_text = GetDocument()->GetEditCtrl()->GetSelectedText();
	}

	m_search_data.m_replace_selected_area = GetDocument()->GetEditCtrl()->HaveSelectedMultiLine();

	m_replace_dlg.ShowDialog(this, WM_SEARCH_TEXT, WM_REPLACE_TEXT, &m_search_data);
}

void COeditView::OnUpdateSelected(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetEditCtrl()->HaveSelected());
}

void COeditView::OnUpdateCommentEdit(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(g_str_token.HaveRowComment());
}

void COeditView::OnUpdateMultiLineEdit(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetEditCtrl()->HaveSelectedMultiLine());
}

void COeditView::OnEditIndent() 
{
	GetDocument()->GetEditCtrl()->InsertTab(FALSE);
}

void COeditView::OnEditIndentRev() 
{
	GetDocument()->GetEditCtrl()->InsertTab(TRUE);
}

void COeditView::OnEditIndentSpace() 
{
	GetDocument()->GetEditCtrl()->IndentSpace(FALSE);
}

void COeditView::OnEditIndentSpaceRev() 
{
	GetDocument()->GetEditCtrl()->IndentSpace(TRUE);
}

void COeditView::OnEditReverseRows() 
{
	GetDocument()->GetEditCtrl()->ReverseSelectedRows();
}

void COeditView::OnEditSortRowsAsc() 
{
	GetDocument()->GetEditCtrl()->SortSelectedRows(FALSE);
}

void COeditView::OnEditSortRowsDesc() 
{
	GetDocument()->GetEditCtrl()->SortSelectedRows(TRUE);
}

void COeditView::OnToLower() 
{
	GetDocument()->GetEditCtrl()->ToLower();
}

void COeditView::OnToUpper() 
{
	GetDocument()->GetEditCtrl()->ToUpper();
}

void COeditView::OnFilePrint() 
{
	CWaitCursor	wait_cursor;
	CPrintDlg	dlg;
	TCHAR		msg_buf[1024];

	dlg.m_line_mode = g_option.text_editor.line_mode;
	dlg.m_line_len = g_option.text_editor.line_len;
	dlg.m_row_num_digit = GetDocument()->GetEditCtrl()->GetRowNumDigit();

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
		title = "";
	}

	int line_len = -1;
	if(g_option.text_editor.line_mode == EC_LINE_MODE_LEN) line_len = g_option.text_editor.line_len;
	
	CRect	space(dlg.m_space_left, dlg.m_space_top, dlg.m_space_right, dlg.m_space_bottom);

	CEditData *edit_data = GetDocument()->GetEditData();
	if(dlg.m_print_selected_area) {
		if(!GetDocument()->GetEditCtrl()->HaveSelected()) {
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
	BOOL haveSelected = GetDocument()->GetEditCtrl()->HaveSelected();
	
	if(bInfrateSelectedArea) GetDocument()->GetEditCtrl()->InfrateSelectedArea();

	GetDocument()->GetEditCtrl()->ReplaceText(search_text, replace_text, 1, 
		TRUE, TRUE, TRUE, TRUE, &replace_cnt, NULL);

	if(!haveSelected) {
		GetDocument()->GetEditCtrl()->ClearSelected();
		if(replace_cnt == 0) GetDocument()->GetEditCtrl()->MoveCaretPos(cur_pt.y, cur_pt.x);
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
	GetDocument()->GetEditCtrl()->SetLineMode(EC_LINE_MODE_LEN, g_option.text_editor.line_len);
	g_option.text_editor.line_mode = EC_LINE_MODE_LEN;
}

void COeditView::OnLineModeNormal() 
{
	GetDocument()->GetEditCtrl()->SetLineMode(EC_LINE_MODE_NORMAL);
	g_option.text_editor.line_mode = EC_LINE_MODE_NORMAL;
}

void COeditView::OnLineModeRight() 
{
	GetDocument()->GetEditCtrl()->SetLineMode(EC_LINE_MODE_RIGHT);
	g_option.text_editor.line_mode = EC_LINE_MODE_RIGHT;
}

void COeditView::OnUpdateLineModeLen(CCmdUI* pCmdUI) 
{
	if(GetDocument()->GetEditCtrl()->GetLineMode() == EC_LINE_MODE_LEN) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
	}
}

void COeditView::OnUpdateLineModeNormal(CCmdUI* pCmdUI) 
{
	if(GetDocument()->GetEditCtrl()->GetLineMode() == EC_LINE_MODE_NORMAL) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
	}
}

void COeditView::OnUpdateLineModeRight(CCmdUI* pCmdUI) 
{
	if(GetDocument()->GetEditCtrl()->GetLineMode() == EC_LINE_MODE_RIGHT) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
	}
}

void COeditView::OnEditDelete() 
{
	GetDocument()->GetEditCtrl()->DeleteKey();
}

void COeditView::OnColorInput() 
{
	m_color_dlg.ShowDialog(this, WM_PASTE_COLOR);
}

void COeditView::OnUpdateColorInput(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(TRUE);
}

void COeditView::OnClearSearchText() 
{
	GetDocument()->GetEditCtrl()->ClearSearchText();
}

BOOL COeditView::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct) 
{
	switch(pCopyDataStruct->dwData) {
	case WM_COPY_DATA_GET_FILE_NAME:
		{
			TCHAR	*file_name = (TCHAR *)pCopyDataStruct->lpData;
			return GetDocument()->FileIsOpen(file_name);
		}
	case WM_COPY_DATA_OPEN_FILE:
		{
			TCHAR	*file_name = (TCHAR *)pCopyDataStruct->lpData;
			//return GetDocument()->OnOpenDocument(file_name);
			CDocument *pdoc = AfxGetApp()->OpenDocumentFile(file_name);
			if(pdoc == NULL) return FALSE;
			return TRUE;
		}
	case WM_COPY_DATA_SET_SEARCH_DATA:
		{
			struct _set_search_data_st_otbedit *search_data = (struct _set_search_data_st_otbedit *)pCopyDataStruct->lpData;

			if(GetDocument()->ChangeDocData(search_data->file_name) == FALSE) return FALSE;

			GetDocument()->GetEditCtrl()->SaveSearchData2(search_data->search_text,
				search_data->distinct_lwr_upr, search_data->distinct_width_ascii, search_data->regexp);
			GetDocument()->GetEditData()->set_cur(search_data->row, 0);
			GetDocument()->GetEditCtrl()->SetCaret(TRUE, 1);

			m_search_data.m_search_text = search_data->search_text;
			m_search_data.m_distinct_lwr_upr = search_data->distinct_lwr_upr;
			m_search_data.m_distinct_width_ascii = search_data->distinct_width_ascii;
			m_search_data.m_regexp = search_data->regexp;

			if(AfxGetMainWnd()->IsIconic()) {
				AfxGetMainWnd()->ShowWindow(SW_SHOWNORMAL);
			}

			return TRUE;
		}
	case WM_COPY_DATA_ACTIVE_FILE:
		{
			TCHAR	*file_name = (TCHAR *)pCopyDataStruct->lpData;
			if(GetDocument()->ChangeDocData(file_name) == FALSE) return FALSE;
			SetForegroundWindow();
			return TRUE;
		}
	case WM_COPY_DATA_SET_OPTION:
		{
			struct _set_option_st *option_data = (struct _set_option_st *)pCopyDataStruct->lpData;

			if(GetDocument()->ChangeDocData(option_data->file_name) == FALSE) return FALSE;

			if(option_data->row >= 0 && option_data->row < GetDocument()->GetEditData()->get_row_cnt()) {
				GetDocument()->GetEditData()->set_cur(option_data->row, 0);
				GetDocument()->GetEditCtrl()->SetCaret(TRUE, 1);
			}
			if(_tcslen(option_data->mode) > 0) {
				((COeditApp *)AfxGetApp)->SetEditMode(g_lang_setting_list.GetSettingIdx(option_data->mode));
				GetDocument()->UpdateAllViews(NULL, UPD_EDIT_MODE);
				AfxGetApp()->OnIdle(0);
			}
			SetFocus();

			return TRUE;
		}
	}

	return CView::OnCopyData(pWnd, pCopyDataStruct);
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

int COeditView::TabHitTest(CPoint point)
{
	m_tab_ctrl.ScreenToClient(&point);

	TCHITTESTINFO tc_info;
	tc_info.pt = point;
	tc_info.flags = TCHT_ONITEM;

	return m_tab_ctrl.HitTest(&tc_info);
}

static CString LoadMenuString(UINT nID)
{
	CString menu_str;
	if(menu_str.LoadString(nID) == FALSE) return "";

	int n = menu_str.Find('\n', 0);
	if(n == -1) return menu_str;

	menu_str = menu_str.Right(menu_str.GetLength() - n - 1);
	return menu_str;
}

void COeditView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CMenu	menu;
	POINT	pt;
	CMenu	url_menu;

	pt = point;
	ScreenToClient(&pt);

	if(point.y >= 0 && pt.y < m_tab_height) {
		int tab_idx = TabHitTest(point);
		if(tab_idx == -1) return;
		if(tab_idx != m_tab_ctrl.GetCurSel()) {
			GetDocument()->ChangeDocData(GetDocIdx(tab_idx));
		}

		menu.LoadMenu(IDR_TAB_MENU);
	} else {
		menu.LoadMenu(IDR_EDIT_MENU);
		if(GetDocument()->GetEditCtrl()->HitTestClickableURL()) {
			url_menu.LoadMenu(IDR_URL_MENU);
			CString		menu_str;
			menu.GetSubMenu(0)->InsertMenu(10, MF_BYPOSITION | MF_SEPARATOR, 0, _T(""));
			menu_str = LoadMenuString(ID_DO_CLICKABLE_URL);
			menu.GetSubMenu(0)->InsertMenu(11, MF_BYPOSITION | MF_POPUP, (UINT_PTR)url_menu.m_hMenu, menu_str);
			menu_str = LoadMenuString(ID_COPY_CLICKABLE_URL);
			menu.GetSubMenu(0)->InsertMenu(12, MF_BYPOSITION, ID_COPY_CLICKABLE_URL, menu_str);
		}
	}

	UINT flags = TPM_LEFTALIGN | TPM_RIGHTBUTTON;

	// キーボード操作のときは、キャレット付近にメニューを表示する
	if(point.y < 0) {
		CPoint caret_pos = GetDocument()->GetEditCtrl()->GetCaretPos();
		GetDocument()->GetEditCtrl()->ClientToScreen(&caret_pos);

		point.x = caret_pos.x;

		if(caret_pos.y + GetPopupMenuHeight(menu.GetSubMenu(0)->GetSafeHmenu()) > ::GetSystemMetrics(SM_CYSCREEN)) {
			point.y = caret_pos.y;
			flags |= TPM_BOTTOMALIGN;
		} else {
			point.y = caret_pos.y + GetDocument()->GetEditCtrl()->GetRowHeight();
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

	GetDocument()->GetEditCtrl()->MoveCaretPos(dlg.m_line_no - 1, 0);
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

	CString str = GetDocument()->GetEditCtrl()->GetSelectedText();
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

void COeditView::OnCharLeft() { GetDocument()->GetEditCtrl()->CharLeft(FALSE); }
void COeditView::OnCharLeftExtend() { GetDocument()->GetEditCtrl()->CharLeft(TRUE); }
void COeditView::OnCharRight() { GetDocument()->GetEditCtrl()->CharRight(FALSE); }
void COeditView::OnCharRightExtend() { GetDocument()->GetEditCtrl()->CharRight(TRUE); }

void COeditView::OnLineUp() { GetDocument()->GetEditCtrl()->LineUp(FALSE); }
void COeditView::OnLineUpExtend() { GetDocument()->GetEditCtrl()->LineUp(TRUE); }
void COeditView::OnLineDown() { GetDocument()->GetEditCtrl()->LineDown(FALSE); }
void COeditView::OnLineDownExtend() { GetDocument()->GetEditCtrl()->LineDown(TRUE); }


void COeditView::OnCharLeftExtendBox() 
{
	if(!GetDocument()->GetEditCtrl()->HaveSelected()) GetDocument()->GetEditCtrl()->NextBoxSelect();
	GetDocument()->GetEditCtrl()->CharLeft(TRUE);
}

void COeditView::OnCharRightExtendBox() 
{
	if(!GetDocument()->GetEditCtrl()->HaveSelected()) GetDocument()->GetEditCtrl()->NextBoxSelect();
	GetDocument()->GetEditCtrl()->CharRight(TRUE);
}

void COeditView::OnLineDownExtendBox() 
{
	if(!GetDocument()->GetEditCtrl()->HaveSelected()) GetDocument()->GetEditCtrl()->NextBoxSelect();
	GetDocument()->GetEditCtrl()->LineDown(TRUE);
}

void COeditView::OnLineUpExtendBox() 
{
	if(!GetDocument()->GetEditCtrl()->HaveSelected()) GetDocument()->GetEditCtrl()->NextBoxSelect();
	GetDocument()->GetEditCtrl()->LineUp(TRUE);
}

void COeditView::OnWordLeft() { GetDocument()->GetEditCtrl()->WordLeft(FALSE); }
void COeditView::OnWordLeftExtend() { GetDocument()->GetEditCtrl()->WordLeft(TRUE); }
void COeditView::OnWordRight() { GetDocument()->GetEditCtrl()->WordRight(FALSE); }
void COeditView::OnWordRightExtend() { GetDocument()->GetEditCtrl()->WordRight(TRUE); }

void COeditView::OnLineStart() { GetDocument()->GetEditCtrl()->LineStart(FALSE); }
void COeditView::OnLineStartExtend() { GetDocument()->GetEditCtrl()->LineStart(TRUE); }
void COeditView::OnLineEnd() { GetDocument()->GetEditCtrl()->LineEnd(FALSE); }
void COeditView::OnLineEndExtend() { GetDocument()->GetEditCtrl()->LineEnd(TRUE); }

void COeditView::OnSplitStart() { GetDocument()->GetEditCtrl()->SplitStart(FALSE); }
void COeditView::OnSplitStartExtend() { GetDocument()->GetEditCtrl()->SplitStart(TRUE); }
void COeditView::OnSplitEnd() { GetDocument()->GetEditCtrl()->SplitEnd(FALSE); }
void COeditView::OnSplitEndExtend() { GetDocument()->GetEditCtrl()->SplitEnd(TRUE); }

void COeditView::OnDocumentStart() { GetDocument()->GetEditCtrl()->DocumentStart(FALSE); }
void COeditView::OnDocumentStartExtend() { GetDocument()->GetEditCtrl()->DocumentStart(TRUE); }
void COeditView::OnDocumentEnd() { GetDocument()->GetEditCtrl()->DocumentEnd(FALSE); }
void COeditView::OnDocumentEndExtend() { GetDocument()->GetEditCtrl()->DocumentEnd(TRUE); }

void COeditView::OnEditBackspace() { GetDocument()->GetEditCtrl()->BackSpace(); }
void COeditView::OnKeywordCompletion() { GetDocument()->GetEditCtrl()->KeywordCompletion(FALSE); }
void COeditView::OnKeywordCompletionRev() { GetDocument()->GetEditCtrl()->KeywordCompletion(TRUE); }

void COeditView::OnScrollUp() { GetDocument()->GetEditCtrl()->ScrollUp(); }
void COeditView::OnScrollDown() { GetDocument()->GetEditCtrl()->ScrollDown(); }
void COeditView::OnScrollPageUp() { GetDocument()->GetEditCtrl()->ScrollPageUp(); }
void COeditView::OnScrollPageDown() { GetDocument()->GetEditCtrl()->ScrollPageDown(); }

void COeditView::OnPageUp() { GetDocument()->GetEditCtrl()->PageUp(FALSE); }
void COeditView::OnPageUpExtend() { GetDocument()->GetEditCtrl()->PageUp(TRUE); }
void COeditView::OnPageDown() { GetDocument()->GetEditCtrl()->PageDown(FALSE); }
void COeditView::OnPageDownExtend() { GetDocument()->GetEditCtrl()->PageDown(TRUE); }

void COeditView::OnSelectRow() { GetDocument()->GetEditCtrl()->SelectRow(); }
void COeditView::OnSelectWord() { GetDocument()->GetEditCtrl()->SelectWord(); }

void COeditView::OnBoxSelect() { GetDocument()->GetEditCtrl()->NextBoxSelect(); }


DROPEFFECT COeditView::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	return CView::OnDragEnter(pDataObject, dwKeyState, point);
}

void COeditView::OnDragLeave() 
{
	CView::OnDragLeave();
}

DROPEFFECT COeditView::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	return CView::OnDragOver(pDataObject, dwKeyState, point);
}

BOOL COeditView::OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) 
{
	return CView::OnDrop(pDataObject, dropEffect, point);
}

void COeditView::OnSetBookMark() { GetDocument()->GetEditCtrl()->SetBookMark(); }
void COeditView::OnJumpBookMark() { GetDocument()->GetEditCtrl()->JumpBookMark(FALSE, FALSE); }
void COeditView::OnJumpBookMarkBack() { GetDocument()->GetEditCtrl()->JumpBookMark(TRUE, FALSE); }
void COeditView::OnJumpBookMarkExtend() { GetDocument()->GetEditCtrl()->JumpBookMark(FALSE, TRUE); }
void COeditView::OnJumpBookMarkBackExtend() { GetDocument()->GetEditCtrl()->JumpBookMark(TRUE, TRUE); }
void COeditView::OnClearAllBookMark() { GetDocument()->GetEditCtrl()->ClearAllBookMark(); }

void COeditView::OnToggleOverwrite() { GetDocument()->GetEditCtrl()->ToggleOverwrite(); }
void COeditView::OnUpdateOverwrite(CCmdUI* pCmdUI) { pCmdUI->Enable(GetDocument()->GetEditCtrl()->GetOverwrite()); }

void COeditView::PreChangeEditData()
{
}

void COeditView::PostChangeEditData(int idx)
{
	if(GetDocument()->GetEditCtrl()->GetSafeHwnd() != NULL) {
		GetDocument()->GetEditCtrl()->CheckCommentRow();
		SetEditorOption();

		CRect	win_rect;
		GetClientRect(&win_rect);

		GetDocument()->GetEditCtrl()->MoveWindow(0, m_tab_height,
				win_rect.Width(), win_rect.Height() - m_tab_height);

		GetDocument()->GetEditCtrl()->SetFocus();
	}

	int		tab_idx = SearchTabIdx(idx);
	if(tab_idx != -1) m_tab_ctrl.SetCurSel(tab_idx);

	Invalidate();
}

void COeditView::OnTabSelChanged()
{
	int	idx = m_tab_ctrl.GetCurSel();
	GetDocument()->ChangeDocData(GetDocIdx(idx));
}

void COeditView::OnTabToolTipName(NMHDR *p_nmhdr)
{
	struct file_tab_ctrl_nmhdr *h = (struct file_tab_ctrl_nmhdr *)p_nmhdr;

    CString file_name = GetDocument()->GetPathName_Idx(GetDocIdx(static_cast<int>(h->nmhdr.idFrom)));
	if(_tcschr(file_name.GetBuffer(0), L':') != NULL) {
		_stprintf(h->str_buf, _T("%.*s"), (int)sizeof(h->str_buf) - 1, file_name.GetBuffer(0));
	}
}

void COeditView::CloseTab(int tab_idx)
{
	int cur_idx = GetDocIdx(m_tab_ctrl.GetCurSel());

	GetDocument()->ChangeDocData(GetDocIdx(tab_idx));
	GetDocument()->DeleteDocData();

	GetDocument()->ChangeDocData(cur_idx);
}

BOOL COeditView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	switch(wParam) {
	case TAB_CTRL_ID:
		{
			NMHDR *p_nmhdr = (NMHDR *)lParam;
			switch(p_nmhdr->code) {
			case TCN_SELCHANGE:
				OnTabSelChanged();
				break;
			case TC_MCLICK:
				if(g_option.tab_close_at_mclick) {
                    CloseTab(static_cast<int>(p_nmhdr->idFrom));
				}
				break;
			case TC_MOVE_TAB:
				{
					int cur_pos = m_tab_ctrl.GetCurSel();
                    int new_pos = static_cast<int>(p_nmhdr->idFrom);
					MoveTab(cur_pos, new_pos);
				}
				break;
			case TC_CLOSE_BTN:
                CloseTab(static_cast<int>(p_nmhdr->idFrom));
				break;
			case TC_TOOL_TIP_NAME:
				OnTabToolTipName(p_nmhdr);
				break;
			}
		}
		break;
	}
	
	return CView::OnNotify(wParam, lParam, pResult);
}

int COeditView::SearchTabIdx(int doc_idx)
{
	int		i;
	for(i = 0; i < m_tab_ctrl.GetItemCount(); i++) {
		if(GetDocIdx(i) == doc_idx) return i;
	}
	return -1;
}

void COeditView::CalcCloseButtonRect(RECT *lpRect)
{
	CRect	cli_rect;
	GetClientRect(cli_rect);

	CRect	btn_rect;
	m_close_btn.GetClientRect(btn_rect);
	btn_rect.OffsetRect(cli_rect.Width() - btn_rect.Width(), 1);

	*lpRect = btn_rect;
}

void COeditView::OnPaint() 
{
	CPaintDC dc(this); // 描画用のデバイス コンテキスト
	
	// TODO: この位置にメッセージ ハンドラ用のコードを追加してください

	CRect	cli_rect;
	GetClientRect(cli_rect);

	CRect	rect(cli_rect.Width() - CLOSE_BUTTON_WIDTH, 0, cli_rect.Width(), m_tab_height);
	if(dc.RectVisible(rect)) {
		dc.FillSolidRect(rect, ::GetSysColor(COLOR_BTNFACE));
	}

	if(m_tab_ctrl.GetItemCount() == 0) {
		dc.FillSolidRect(cli_rect, ::GetSysColor(COLOR_BTNFACE));
	}
}

int COeditView::GetDocIdx(int tab_idx)
{
	TC_ITEM	item;
	item.mask = TCIF_PARAM;
	m_tab_ctrl.GetItem(tab_idx, &item);
    return static_cast<int>(item.lParam);
}

void COeditView::OnNextWindow() 
{
	ASSERT(m_tab_ctrl.GetItemCount() > 1);

	int idx = m_tab_ctrl.GetCurSel();
	if(idx == -1) return;

	idx++;
	if(idx == m_tab_ctrl.GetItemCount()) idx = 0;

	m_tab_ctrl.SetCurSel(idx);
	GetDocument()->ChangeDocData(GetDocIdx(idx));
}

void COeditView::OnPrevWindow() 
{
	ASSERT(m_tab_ctrl.GetItemCount() > 1);

	int idx = m_tab_ctrl.GetCurSel();
	if(idx == -1) return;

	idx--;
	if(idx == -1) idx = m_tab_ctrl.GetItemCount() - 1;

	m_tab_ctrl.SetCurSel(idx);
	GetDocument()->ChangeDocData(GetDocIdx(idx));
}

void COeditView::OnUpdateNextWindow(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_tab_ctrl.GetItemCount() > 1);
}

void COeditView::OnUpdatePrevWindow(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_tab_ctrl.GetItemCount() > 1);
}

void COeditView::MoveTab(int old_pos, int new_pos)
{
	TCHAR tab_name[_MAX_PATH];
	TC_ITEM	item;
	item.mask = TCIF_PARAM | TCIF_TEXT;
	item.pszText = tab_name;
	item.cchTextMax = sizeof(tab_name);
	m_tab_ctrl.GetItem(old_pos, &item);

	m_tab_ctrl.DeleteItem(old_pos);
	m_tab_ctrl.InsertItem(new_pos, &item);

//	m_tab_ctrl.SetCurSel(new_pos);

//  キャレットが消えてしまう
    GetDocument()->ChangeDocData(static_cast<int>(item.lParam));
}

void COeditView::SortTab()
{
	// 選択ソート
	int		i, j, min;
	CString	str_j, str_min;

	for(i = 0; i < m_tab_ctrl.GetItemCount() - 1; i++) {
		min = i;
		for(j = i + 1; j < m_tab_ctrl.GetItemCount(); j++) {
			if(GetTabName(GetDocument()->GetPathName_Idx(GetDocIdx(j))) < 
				GetTabName(GetDocument()->GetPathName_Idx(GetDocIdx(min)))) {
				min = j;
			}
		}
		if(i != min) MoveTab(min, i);
	}

	GetDocument()->ChangeDocData(GetDocIdx(0));
}

void COeditView::OnTabMoveLeft() 
{
	int tab_idx = m_tab_ctrl.GetCurSel();
	if(tab_idx == -1 || tab_idx == 0) return;

	MoveTab(tab_idx, tab_idx - 1);
}

void COeditView::OnTabMoveRight() 
{
	int tab_idx = m_tab_ctrl.GetCurSel();
	if(tab_idx == -1 || tab_idx == m_tab_ctrl.GetItemCount() - 1) return;

	MoveTab(tab_idx, tab_idx + 1);
}

BOOL COeditView::HaveLeftTab()
{
	if(m_tab_ctrl.GetItemCount() == 1 || m_tab_ctrl.GetCurSel() == 0) return FALSE;
	return TRUE;
}

BOOL COeditView::HaveRightTab()
{
	if(m_tab_ctrl.GetItemCount() == 1 || m_tab_ctrl.GetCurSel() == m_tab_ctrl.GetItemCount() - 1) return FALSE;
	return TRUE;
}

void COeditView::OnUpdateTabMoveLeft(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(HaveLeftTab());
}

void COeditView::OnUpdateTabMoveRight(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(HaveRightTab());
}

void COeditView::OnTabMoveTop() 
{
	int tab_idx = m_tab_ctrl.GetCurSel();
	if(tab_idx == -1 || tab_idx == 0) return;

	MoveTab(tab_idx, 0);
}

void COeditView::OnTabMoveLast() 
{
	int tab_idx = m_tab_ctrl.GetCurSel();
	if(tab_idx == -1 || tab_idx == m_tab_ctrl.GetItemCount() - 1) return;

	MoveTab(tab_idx, m_tab_ctrl.GetItemCount());
}

void COeditView::OnUpdateTabMoveTop(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(HaveLeftTab());
}

void COeditView::OnUpdateTabMoveLast(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(HaveRightTab());
}

void COeditView::OnSpaceToTab() 
{
	BOOL haveSelected = GetDocument()->GetEditCtrl()->HaveSelected();

	GetDocument()->GetEditCtrl()->InfrateSelectedArea();
	GetDocument()->GetEditCtrl()->SpaceToTab();

	if(!haveSelected) GetDocument()->GetEditCtrl()->ClearSelected();
}

void COeditView::OnTabToSpace() 
{
	BOOL haveSelected = GetDocument()->GetEditCtrl()->HaveSelected();

	GetDocument()->GetEditCtrl()->InfrateSelectedArea();
	GetDocument()->GetEditCtrl()->TabToSpace();

	if(!haveSelected) GetDocument()->GetEditCtrl()->ClearSelected();
}

void COeditView::ChangeTabMode()
{
	int		tab_idx;
	CString path_name;
	CString new_tab_name;
	TC_ITEM	item;

	if(g_option.tab_multi_row) {
		m_tab_ctrl.ModifyStyle(TCS_SINGLELINE, TCS_MULTILINE);
	} else {
		m_tab_ctrl.ModifyStyle(TCS_MULTILINE, TCS_SINGLELINE);
	}

	for(tab_idx = 0; tab_idx < m_tab_ctrl.GetItemCount(); tab_idx++) {
		item.mask = TCIF_PARAM;
		m_tab_ctrl.GetItem(tab_idx, &item);

        path_name = GetDocument()->GetPathName_Idx(static_cast<int>(item.lParam));
		new_tab_name = GetTabName(path_name);

        if (GetDocument()->IsModified_Idx(static_cast<int>(item.lParam))) {
			new_tab_name += " *";
		}
		m_tab_ctrl.SetItemText(tab_idx, new_tab_name);
	}

	RecalcLayout();
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

void COeditView::OnViewBrowser03() 
{
	if(GetDocument()->DoFileSave() == FALSE) return;
	CString filename = GetDocument()->GetPathName();
	ViewBrowser(_T("Browser03"), g_option.browser_03.GetBuffer(0), filename.GetBuffer(0), FALSE);
}

void COeditView::OnUpdateViewBrowser03(CCmdUI* pCmdUI) 
{
//	pCmdUI->Enable(g_str_token.GetTagMode());
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
//	pCmdUI->Enable(g_str_token.GetTagMode());
	pCmdUI->Enable(TRUE);
}

void COeditView::CheckBtn()
{
	m_close_btn.EnableWindow(!GetDocument()->IsEmpty());
}

void COeditView::OnCopyClickableUrl() 
{
	GetDocument()->GetEditCtrl()->CopyClickable();
}

void COeditView::OnUpdateClickableURL(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetDocument()->GetEditCtrl()->HitTestClickableURL());
}

void COeditView::OnEditDeleteRow() 
{
	GetDocument()->GetEditCtrl()->DeleteRow();
}

void COeditView::OnEditDeleteAfterCursor() 
{
	GetDocument()->GetEditCtrl()->DeleteAfterCaret();
}

void COeditView::OnLastEditPos() 
{
	GetDocument()->GetEditCtrl()->MoveLastEditPos();
}

void COeditView::OnEditCopyRow() 
{
	GetDocument()->GetEditCtrl()->CopyRow();
}

void COeditView::OnEditCutRow() 
{
	GetDocument()->GetEditCtrl()->CutRow();
}

void COeditView::OnEditPasteRow() 
{
	GetDocument()->GetEditCtrl()->PasteRow(FALSE);
}

void COeditView::OnEditPasteRowUp() 
{
	GetDocument()->GetEditCtrl()->PasteRow(TRUE);
}

void COeditView::OnSortTab() 
{
	SortTab();	
}

void COeditView::OnUpdateSortTab(CCmdUI* pCmdUI) 
{
	if(m_tab_ctrl.GetItemCount() <= 1) {
		pCmdUI->Enable(FALSE);
	} else {
		pCmdUI->Enable(TRUE);
	}
}

void COeditView::OnViewClickableUrl02() 
{
	CString url = GetDocument()->GetEditCtrl()->GetClickableURL();
	if(url == _T("")) return;
	ViewBrowser(_T("Browser02"), g_option.browser_02.GetBuffer(0), url.GetBuffer(0), FALSE);
}

void COeditView::OnViewClickableUrl01() 
{
	CString url = GetDocument()->GetEditCtrl()->GetClickableURL();
	if(url == _T("")) return;
	ViewBrowser(_T("Browser01"), g_option.browser_01.GetBuffer(0), url.GetBuffer(0), FALSE);
}

void COeditView::OnViewClickableUrl03() 
{
	CString url = GetDocument()->GetEditCtrl()->GetClickableURL();
	if(url == _T("")) return;
	ViewBrowser(_T("Browser03"), g_option.browser_03.GetBuffer(0), url.GetBuffer(0), FALSE);
}

void COeditView::OnViewClickableUrl01NewWindow() 
{
	CString url = GetDocument()->GetEditCtrl()->GetClickableURL();
	if(url == _T("")) return;
	ViewBrowser(_T("Browser01"), g_option.browser_01.GetBuffer(0), url.GetBuffer(0), TRUE);
}

void COeditView::OnViewClickableUrl03NewWindow() 
{
	CString url = GetDocument()->GetEditCtrl()->GetClickableURL();
	if(url == _T("")) return;
	ViewBrowser(_T("Browser03"), g_option.browser_03.GetBuffer(0), url.GetBuffer(0), TRUE);
}

void COeditView::OnEditJoinRow() 
{
	GetDocument()->GetEditCtrl()->JoinRow();
}

void COeditView::OnRedraw() 
{
	GetDocument()->GetEditCtrl()->Redraw();
}

void COeditView::OnTagJump()
{
	int row = GetDocument()->GetEditData()->get_cur_row();
	CEditCtrl *cur_ctrl = GetDocument()->GetEditCtrl();
	if(TagJump(GetDocument()->GetEditData()->get_row_buf(row)) != FALSE) {
		cur_ctrl->ClearSelected();
	}
}

void COeditView::OnInsertDateTime() 
{
	GetDocument()->GetEditCtrl()->InsertDateTime();
}

void COeditView::OnHankakuToZenkaku() 
{
	GetDocument()->GetEditCtrl()->HankakuToZenkaku(TRUE, TRUE);
}

void COeditView::OnHankakuToZenkakuAlpha() 
{
	GetDocument()->GetEditCtrl()->HankakuToZenkaku(TRUE, FALSE);
}

void COeditView::OnHankakuToZenkakuKata() 
{
	GetDocument()->GetEditCtrl()->HankakuToZenkaku(FALSE, TRUE);
}

void COeditView::OnZenkakuToHankaku() 
{
	GetDocument()->GetEditCtrl()->ZenkakuToHankaku(TRUE, TRUE);
}

void COeditView::OnZenkakuToHankakuAlpha() 
{
	GetDocument()->GetEditCtrl()->ZenkakuToHankaku(TRUE, FALSE);
}

void COeditView::OnZenkakuToHankakuKata() 
{
	GetDocument()->GetEditCtrl()->ZenkakuToHankaku(FALSE, TRUE);
}

BOOL COeditView::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;
}

void COeditView::OnSplitRows() 
{
	GetDocument()->GetEditCtrl()->SplitSelectedRows();
}

void COeditView::OnJoinRows() 
{
	ReplaceText(_T("\n"), _T(""), FALSE);
}

void COeditView::OnInputReturn() 
{
	GetDocument()->GetEditCtrl()->InputChar(VK_RETURN);
}

void COeditView::OnEditDeleteBeforeCursor() 
{
	GetDocument()->GetEditCtrl()->DeleteBeforeCaret();	
}

void COeditView::OnInputBr() 
{
	GetDocument()->GetEditCtrl()->Paste(_T("<br>\n"));
}

void COeditView::OnEditDeleteWordAfterCursor() 
{
	GetDocument()->GetEditCtrl()->DeleteWordAfterCaret();
}

void COeditView::OnEditDeleteWordBeforeCursor() 
{
	GetDocument()->GetEditCtrl()->DeleteWordBeforeCaret();
}

void COeditView::OnInvertBrackets() 
{
	GetDocument()->GetEditCtrl()->InvertBrackets();
}

void COeditView::OnGoMatchBrackets() 
{
	GetDocument()->GetEditCtrl()->GoMatchBrackets();	
}

void COeditView::SaveAllDocument()
{
	int cnt = m_tab_ctrl.GetItemCount();
	int cur_sel = m_tab_ctrl.GetCurSel();

	int		i;

	// 上書き保存
	for(i = 0; i < cnt; i++) {
		GetDocument()->ChangeDocData(GetDocIdx(i));
		if(!GetDocument()->IsSaveAsNewFile()) {
			if(GetDocument()->DoSave(GetDocument()->GetPathName()) == FALSE) break;
		}
	}

	// 名前を付けて保存
	for(i = 0; i < cnt; i++) {
		GetDocument()->ChangeDocData(GetDocIdx(i));
		if(GetDocument()->IsSaveAsNewFile()) {
			if(GetDocument()->DoSave(NULL) == FALSE) break;
		}
	}

	// 表示していたファイルに戻す
	GetDocument()->ChangeDocData(GetDocIdx(cur_sel));
}


void COeditView::OnSpaceLeft() { GetDocument()->GetEditCtrl()->SpaceLeft(FALSE); }
void COeditView::OnSpaceLeftExtend() { GetDocument()->GetEditCtrl()->SpaceLeft(TRUE); }
void COeditView::OnSpaceRight() { GetDocument()->GetEditCtrl()->SpaceRight(FALSE); }
void COeditView::OnSpaceRightExtend() { GetDocument()->GetEditCtrl()->SpaceRight(TRUE); }

void COeditView::OnEditDelDupRows() 
{
	int replace_cnt;
	GetDocument()->GetEditCtrl()->ReplaceText(_T("^(.*)$\\n(?:^\\1$(\\n?))+"), _T("\\1\\2"), 
		1, TRUE, TRUE, TRUE, TRUE, &replace_cnt, NULL);
}

void COeditView::OnUpdateEditDelDupRows(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetEditCtrl()->HaveSelectedMultiLine());
}

void COeditView::OnScmEval() 
{
	CCopyLineModeSetting l(GetDocument()->GetEditCtrl()->GetEditData(), LINE_TYPE_LF);
	CString code = GetDocument()->GetEditCtrl()->GetSelectedText();
	scm_eval_str(g_sc, code);
}

void COeditView::OnUpdateScmEval(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetEditCtrl()->HaveSelected());
}

void COeditView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if(point.y < m_tab_height && g_option.tab_create_at_dblclick) {
		AfxGetMainWnd()->SendMessage(WM_COMMAND, ID_FILE_NEW, 0);
	}
//	CView::OnLButtonDblClk(nFlags, point);
}

void COeditView::GetLeftTabIdx(std::vector<int> *arr)
{
	int tab_idx = m_tab_ctrl.GetCurSel();
	if(tab_idx == -1) return;

	int i;
	int cnt = tab_idx;

	for(i = 0; i < cnt; i++) {
		arr->push_back(GetDocIdx(i));
	}
}

void COeditView::GetRightTabIdx(std::vector<int> *arr)
{
	int tab_idx = m_tab_ctrl.GetCurSel();
	if(tab_idx == -1) return;

	int i;
	int cnt = m_tab_ctrl.GetItemCount() - tab_idx - 1;

	for(i = 0; i < cnt; i++) {
		arr->push_back(GetDocIdx(tab_idx + i + 1));
	}
}

void COeditView::OnTabCloseLeft() 
{
	std::vector<int> doc_idx_arr;
	GetLeftTabIdx(&doc_idx_arr);
	GetDocument()->CloseArr(&doc_idx_arr);
}

void COeditView::OnUpdateTabCloseLeft(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(HaveLeftTab());
}

void COeditView::OnTabCloseRight() 
{
	int tab_idx = m_tab_ctrl.GetCurSel();
	if(tab_idx == -1) return;

	std::vector<int> doc_idx_arr;
	GetRightTabIdx(&doc_idx_arr);
	GetDocument()->CloseArr(&doc_idx_arr);
}

void COeditView::OnUpdateTabCloseRight(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(HaveRightTab());
}

void COeditView::OnTabCloseOther() 
{
	std::vector<int> doc_idx_arr;
	GetLeftTabIdx(&doc_idx_arr);
	GetRightTabIdx(&doc_idx_arr);
	GetDocument()->CloseArr(&doc_idx_arr);
}

void COeditView::OnUpdateTabCloseOther(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(HaveLeftTab() || HaveRightTab());
}

void COeditView::OnWordLeftStopWb() { GetDocument()->GetEditCtrl()->WordLeftStopWB(FALSE); }
void COeditView::OnWordRightStopWb() { GetDocument()->GetEditCtrl()->WordRightStopWB(FALSE); }
void COeditView::OnWordLeftStopWbExtend() { GetDocument()->GetEditCtrl()->WordLeftStopWB(TRUE); }
void COeditView::OnWordRightStopWbExtend() { GetDocument()->GetEditCtrl()->WordRightStopWB(TRUE); }
