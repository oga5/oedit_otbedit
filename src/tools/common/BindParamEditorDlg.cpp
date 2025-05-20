// BindParamEditorDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "global.h"
#include "BindParamEditorDlg.h"
#include "mbutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBindParamEditorDlg ダイアログ

CBindParamEditorDlg::CBindParamEditorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBindParamEditorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBindParamEditorDlg)
		// メモ - ClassWizard はこの位置にマッピング用のマクロを追加または削除します。
	//}}AFX_DATA_INIT

	m_bind_data_org = NULL;
	m_bind_data_tmp = NULL;
	m_bind_param_editor_grid_cell_width[0] = m_bind_param_editor_grid_cell_width[1] = 50;
	m_sql = NULL;
	m_str_token = NULL;
}

void CBindParamEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBindParamEditorDlg)
		// メモ - ClassWizard はこの位置にマッピング用のマクロを追加または削除します。
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBindParamEditorDlg, CDialog)
	//{{AFX_MSG_MAP(CBindParamEditorDlg)
	ON_BN_CLICKED(IDC_BTN_OK, OnBtnOk)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDCANCEL, &CBindParamEditorDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBindParamEditorDlg メッセージ ハンドラ

BOOL CBindParamEditorDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: この位置に初期化の補足処理を追加してください
	InitData();
	CreateGridCtrl();
	MoveCenter();

	m_grid_ctrl.SetFocus();
	m_grid_ctrl.SetCell(1, 0);

	m_bind_param_editor_grid_cell_width[0] = 100;
	m_bind_param_editor_grid_cell_width[1] = 200;

	return FALSE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CBindParamEditorDlg::InitData()
{
	const TCHAR		*p;
	CString			tmp_value;
	CStringArray	tmp_str_arr;
	CMapStringToString	tmp_str_map;
	TCHAR	word_buf[1024 * 32];
	CString		bind_name;

	// バインド変数の数を数える
	tmp_str_map.RemoveAll();
	tmp_str_arr.RemoveAll();

	for(p = m_sql; p != NULL;) {
#ifdef OSQLEDIT
		// 大文字小文字が違っても同一の変数とみなすが、画面表示は入力された文字列を表示したい
		TCHAR	word_buf_upr[1024 * 32];

		p = m_str_token->get_word(p, word_buf, sizeof(word_buf));
		if(!m_str_token->isBindParam(word_buf)) continue;

		_tcscpy(word_buf_upr, word_buf);
		my_mbsupr_1byte(word_buf_upr);

		if(tmp_str_map.Lookup(word_buf_upr + 1, tmp_value)) continue;
		tmp_str_map.SetAt(word_buf_upr + 1, _T(""));
		tmp_str_arr.Add(word_buf + 1);
#endif

#ifdef PSQLEDIT
		p = m_str_token->skipCommentAndSpace(p);
		if(p == NULL) break;

		if(!m_str_token->isBindParam(p)) {
			int len = m_str_token->get_word_len(p, FALSE);
			p = p + len;
			continue;
		}

		p = m_str_token->get_word_for_bind(p, word_buf, sizeof(word_buf), bind_name);

		if(tmp_str_map.Lookup(bind_name, tmp_value)) continue;
		tmp_str_map.SetAt(bind_name, _T(""));
		tmp_str_arr.Add(bind_name);
#endif
	}

	m_grid_data.Init((int)tmp_str_arr.GetSize(), 2);
	m_grid_data.SetEditableCell(0, TRUE);
	m_grid_data.SetEditableCell(1, TRUE);

	m_grid_data.SetColName(0, _T("変数名"));
	m_grid_data.SetColName(1, _T("値"));

	// バインド変数をグリッドに展開
	for(int row = 0; row < tmp_str_arr.GetSize(); row++) {
		if(m_bind_data_org->Lookup(tmp_str_arr.GetAt(row), tmp_value) == FALSE) {
			if(m_bind_data_tmp->Lookup(tmp_str_arr.GetAt(row), tmp_value) == FALSE) {
				tmp_value = _T("");
			}
		}
		m_grid_data.InitData(row, 0, tmp_str_arr.GetAt(row));
		m_grid_data.InitData(row, 1, tmp_value);
	}

	m_grid_data.SetEditableCell(0, FALSE);
}

void CBindParamEditorDlg::CreateGridCtrl()
{
	RECT rect;
	GetClientRect(&rect);

	RECT button_rect;
	GetDlgItem(IDCANCEL)->GetWindowRect(&button_rect);
	ScreenToClient(&button_rect);
	
	m_grid_data.SetDispColWidth(0, m_bind_param_editor_grid_cell_width[0]);
	m_grid_data.SetDispColWidth(1, m_bind_param_editor_grid_cell_width[1]);

	m_grid_ctrl.SetGridData(&m_grid_data);
	m_grid_ctrl.Create(NULL, NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP, 
		CRect(20, 15, rect.right - 20, button_rect.top - 10), 
		this, NULL);

	m_grid_ctrl.SetColor(GRID_UPDATE_COLOR, m_grid_ctrl.GetColor(GRID_BG_COLOR));

	DWORD grid_option = GRS_COL_HEADER | GRS_ROW_HEADER | GRS_MULTI_SELECT | GRS_ALLOW_F2_ENTER_EDIT;
	if(g_option.text_editor.ime_caret_color) grid_option |= GRS_IME_CARET_COLOR;
	if(g_option.grid.show_line_feed) grid_option |= GRS_SHOW_LINE_FEED;
	if(g_option.grid.show_tab) grid_option |= GRS_SHOW_TAB;
	if(g_option.grid.show_space) grid_option |= GRS_SHOW_SPACE;
	if(g_option.grid.show_2byte_space) grid_option |= GRS_SHOW_2BYTE_SPACE;
	m_grid_ctrl.SetGridStyle(grid_option);
	
	m_grid_ctrl.SetFont(&g_font);
}

void CBindParamEditorDlg::OnBtnOk() 
{
	m_bind_data_tmp->RemoveAll();

	m_grid_ctrl.LeaveEdit();

	CString	tmp_value;

	for(int row = 0; row < m_grid_data.Get_RowCnt(); row++) {
		m_bind_data_tmp->SetAt(m_grid_data.Get_ColData(row, 0), m_grid_data.Get_ColData(row, 1));

		// bindコマンドの設定を書き換える
		if(m_bind_data_org->Lookup(m_grid_data.Get_ColData(row, 0), tmp_value) != FALSE) {
			m_bind_data_org->SetAt(m_grid_data.Get_ColData(row, 0), m_grid_data.Get_ColData(row, 1));
		}
	}

	SaveGridCellWidth();

	CDialog::OnOK();
}

void CBindParamEditorDlg::MoveCenter()
{
	CRect	main_rect;
	AfxGetMainWnd()->GetWindowRect(main_rect);

	CRect	my_rect;
	GetWindowRect(my_rect);

	int x = main_rect.left + main_rect.Width() / 2 - my_rect.Width() / 2;
	int y = main_rect.top + main_rect.Height() / 2 - my_rect.Height() / 2;

	SetWindowPos(NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
}

void CBindParamEditorDlg::OnOK() 
{
	//CDialog::OnOK();
}

BOOL CBindParamEditorDlg::PreTranslateMessage(MSG* pMsg) 
{
	if(g_bind_dlg_accel != NULL) {
		::TranslateAccelerator(GetSafeHwnd(), g_bind_dlg_accel, pMsg);
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CBindParamEditorDlg::OnEditUndo() 
{
	m_grid_ctrl.Undo();
	if(!m_grid_ctrl.IsEnterEdit()) m_grid_ctrl.SetFocus();
}

void CBindParamEditorDlg::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_grid_ctrl.CanUndo());
}

void CBindParamEditorDlg::OnEditRedo() 
{
	m_grid_ctrl.Redo();
	if(!m_grid_ctrl.IsEnterEdit()) m_grid_ctrl.SetFocus();
}

void CBindParamEditorDlg::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_grid_ctrl.CanRedo());
}

void CBindParamEditorDlg::OnEditCopy() 
{
	m_grid_ctrl.Copy();
}

void CBindParamEditorDlg::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_grid_ctrl.CanCopy());
}

void CBindParamEditorDlg::OnEditCut() 
{
	m_grid_ctrl.Cut();
}

void CBindParamEditorDlg::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_grid_ctrl.CanCut());
}

void CBindParamEditorDlg::OnEditPaste() 
{
	m_grid_ctrl.Paste();
}

void CBindParamEditorDlg::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_grid_ctrl.CanPaste());
}

void CBindParamEditorDlg::SaveGridCellWidth()
{
	m_bind_param_editor_grid_cell_width[0] = m_grid_ctrl.GetDispColWidth(0);
	m_bind_param_editor_grid_cell_width[1] = m_grid_ctrl.GetDispColWidth(1);
}

void CBindParamEditorDlg::OnBnClickedCancel()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	SaveGridCellWidth();
	CDialog::OnCancel();
}
