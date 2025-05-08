/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // PrintDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "PrintDlg.h"
#include "OWinApp.h"

#include "printeditdata.h"
#include "file_winutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

struct CPrintDlg::backup_data_st CPrintDlg::m_backup_data = {NULL, 0, NULL};

static void ErrorMessage(HWND hwnd)
{
	CString msg;
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0,
		msg.GetBuffer(1000), 1000, NULL);
	msg.ReleaseBuffer();
	::MessageBox(hwnd, msg.GetBuffer(0), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
}

static int get_default_printer(TCHAR *printer_name)
{
	PRINTDLG	pd;
	DEVMODE		*dm = NULL;
	DEVNAMES	*dn = NULL;

	memset(&pd, 0, sizeof(PRINTDLG));
	pd.lStructSize = sizeof(PRINTDLG);
	pd.Flags = PD_RETURNDEFAULT;

	if(PrintDlg(&pd) == FALSE) return 1;

	dm = (DEVMODE *)GlobalLock(pd.hDevMode);
	dn = (DEVNAMES *)GlobalLock(pd.hDevNames);

	_tcscpy(printer_name, dm->dmDeviceName);

	if(GlobalUnlock(pd.hDevMode) == FALSE && GetLastError() != NOERROR) {
		TRACE1("ERROR %d\n", GetLastError());
	}
	if(GlobalUnlock(pd.hDevNames) == FALSE && GetLastError() != NOERROR) {
		TRACE1("ERROR %d\n", GetLastError());
	}
	if(GlobalFree(pd.hDevMode) != NULL) {
		TRACE1("ERROR %d\n", GetLastError());
	}
	if(GlobalFree(pd.hDevNames) != NULL) {
		TRACE1("ERROR %d\n", GetLastError());
	}

	return 0;
}

static int dlg_combo_set_printer(CComboBox *combo, TCHAR *msg_buf)
{
	DWORD	dwNeeded = 0;
	DWORD	dwReturned = 0;
	LPBYTE	pbyteBuf = NULL;
	LPBYTE	pbyteSrch;
	PRINTER_INFO_1	*prninfo;
	DWORD	i;
	int		bufsize;

	bufsize = 1024 * 32;
	pbyteBuf = (LPBYTE)calloc(bufsize, 1);
	if(pbyteBuf == NULL) {
		return 1;
	}

	if(EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
		NULL, 1, pbyteBuf, bufsize,
		&dwNeeded, &dwReturned) == FALSE) {
		free(pbyteBuf);
		return 1;
	}

	pbyteSrch = pbyteBuf;
	for(i = 0; i < dwReturned; i++) {
		prninfo = (PRINTER_INFO_1 *)pbyteSrch;
		combo->AddString(prninfo->pName);
		pbyteSrch += sizeof(PRINTER_INFO_1);
	}

	free(pbyteBuf);

	return 0;
}

#define BYTEBUF_SIZE	1024 * 10

/////////////////////////////////////////////////////////////////////////////
// CPrintDlg ダイアログ


CPrintDlg::CPrintDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPrintDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPrintDlg)
	m_space_bottom = 0;
	m_space_left = 0;
	m_space_right = 0;
	m_space_top = 0;
	m_print_page = FALSE;
	m_print_title = FALSE;
	m_page_info = _T("");
	m_print_row_num = FALSE;
	m_font_size = _T("");
	m_font_face_name = _T("");
	m_print_date = FALSE;
	m_print_selected_area = FALSE;
	//}}AFX_DATA_INIT

	m_prninfo = NULL;
	printer_dc = NULL;
}

CPrintDlg::~CPrintDlg()
{
	DeletePrnInfo();
	DeletePrinterDC();
}

BOOL CPrintDlg::CreatePrinterDC()
{
	DeletePrinterDC();

	// DC作成
	printer_dc = CreateDC(NULL, m_prninfo->pPrinterName, NULL, m_prninfo->pDevMode);
	if(printer_dc == NULL) {
		ErrorMessage(GetSafeHwnd());
		return FALSE;
	}

	return TRUE;
}

void CPrintDlg::DeletePrinterDC()
{
	if(printer_dc != NULL) {
		DeleteDC(printer_dc);
		printer_dc = NULL;
	}
}

void CPrintDlg::DeletePrnInfo()
{
	if(m_prninfo != NULL) {
		free(m_prninfo);
		m_prninfo = NULL;
		m_prninfo_size = 0;
	}
}

BOOL CPrintDlg::CreatePrnInfo()
{
	HANDLE		hPrinter = NULL;
	DWORD		dwNeeded = 0;

	DeletePrnInfo();
	DeletePrinterDC();

	// プリンタの設定を取得
	if(::OpenPrinter(m_printer_name.GetBuffer(0), &hPrinter, NULL) == FALSE) {
		goto ERR1;
	}

	// 必要なメモリ量を取得
	GetPrinter(hPrinter, 2, 0, 0, &m_prninfo_size);

	// メモリ確保
	m_prninfo = (PRINTER_INFO_2 *)calloc(m_prninfo_size, 1);
	if(m_prninfo == NULL) {
		goto ERR1;
	}

	// プリンタ情報を取得
	if(GetPrinter(hPrinter, 2, (unsigned char *)m_prninfo, m_prninfo_size, &dwNeeded) == FALSE) {
		goto ERR1;
	}

	::ClosePrinter(hPrinter);

	// DC作成
	if(CreatePrinterDC() == FALSE) return FALSE;

	return TRUE;

ERR1:
	if(hPrinter != NULL) ::ClosePrinter(hPrinter);
	ErrorMessage(GetSafeHwnd());
	return FALSE;
}

void CPrintDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrintDlg)
	DDX_Control(pDX, IDC_PRINTER_SETUP, m_btn_printer_setup);
	DDX_Control(pDX, IDOK, m_ok);
	DDX_Control(pDX, IDC_COMBO_PRINTER, m_combo_printer);
	DDX_Text(pDX, IDC_EDIT_SPACE_BOTTOM, m_space_bottom);
	DDX_Text(pDX, IDC_EDIT_SPACE_LEFT, m_space_left);
	DDX_Text(pDX, IDC_EDIT_SPACE_RIGHT, m_space_right);
	DDX_Text(pDX, IDC_EDIT_SPACE_TOP, m_space_top);
	DDX_Check(pDX, IDC_CHECK_PRINT_PAGE, m_print_page);
	DDX_Check(pDX, IDC_CHECK_PRINT_TITLE, m_print_title);
	DDX_Text(pDX, IDC_STATIC_PAGE_INFO, m_page_info);
	DDX_Check(pDX, IDC_CHECK_PRINT_ROW_NUM, m_print_row_num);
	DDX_Text(pDX, IDC_STATIC_FONT_SIZE, m_font_size);
	DDX_Text(pDX, IDC_STATIC_FONT, m_font_face_name);
	DDX_Check(pDX, IDC_CHECK_PRINT_DATE, m_print_date);
	DDX_Check(pDX, IDC_CHECK_PRINT_SELECTED_AREA, m_print_selected_area);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrintDlg, CDialog)
	//{{AFX_MSG_MAP(CPrintDlg)
	ON_EN_CHANGE(IDC_EDIT_SPACE_BOTTOM, OnChangeEditSpaceBottom)
	ON_EN_CHANGE(IDC_EDIT_SPACE_LEFT, OnChangeEditSpaceLeft)
	ON_EN_CHANGE(IDC_EDIT_SPACE_RIGHT, OnChangeEditSpaceRight)
	ON_EN_CHANGE(IDC_EDIT_SPACE_TOP, OnChangeEditSpaceTop)
	ON_CBN_SELENDOK(IDC_COMBO_PRINTER, OnSelendokComboPrinter)
	ON_BN_CLICKED(IDC_CHECK_PRINT_ROW_NUM, OnCheckPrintRowNum)
	ON_BN_CLICKED(IDC_PRINTER_SETUP, OnPrinterSetup)
	ON_BN_CLICKED(IDC_SET_FONT, OnSetFont)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrintDlg メッセージ ハンドラ

BOOL CPrintDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	LoadPrintOption();

	m_font_size.Format(_T("%d"), m_font_point / 10);

	// TODO: この位置に初期化の補足処理を追加してください
	TCHAR	msg_buf[1024];
	if(dlg_combo_set_printer(&m_combo_printer, msg_buf) != 0) {
		MessageBox(msg_buf, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		EndDialog(IDCANCEL);
		return FALSE;
	}

	if(m_backup_data.printer_name != NULL) {
		m_printer_name = m_backup_data.printer_name;
	} else {
		m_printer_name = GetOWinApp()->GetIniFileString(_T("PRINT"), _T("PRINTER_NAME"), _T(""));
		if(m_printer_name.IsEmpty()) {
			TCHAR	default_printer[32] = _T("");
			get_default_printer(default_printer);
			m_printer_name = default_printer;
		}
	}

	UpdateData(FALSE);

	if(m_combo_printer.GetCurSel() != CB_ERR) {
		if(m_backup_data.printer_name != NULL && 
			m_printer_name.Compare(m_backup_data.printer_name) == 0 &&
			m_backup_data.prninfo != NULL) {
			m_prninfo = DuplicatePrnInfo(m_printer_name.GetBuffer(0),
				m_backup_data.prninfo, m_backup_data.prninfo_size);
			if(m_prninfo == NULL) {
				EndDialog(IDCANCEL);
				return TRUE;
			}
			m_prninfo_size = m_backup_data.prninfo_size;
			CreatePrinterDC();
		} else {
			CreatePrnInfo();
		}
	}

	VerifySpace();
	SetPageInfo();
	CheckBtn();

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CPrintDlg::OnOK()
{
	UpdateData(TRUE);

	VerifySpace();
	if(m_print_row_num == FALSE) m_row_num_digit = 0;

	SavePrintOption();
	CreateBackupData();

	COWinApp *pApp = GetOWinApp();
	pApp->WriteIniFileString(_T("PRINT"), _T("PRINTER_NAME"), m_printer_name);
	pApp->SaveIniFile();

	CDialog::OnOK();
}

void CPrintDlg::SetPageInfo()
{
	UpdateData(TRUE);

	if(printer_dc == NULL) {
		m_page_info.Format(_T(""));
		UpdateData(FALSE);
		return;
	}

	RECT	space;
	
	space.top = m_space_top;
	space.bottom = m_space_bottom;
	space.left = m_space_left;
	space.right = m_space_right;

	int line_len = -1;
	if(m_line_mode == EC_LINE_MODE_LEN) line_len = m_line_len;
	
	int rows_par_page, chars_par_page;

	int row_num_digit = m_row_num_digit;
	if(m_print_row_num == FALSE) row_num_digit = 0;

	get_print_info(printer_dc, m_font_face_name.GetBuffer(0), m_font_point, &space,
		line_len, row_num_digit, &rows_par_page, &chars_par_page);

	m_page_info.Format(_T("%d行, %d列"), rows_par_page, chars_par_page);

	UpdateData(FALSE);
}

void CPrintDlg::OnChangeEditSpaceBottom() 
{
	SetPageInfo();
}

void CPrintDlg::OnChangeEditSpaceLeft() 
{
	SetPageInfo();
}

void CPrintDlg::OnChangeEditSpaceRight() 
{
	SetPageInfo();
}

void CPrintDlg::OnChangeEditSpaceTop() 
{
	SetPageInfo();
}

void CPrintDlg::VerifySpace()
{
	UpdateData(TRUE);
	
	if(m_space_top < 0) m_space_top = 0;
	if(m_space_bottom < 0) m_space_top = 0;
	if(m_space_left < 0) m_space_top = 0;
	if(m_space_right < 0) m_space_top = 0;
	if(m_space_top > 50) m_space_top = 50;
	if(m_space_bottom > 50) m_space_top = 50;
	if(m_space_left > 50) m_space_top = 50;
	if(m_space_right > 50) m_space_top = 50;

	UpdateData(FALSE);
}

void CPrintDlg::OnSelendokComboPrinter() 
{
	UpdateData(TRUE);

	DeletePrnInfo();
	DeletePrinterDC();

	if(m_combo_printer.GetCurSel() != CB_ERR) {
		CreatePrnInfo();
	}

	SetPageInfo();
	CheckBtn();
}

void CPrintDlg::OnCheckPrintRowNum() 
{
	SetPageInfo();
}

void CPrintDlg::OnPrinterSetup() 
{
	if(m_prninfo == NULL || m_prninfo->pDevMode == NULL) return;

	HANDLE		hPrinter = NULL;

	// プリンタの設定を取得
	if(::OpenPrinter(m_printer_name.GetBuffer(0), &hPrinter, NULL) == FALSE) {
		ErrorMessage(GetSafeHwnd());
		goto ERR1;
	}

	if(DocumentProperties(GetSafeHwnd(), hPrinter, m_printer_name.GetBuffer(0),
		m_prninfo->pDevMode, m_prninfo->pDevMode, DM_IN_PROMPT | DM_OUT_BUFFER | DM_IN_BUFFER) >= 0) {

		// プリンタDCを作り直す
		if(CreatePrinterDC() == FALSE) goto ERR1;

		SetPageInfo();
	}

	::ClosePrinter(hPrinter);

	return;

ERR1:
	if(hPrinter != NULL) ::ClosePrinter(hPrinter);
	return;
}

void CPrintDlg::OnSetFont() 
{
	CFontDialog		fontdlg;
	CFont			font;

	// ダイアログ初期化
	fontdlg.m_cf.Flags &= ~CF_EFFECTS;
	fontdlg.m_cf.Flags |= CF_INITTOLOGFONTSTRUCT;
	
	font.CreatePointFont(m_font_point, m_font_face_name);
	font.GetLogFont(fontdlg.m_cf.lpLogFont);

	// ダイアログ表示
	if(fontdlg.DoModal() != IDOK) return;

	UpdateData(TRUE);
	m_font_face_name = fontdlg.GetFaceName();
	m_font_point = fontdlg.GetSize();
	m_font_size.Format(_T("%d"), m_font_point / 10);
	UpdateData(FALSE);

	SetPageInfo();
}

void CPrintDlg::LoadPrintOption()
{
	COWinApp *pApp = GetOWinApp();

	m_space_top = pApp->GetIniFileInt(_T("PRINT"), _T("SPACE_TOP"), 20);
	m_space_bottom = pApp->GetIniFileInt(_T("PRINT"), _T("SPACE_BOTTOM"), 15);
	m_space_left = pApp->GetIniFileInt(_T("PRINT"), _T("SPACE_LEFT"), 15);
	m_space_right = pApp->GetIniFileInt(_T("PRINT"), _T("SPACE_RIGHT"), 15);
	m_print_title = pApp->GetIniFileInt(_T("PRINT"), _T("PRINT_TITLE"), 1);
	m_print_page = pApp->GetIniFileInt(_T("PRINT"), _T("PRINT_PAGE"), FALSE);
	m_print_row_num = pApp->GetIniFileInt(_T("PRINT"), _T("PRINT_ROW_NUM"), FALSE);
	m_print_date = pApp->GetIniFileInt(_T("PRINT"), _T("PRINT_DATE"), FALSE);

	m_font_face_name = pApp->GetIniFileString(_T("PRINT"), _T("FONT_FACE_NAME"), _T("ＭＳ ゴシック"));
	m_font_point = pApp->GetIniFileInt(_T("PRINT"), _T("FONT_POINT"), 110);
}

void CPrintDlg::SavePrintOption()
{
	COWinApp *pApp = GetOWinApp();

	pApp->WriteIniFileInt(_T("PRINT"), _T("SPACE_TOP"), m_space_top);
	pApp->WriteIniFileInt(_T("PRINT"), _T("SPACE_BOTTOM"), m_space_bottom);
	pApp->WriteIniFileInt(_T("PRINT"), _T("SPACE_LEFT"), m_space_left);
	pApp->WriteIniFileInt(_T("PRINT"), _T("SPACE_RIGHT"), m_space_right);
	pApp->WriteIniFileInt(_T("PRINT"), _T("PRINT_TITLE"), m_print_title);
	pApp->WriteIniFileInt(_T("PRINT"), _T("PRINT_PAGE"), m_print_page);
	pApp->WriteIniFileInt(_T("PRINT"), _T("PRINT_ROW_NUM"), m_print_row_num);
	pApp->WriteIniFileInt(_T("PRINT"), _T("PRINT_DATE"), m_print_date);

	pApp->WriteIniFileString(_T("PRINT"), _T("FONT_FACE_NAME"), m_font_face_name);
	pApp->WriteIniFileInt(_T("PRINT"), _T("FONT_POINT"), m_font_point);
	
	pApp->SaveIniFile();
}

BOOL CPrintDlg::UpdateData(BOOL bSaveAndValidate)
{
	if(bSaveAndValidate) {
		int idx = m_combo_printer.GetCurSel();
		if(idx != CB_ERR) {
			m_combo_printer.GetLBText(idx, m_printer_name);
		} else {
			m_printer_name = "";
		}
	} else {
		int idx = m_combo_printer.FindStringExact(-1, m_printer_name);
		if(idx != CB_ERR) {
			m_combo_printer.SetCurSel(idx);
		}
	}

	return CDialog::UpdateData(bSaveAndValidate);
}

PRINTER_INFO_2 *CPrintDlg::DuplicatePrnInfo(TCHAR *printer_name, PRINTER_INFO_2 *prninfo, DWORD prninfo_size)
{
	HANDLE		hPrinter = NULL;
	DWORD		dwNeeded = 0;
	PRINTER_INFO_2	*p = NULL;

	p = (PRINTER_INFO_2 *)calloc(prninfo_size, 1);
	if(p == NULL) goto ERR1;
	
	// プリンタの設定をコピー
	if(::OpenPrinter(printer_name, &hPrinter, NULL) == FALSE) goto ERR1;

	if(GetPrinter(hPrinter, 2, (LPBYTE)p, prninfo_size, &dwNeeded) == FALSE) {
		goto ERR1;
	}
	if(DocumentProperties(GetSafeHwnd(), hPrinter, printer_name,
		p->pDevMode, prninfo->pDevMode, DM_OUT_BUFFER | DM_IN_BUFFER) != IDOK) {
		goto ERR1;
	}
	::ClosePrinter(hPrinter);

	return p;

ERR1:
	if(hPrinter != NULL) ::ClosePrinter(hPrinter);
	ErrorMessage(GetSafeHwnd());
	return NULL;
}

BOOL CPrintDlg::CreateBackupData()
{
	DeleteBackupData();

	m_backup_data.prninfo = DuplicatePrnInfo(m_printer_name.GetBuffer(0), m_prninfo, m_prninfo_size);
	if(m_backup_data.prninfo == NULL) goto ERR1;
	m_backup_data.prninfo_size = m_prninfo_size;

	m_backup_data.printer_name = _tcsdup(m_printer_name.GetBuffer(0));
	if(m_backup_data.printer_name == NULL) {
		ErrorMessage(GetSafeHwnd());
		goto ERR1;
	}

	return TRUE;

ERR1:
	DeleteBackupData();
	return FALSE;
}

void CPrintDlg::DeleteBackupData()
{
	if(m_backup_data.prninfo != NULL) {
		free(m_backup_data.prninfo);
		m_backup_data.prninfo = NULL;
		m_backup_data.prninfo_size = 0;
	}
	if(m_backup_data.printer_name != NULL) {
		free(m_backup_data.printer_name);
		m_backup_data.printer_name = NULL;
	}
}

void CPrintDlg::CheckBtn()
{
	if(m_combo_printer.GetCurSel() != CB_ERR) {
		m_ok.EnableWindow(TRUE);
	} else {
		m_ok.EnableWindow(FALSE);
	}

	if(m_prninfo != NULL && m_prninfo->pDevMode != NULL && printer_dc != NULL) {
		m_btn_printer_setup.EnableWindow(TRUE);
	} else {
		m_btn_printer_setup.EnableWindow(FALSE);
	}
}

