/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

// oeditView.h : COeditView クラスの宣言およびインターフェイスの定義をします。
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_OEDITVIEW_H__52A23F4D_4E07_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_OEDITVIEW_H__52A23F4D_4E07_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "oeditdoc.h"
#include "CodeAssistEditCtrl.h"
#include "searchDlg.h"
#include "replaceDlg.h"
#include "filterDlg.h"
#include "printDlg.h"
#include "SelectColorDlg.h"
#include "FileTabCtrl.h"

#include "CloseBtn.h"

#include "greputil.h"
#include <vector>

class COeditView : public CView
{
protected: // シリアライズ機能のみから作成します。
	COeditView();
	DECLARE_DYNCREATE(COeditView)

// アトリビュート
public:
	COeditDoc* GetDocument();
	CEditCtrl* GetEditCtrl() { return GetDocument()->GetEditCtrl(); }

	void SetSearchText(const TCHAR *search_text, BOOL distinct_lwr_upr, BOOL distinct_width_ascii);

// オペレーション
public:

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(COeditView)
	public:
	virtual void OnDraw(CDC* pDC);  // このビューを描画する際にオーバーライドされます。
	virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual void OnDragLeave();
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~COeditView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(COeditView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnEditCut();
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnSelectAll();
	afx_msg void OnSearch();
	afx_msg void OnSearchNext();
	afx_msg void OnSearchPrev();
	afx_msg void OnReplace();
	afx_msg void OnEditIndent();
	afx_msg void OnEditIndentRev();
	afx_msg void OnEditSortRowsAsc();
	afx_msg void OnEditSortRowsDesc();
	afx_msg void OnToLower();
	afx_msg void OnToUpper();
	afx_msg void OnEditReverseRows();
	afx_msg void OnFilePrint();
	afx_msg void OnDeleteSpace();
	afx_msg void OnInsertGt();
	afx_msg void OnLineModeLen();
	afx_msg void OnLineModeNormal();
	afx_msg void OnLineModeRight();
	afx_msg void OnUpdateLineModeLen(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLineModeNormal(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLineModeRight(CCmdUI* pCmdUI);
	afx_msg void OnEditDelete();
	afx_msg void OnColorInput();
	afx_msg void OnViewBrowser03();
	afx_msg void OnUpdateViewBrowser03(CCmdUI* pCmdUI);
	afx_msg void OnUpdateColorInput(CCmdUI* pCmdUI);
	afx_msg void OnClearSearchText();
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	afx_msg void OnInsertComment();
	afx_msg void OnDeleteComment();
	afx_msg void OnDeleteGt();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnLineJump();
	afx_msg void OnSearchFile();
	afx_msg void OnCharLeft();
	afx_msg void OnCharLeftExtend();
	afx_msg void OnCharRight();
	afx_msg void OnCharRightExtend();
	afx_msg void OnLineUp();
	afx_msg void OnLineUpExtend();
	afx_msg void OnLineDown();
	afx_msg void OnLineDownExtend();
	afx_msg void OnWordLeft();
	afx_msg void OnWordLeftExtend();
	afx_msg void OnWordRight();
	afx_msg void OnWordRightExtend();
	afx_msg void OnLineStart();
	afx_msg void OnLineStartExtend();
	afx_msg void OnLineEnd();
	afx_msg void OnLineEndExtend();
	afx_msg void OnDocumentEnd();
	afx_msg void OnDocumentEndExtend();
	afx_msg void OnDocumentStart();
	afx_msg void OnDocumentStartExtend();
	afx_msg void OnEditBackspace();
	afx_msg void OnKeywordCompletion();
	afx_msg void OnKeywordCompletionRev();
	afx_msg void OnScrollUp();
	afx_msg void OnScrollDown();
	afx_msg void OnScrollPageUp();
	afx_msg void OnScrollPageDown();
	afx_msg void OnPageUp();
	afx_msg void OnPageUpExtend();
	afx_msg void OnPageDown();
	afx_msg void OnPageDownExtend();
	afx_msg void OnSelectRow();
	afx_msg void OnBoxSelect();
	afx_msg void OnSetBookMark();
	afx_msg void OnJumpBookMark();
	afx_msg void OnJumpBookMarkBack();
	afx_msg void OnClearAllBookMark();
	afx_msg void OnToggleOverwrite();
	afx_msg void OnPaint();
	afx_msg void OnNextWindow();
	afx_msg void OnPrevWindow();
	afx_msg void OnTabMoveLeft();
	afx_msg void OnTabMoveRight();
	afx_msg void OnUpdateTabMoveLeft(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTabMoveRight(CCmdUI* pCmdUI);
	afx_msg void OnTabMoveTop();
	afx_msg void OnTabMoveLast();
	afx_msg void OnUpdateTabMoveLast(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTabMoveTop(CCmdUI* pCmdUI);
	afx_msg void OnSpaceToTab();
	afx_msg void OnTabToSpace();
	afx_msg void OnJumpBookMarkBackExtend();
	afx_msg void OnJumpBookMarkExtend();
	afx_msg void OnViewBrowser01();
	afx_msg void OnUpdateViewBrowser01(CCmdUI* pCmdUI);
	afx_msg void OnCopyClickableUrl();
	afx_msg void OnEditDeleteRow();
	afx_msg void OnEditDeleteAfterCursor();
	afx_msg void OnLastEditPos();
	afx_msg void OnEditCopyRow();
	afx_msg void OnEditCutRow();
	afx_msg void OnEditPasteRow();
	afx_msg void OnEditPasteRowUp();
	afx_msg void OnSplitStart();
	afx_msg void OnSplitStartExtend();
	afx_msg void OnSplitEnd();
	afx_msg void OnSplitEndExtend();
	afx_msg void OnSelectWord();
	afx_msg void OnSortTab();
	afx_msg void OnUpdateSortTab(CCmdUI* pCmdUI);
	afx_msg void OnViewClickableUrl01();
	afx_msg void OnViewClickableUrl01NewWindow();
	afx_msg void OnViewClickableUrl03();
	afx_msg void OnViewClickableUrl03NewWindow();
	afx_msg void OnEditJoinRow();
	afx_msg void OnRedraw();
	afx_msg void OnSearchSelected();
	afx_msg void OnTagJump();
	afx_msg void OnDeleteSpaceEnd();
	afx_msg void OnInsertDateTime();
	afx_msg void OnHankakuToZenkaku();
	afx_msg void OnHankakuToZenkakuAlpha();
	afx_msg void OnHankakuToZenkakuKata();
	afx_msg void OnZenkakuToHankaku();
	afx_msg void OnZenkakuToHankakuAlpha();
	afx_msg void OnZenkakuToHankakuKata();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSplitRows();
	afx_msg void OnJoinRows();
	afx_msg void OnInputReturn();
	afx_msg void OnFilter();
	afx_msg void OnUpdateNextWindow(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePrevWindow(CCmdUI* pCmdUI);
	afx_msg void OnEditDeleteBeforeCursor();
	afx_msg void OnInputBr();
	afx_msg void OnEditDeleteWordAfterCursor();
	afx_msg void OnEditDeleteWordBeforeCursor();
	afx_msg void OnInvertBrackets();
	afx_msg void OnGoMatchBrackets();
	afx_msg void OnEditIndentSpace();
	afx_msg void OnEditIndentSpaceRev();
	afx_msg void OnSpaceLeft();
	afx_msg void OnSpaceLeftExtend();
	afx_msg void OnSpaceRight();
	afx_msg void OnSpaceRightExtend();
	afx_msg void OnEditDelDupRows();
	afx_msg void OnUpdateEditDelDupRows(CCmdUI* pCmdUI);
	afx_msg void OnSearchSelectedPrev();
	afx_msg void OnCharLeftExtendBox();
	afx_msg void OnCharRightExtendBox();
	afx_msg void OnLineDownExtendBox();
	afx_msg void OnLineUpExtendBox();
	afx_msg void OnScmEval();
	afx_msg void OnUpdateScmEval(CCmdUI* pCmdUI);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnBoxPaste();
	afx_msg void OnUpdateBoxPaste(CCmdUI* pCmdUI);
	afx_msg void OnTabCloseLeft();
	afx_msg void OnUpdateTabCloseLeft(CCmdUI* pCmdUI);
	afx_msg void OnTabCloseOther();
	afx_msg void OnUpdateTabCloseOther(CCmdUI* pCmdUI);
	afx_msg void OnTabCloseRight();
	afx_msg void OnUpdateTabCloseRight(CCmdUI* pCmdUI);
	afx_msg void OnViewBrowser02();
	afx_msg void OnUpdateViewBrowser02(CCmdUI* pCmdUI);
	afx_msg void OnViewClickableUrl02();
	afx_msg void OnWordLeftStopWb();
	afx_msg void OnWordLeftStopWbExtend();
	afx_msg void OnWordRightStopWb();
	afx_msg void OnWordRightStopWbExtend();
	//}}AFX_MSG
	afx_msg void OnUpdateSelected(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMultiLineEdit(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCommentEdit(CCmdUI* pCmdUI);
	afx_msg void OnUpdateOverwrite(CCmdUI* pCmdUI);
	afx_msg void OnUpdateClickableURL(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

private:
	//CEditCtrl	m_edit_ctrl;
	CSearchDlgData	m_search_data;
	CSearchDlg		m_search_dlg;
	CReplaceDlg		m_replace_dlg;
	CFilterDlg		m_filter_dlg;
	CSelectColorDlg	m_color_dlg;

	CCloseBtn		m_close_btn;
	CFileTabCtrl	m_tab_ctrl;
	int				m_tab_height;

	void CheckBtn();
	void OnTabSelChanged();
	void OnTabToolTipName(NMHDR *p_nmhdr);

	void SetEditorOption();
	void ReplaceText(TCHAR *search_text, TCHAR *replace_text, BOOL bInfrateSelectedArea = TRUE);

	void PreChangeEditData();
	void PostChangeEditData(int idx);

	void SetFont();
	void RecalcLayout();
	int GetTabHeight();

	void CalcCloseButtonRect(RECT *lpRect);

	CString GetTabName(CString path_name);

	int SearchTabIdx(int doc_idx);
	int GetDocIdx(int tab_idx);
	int TabHitTest(CPoint point);
	void MoveTab(int old_pos, int new_pos);
	void SortTab();
	void ChangeTabMode();

	void SetSearchSelected();
	void SearchMsg(int ret_v, int dir, BOOL b_looped);

	void SaveAllDocument();

	BOOL HaveLeftTab();
	BOOL HaveRightTab();

	void GetLeftTabIdx(std::vector<int> *arr);
	void GetRightTabIdx(std::vector<int> *arr);

	void CloseTab(int tab_idx);
};

#ifndef _DEBUG  // oeditView.cpp ファイルがデバッグ環境の時使用されます。
inline COeditDoc* COeditView::GetDocument()
   { return (COeditDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_OEDITVIEW_H__52A23F4D_4E07_11D5_8505_00E018A83B1B__INCLUDED_)
