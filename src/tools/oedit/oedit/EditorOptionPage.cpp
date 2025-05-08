/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // EditorOptionPage.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "oedit.h"
#include "EditorOptionPage.h"

#include "fileutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditorOptionPage プロパティ ページ

IMPLEMENT_DYNCREATE(CEditorOptionPage, CPropertyPage)

CEditorOptionPage::CEditorOptionPage() : CPropertyPage(CEditorOptionPage::IDD)
{
	//{{AFX_DATA_INIT(CEditorOptionPage)
	m_show_line_feed = FALSE;
	m_show_tab = FALSE;
	m_tabstop = 0;
	m_show_row_num = FALSE;
	m_show_col_num = FALSE;
	m_row_space = 0;
	m_char_space = 0;
	m_left_space = 0;
	m_top_space = 0;
	m_line_len = 0;
	m_show_2byte_space = FALSE;
	m_clickable_url = FALSE;
	m_show_row_line = FALSE;
	m_show_edit_row = FALSE;
	m_wheel_scroll_row = 0;
	m_show_space = FALSE;
	m_show_line_end = FALSE;
	m_show_col_num_split_window = FALSE;
	m_show_brackets_bold = FALSE;
	m_show_ruled_line = FALSE;
	//}}AFX_DATA_INIT
}

CEditorOptionPage::~CEditorOptionPage()
{
}

void CEditorOptionPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditorOptionPage)
	DDX_Check(pDX, IDC_CHECK_SHOW_LINE_FEED, m_show_line_feed);
	DDX_Check(pDX, IDC_CHECK_SHOW_TAB, m_show_tab);
	DDX_Text(pDX, IDC_EDIT_TABSTOP, m_tabstop);
	DDV_MinMaxInt(pDX, m_tabstop, 1, 2147483647);
	DDX_Check(pDX, IDC_CHECK_SHOW_ROW_NUM, m_show_row_num);
	DDX_Check(pDX, IDC_CHECK_SHOW_COL_NUM, m_show_col_num);
	DDX_Text(pDX, IDC_EDIT_ROW_SPACE, m_row_space);
	DDX_Text(pDX, IDC_EDIT_CHAR_SPACE, m_char_space);
	DDX_Text(pDX, IDC_EDIT_LEFT_SPACE, m_left_space);
	DDX_Text(pDX, IDC_EDIT_TOP_SPACE, m_top_space);
	DDX_Text(pDX, IDC_EDIT_LINE_LEN, m_line_len);
	DDV_MinMaxInt(pDX, m_line_len, 1, 2147483647);
	DDX_Check(pDX, IDC_CHECK_SHOW_2BYTE_SPACE, m_show_2byte_space);
	DDX_Check(pDX, IDC_CHECK_CLICKABLE_URL, m_clickable_url);
	DDX_Check(pDX, IDC_CHECK_SHOW_ROW_LINE, m_show_row_line);
	DDX_Check(pDX, IDC_CHECK_SHOW_EDIT_ROW, m_show_edit_row);
	DDX_Text(pDX, IDC_EDIT_WHEEL_SCROLL_ROW, m_wheel_scroll_row);
	DDX_Check(pDX, IDC_CHECK_SHOW_SPACE, m_show_space);
	DDX_Check(pDX, IDC_CHECK_SHOW_LINE_END, m_show_line_end);
	DDX_Check(pDX, IDC_CHECK_SHOW_COL_NUM_SPLIT_WINDOW, m_show_col_num_split_window);
	DDX_Check(pDX, IDC_CHECK_SHOW_BRACKETS_BOLD, m_show_brackets_bold);
	DDX_Check(pDX, IDC_CHECK_SHOW_RULED_LINE, m_show_ruled_line);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditorOptionPage, CPropertyPage)
	//{{AFX_MSG_MAP(CEditorOptionPage)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_CHECK_SHOW_LINE_FEED, OnCheckShowLineFeed)
	ON_BN_CLICKED(IDC_CHECK_SHOW_TAB, OnCheckShowTab)
	ON_EN_CHANGE(IDC_EDIT_TABSTOP, OnChangeEditTabstop)
	ON_BN_CLICKED(IDC_BTN_TEXT_COLOR, OnBtnTextColor)
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_BTN_PEN_COLOR, OnBtnPenColor)
	ON_BN_CLICKED(IDC_BTN_DEFAULT_COLOR, OnBtnDefaultColor)
	ON_BN_CLICKED(IDC_BTN_BG_COLOR, OnBtnBgColor)
	ON_BN_CLICKED(IDC_BTN_COMMENT_COLOR, OnBtnCommentColor)
	ON_BN_CLICKED(IDC_BTN_KEYWORD_COLOR, OnBtnKeywordColor)
	ON_BN_CLICKED(IDC_BTN_QUOTE_COLOR, OnBtnQuoteColor)
	ON_BN_CLICKED(IDC_CHECK_SHOW_COL_NUM, OnCheckShowColNum)
	ON_BN_CLICKED(IDC_CHECK_SHOW_ROW_NUM, OnCheckShowRowNum)
	ON_EN_CHANGE(IDC_EDIT_ROW_SPACE, OnChangeEditRowSpace)
	ON_EN_CHANGE(IDC_EDIT_CHAR_SPACE, OnChangeEditCharSpace)
	ON_EN_CHANGE(IDC_EDIT_LEFT_SPACE, OnChangeEditLeftSpace)
	ON_EN_CHANGE(IDC_EDIT_TOP_SPACE, OnChangeEditTopSpace)
	ON_EN_CHANGE(IDC_EDIT_LINE_LEN, OnChangeEditLineLen)
	ON_BN_CLICKED(IDC_CHECK_SHOW_2BYTE_SPACE, OnCheckShow2byteSpace)
	ON_BN_CLICKED(IDC_BTN_SEARCH_COLOR, OnBtnSearchColor)
	ON_BN_CLICKED(IDC_BTN_SELECTED_COLOR, OnBtnSelectedColor)
	ON_BN_CLICKED(IDC_CHECK_CLICKABLE_URL, OnCheckClickableUrl)
	ON_BN_CLICKED(IDC_BTN_RULER_COLOR, OnBtnRulerColor)
	ON_BN_CLICKED(IDC_CHECK_SHOW_ROW_LINE, OnCheckShowRowLine)
	ON_BN_CLICKED(IDC_BTN_OPERATOR_COLOR, OnBtnOperatorColor)
	ON_BN_CLICKED(IDC_CHECK_SHOW_EDIT_ROW, OnCheckShowEditRow)
	ON_BN_CLICKED(IDC_BTN_KEYWORD2_COLOR, OnBtnKeyword2Color)
	ON_EN_CHANGE(IDC_EDIT_WHEEL_SCROLL_ROW, OnChangeEditWheelScrollRow)
	ON_BN_CLICKED(IDC_CHECK_SHOW_SPACE, OnCheckShowSpace)
	ON_BN_CLICKED(IDC_CHECK_SHOW_LINE_END, OnCheckShowLineEnd)
	ON_BN_CLICKED(IDC_CHECK_SHOW_BRACKETS_BOLD, OnCheckShowBracketsBold)
	ON_BN_CLICKED(IDC_BTN_IME_CARET_COLOR, OnBtnImeCaretColor)
	ON_BN_CLICKED(IDC_BTN_RULED_LINE_COLOR, OnBtnRuledLineColor)
	ON_BN_CLICKED(IDC_CHECK_SHOW_RULED_LINE, OnCheckShowRuledLine)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditorOptionPage メッセージ ハンドラ

int CEditorOptionPage::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CPropertyPage::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: この位置に固有の作成用コードを追加してください
	m_edit_data.paste(
		_T("<body bgcolor=\"#eeeeee\">\n")
		_T("<!-- comment -->\n")
		_T("\t<h2>タイトル　</h2>\n")
		_T("\t<a href=\"http://www.hi-ho.ne.jp/a_ogawa/\">\n")
	);
	m_edit_data.set_cur(0, 0);

	return 0;
}

BOOL CEditorOptionPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: この位置に初期化の補足処理を追加してください
	UpdateData(FALSE);

	CreateEditCtrl();

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

BOOL CEditorOptionPage::OnApply() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
	
	return CPropertyPage::OnApply();
}

void CEditorOptionPage::CreateEditCtrl()
{
	// HTMLモードのCStrTokenを作る
	TCHAR	msg_buf[512];
	CString	keyword_file;
	keyword_file.Format(_T("%s\\data\\html.txt"), GetAppPath().GetBuffer(0));

	m_str_token.SetKeywordLowerUpper(TRUE);
	m_str_token.SetOpenComment(_T("<!--"));
	m_str_token.SetCloseComment(_T("-->"));
	m_str_token.SetQuoteChars(_T("\""));
	m_str_token.SetBreakChars(_T(" ,.:;(){}[]\"=></"));
	m_str_token.SetOperatorChars(_T("=/<>"));
	m_str_token.SetBracketChars(_T(">)}"));
	m_str_token.SetTagMode(TRUE);

	if(m_str_token.initDefaultKeyword(keyword_file.GetBuffer(0), msg_buf) != 0) {
		// FIXME: メッセージを親切にしてから表示する
		//AfxMessageBox(msg_buf, MB_OK);
	}

	RECT rect;
	GetClientRect(&rect);

	RECT button_rect;
	GetDlgItem(IDC_BTN_DEFAULT_COLOR)->GetWindowRect(&button_rect);
	ScreenToClient(&button_rect);

	m_edit_data.set_str_token(&m_str_token);

	m_edit_ctrl.SetEditData(&m_edit_data);
	m_edit_ctrl.SetReadOnly(TRUE);

	m_edit_ctrl.Create(NULL, NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER, 
		CRect(20, button_rect.bottom + 15, rect.right - 20, rect.bottom - 10), 
		this, NULL);
	m_edit_ctrl.SetFont(&g_font);

	SetEditColor();
	SetEditorOption();

	m_edit_ctrl.SearchNext(_T("body"), TRUE, FALSE, NULL);
	m_edit_ctrl.MoveCaretPos(0, 1);
	m_edit_ctrl.SetFocus();
}

void CEditorOptionPage::SetEditorOption()
{
	UpdateData(TRUE);

	QWORD		option = ECS_IME_CARET_COLOR;

	if(m_tabstop < 1) m_tabstop = 1;
	if(m_tabstop > 200) m_tabstop = 200;
	if(m_row_space < 0) m_row_space = 0;
	if(m_row_space > 20) m_row_space = 20;
	if(m_char_space < 0) m_char_space = 0;
	if(m_char_space > 20) m_char_space = 20;
	if(m_top_space < 0) m_top_space = 0;
	if(m_top_space > 20) m_top_space = 20;
	if(m_left_space < 0) m_left_space = 0;
	if(m_left_space > 20) m_left_space = 20;
	if(m_line_len < 20) m_line_len = 20;
	if(m_line_len > 200) m_line_len = 200;
	if(m_wheel_scroll_row < 1) m_wheel_scroll_row = 1;
	if(m_wheel_scroll_row > 120) m_wheel_scroll_row = 120;

	if(m_edit_data.get_tabstop() != m_tabstop) {
		m_edit_data.set_tabstop(m_tabstop);
	}

	if(m_edit_ctrl.GetRowSpace() != m_row_space || 
		m_edit_ctrl.GetCharSpaceSetting() != m_char_space ||
		m_edit_ctrl.GetTopSpace() != m_top_space ||
		m_edit_ctrl.GetLeftSpace() != m_left_space) {

		m_edit_ctrl.SetSpaces(m_row_space, m_char_space, m_top_space, m_left_space);
	}

	if(m_show_line_feed) {
		option |= ECS_SHOW_LINE_FEED;
	}
	if(m_show_line_end) {
		option |= ECS_SHOW_LINE_END;
	}
	if(m_show_tab) {
		option |= ECS_SHOW_TAB;
	}
	if(m_show_row_num) {
		option |= ECS_SHOW_ROW_NUM;
	}
	if(m_show_col_num) {
		option |= ECS_SHOW_COL_NUM;
	}
	if(m_show_space) {
		option |= ECS_SHOW_SPACE;
	}
	if(m_show_2byte_space) {
		option |= ECS_SHOW_2BYTE_SPACE;
	}
	if(m_show_row_line) {
		option |= ECS_SHOW_ROW_LINE;
	}
	if(m_show_ruled_line) {
		option |= ECS_SHOW_RULED_LINE;
	}
	if(m_show_edit_row) {
		option |= ECS_SHOW_EDIT_ROW;
	}
	if(m_show_brackets_bold) {
		option |= ECS_SHOW_BRACKETS_BOLD;
	}
	if(m_clickable_url) {
		option |= ECS_CLICKABLE_URL;
	}

	option |= ECS_ON_DIALOG;
	option |= ECS_HTML_MODE;

	m_edit_ctrl.SetExStyle2(option);
	m_edit_ctrl.Redraw();
	m_edit_ctrl.SetCaret(FALSE, 1);
}

void CEditorOptionPage::OnCheckShowLineFeed() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnCheckShowTab() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnChangeEditTabstop() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: この位置にメッセージ ハンドラ用のコードを追加するかまたはデフォルトの処理を呼び出してください
	switch(nIDCtl) {
	case IDC_BTN_TEXT_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[TEXT_COLOR]);
		break;
	case IDC_BTN_KEYWORD_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[KEYWORD_COLOR]);
		break;
	case IDC_BTN_KEYWORD2_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[KEYWORD2_COLOR]);
		break;
	case IDC_BTN_COMMENT_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[COMMENT_COLOR]);
		break;
	case IDC_BTN_BG_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[BG_COLOR]);
		break;
	case IDC_BTN_PEN_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[PEN_COLOR]);
		break;
	case IDC_BTN_QUOTE_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[QUOTE_COLOR]);
		break;
	case IDC_BTN_SEARCH_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[SEARCH_COLOR]);
		break;
	case IDC_BTN_SELECTED_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[SELECTED_COLOR]);
		break;
	case IDC_BTN_RULER_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[RULER_COLOR]);
		break;
	case IDC_BTN_OPERATOR_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[OPERATOR_COLOR]);
		break;
	case IDC_BTN_IME_CARET_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[IME_CARET_COLOR]);
		break;
	case IDC_BTN_RULED_LINE_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[RULED_LINE_COLOR]);
		break;
	}

	CPropertyPage::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CEditorOptionPage::DrawColorBtn(LPDRAWITEMSTRUCT lpDrawItemStruct, COLORREF clr)
{
	CRect rect(lpDrawItemStruct->rcItem);

	::DrawEdge(lpDrawItemStruct->hDC, rect, EDGE_BUMP, BF_RECT);

	rect.DeflateRect(2, 2);
	::SetBkColor(lpDrawItemStruct->hDC, clr);
	::ExtTextOut(lpDrawItemStruct->hDC, 0, 0, ETO_OPAQUE, 
		rect, NULL, 0, NULL);
}

void CEditorOptionPage::ChooseColor(int color_id)
{
	CColorDialog	dlg(m_color[color_id]);

	if(dlg.DoModal() != IDOK) return;

	m_color[color_id] = dlg.GetColor();
	m_edit_ctrl.SetColor(color_id, m_color[color_id]);
	m_edit_ctrl.Redraw();
	Invalidate();
}

void CEditorOptionPage::SetEditColor()
{
	for(int i = 0; i < EDIT_CTRL_COLOR_CNT; i++) {
		m_edit_ctrl.SetColor(i, m_color[i]);
	}
	m_edit_ctrl.Redraw();
}

void CEditorOptionPage::SetDefaultEditColor()
{
	m_color[TEXT_COLOR] = RGB(0, 0, 0);
	m_color[KEYWORD_COLOR] = RGB(0, 0, 205);
	m_color[KEYWORD2_COLOR] = RGB(0, 128, 192);
	m_color[COMMENT_COLOR] = RGB(0, 120, 0);
	m_color[BG_COLOR] = RGB(255, 255, 255);
	m_color[PEN_COLOR] = RGB(0, 150, 150);
	m_color[QUOTE_COLOR] = RGB(220, 0, 0);
	m_color[SEARCH_COLOR] = RGB(200, 200, 200);
	m_color[SELECTED_COLOR] = RGB(0, 0, 50);
	m_color[RULER_COLOR] = RGB(0, 100, 0);
	m_color[OPERATOR_COLOR] = RGB(128, 0, 0);
	m_color[IME_CARET_COLOR] = RGB(0x00, 0xa0, 0x00);
	m_color[RULED_LINE_COLOR] = RGB(180, 180, 180);

	SetEditColor();
	Invalidate();
}

void CEditorOptionPage::OnBtnDefaultColor() 
{
	SetDefaultEditColor();
}

void CEditorOptionPage::OnBtnTextColor() 
{
	ChooseColor(TEXT_COLOR);
}

void CEditorOptionPage::OnBtnPenColor() 
{
	ChooseColor(PEN_COLOR);
}

void CEditorOptionPage::OnBtnBgColor() 
{
	ChooseColor(BG_COLOR);
}

void CEditorOptionPage::OnBtnCommentColor() 
{
	ChooseColor(COMMENT_COLOR);
}

void CEditorOptionPage::OnBtnKeywordColor() 
{
	ChooseColor(KEYWORD_COLOR);
}

void CEditorOptionPage::OnBtnQuoteColor() 
{
	ChooseColor(QUOTE_COLOR);
}

void CEditorOptionPage::OnCheckShowColNum() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnCheckShowRowNum() 
{
	SetEditorOption();	
}

void CEditorOptionPage::OnChangeEditRowSpace() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnChangeEditCharSpace() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnChangeEditLeftSpace() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnChangeEditTopSpace() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnChangeEditLineLen() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnCheckShow2byteSpace() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnBtnSearchColor() 
{
	ChooseColor(SEARCH_COLOR);
}

void CEditorOptionPage::OnBtnSelectedColor() 
{
	ChooseColor(SELECTED_COLOR);
}

void CEditorOptionPage::OnCheckClickableUrl() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnBtnRulerColor() 
{
	ChooseColor(RULER_COLOR);
}

void CEditorOptionPage::OnCheckShowRowLine() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnBtnOperatorColor() 
{
	ChooseColor(OPERATOR_COLOR);
}

void CEditorOptionPage::OnCheckShowEditRow() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnBtnKeyword2Color() 
{
	ChooseColor(KEYWORD2_COLOR);
}

void CEditorOptionPage::OnChangeEditWheelScrollRow() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnCheckShowSpace() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnCheckShowLineEnd() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnCheckShowBracketsBold() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnBtnImeCaretColor() 
{
	ChooseColor(IME_CARET_COLOR);
}

void CEditorOptionPage::OnBtnRuledLineColor() 
{
	ChooseColor(RULED_LINE_COLOR);
}

void CEditorOptionPage::OnCheckShowRuledLine() 
{
	SetEditorOption();
}
