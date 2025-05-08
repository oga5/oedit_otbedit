/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 // CodeAssistEditCtrl.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "CodeAssistEditCtrl.h"

#include "octrl_util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

struct win_user_data {
	WNDPROC					wnd_proc;
	CCodeAssistEditCtrl		*wnd;
	LONG_PTR				old_user_data;
};

// メインウィンドウの，サブクラス後のウィンドウプロシージャ
static LRESULT CALLBACK MainWnd_SubclassWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	struct win_user_data *user_data = (struct win_user_data *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	WNDPROC wnd_proc = user_data->wnd_proc;

	switch(message) {
	case WM_NCACTIVATE:
		if((BOOL)wParam == FALSE) {
			// コードアシストウィンドウの表示中は，ウィンドウタイトルを非アクティブにしない
			return TRUE;
		}
		break;
	case WM_NCLBUTTONDOWN:	// 非クライアント領域(タイトルバー，メニューなど)をクリック
		user_data->wnd->AssistWndCanceled();
		break;
	case WM_PARENTNOTIFY:	// ツールバー上でクリック
		if(wParam == WM_LBUTTONDOWN) {
			user_data->wnd->AssistWndCanceled();
		}
		break;
	}

	return (CallWindowProc(wnd_proc, hwnd, message, wParam, lParam));
}

void CCodeAssistEditCtrl::SubclassMainWndOn()
{
	HWND hwnd = AfxGetMainWnd()->GetSafeHwnd();

	//
	// FIXME: m_subclass_ref_cntはEditCtrlごとに保持しているため、MainWndに対して
	// 複数回数Subclassしてしまう可能性がある
	// 複数Subclass化すると、user_data->wnd_procがMainWnd_SubclassWndProcを設定して
	// しまうため、無限ループになり落ちる
	//
	// (1)exists( などでツールチップを表示中に、Ctrl+Nで新ウィンドウを作成
	// (2)新ウィンドウで、コード補完のウィンドウを表示するところでエラーになる
	//

	if(m_subclass_ref_cnt == 0) {
		struct win_user_data *user_data = (struct win_user_data *)malloc(sizeof(struct win_user_data));
		if(user_data == NULL) return;

		struct win_user_data *old_user_data = (struct win_user_data *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		ASSERT(old_user_data == NULL);
		if(old_user_data != NULL) {
			return;
		}

		user_data->wnd = this;
		user_data->old_user_data = GetWindowLongPtr(hwnd, GWLP_USERDATA);
		user_data->wnd_proc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_WNDPROC);

		// サブクラス化
		// 古いウィンドウプロシージャを保存する
		::SetWindowLongPtr (hwnd, GWLP_USERDATA, (LONG_PTR)user_data);
		// ウィンドウプロシージャを切り替える
		::SetWindowLongPtr (hwnd, GWLP_WNDPROC, (LONG_PTR)MainWnd_SubclassWndProc);
	}

	m_subclass_ref_cnt++;
}

void CCodeAssistEditCtrl::SubclassMainWndOff()
{
	m_subclass_ref_cnt--;

	if(m_subclass_ref_cnt == 0) {
		HWND hwnd = AfxGetMainWnd()->GetSafeHwnd();
		struct win_user_data *user_data = (struct win_user_data *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if(user_data == NULL) return;

		// サブクラスを元に戻す
		::SetWindowLongPtr (hwnd, GWLP_WNDPROC, (LONG_PTR)user_data->wnd_proc);
		::SetWindowLongPtr (hwnd, GWLP_USERDATA, user_data->old_user_data);

		free(user_data);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCodeAssistEditCtrl

CCodeAssistEditCtrl::CCodeAssistEditCtrl()
{
	m_assist.assist_on = FALSE;
	m_list_maker = &m_dummy_list_maker;
	m_use_keyword_window = FALSE;
	m_enable_code_assist = FALSE;
	m_enable_code_assist_incr_search = FALSE;
	m_enable_code_assist_gray_candidate = FALSE;
	m_assist_match_type = ASSIST_FORWARD_MATCH;

	m_subclass_ref_cnt = 0;
}

CCodeAssistEditCtrl::~CCodeAssistEditCtrl()
{
}


BEGIN_MESSAGE_MAP(CCodeAssistEditCtrl, CEditCtrl)
	//{{AFX_MSG_MAP(CCodeAssistEditCtrl)
	ON_WM_CHAR()
	ON_WM_CREATE()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_KILLFOCUS()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCodeAssistEditCtrl メッセージ ハンドラ
BOOL CCodeAssistEditCtrl::CommitKey(UINT nChar) 
{
	if(m_code_wnd.GetAssistMode() == AssistCommit) {
		if(PasteAssistData()) {
			if(IsCodeAssistOnChar(nChar)) return FALSE;
			if(nChar != '\r' && nChar != '\n' && nChar != '\t') InputChar(nChar);
			return TRUE;
		}
	} else if(nChar == '\r' || nChar == '\n' || nChar == '\t') {
		if(m_code_wnd.GetCurGridData()->get_cur_row() < 0) {
			m_code_wnd.LineDown(FALSE);
		}
		m_code_wnd.SetAssistMode(AssistCommit);
		return TRUE;
	}
	return FALSE;
}

void CCodeAssistEditCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(IsCodeAssistOn()) {
		if(IsCommitChar(nChar)) {
			CString word = GetWord();
			word += (TCHAR)nChar;

			// word + 入力したキーで、一致する候補があるか調べる
			// incremental searchのとき、表示中の候補を壊さないようにする
			if(m_code_wnd.GetMatchData(word, NULL, TRUE)) {
				// 候補がある場合、再検索
				m_code_wnd.GetMatchData(word, NULL, FALSE);
			} else {
				if(CommitKey(nChar)) return;
			}
		} else if(m_edit_data->get_str_token()->isBreakChar(nChar)) {
			CodeAssistOff();
		}
	}

	CEditCtrl::OnChar(nChar, nRepCnt, nFlags);

	if(IsCodeAssistOn()) {
		m_code_wnd.SelectData(GetWord());
	} else if(m_enable_code_assist && IsCodeAssistOnChar(nChar)) {
		CodeAssistOn();
	}
}

void CCodeAssistEditCtrl::BackSpace()
{
	if(IsCodeAssistOn() && !HaveSelected() &&
		m_assist.start_pos.y == m_edit_data->get_cur_row() &&
		m_assist.start_pos.x == m_edit_data->get_cur_col()) {
		CodeAssistOff();
		return;
	}

	CEditCtrl::BackSpace();

	if(IsCodeAssistOn()) {
		m_code_wnd.SelectData(GetWord());
	}
}

void CCodeAssistEditCtrl::DeleteKey()
{
	if(IsCodeAssistOn() &&
		m_assist.end_pos.y == m_edit_data->get_cur_row() &&
		m_assist.end_pos.x == m_edit_data->get_cur_col()) {
		CodeAssistOff();
		return;
	}

	CEditCtrl::DeleteKey();

	if(IsCodeAssistOn()) {
		m_code_wnd.SelectData(GetWord());
	}
}

void CCodeAssistEditCtrl::InsertTab(BOOL del)
{
	if(IsCodeAssistOn()) {
		CommitKey('\t');
		return;
	}

	CEditCtrl::InsertTab(del);
}

int CCodeAssistEditCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CEditCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_code_wnd.SetGridData(&m_assist.assist_data);
	if(m_code_wnd.Create(this) == FALSE) return -1;

	m_code_wnd.SetGridStyle(m_code_wnd.GetGridStyle() | GRS_SHOW_CELL_ALWAYS_HALF);

	return 0;
}

void CCodeAssistEditCtrl::SetCodeAssistWindowPos()
{
	POINT	disp_pt, caret_pt;
	GetDispDataPoint(m_assist.start_pos, &disp_pt);
	GetDispCaretPoint(disp_pt, &caret_pt);

	CRect	win_rect;
	GetWindowRect(win_rect);
	caret_pt.x += win_rect.left - 5;
	caret_pt.y += win_rect.top + GetRowHeight();

	m_code_wnd.SetAssistWindowPos(caret_pt, GetRowHeight());
}

void CCodeAssistEditCtrl::AssistWindowOn(ASSIST_MODE mode)
{
	if(mode == ASSIST_KEYWORD && m_assist.assist_data.Get_RowCnt() == 1) {
		// 選択候補をテキストの入力によって動的に絞り込む設定のとき、
		// コード補完が正しく動作するようにする
		// GridDataを切り替えないと、絞込み前の候補の先頭が選択されてしまう
		m_code_wnd.SetIncrSearch(FALSE);
		m_code_wnd.SetGridData(&m_assist.assist_data);
		PasteAssistDataMain(0);
		return;
	}

	m_assist.assist_data.SetKeywordColor(GetColor(KEYWORD_COLOR));
	m_assist.assist_data.SetKeyword2Color(GetColor(KEYWORD2_COLOR));
	m_assist.assist_data.SetTextColor(GetColor(TEXT_COLOR));
	m_assist.assist_data.SetBkColor(GetColor(BG_COLOR));
	m_assist.assist_data.SetGrayCandidate(m_enable_code_assist_gray_candidate);

	m_code_wnd.SetCharSpace(GetCharSpaceSetting());

	m_code_wnd.SetAssistMatchType(m_assist_match_type);
	m_code_wnd.SetIncrSearch(m_enable_code_assist_incr_search);
	m_code_wnd.SetGridData(&m_assist.assist_data);
	m_code_wnd.UnSelect();

	SetCodeAssistWindowPos();
	m_code_wnd.ShowWindow(SW_SHOWNA);

	// ウィンドウプロシージャを切り替える
	SubclassMainWndOn();

	m_assist.assist_on = TRUE;
	m_assist.assist_mode = mode;
	m_assist.row_len = m_edit_data->get_row_len(m_assist.start_pos.y);
}

void CCodeAssistEditCtrl::CodeAssistOn(POINT *end_pt)
{
	if(IsCodeAssistOn()) return;
	if(m_list_maker->MakeList(&(m_assist.assist_data), m_edit_data) == FALSE) return;

	m_assist.start_pos.y = m_edit_data->get_cur_row();
	m_assist.start_pos.x = m_edit_data->get_cur_col();

	m_assist.close_pos = m_assist.start_pos;
	if(end_pt == NULL) {
		m_assist.end_pos = m_assist.start_pos;
	} else {
		m_assist.end_pos = *end_pt;
	}

	AssistWindowOn(ASSIST_CODE);
}

void CCodeAssistEditCtrl::CodeAssistOnCommand()
{
	if(IsCodeAssistOn()) {
		m_code_wnd.LineDown(TRUE);
		return;
	}

	POINT	cur_pt;
	cur_pt.y = m_edit_data->get_cur_row();
	cur_pt.x = m_edit_data->get_cur_col();

	if(!IsCodeAssistOnCommand()) {
		KeywordAssist(FALSE);
		return;
	}

	CodeAssistOn(&cur_pt);

	m_edit_data->set_cur(cur_pt.y, cur_pt.x);

	if(IsCodeAssistOn()) {
		m_code_wnd.SelectData(GetWord());
	}
}

void CCodeAssistEditCtrl::CodeAssistOff()
{
	if(!IsCodeAssistOn()) return;

	m_code_wnd.ShowWindow(SW_HIDE);

	// ウィンドウプロシージャを切り替える
	SubclassMainWndOff();

	m_assist.assist_on = FALSE;
}

void CCodeAssistEditCtrl::CaretMoved()
{
	if(!IsCodeAssistOn()) return;

	if(m_assist.row_len != m_edit_data->get_row_len(m_assist.start_pos.y)) {
		m_assist.end_pos.x += (m_edit_data->get_row_len(m_assist.start_pos.y) - m_assist.row_len);
		m_assist.row_len = m_edit_data->get_row_len(m_assist.start_pos.y);
	}

	if(m_assist.start_pos.y != m_edit_data->get_cur_row()) {
		CodeAssistOff();
		return;
	} else {
		if(m_assist.close_pos.x > m_edit_data->get_cur_col() ||
			m_assist.end_pos.x < m_edit_data->get_cur_col()) {
			CodeAssistOff();
			return;
		}
	}
}

void CCodeAssistEditCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch(nChar) {
	case VK_ESCAPE:
		if(IsCodeAssistOn()) CodeAssistOff();
		break;
	}
	
	CEditCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CCodeAssistEditCtrl::KeywordAssist(BOOL reverse)
{
	if(IsCodeAssistOn()) {
		if(!reverse) {
			m_code_wnd.LineDown(TRUE);
		} else {
			m_code_wnd.LineUp(TRUE);
		}
		return;
	}

	POINT	pt1, pt2;
	// 単語の末尾位置を取得
	pt2.y = m_edit_data->get_cur_row();
	pt2.x = m_edit_data->get_cur_col();
	if(pt2.x == 0) return;

	// 単語の先頭位置を取得
	m_edit_data->move_break_char(-1);
	pt1.y = m_edit_data->get_cur_row();
	pt1.x = m_edit_data->get_cur_col();
	m_edit_data->set_cur(pt2.y, pt2.x);

	CString org_str = m_edit_data->get_point_text(pt1, pt2);
	if(org_str == _T("")) return;

	m_assist.assist_data.InitData(FALSE);

	m_edit_data->get_str_token()->SetAssistMatchType(m_assist_match_type);
	m_edit_data->get_str_token()->GetCompletionList(m_assist.assist_data, org_str);
	AddSpecialKeyword(m_assist.assist_data, org_str);

	if(m_assist.assist_data.Get_RowCnt() == 0) return;

	m_assist.start_pos = pt1;
	m_assist.end_pos = pt2;
	m_assist.close_pos = pt2;

	AssistWindowOn(ASSIST_KEYWORD);

	if(!reverse) {
		m_code_wnd.SetAssistMode(AssistCommit);
		m_code_wnd.DocumentStart();
	} else {
		m_code_wnd.SetAssistMode(AssistCommit);
		m_code_wnd.DocumentEnd();
	}
}

void CCodeAssistEditCtrl::KeywordCompletion(BOOL reverse)
{
	if(HaveSelected() == TRUE) {
		ClearSelected();
		return;
	}

	if(IsReadOnly()) return;

	if(m_use_keyword_window) {
		KeywordAssist(reverse);
	} else {
		CEditCtrl::KeywordCompletion(reverse);
	}
}

void CCodeAssistEditCtrl::SetFont(CFont *font)
{
	m_code_wnd.SetFont(font);
	CEditCtrl::SetFont(font);
}

void CCodeAssistEditCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if(IsCodeAssistOn()) {
		CodeAssistOff();
	}
	
	CEditCtrl::OnLButtonDown(nFlags, point);
}

BOOL CCodeAssistEditCtrl::PasteAssistDataMain(int row)
{
	if(m_assist.start_pos.x != m_edit_data->get_cur_col()) {
		ASSERT(m_assist.end_pos.x <= m_edit_data->get_row_len(m_assist.end_pos.y));
		
		// Release環境で，end_pos.xの計算ミスから保護する
		if(m_assist.end_pos.x > m_edit_data->get_row_len(m_assist.end_pos.y)) {
			m_assist.end_pos.x = m_edit_data->get_row_len(m_assist.end_pos.y);
		}

		SetSelectedPoint(m_assist.start_pos, m_assist.end_pos);
	}

	CCodeAssistData *data = m_code_wnd.GetCurGridData();
	DoCodePaste(data->Get_ColData(row, 0), data->Get_ColData(row, 1), data->Get_ColData(row, 2));

	return TRUE;
}

BOOL CCodeAssistEditCtrl::PasteAssistData()
{
	if(!IsCodeAssistOn()) return FALSE;

	int row = m_code_wnd.GetCurGridData()->get_cur_row();
	if(m_code_wnd.GetAssistMode() == AssistTemp || row < 0) {
		CodeAssistOff();
		return FALSE;
	}

	CodeAssistOff();

	return PasteAssistDataMain(row);
}

LRESULT CCodeAssistEditCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message) {
	case CAW_WM_DBLCLICK:
		PasteAssistData();
		break;
	case CAW_WM_ACTIVATEAPP:
		if(IsCodeAssistOn()) {
			CodeAssistOff();
			// ウィンドウタイトルを描画
			if(AfxGetMainWnd() != NULL && (BOOL)wParam == FALSE) {
				AfxGetMainWnd()->PostMessage(WM_NCACTIVATE, 0, 0);
			}
		}
		break;
	}
	
	LRESULT result = CEditCtrl::WindowProc(message, wParam, lParam);

	switch(message) {
	case WM_IME_CHAR:
		// 選択候補を表示中にMulti byte文字が入力されたとき、キーワードを選択する
		// OnCharが呼ばれないのでここで処理する
		if(IsCodeAssistOn()) {
			m_code_wnd.SelectData(GetWord());
		}
		break;
	}

	return result;
}

CString CCodeAssistEditCtrl::GetWord()
{
	if(!IsCodeAssistOn()) return _T("");

	CString buf = _T("");

	if(m_assist.end_pos.x > m_assist.start_pos.x) {
		TCHAR *p = m_edit_data->get_row_buf(m_assist.start_pos.y);
		buf.Format(_T("%.*s"), m_assist.end_pos.x - m_assist.start_pos.x, p + m_assist.start_pos.x);
	}

	return buf;
}

void CCodeAssistEditCtrl::LineUp(BOOL extend)
{
	if(IsCodeAssistOn()) {
		m_code_wnd.SetAssistMode(AssistCommit);

		CString word = _T("");
		if(extend) word = GetWord();

		m_code_wnd.LineUp(FALSE, word);
		return;	
	}

	CEditCtrl::LineUp(extend);
}

void CCodeAssistEditCtrl::LineDown(BOOL extend)
{
	if(IsCodeAssistOn()) {
		m_code_wnd.SetAssistMode(AssistCommit);

		CString word = _T("");
		if(extend) word = GetWord();

		m_code_wnd.LineDown(FALSE, word);
		return;	
	}

	CEditCtrl::LineDown(extend);
}

void CCodeAssistEditCtrl::PageUp(BOOL extend)
{
	if(IsCodeAssistOn()) {
		m_code_wnd.SetAssistMode(AssistCommit);
		m_code_wnd.PageUp();
		return;	
	}

	CEditCtrl::PageUp(extend);
}

void CCodeAssistEditCtrl::PageDown(BOOL extend)
{
	if(IsCodeAssistOn()) {
		m_code_wnd.SetAssistMode(AssistCommit);
		m_code_wnd.PageDown();
		return;	
	}

	CEditCtrl::PageDown(extend);
}

void CCodeAssistEditCtrl::LineStart(BOOL extend)
{
	if(IsCodeAssistOn()) {
		m_code_wnd.SetAssistMode(AssistCommit);
		m_code_wnd.DocumentStart();
		return;	
	}
	CEditCtrl::LineStart(extend);
}

void CCodeAssistEditCtrl::LineEnd(BOOL extend)
{
	if(IsCodeAssistOn()) {
		m_code_wnd.SetAssistMode(AssistCommit);
		m_code_wnd.DocumentEnd();
		return;	
	}
	CEditCtrl::LineEnd(extend);
}

void CCodeAssistEditCtrl::SplitStart(BOOL extend)
{
	if(IsCodeAssistOn()) {
		m_code_wnd.SetAssistMode(AssistCommit);
		m_code_wnd.DocumentStart();
		return;	
	}
	CEditCtrl::SplitStart(extend);
}

void CCodeAssistEditCtrl::SplitEnd(BOOL extend)
{
	if(IsCodeAssistOn()) {
		m_code_wnd.SetAssistMode(AssistCommit);
		m_code_wnd.DocumentEnd();
		return;	
	}

	CEditCtrl::SplitEnd(extend);
}

void CCodeAssistEditCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	if(IsCodeAssistOn()) {
		if(pNewWnd != NULL && pNewWnd != this && pNewWnd != &m_code_wnd && pNewWnd != GetParent()) {
			CodeAssistOff();
		}
	}

	CEditCtrl::OnKillFocus(pNewWnd);
}

void CCodeAssistEditCtrl::OnDestroy() 
{
	if(IsCodeAssistOn()) CodeAssistOff();

	CEditCtrl::OnDestroy();
	
	m_code_wnd.DestroyWindow();
}

BOOL CCodeAssistEditCtrl::IsCommitChar(unsigned int nChar)
{
	if(nChar == '\t' || nChar == '\r' || nChar == '\n') return TRUE;
	return m_edit_data->get_str_token()->isBreakChar(nChar);
}

void CCodeAssistEditCtrl::SetCodeAssistWndColor(int type, COLORREF color)
{
	m_code_wnd.SetColor(type, color);
}

void CCodeAssistEditCtrl::SetCodeAssistWnd_InvertSelectText(BOOL invert)
{
	if(invert) {
		m_code_wnd.SetGridStyle(m_code_wnd.GetGridStyle() | GRS_INVERT_SELECT_TEXT);
	} else {
		m_code_wnd.SetGridStyle(m_code_wnd.GetGridStyle() & (~GRS_INVERT_SELECT_TEXT));
	}
}

void CCodeAssistEditCtrl::SetCodeAssistDispCnt(int cnt)
{
	m_code_wnd.SetDispCnt(cnt);
}

void CCodeAssistEditCtrl::SetCodeAssistMaxCommentDispWidth(int w)
{
	m_code_wnd.SetCodeAssistMaxCommentDispWidth(w);
}

void CCodeAssistEditCtrl::AssistWndCanceled()
{
	if(IsCodeAssistOn()) {
		CodeAssistOff();
	}
}

void CCodeAssistEditCtrl::ScrollUp()
{
	if(IsCodeAssistOn()) {
		m_code_wnd.ScrollUp();
		return;	
	}

	CEditCtrl::ScrollUp();
}

void CCodeAssistEditCtrl::ScrollDown()
{
	if(IsCodeAssistOn()) {
		m_code_wnd.ScrollDown();
		return;	
	}

	CEditCtrl::ScrollDown();
}

void CCodeAssistEditCtrl::ScrollPageUp()
{
	if(IsCodeAssistOn()) {
		m_code_wnd.ScrollPageUp();
		return;	
	}

	CEditCtrl::ScrollPageUp();
}

void CCodeAssistEditCtrl::ScrollPageDown()
{
	if(IsCodeAssistOn()) {
		m_code_wnd.ScrollPageDown();
		return;	
	}

	CEditCtrl::ScrollPageDown();
}

void CCodeAssistEditCtrl::DocumentStart(BOOL extend)
{
	if(IsCodeAssistOn()) {
		m_code_wnd.SetAssistMode(AssistCommit);
		m_code_wnd.DocumentStart();
		return;	
	}
	CEditCtrl::DocumentStart(extend);
}

void CCodeAssistEditCtrl::DocumentEnd(BOOL extend)
{
	if(IsCodeAssistOn()) {
		m_code_wnd.SetAssistMode(AssistCommit);
		m_code_wnd.DocumentEnd();
		return;	
	}
	CEditCtrl::DocumentEnd(extend);
}
