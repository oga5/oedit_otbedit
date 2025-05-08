// oextregDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "oextreg.h"
#include "oextregDlg.h"

//#include "afxisapi.h"
#include "afxpriv.h"

#include "file_winutil.h"
#include "fileutil.h"

#include "extreg.h"

#include "AddExtDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum connect_info_list {
	LIST_FILTER_EXT,
	LIST_FILE_TYPE_NAME,
	LIST_EXPLORER_RCLICK,
};

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

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	TCHAR	filename[MAX_PATH];
	CString	file_version;

	GetModuleFileName(AfxGetInstanceHandle(), filename, sizeof(filename));
	GetFileVersion(filename, &file_version);
	
	m_static_version = "oextreg Version " + file_version;
	UpdateData(FALSE);
	
	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COextregDlg ダイアログ

COextregDlg::COextregDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COextregDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COextregDlg)
	m_check_java = FALSE;
	m_check_txt = FALSE;
	m_ie_source_viewer = FALSE;
	//}}AFX_DATA_INIT
	// メモ: LoadIcon は Win32 の DestroyIcon のサブシーケンスを要求しません。
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hSmallIcon = (HICON)::LoadImage(AfxGetInstanceHandle(),
		MAKEINTRESOURCE(IDR_MAINFRAME),
		IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
}

void COextregDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COextregDlg)
	DDX_Control(pDX, IDC_BTN_UPDATE, m_btn_update);
	DDX_Control(pDX, IDC_BTN_DELETE, m_btn_delete);
	DDX_Control(pDX, IDC_EXT_LIST, m_ext_list);
	DDX_Check(pDX, IDC_CHECK_JAVA, m_check_java);
	DDX_Check(pDX, IDC_CHECK_TXT, m_check_txt);
	DDX_Check(pDX, IDC_IE_SOURCE_VIEWER, m_ie_source_viewer);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COextregDlg, CDialog)
	//{{AFX_MSG_MAP(COextregDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_EXT_LIST, OnItemchangedExtList)
	ON_BN_CLICKED(IDC_BTN_ADD, OnBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DELETE, OnBtnDelete)
	ON_BN_CLICKED(IDC_BTN_UPDATE, OnBtnUpdate)
	ON_BN_CLICKED(IDC_CHECK_JAVA, OnCheckJava)
	ON_BN_CLICKED(IDC_CHECK_TXT, OnCheckTxt)
	ON_BN_CLICKED(IDC_IE_SOURCE_VIEWER, OnIeSourceViewer)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COextregDlg メッセージ ハンドラ

BOOL COextregDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

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
	ListView_SetExtendedListViewStyle(m_ext_list.GetSafeHwnd(), LVS_EX_FULLROWSELECT);

	m_ext_list.InsertColumn(LIST_FILTER_EXT, _T("拡張子"), LVCFMT_LEFT, 50);
	m_ext_list.InsertColumn(LIST_FILE_TYPE_NAME, _T("ファイル種別"), LVCFMT_LEFT, 200);
	m_ext_list.InsertColumn(LIST_EXPLORER_RCLICK, _T("右クリック"), LVCFMT_LEFT, 80);

	m_ext_list.SetWindowText(OEXTREG_WINDOW_NAME);

	InitExtList();
	InitIESourceViewer();
	
	return TRUE;  // TRUE を返すとコントロールに設定したフォーカスは失われません。
}

void COextregDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void COextregDlg::OnPaint() 
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
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// システムは、ユーザーが最小化ウィンドウをドラッグしている間、
// カーソルを表示するためにここを呼び出します。
HCURSOR COextregDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hSmallIcon;
}

/////////////////////////////////////////////////////////////////////

CString MakeFileTypeId(TCHAR *ext)
{
	CString fileTypeId;

	if(_tcscmp(ext, _T("txt")) == 0 || _tcscmp(ext, _T(".txt")) == 0) {
		fileTypeId.Format(_T("%s.Document"), APP_NAME);
	} else if(ext[0] == '.') {
		fileTypeId.Format(_T("%s_%s.Document"), APP_NAME, ext + 1);
	} else {
		fileTypeId.Format(_T("%s_%s.Document"), APP_NAME, ext);
	}

	return fileTypeId;
}

void COextregDlg::RegisterShellFileTypes(TCHAR *ext, TCHAR *fileTypeName, BOOL b_explorer_rclick)
{
	CString	filterExt = MakeFilterExt(ext);
	CString fileTypeId = MakeFileTypeId(ext);

	CString fileTypeId2 = GetFileTypeId(ext);
	if(fileTypeId2 != "" && fileTypeId2 != fileTypeId) {
		CString		msg;
		msg.Format(_T("%sは，%sに関連付けされています。変更してよろしいですか？"),
			filterExt, GetFileTypeName(ext));
		if(MessageBox(msg, _T("確認"), MB_YESNO | MB_ICONQUESTION) == IDNO) return;
	}

	UnRegisterShellFileTypes(ext);

	CString	strTemp;
	CString strPathName;

	CString appFileName;
	appFileName.Format(_T("%s%s.exe"), GetShortAppPath(), APP_NAME);

	CString iconFileName = GetShortAppPath() + _T("oextreg.exe");
	int		iconIdx = 1;

	::RegisterShellFileTypes(ext, fileTypeId.GetBuffer(0), fileTypeName,
		b_explorer_rclick, appFileName.GetBuffer(0), iconFileName.GetBuffer(0), iconIdx, FALSE);
}

void COextregDlg::UnRegisterShellFileTypes(TCHAR *ext)
{
	::UnRegisterShellFileTypes(ext);
}

BOOL COextregDlg::CheckRegisterShellFileTypes(TCHAR *ext)
{
	return ::CheckRegisterShellFileTypes(ext, MakeFileTypeId(ext).GetBuffer(0));
}

void COextregDlg::ClearExtList()
{
	m_ext_list.DeleteAllItems();

	m_check_txt = FALSE;
	m_check_java = FALSE;
}

void COextregDlg::InitExtList()
{
	CWaitCursor		wait_cursor;

	HKEY			hk;
	TCHAR			subkey[1024];
	DWORD			subkey_size;
	int				idx = 0;
	FILETIME		ft;

	ClearExtList();

	if(RegOpenKeyEx(HKEY_CLASSES_ROOT, NULL, 0, KEY_QUERY_VALUE, &hk) != ERROR_SUCCESS) {
		return;
	}

	for(idx = 0;; idx++) {
		_tcscpy(subkey, _T(""));
		subkey_size = sizeof(subkey);
		if(RegEnumKeyEx(hk, idx, subkey, &subkey_size, NULL,
			NULL, NULL, &ft) != ERROR_SUCCESS) break;

		if(subkey[0] == '.' && CheckRegisterShellFileTypes(subkey)) {
			AddExtList(subkey);
		}
	}

	RegCloseKey(hk);

	UpdateData(FALSE);
	CheckBtn();
}

void COextregDlg::AddExtList(TCHAR *ext)
{
	CString			filterExt = MakeFilterExt(ext);
	CString			fileTypeId = GetFileTypeId(ext);
	CString			fileTypeName = GetFileTypeName(ext);
	BOOL			explorer_rclick = CheckExplorerRclick(ext);

	LV_ITEM		item;
	item.mask = LVIF_TEXT | LVIF_PARAM;
	item.iSubItem = 0;
	item.iItem = 0;
	item.pszText = filterExt.GetBuffer(0);
	item.lParam = NULL;
	item.iItem = m_ext_list.InsertItem(&item);

	m_ext_list.SetItemText(item.iItem, LIST_FILE_TYPE_NAME, fileTypeName);

	if(explorer_rclick) {
		m_ext_list.SetItemText(item.iItem, LIST_EXPLORER_RCLICK, _T("○"));
	}

	if(filterExt == _T(".txt")) m_check_txt = TRUE;
	else if(filterExt == _T(".java")) m_check_java = TRUE;
}


void COextregDlg::OnOK() 
{
	//CDialog::OnOK();
}

void COextregDlg::OnItemchangedExtList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください

	*pResult = 0;

	if((pNMListView->uChanged & LVIF_STATE) == 0) return;

	CheckBtn();
}

void COextregDlg::CheckBtn()
{
	int	selected_row;
	selected_row = m_ext_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1) {
		m_btn_update.EnableWindow(FALSE);
		m_btn_delete.EnableWindow(FALSE);
		return;
	}
	m_btn_update.EnableWindow(TRUE);
	m_btn_delete.EnableWindow(TRUE);
}

void COextregDlg::OnBtnAdd() 
{
	CAddExtDlg		dlg;
	
	if(dlg.DoModal() != IDOK) return;

	RegisterShellFileTypes(dlg.m_ext.GetBuffer(0), dlg.m_file_type_name.GetBuffer(0),
		dlg.m_explorer_rclick);	

	InitExtList();
	SelectList(dlg.m_ext.GetBuffer(0));
}

void COextregDlg::OnBtnDelete() 
{
	int	selected_row;
	selected_row = m_ext_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1) return;

	CString		ext;
	int			len = _MAX_PATH * 2;
	m_ext_list.GetItemText(selected_row, LIST_FILTER_EXT, ext.GetBuffer(len), len);
	ext.ReleaseBuffer();

	CString		msg;
	msg.Format(_T("%sを削除します。よろしいですか？"), ext);

	if(MessageBox(msg, _T("削除"), MB_YESNO | MB_ICONQUESTION) == IDNO) return;

	UnRegisterShellFileTypes(ext.GetBuffer(0));
	InitExtList();
}

void COextregDlg::OnBtnUpdate() 
{
	CAddExtDlg		dlg;
	
	int			selected_row;
	selected_row = m_ext_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1) return;

	int			len = _MAX_PATH * 2;
	m_ext_list.GetItemText(selected_row, LIST_FILTER_EXT, dlg.m_ext.GetBuffer(len), len);
	dlg.m_ext.ReleaseBuffer();

	dlg.m_file_type_name = GetFileTypeName(dlg.m_ext.GetBuffer(0));
	dlg.m_explorer_rclick = CheckExplorerRclick(dlg.m_ext.GetBuffer(0));

	dlg.m_update = TRUE;

	if(dlg.DoModal() != IDOK) return;

	RegisterShellFileTypes(dlg.m_ext.GetBuffer(0), dlg.m_file_type_name.GetBuffer(0),
		dlg.m_explorer_rclick);	

	InitExtList();
	SelectList(dlg.m_ext.GetBuffer(0));
}

void COextregDlg::SelectList(TCHAR *ext)
{
	CString			filterExt = MakeFilterExt(ext);
	int				len = _MAX_PATH * 2;
	CString			ext2;

	int		i;
	for(i = 0; i < m_ext_list.GetItemCount(); i++) {
		m_ext_list.GetItemText(i, LIST_FILTER_EXT, ext2.GetBuffer(len), len);
		ext2.ReleaseBuffer();
		if(filterExt.Compare(ext2) == 0) {
			m_ext_list.SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			m_ext_list.EnsureVisible(i, FALSE);
			m_ext_list.RedrawItems(i, i);
			return;
		}
	}
}

void COextregDlg::OnCheckJava() 
{
	UpdateData(TRUE);

	if(m_check_java) {
		RegisterShellFileTypes(_T(".java"), _T("Javaソースファイル"), FALSE);	
	} else {
		UnRegisterShellFileTypes(_T(".java"));
	}

	InitExtList();
	if(m_check_java) SelectList(_T(".java"));
}

void COextregDlg::OnCheckTxt() 
{
	UpdateData(TRUE);

	if(m_check_txt) {
		RegisterShellFileTypes(_T(".txt"), _T("テキストファイル"), TRUE);
	} else {
		UnRegisterShellFileTypes(_T(".txt"));
	}

	InitExtList();
	if(m_check_txt) SelectList(_T(".txt"));
}

void COextregDlg::OnIeSourceViewer() 
{
	UpdateData(TRUE);

	if(m_ie_source_viewer) {
		CString msg;
		msg.Format(_T("%sをInternetExplorerのソースを表示するエディタに設定します。\n")
			_T("よろしいですか？"), APP_NAME);
		if(MessageBox(msg, _T("確認"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
			CString appFileName;
			appFileName.Format(_T("%s%s.exe"), GetAppPath(), APP_NAME);
			SetIESourceViewer(appFileName.GetBuffer(0));
		} else {
			m_ie_source_viewer = FALSE;
			UpdateData(FALSE);
		}
	} else {
		UnSetIESourceViewer();
	}
}

void COextregDlg::InitIESourceViewer()
{
	CString appFileName;
	appFileName.Format(_T("%s%s.exe"), GetAppPath(), APP_NAME);

	m_ie_source_viewer = CheckIESourceViewer(appFileName.GetBuffer(0));

	UpdateData(FALSE);
}

