/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#if !defined(AFX_CODEASSISTEDITCTRL_H__6475E16A_9E47_410F_B5DC_BBE80C2800CD__INCLUDED_)
#define AFX_CODEASSISTEDITCTRL_H__6475E16A_9E47_410F_B5DC_BBE80C2800CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CodeAssistEditCtrl.h : ヘッダー ファイル
//

#include "EditCtrl.h"
#include "CodeAssistWnd.h"
#include "CodeAssistListMaker.h"
#include "PtrGridData.h"
#include "StrToken.h"

/////////////////////////////////////////////////////////////////////////////
// CCodeAssistEditCtrl ウィンドウ

typedef enum {
	ASSIST_CODE,
	ASSIST_KEYWORD,
} ASSIST_MODE;

struct code_assist_st {
	BOOL			assist_on{ FALSE };
	POINT			start_pos{ 0, 0 };
	POINT			end_pos{ 0, 0 };
	POINT			close_pos{ 0, 0 };
	int				row_len{ 0 };
	CCodeAssistData	assist_data;
	ASSIST_MODE		assist_mode{ ASSIST_CODE };
};

class CCodeAssistEditCtrl : public CEditCtrl
{
// コンストラクション
public:
	CCodeAssistEditCtrl();

// アトリビュート
public:

// オペレーション
public:
	void SetCodeAssistListMaker(CCodeAssistListMaker *list_maker) { m_list_maker = list_maker; }

	void KeywordAssist(BOOL reverse);

	void CodeAssistOn(POINT *end_pt = NULL);
	void CodeAssistOff();
	void CodeAssistOnCommand();
	BOOL IsCodeAssistOn() { return m_assist.assist_on; }
	ASSIST_MODE GetAssistMode() { return m_assist.assist_mode; }

	void SetUseKeywordWindow(BOOL use_keyword_window) { m_use_keyword_window = use_keyword_window; }
	void SetEnableCodeAssist(BOOL enable) { m_enable_code_assist = enable; }
	void SetEnableCodeAssistIncrSearch(BOOL enable) { m_enable_code_assist_incr_search = enable; }
	void SetEnableCodeAssistGrayCandidate(BOOL enable) { m_enable_code_assist_gray_candidate = enable; }
	void SetAssistMatchType(int t) { m_assist_match_type = t; }

	void SetCodeAssistWndColor(int type, COLORREF color);
	void SetCodeAssistWnd_InvertSelectText(BOOL invert);
	void SetCodeAssistDispCnt(int cnt);
	void SetCodeAssistMaxCommentDispWidth(int w);

	virtual void AssistWndCanceled();

	// CEditCtrlのoverride 
	virtual void SetFont(CFont *font);

	// FIXME:virtual関数にしたほうがいいか
	void KeywordCompletion(BOOL reverse);
	void BackSpace();
	void DeleteKey();
	void InsertTab(BOOL del);

	// FIXME:virtual関数にしたほうがいいか
	void LineUp(BOOL extend);
	void LineDown(BOOL extend);
	void PageUp(BOOL extend);
	void PageDown(BOOL extend);
	void LineStart(BOOL extend);
	void LineEnd(BOOL extend);
	void SplitStart(BOOL extend);
	void SplitEnd(BOOL extend);
	void ScrollUp();
	void ScrollDown();
	void ScrollPageUp();
	void ScrollPageDown();
	void DocumentStart(BOOL extend);
	void DocumentEnd(BOOL extend);

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CCodeAssistEditCtrl)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CCodeAssistEditCtrl();

	// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CCodeAssistEditCtrl)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CCodeAssistWnd	m_code_wnd;
	BOOL			m_use_keyword_window;
	BOOL			m_enable_code_assist;
	BOOL			m_enable_code_assist_incr_search;
	BOOL			m_enable_code_assist_gray_candidate;
	int				m_assist_match_type;

	int				m_subclass_ref_cnt;

	struct code_assist_st m_assist;

	CCodeAssistListMaker *m_list_maker;
	CCodeAssistListMaker m_dummy_list_maker;

protected:
	virtual void CaretMoved();
	BOOL PasteAssistData();
	BOOL PasteAssistDataMain(int row);

	void AssistWindowOn(ASSIST_MODE mode);
	BOOL CommitKey(UINT nChar);

	void SetCodeAssistWindowPos();

	CString GetWord();

	void SubclassMainWndOn();
	void SubclassMainWndOff();

	virtual void DoCodePaste(const TCHAR *paste_str, const TCHAR *type, const TCHAR *hint) { Paste(paste_str); }
	virtual BOOL IsCommitChar(unsigned int nChar);
	virtual BOOL IsCodeAssistOnChar(unsigned int nChar) { return FALSE; }
	virtual BOOL IsCodeAssistOnCharCommand(unsigned int nChar) { return IsCodeAssistOnChar(nChar); }
	virtual BOOL IsCodeAssistOnCommand() { return FALSE; }

	virtual BOOL AddSpecialKeyword(CCodeAssistData &grid_data, const TCHAR *str) { return FALSE; }
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_CODEASSISTEDITCTRL_H__6475E16A_9E47_410F_B5DC_BBE80C2800CD__INCLUDED_)
