// ogrepDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "ogrep.h"
#include "ogrepDlg.h"

#include "fileutil.h"
#include "ogrep_util.h"
#include "CommonCancelDlg.h"
#include "file_winutil.h"
#include "greputil.h"
#include "filenames.h"

#include "UnicodeArchive.h"

#include <process.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static const char *THIS_FILE = __FILE__;
#endif

#define OGREP_DLG_MIN_HEIGHT	300
#define OGREP_DLG_MIN_WIDTH		560

static void on_drop_files(HWND hwnd, HDROP hDropInfo) 
{
	UINT nFiles = ::DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);
	if(nFiles == 0) return;

	// ひとつめのファイル
	TCHAR szFileName[_MAX_PATH];
	::DragQueryFile(hDropInfo, 0, szFileName, _MAX_PATH);

	::DragFinish(hDropInfo);

	// ファイルが指定されたら、フォルダ名に変換
	if(is_file_exist(szFileName)) {
		make_parent_dirname(szFileName);
	}

	SetWindowText(hwnd, szFileName);
}

// Folder入力欄のサブクラス
static LRESULT CALLBACK Edit_SubclassWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
	case WM_DROPFILES:
		on_drop_files(hwnd, (HDROP)wParam);
		break;
	}

	return (CallWindowProc((WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA),
		hwnd, message, wParam, lParam));
}

/////////////////////////////////////////////////////////////////////////////
// アプリケーションのバージョン情報で使われている CAboutDlg ダイアログ

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// ダイアログ データ
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CString	m_static_version;
	//}}AFX_DATA

	// ClassWizard は仮想関数のオーバーライドを生成します
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV のサポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	m_static_version = _T("");
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Text(pDX, IDC_STATIC_VERSION, m_static_version);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COgrepDlg ダイアログ

COgrepDlg::COgrepDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COgrepDlg::IDD, pParent)
	, m_check_distinct_width_ascii(FALSE)
{
	//{{AFX_DATA_INIT(COgrepDlg)
	m_check_distinct_lwr_upr = FALSE;
	m_check_regexp = FALSE;
	m_check_search_sub_folder = FALSE;
	m_file_type = _T("");
	m_search_folder = _T("");
	m_search_text = _T("");
	//}}AFX_DATA_INIT
	// メモ: LoadIcon は Win32 の DestroyIcon のサブシーケンスを要求しません。
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hSmallIcon = (HICON)::LoadImage(AfxGetInstanceHandle(),
		MAKEINTRESOURCE(IDR_MAINFRAME),
		IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	m_kanji_code = UnknownKanjiCode;
	m_oedit_mode = TRUE;

	m_searched_data.m_check_distinct_lwr_upr = m_check_distinct_lwr_upr;
	m_searched_data.m_check_distinct_width_ascii = m_check_distinct_width_ascii;
	m_searched_data.m_check_regexp = m_check_regexp;
}

void COgrepDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COgrepDlg)
	DDX_Control(pDX, IDC_COMBO_KANJI_CODE, m_combo_kanji_code);
	DDX_Control(pDX, IDC_COMBO_SEARCH_TEXT, m_combo_search_text);
	DDX_Control(pDX, IDC_COMBO_SEARCH_FOLDER, m_combo_search_folder);
	DDX_Control(pDX, IDC_COMBO_FILE_TYPE, m_combo_file_type);
	DDX_Control(pDX, ID_BTN_SEARCH, m_btn_search);
	DDX_Check(pDX, IDC_CHECK_DISTINCT_LWR_UPR, m_check_distinct_lwr_upr);
	DDX_Check(pDX, IDC_CHECK_REGEXP, m_check_regexp);
	DDX_Check(pDX, IDC_CHECK_SEARCH_SUB_FOLDER, m_check_search_sub_folder);
	DDX_CBString(pDX, IDC_COMBO_FILE_TYPE, m_file_type);
	DDX_CBString(pDX, IDC_COMBO_SEARCH_FOLDER, m_search_folder);
	DDX_CBString(pDX, IDC_COMBO_SEARCH_TEXT, m_search_text);
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_CHECK_DISTINCT_WIDTH_ASCII, m_check_distinct_width_ascii);
}

BEGIN_MESSAGE_MAP(COgrepDlg, CDialog)
	//{{AFX_MSG_MAP(COgrepDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CREATE()
	ON_CBN_EDITCHANGE(IDC_COMBO_FILE_TYPE, OnEditchangeComboFileType)
	ON_CBN_EDITCHANGE(IDC_COMBO_SEARCH_FOLDER, OnEditchangeComboSearchFolder)
	ON_CBN_EDITCHANGE(IDC_COMBO_SEARCH_TEXT, OnEditchangeComboSearchText)
	ON_CBN_SELENDOK(IDC_COMBO_FILE_TYPE, OnSelendokComboFileType)
	ON_CBN_SELENDOK(IDC_COMBO_SEARCH_FOLDER, OnSelendokComboSearchFolder)
	ON_CBN_SELENDOK(IDC_COMBO_SEARCH_TEXT, OnSelendokComboSearchText)
	ON_BN_CLICKED(ID_BTN_SEARCH, OnBtnSearch)
	ON_BN_CLICKED(IDC_BTN_SELECT_FOLDER, OnBtnSelectFolder)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COgrepDlg メッセージ ハンドラ


void COgrepDlg::CheckOeditMode()
{
	m_oedit_mode = TRUE;

	CString otbedit_file_name = GetAppPath() + "otbedit.exe";
	if(is_file_exist(otbedit_file_name)) {
		m_oedit_mode = FALSE;
	}

	CString oedit_file_name = GetAppPath() + "oedit.exe";
	if(is_file_exist(oedit_file_name)) {
		m_oedit_mode = TRUE;
	}
}

BOOL COgrepDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CheckOeditMode();

	// "バージョン情報..." メニュー項目をシステム メニューへ追加します。

	// IDM_ABOUTBOX はコマンド メニューの範囲でなければなりません。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// このダイアログ用のアイコンを設定します。フレームワークはアプリケーションのメイン
	// ウィンドウがダイアログでない時は自動的に設定しません。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンを設定
	SetIcon(m_hSmallIcon, FALSE);		// 小さいアイコンを設定
	
	// TODO: 特別な初期化を行う時はこの場所に追加してください。
	CUnicodeArchive::SetKanjiCodeCombo(&m_combo_kanji_code, m_kanji_code, TRUE);

	CreateEditCtrl();

	m_check_distinct_lwr_upr = AfxGetApp()->GetProfileInt(_T("GREP"), _T("DISTINCT_LWR_UPR"), 0);
	m_check_distinct_width_ascii = AfxGetApp()->GetProfileInt(_T("GREP"), _T("DISTINCT_WIDTH_ASCII"), 0);
	m_check_regexp = AfxGetApp()->GetProfileInt(_T("GREP"), _T("REGEXP"), 0);
	m_check_search_sub_folder = AfxGetApp()->GetProfileInt(_T("GREP"), _T("SEARCH_SUB_FOLDER"), 0);

	UpdateData(FALSE);

	SetFileType();

	LoadComboData(&m_combo_search_text, _T("search_text"));
	LoadComboData(&m_combo_file_type, _T("file_type"));
	LoadComboData(&m_combo_search_folder, _T("search_folder"));

	CBootParameter boot_p;
	TCHAR msg_buf[1024];
	if(boot_p.InitParameter(AfxGetApp()->m_lpCmdLine, msg_buf, FALSE)) {
		if(boot_p.GetFileCnt() > 0 && boot_p.GetFileName(0) != _T("")) {
			m_combo_search_text.SetWindowText(boot_p.GetFileName(0));
		}
		if(boot_p.GetBootDir() != _T("")) {
			m_combo_search_folder.SetWindowText(boot_p.GetBootDir());
		}
		if(!boot_p.GetOption(_T("c")).IsEmpty()) {
			int idx = m_combo_kanji_code.FindStringExact(-1, boot_p.GetOption(_T("c")));
			if(idx != CB_ERR) m_combo_kanji_code.SetCurSel(idx);
		}
	}

	CheckBtn();

	m_edit_ctrl.SetWindowText(OGREP_WINDOW_NAME);

	// サブクラス化
	CWnd *child = m_combo_search_folder.ChildWindowFromPoint(CPoint(10, 10));
	if(child != NULL) {
		HWND hwnd = child->GetSafeHwnd();
		// 古いウィンドウプロシージャを保存する
		::SetWindowLongPtr(hwnd, GWLP_USERDATA, GetWindowLongPtr(hwnd, GWLP_WNDPROC));
		// ウィンドウプロシージャを切り替える
		::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)Edit_SubclassWndProc);

		// フォルダ／ファイルのDrag and dropを受け付ける
		child->DragAcceptFiles(TRUE);
	}

	LoadDialogPosition();

	return TRUE;  // TRUE を返すとコントロールに設定したフォーカスは失われません。
}

void COgrepDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// もしダイアログボックスに最小化ボタンを追加するならば、アイコンを描画する
// コードを以下に記述する必要があります。MFC アプリケーションは document/view
// モデルを使っているので、この処理はフレームワークにより自動的に処理されます。

void COgrepDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画用のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// クライアントの矩形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンを描画します。
		dc.DrawIcon(x, y, m_hSmallIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// システムは、ユーザーが最小化ウィンドウをドラッグしている間、
// カーソルを表示するためにここを呼び出します。
HCURSOR COgrepDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hSmallIcon;
}

void COgrepDlg::OnOK() 
{
	// TODO: この位置にその他の検証用のコードを追加してください
	
	//CDialog::OnOK();
}

int COgrepDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: この位置に固有の作成用コードを追加してください
	
	return 0;
}

void COgrepDlg::CreateEditCtrl()
{
	RECT rect;
	GetClientRect(&rect);

	RECT button_rect;
	GetDlgItem(IDC_BTN_HIDDEN)->GetWindowRect(&button_rect);
	ScreenToClient(&button_rect);

	m_edit_ctrl.SetEditData(&m_edit_data);
	m_edit_ctrl.SetReadOnly(TRUE);

	m_edit_ctrl.Create(NULL, NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER, 
		CRect(10, button_rect.bottom + 10, rect.right - 10, rect.bottom - 10), 
		this, NULL);

	SetEditorOption();
}

struct _st_option {
	struct {
		int		tabstop{ 4 };				// tabstop count of textctrl
		int		row_space{ 0 };
		int		char_space{ 0 };
		int		top_space{ 0 };
		int		left_space{ 4 };
		BOOL	show_line_feed{ FALSE };
		BOOL	show_tab{ FALSE };
		BOOL	show_row_num{ FALSE };
		BOOL	show_col_num{ FALSE };
		BOOL	show_2byte_space{ FALSE };
		int		line_mode{ 0 };
		int		line_len{ 0 };
		COLORREF	color[EDIT_CTRL_COLOR_CNT];
		BOOL	auto_indent{ FALSE };
	} text_editor;

	struct _st_option_font {
		CString		face_name;
		int			point{ 0 };
	} font;
};

void COgrepDlg::SetEditorOption()
{
	struct _st_option	option_data;

	const TCHAR *org_profile_name = AfxGetApp()->m_pszProfileName;

	TCHAR ini_file_name[MAX_PATH];
	if(m_oedit_mode) {
		_stprintf(ini_file_name, _T("%soedit.ini"), GetAppPath().GetBuffer(0));
	} else {
		_stprintf(ini_file_name, _T("%sotbedit.ini"), GetAppPath().GetBuffer(0));
	}
	AfxGetApp()->m_pszProfileName = ini_file_name;

	option_data.text_editor.tabstop = AfxGetApp()->GetProfileInt(_T("TEXT_EDITOR"), _T("TABSTOP"), 4);
	option_data.text_editor.row_space = AfxGetApp()->GetProfileInt(_T("TEXT_EDITOR"), _T("ROW_SPACE"), 0);
	option_data.text_editor.char_space = AfxGetApp()->GetProfileInt(_T("TEXT_EDITOR"), _T("CHAR_SPACE"), 0);
	option_data.text_editor.top_space = AfxGetApp()->GetProfileInt(_T("TEXT_EDITOR"), _T("TOP_SPACE"), 5);
	option_data.text_editor.left_space = AfxGetApp()->GetProfileInt(_T("TEXT_EDITOR"), _T("LEFT_SPACE"), 5);
	option_data.text_editor.show_line_feed = AfxGetApp()->GetProfileInt(_T("TEXT_EDITOR"), _T("SHOW_LINE_FEED"), 0);
	option_data.text_editor.show_tab = AfxGetApp()->GetProfileInt(_T("TEXT_EDITOR"), _T("SHOW_TAB"), 0);
	option_data.text_editor.show_2byte_space = AfxGetApp()->GetProfileInt(_T("TEXT_EDITOR"), _T("SHOW_2BYTE_SPACE"), 0);
	option_data.text_editor.show_row_num = AfxGetApp()->GetProfileInt(_T("TEXT_EDITOR"), _T("SHOW_ROW_NUM"), 0);
	option_data.text_editor.show_col_num = AfxGetApp()->GetProfileInt(_T("TEXT_EDITOR"), _T("SHOW_COL_NUM"), 0);
	option_data.text_editor.line_mode = AfxGetApp()->GetProfileInt(_T("TEXT_EDITOR"), _T("LINE_MODE"), EC_LINE_MODE_NORMAL);
	option_data.text_editor.line_len = AfxGetApp()->GetProfileInt(_T("TEXT_EDITOR"), _T("LINE_LEN"), 80);
	option_data.text_editor.auto_indent = AfxGetApp()->GetProfileInt(_T("TEXT_EDITOR"), _T("AUTO_INDENT"), FALSE);

	option_data.text_editor.color[TEXT_COLOR] = AfxGetApp()->GetProfileInt(_T("TEXT_EDITOR"), _T("TEXT_COLOR"), RGB(0, 0, 0));
	option_data.text_editor.color[KEYWORD_COLOR] = AfxGetApp()->GetProfileInt(_T("TEXT_EDITOR"), _T("KEYWORD_COLOR"), RGB(0, 0, 205));
	option_data.text_editor.color[COMMENT_COLOR] = AfxGetApp()->GetProfileInt(_T("TEXT_EDITOR"), _T("COMMENT_COLOR"), RGB(0, 120, 0));
	option_data.text_editor.color[BG_COLOR] = AfxGetApp()->GetProfileInt(_T("TEXT_EDITOR"), _T("BG_COLOR"), RGB(255, 255, 255));
	option_data.text_editor.color[PEN_COLOR] =	AfxGetApp()->GetProfileInt(_T("TEXT_EDITOR"), _T("PEN_COLOR"), RGB(0, 150, 150));
	option_data.text_editor.color[QUOTE_COLOR] = AfxGetApp()->GetProfileInt(_T("TEXT_EDITOR"), _T("QUOTE_COLOR"), RGB(220, 0, 0));
	option_data.text_editor.color[SEARCH_COLOR] = AfxGetApp()->GetProfileInt(_T("TEXT_EDITOR"), _T("SRARCH_COLOR"), RGB(255, 255, 50));
	option_data.text_editor.color[SELECTED_COLOR] = AfxGetApp()->GetProfileInt(_T("TEXT_EDITOR"), _T("SELECTED_COLOR"), RGB(0, 0, 50));

	option_data.font.face_name = AfxGetApp()->GetProfileString(_T("FONT"), _T("FACENAME"), _T("Terminal"));
	option_data.font.point = AfxGetApp()->GetProfileInt(_T("FONT"), _T("POINT"), 140);

	AfxGetApp()->m_pszProfileName = org_profile_name;

	m_font.CreatePointFont(option_data.font.point, option_data.font.face_name);
	m_edit_ctrl.SetFont(&m_font);

	m_edit_data.set_tabstop(option_data.text_editor.tabstop);
	m_edit_ctrl.SetSpaces(option_data.text_editor.row_space,
		option_data.text_editor.char_space,
		option_data.text_editor.top_space,
		option_data.text_editor.left_space);

	QWORD option = 0;

	if(option_data.text_editor.show_line_feed) {
		option |= ECS_SHOW_LINE_FEED;
	}
	if(option_data.text_editor.show_tab) {
		option |= ECS_SHOW_TAB;
	}
/*
	if(option_data.text_editor.show_row_num) {
		option |= ECS_SHOW_ROW_NUM;
	}
	if(option_data.text_editor.show_col_num) {
		option |= ECS_SHOW_COL_NUM;
	}
*/
	if(option_data.text_editor.show_2byte_space) {
		option |= ECS_SHOW_2BYTE_SPACE;
	}

	option |= ECS_ON_DIALOG;

	m_edit_ctrl.SetExStyle2(option);

	for(int i = 0; i < EDIT_CTRL_COLOR_CNT; i++) {
		m_edit_ctrl.SetColor(i, option_data.text_editor.color[i]);
	}
	m_edit_ctrl.Redraw();
}

void COgrepDlg::CheckBtn(CString *search_text, CString *file_type, CString *search_folder)
{
	UpdateData(TRUE);

	if(search_text == NULL) search_text = &m_search_text;
	if(file_type == NULL) file_type = &m_file_type;
	if(search_folder == NULL) search_folder = &m_search_folder;

	if(*search_text == _T("") || *file_type == _T("") || *search_folder == _T("")) {
		m_btn_search.EnableWindow(FALSE);
	} else {
		m_btn_search.EnableWindow(TRUE);
	}
}

void COgrepDlg::OnEditchangeComboFileType() 
{
	CheckBtn();
}

void COgrepDlg::OnEditchangeComboSearchFolder() 
{
	CheckBtn();
}

void COgrepDlg::OnEditchangeComboSearchText() 
{
	CheckBtn();
}

void COgrepDlg::OnSelendokComboFileType() 
{
	CString	text = _T("");

	if(m_combo_file_type.GetCurSel() != CB_ERR) {
		m_combo_file_type.GetLBText(m_combo_file_type.GetCurSel(), text);
	}

	CheckBtn(NULL, &text, NULL);
}

void COgrepDlg::OnSelendokComboSearchFolder() 
{
	CString	text = _T("");

	if(m_combo_search_folder.GetCurSel() != CB_ERR) {
		m_combo_search_folder.GetLBText(m_combo_search_folder.GetCurSel(), text);
	}

	CheckBtn(NULL, NULL, &text);
}

void COgrepDlg::OnSelendokComboSearchText() 
{
	CString	text = _T("");

	if(m_combo_search_text.GetCurSel() != CB_ERR) {
		m_combo_search_text.GetLBText(m_combo_search_text.GetCurSel(), text);
	}

	CheckBtn(&text, NULL, NULL);
}

void COgrepDlg::SetFileType()
{
	CString		section = _T("file_type");
	CWinApp*	pApp = AfxGetApp();

	if(pApp->GetProfileString(section, _T("0"), _T("")) != _T("")) return;

	pApp->WriteProfileString(section, _T("0"), _T("*.txt;*.htm;*.html;*.c;*.cpp;*.h;*.java;*.pl;*.cgi;*.sql;*.ora;*.log"));
	pApp->WriteProfileString(section, _T("1"), _T("*.*"));
	pApp->WriteProfileString(section, _T("2"), _T("*.txt"));
	pApp->WriteProfileString(section, _T("3"), _T("*.htm;*.html"));
	pApp->WriteProfileString(section, _T("4"), _T("*.c;*.cpp;*.h"));
	pApp->WriteProfileString(section, _T("5"), _T("*.java"));
	pApp->WriteProfileString(section, _T("6"), _T("*.pl;*.cgi"));
	pApp->WriteProfileString(section, _T("7"), _T("*.sql;*.ora;*.log"));
}

void COgrepDlg::LoadComboData(CComboBox *combo_box, CString section)
{
	int			i;
	CString		entry;
	CString		text;
	CWinApp*	pApp = AfxGetApp();

	// コンボボックスをクリア
	for(; combo_box->GetCount() != 0;) {
		combo_box->DeleteString(0);
	}

	// レジストリのテキストを取得
	for(i = 0 ; i < SAVE_COMBO_CNT; i++) {
		// テキストを取得
		entry.Format(_T("%d"), i);
		text = pApp->GetProfileString(section, entry, _T(""));

		if(text != _T("")) {
			combo_box->InsertString(i, text);
		}
	}

	if(combo_box->GetCount() != 0) combo_box->SetCurSel(0);
}

void COgrepDlg::SetComboData(CComboBox *combo_box, CString data)
{
	CString		combo_str;
	int			idx;

	for(idx = 0; idx < combo_box->GetCount(); idx++) {
		combo_box->GetLBText(idx, combo_str);
		if(combo_str == data) {
			combo_box->DeleteString(idx);
			break;
		}
	}

	// 今回使用したテキストを先頭に保存
	combo_box->InsertString(0, data);

	if(combo_box->GetCount() != 0) combo_box->SetCurSel(0);
}

void COgrepDlg::SaveComboData(CComboBox *combo_box, CString section)
{
	CString		entry;
	CString		combo_str;
	int			i, idx;

	for(i = 0, idx = 0; i < combo_box->GetCount(); i++) {
		combo_box->GetLBText(i, combo_str);
		if(combo_str == "") continue;

		entry.Format(_T("%d"), idx);
		AfxGetApp()->WriteProfileString(section, entry, combo_str);
		idx++;
	}

	// 残りを空白にする
	for(; idx < SAVE_COMBO_CNT; idx++) {
		entry.Format(_T("%d"), idx);
		AfxGetApp()->WriteProfileString(section, entry, NULL);
	}
}

void COgrepDlg::OnBtnSearch() 
{
	CWaitCursor	wait_cursor;

	UpdateData(TRUE);

	if(m_search_text == _T("")) return;

	if(is_directory_exist(m_search_folder.GetBuffer(0)) == FALSE) {
		MessageBox(_T("フォルダが存在しません"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	int kanji_idx = m_combo_kanji_code.GetCurSel();
	if(kanji_idx != CB_ERR) {
        m_kanji_code = static_cast<int>(m_combo_kanji_code.GetItemData(kanji_idx));
	} else {
		m_kanji_code = UnknownKanjiCode;
	}

	m_edit_ctrl.ClearAll();

	TCHAR search_folder[_MAX_PATH];
	_tcscpy(search_folder, m_search_folder.GetBuffer(0));
	make_dirname2(search_folder);

	TCHAR	msg_buf[1024] = _T("");

	_stprintf(msg_buf, _T("'%s' を検索中...\n"), search_folder);
	m_edit_ctrl.Paste(msg_buf);

	_thr_grep_st	grep_st;
	grep_st.edit_ctrl = &m_edit_ctrl;
	grep_st.search_text = m_search_text.GetBuffer(0);
	grep_st.file_type = m_file_type.GetBuffer(0);
	grep_st.search_folder = search_folder;
	grep_st.distinct_lwr_upr = m_check_distinct_lwr_upr;
	grep_st.distinct_width_ascii = m_check_distinct_width_ascii;
	grep_st.regexp = m_check_regexp;
	grep_st.search_sub_folder = m_check_search_sub_folder;
	grep_st.kanji_code = m_kanji_code;
	grep_st.hwnd = NULL;
	grep_st.msg_buf = msg_buf;
	grep_st.ret_v = 0;

	CCommonCancelDlg	dlg;

	// スレッドを作成
	uintptr_t	hThread;
	UINT		thrdaddr;
	hThread = _beginthreadex(NULL, 0, GrepThr,
		&grep_st, CREATE_SUSPENDED, &thrdaddr);
	if(hThread == NULL) {
		MessageBox(_T("スレッド作成エラー"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	dlg.InitData(CND_USE_CANCEL_BTN | CND_USE_STATIC_2,
		(HWND *)&(grep_st.hwnd), (HANDLE)hThread);
	dlg.DoModal();

	WaitForSingleObject((void *)hThread, INFINITE);
	CloseHandle((void *)hThread);

	if(grep_st.ret_v == 2 || grep_st.ret_v == 3) {
		MessageBox(msg_buf, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	SetComboData(&m_combo_search_text, m_search_text);
	SetComboData(&m_combo_file_type, m_file_type);
	SetComboData(&m_combo_search_folder, m_search_folder);

	m_searched_data.m_search_text = m_search_text;
	m_searched_data.m_check_distinct_lwr_upr = m_check_distinct_lwr_upr;
	m_searched_data.m_check_distinct_width_ascii = m_check_distinct_width_ascii;
	m_searched_data.m_check_regexp = m_check_regexp;

	m_edit_ctrl.SaveSearchData2(m_search_text.GetBuffer(0),
		m_check_distinct_lwr_upr, m_check_distinct_width_ascii, m_check_regexp);
	m_edit_ctrl.Paste(msg_buf);
}

void COgrepDlg::OnBtnSelectFolder() 
{
	TCHAR	dir[MAX_PATH];

	UpdateData(TRUE);

	// ディレクトリ名の最後の \をはずす
	TCHAR search_folder[_MAX_PATH];
	_tcscpy(search_folder, m_search_folder.GetBuffer(0));
	make_dirname2(search_folder);

	if(SelectFolder(GetSafeHwnd(), dir, search_folder,
		_T("検索フォルダの選択")) == TRUE) {
		int idx = m_combo_search_folder.FindStringExact(-1, dir);
		if(idx == CB_ERR) {
			m_search_folder = dir;
			UpdateData(FALSE);
		} else {
			m_combo_search_folder.SetCurSel(idx);
		}
	}

	CheckBtn();
}

void COgrepDlg::OnCancel() 
{
	// TODO: この位置に特別な後処理を追加してください。
	UpdateData(TRUE);
	AfxGetApp()->WriteProfileInt(_T("GREP"), _T("DISTINCT_LWR_UPR"), m_check_distinct_lwr_upr);
	AfxGetApp()->WriteProfileInt(_T("GREP"), _T("DISTINCT_WIDTH_ASCII"), m_check_distinct_width_ascii);
	AfxGetApp()->WriteProfileInt(_T("GREP"), _T("REGEXP"), m_check_regexp);
	AfxGetApp()->WriteProfileInt(_T("GREP"), _T("SEARCH_SUB_FOLDER"), m_check_search_sub_folder);
	
	CDialog::OnCancel();
}

void COgrepDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: この位置にメッセージ ハンドラ用のコードを追加してください
	if(::IsWindow(m_edit_ctrl.GetSafeHwnd()) == FALSE) return;

	CRect rect;
	GetClientRect(&rect);

	CRect button_rect;
	GetDlgItem(IDC_BTN_HIDDEN)->GetWindowRect(&button_rect);
	ScreenToClient(&button_rect);

	int width, height;
	width = rect.Width() - 20;
	height = rect.Height() - button_rect.bottom - 20;

	m_edit_ctrl.SetWindowPos(NULL, 10, button_rect.bottom + 15, width, height,
		SWP_NOZORDER | SWP_NOCOPYBITS);
}

LRESULT COgrepDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message) {
	case EC_WM_LBUTTONDBLCLK:
		if(lParam < 0 || lParam >= m_edit_data.get_row_cnt()) break;

        ShowSearchData(m_edit_data.get_row_buf(static_cast<int>(lParam)));
		break;
	}
	
	return CDialog::WindowProc(message, wParam, lParam);
}

void COgrepDlg::OpenFile(TCHAR *file_name)
{
	OeditOpenFile(file_name);
}

BOOL COgrepDlg::GetSearchData(TCHAR *data, TCHAR *file_name, int *row)
{
	TCHAR	*p = data;

	_tcscpy(file_name, _T(""));
	*row = 0;

	// ファイル名を取得
	if(_tcsncmp(p, _T("\\\\"), 2) == 0) {	// ネットワークコンピュータ
		p = _tcschr(p, ':');
		if(p == NULL) return FALSE;
	} else if(_tcsncmp(p, _T("."), 1) == 0) {
		p = _tcschr(p, ':');
		if(p == NULL) return FALSE;
	} else {
		p = _tcschr(p, ':');
		if(p == NULL) return FALSE;
		p++;
		p = _tcschr(p, ':');
		if(p == NULL) return FALSE;
	}

	for(; *p != '('; p--) {
		if(*p == '\0') return FALSE;
	}

	_tcsncpy(file_name, data, p - data);
	file_name[p - data] = '\0';

	// 行番号を取得
	TCHAR	*p1, *p2;
	TCHAR	row_chr[100];

	p1 = p;
	p1++;
	p2 = _tcschr(p1, ')');
	if(p2 == NULL) return FALSE;

	_tcsncpy(row_chr, p1, p2 - p1);
	row_chr[p2 - p1] = '\0';

	*row = _ttoi(row_chr) - 1;

	return TRUE;
}

void COgrepDlg::SetSearchDataOedit(HWND hwnd, int row)
{
	struct _set_search_data_st search_data;

	if(m_searched_data.m_search_text.GetLength() >= sizeof(search_data.search_text)) return;
	_tcscpy(search_data.search_text, m_searched_data.m_search_text.GetBuffer(0));

	search_data.distinct_lwr_upr = m_searched_data.m_check_distinct_lwr_upr;
	search_data.distinct_width_ascii = m_searched_data.m_check_distinct_width_ascii;
	search_data.regexp = m_searched_data.m_check_regexp;
	search_data.row = row;

	COPYDATASTRUCT	copy_data;
	copy_data.dwData = WM_COPY_DATA_SET_SEARCH_DATA;
	copy_data.cbData = sizeof(struct _set_search_data_st);
	copy_data.lpData = &search_data;

	::SendMessage(hwnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&copy_data);
}

void COgrepDlg::ShowSearchDataOedit(TCHAR *data)
{
	TCHAR	fname[_MAX_PATH];
	int		row;

	if(GetSearchData(data, fname, &row) == FALSE) return;

	CString file_name = GetFullPath(fname);

	HWND	hwnd;
	hwnd = OeditCheckFileIsOpen(file_name);
	if(hwnd != NULL) {
		::SetForegroundWindow(hwnd);
	} else {
		OpenFile(file_name.GetBuffer(0));
		hwnd = OeditCheckFileIsOpen(file_name);
		if(hwnd == NULL) return;
	}

	SetSearchDataOedit(hwnd, row);
}

void COgrepDlg::SetSearchDataOtbedit(HWND hwnd, int row, TCHAR *file_name)
{
	struct _set_search_data_st_otbedit search_data;

	_tcscpy(search_data.file_name, file_name);
	
	if(m_searched_data.m_search_text.GetLength() >= sizeof(search_data.search_text)) return;
	_tcscpy(search_data.search_text, m_searched_data.m_search_text.GetBuffer(0));

	search_data.distinct_lwr_upr = m_searched_data.m_check_distinct_lwr_upr;
	search_data.distinct_width_ascii = m_searched_data.m_check_distinct_width_ascii;
	search_data.regexp = m_searched_data.m_check_regexp;
	search_data.row = row;

	COPYDATASTRUCT	copy_data;
	copy_data.dwData = WM_COPY_DATA_SET_SEARCH_DATA;
	copy_data.cbData = sizeof(struct _set_search_data_st_otbedit);
	copy_data.lpData = &search_data;

	::SendMessage(hwnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&copy_data);

	::SetForegroundWindow(hwnd);
}

void COgrepDlg::ShowSearchDataOtbedit(TCHAR *data)
{
	TCHAR	fname[_MAX_PATH];
	int		row;

	if(GetSearchData(data, fname, &row) == FALSE) return;

	CString file_name = GetFullPath(fname);

	HWND	hwnd;
	hwnd = OtbeditCheckFileIsOpen(::GetTopWindow(NULL), file_name);
	if(hwnd == NULL) {
		OtbeditOpenFile(file_name.GetBuffer(0));
		hwnd = OtbeditCheckFileIsOpen(::GetTopWindow(NULL), file_name);
		if(hwnd == NULL) return;
	}
	if(hwnd == NULL) return;

	SetSearchDataOtbedit(hwnd, row, file_name.GetBuffer(0));
}

void COgrepDlg::ShowSearchData(TCHAR *data)
{
	if(m_oedit_mode) {
		ShowSearchDataOedit(data);
	} else {
		ShowSearchDataOtbedit(data);
	}
}

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	TCHAR	filename[MAX_PATH];
	CString	file_version;

	GetModuleFileName(AfxGetInstanceHandle(), filename, MAX_PATH);
	GetFileVersion(filename, &file_version);
	
	m_static_version = _T("OGrep Version ") + file_version;
	UpdateData(FALSE);
	
	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void COgrepDlg::OnDestroy() 
{
	SaveComboData(&m_combo_search_text, _T("search_text"));
	SaveComboData(&m_combo_file_type, _T("file_type"));
	SaveComboData(&m_combo_search_folder, _T("search_folder"));

	CDialog::OnDestroy();
}


void COgrepDlg::LoadDialogPosition()
{
	CWinApp *pApp = GetOWinApp();

	int left = pApp->GetProfileInt(_T("WINDOW"), _T("LEFT"), 0);
	int top = pApp->GetProfileInt(_T("WINDOW"), _T("TOP"), 0);
	int width = pApp->GetProfileInt(_T("WINDOW"), _T("WIDTH"), 0);
	int height = pApp->GetProfileInt(_T("WINDOW"), _T("HEIGHT"), 0);

	if(left < 0) left = 0;
	if(top < 0) top = 0;
	if(width < OGREP_DLG_MIN_WIDTH) width = OGREP_DLG_MIN_WIDTH;
	if(height < OGREP_DLG_MIN_HEIGHT) height = OGREP_DLG_MIN_HEIGHT;

	if(width == 0) return;

	if(left + width >= GetSystemMetrics(SM_CXSCREEN) || 
		top + height >= GetSystemMetrics(SM_CYSCREEN)) {
		// タスクバーに隠れないようにする
		RECT work_rect;
		if(SystemParametersInfo(SPI_GETWORKAREA, 0, &work_rect, 0)) {
			top = work_rect.top;
			left = work_rect.left;
		} else {
			top = 0;
			left = 0;
		}
	}

	if(width >= GetSystemMetrics(SM_CXSCREEN)) {
		width = GetSystemMetrics(SM_CXSCREEN);
	}
	if(height >= GetSystemMetrics(SM_CYSCREEN)) {
		height = GetSystemMetrics(SM_CYSCREEN);
	}

	SetWindowPos(NULL, left, top, width, height, SWP_NOZORDER);
}

void COgrepDlg::SaveDialogPosition()
{
	if(IsZoomed() || IsIconic()) return;

	CWinApp *pApp = GetOWinApp();

	CRect	rect;
	GetWindowRect(rect);

	pApp->WriteProfileInt(_T("WINDOW"), _T("LEFT"), rect.left);
	pApp->WriteProfileInt(_T("WINDOW"), _T("TOP"), rect.top);
	pApp->WriteProfileInt(_T("WINDOW"), _T("WIDTH"), rect.Width());
	pApp->WriteProfileInt(_T("WINDOW"), _T("HEIGHT"), rect.Height());
}

void COgrepDlg::OnClose() 
{
	SaveDialogPosition();
	
	CDialog::OnClose();
}

void COgrepDlg::OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI )
{
	lpMMI->ptMinTrackSize.x = OGREP_DLG_MIN_WIDTH;
	lpMMI->ptMinTrackSize.y = OGREP_DLG_MIN_HEIGHT;
}
