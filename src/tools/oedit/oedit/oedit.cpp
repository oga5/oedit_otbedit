/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // oedit.cpp : �A�v���P�[�V�����p�N���X�̋@�\��`���s���܂��B
//

#include "stdafx.h"
#include <locale.h>

#define MAIN_PROG
#include "oedit.h"

#include "MainFrm.h"
#include "oeditDoc.h"
#include "oeditView.h"

#include "file_winutil.h"
#include "fileutil.h"

//#include "afxisapi.h"
#include "afxpriv.h"

#include "kbmacro.h"
#include "filenames.h"
#include "ExtFileDialog.h"
#include "CommonUtil.h"
#include "FileBookMarkDlg.h"

#include "imm.h"
#include "scm_macro.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static const char *THIS_FILE = __FILE__;
#endif

#if defined(UNICODE) || defined(_UNICODE)
#pragma comment(linker, "/entry:\"wWinMainCRTStartup\"")
#endif

/////////////////////////////////////////////////////////////////////////////
// COeditApp

BEGIN_MESSAGE_MAP(COeditApp, COWinApp)
	//{{AFX_MSG_MAP(COeditApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_OPTION, OnOption)
	ON_COMMAND(ID_SET_FONT, OnSetFont)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_NEW_WINDOW, OnNewWindow)
	ON_COMMAND(ID_EXT_REG, OnExtReg)
	ON_COMMAND(ID_PLAY_KBMACRO, OnPlayKbmacro)
	ON_COMMAND(ID_REC_KBMACRO, OnRecKbmacro)
	ON_UPDATE_COMMAND_UI(ID_REC_KBMACRO, OnUpdateRecKbmacro)
	ON_UPDATE_COMMAND_UI(ID_PLAY_KBMACRO, OnUpdatePlayKbmacro)
	ON_COMMAND(ID_KBMACRO_SAVE, OnKbmacroSave)
	ON_UPDATE_COMMAND_UI(ID_KBMACRO_SAVE, OnUpdateKbmacroSave)
	ON_COMMAND(ID_KBMACRO_LOAD, OnKbmacroLoad)
	ON_COMMAND(ID_KBMACRO_PLAY, OnKbmacroPlay)
	ON_COMMAND(ID_TOGGLE_SHOW_ROW_NUM, OnToggleShowRowNum)
	ON_COMMAND(ID_TOGGLE_SHOW_COL_NUM, OnToggleShowColNum)
	ON_COMMAND(ID_SHOW_REGEXP_HELP, OnShowRegexpHelp)
	ON_COMMAND(ID_SCM_LOAD_FILE, OnScmLoadFile)
	ON_COMMAND(ID_SHOW_SCHEME_HELP, OnShowSchemeHelp)
	ON_COMMAND(ID_SHOW_OEDITMACRO_HELP, OnShowOeditmacroHelp)
	ON_COMMAND(ID_SCM_EVAL_CLIPBOARD, OnScmEvalClipboard)
	ON_COMMAND(ID_FONT_SIZE_LARGE, OnFontSizeLarge)
	ON_COMMAND(ID_FONT_SIZE_SMALL, OnFontSizeSmall)
	//}}AFX_MSG_MAP
	// �W���̃t�@�C����{�h�L�������g �R�}���h
	ON_COMMAND(ID_FILE_NEW, COWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, COWinApp::OnFileOpen)
	// �W���̈���Z�b�g�A�b�v �R�}���h
	ON_COMMAND(ID_FILE_PRINT_SETUP, COWinApp::OnFilePrintSetup)
	//
	ON_COMMAND_EX_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE16, OnOpenRecentFile)
	ON_COMMAND_RANGE(ID_EDIT_MODE_1, ID_EDIT_MODE_1 + MAX_LANGUAGE_CNT, OnEditMode)
	ON_UPDATE_COMMAND_UI_RANGE(ID_EDIT_MODE_1, ID_EDIT_MODE_1 + MAX_LANGUAGE_CNT, OnUpdateEditMode)
	ON_COMMAND(ID_FILE_BOOKMARK, &COeditApp::OnFileBookmark)
	ON_COMMAND(ID_SHOW_CREDITS, &COeditApp::OnShowCredits)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COeditApp �N���X�̍\�z

COeditApp::COeditApp()
{
	// TODO: ���̈ʒu�ɍ\�z�p�R�[�h��ǉ����Ă��������B
	// ������ InitInstance ���̏d�v�ȏ��������������ׂċL�q���Ă��������B
	// �O���[�o���ϐ��̏�����
	g_open_kanji_code = UnknownKanjiCode;
	g_full_screen_mode = FALSE;
	g_sc = NULL;
}

COeditApp::~COeditApp()
{
	ClearKBMacroBuffer();
}

/////////////////////////////////////////////////////////////////////////////
// �B��� COeditApp �I�u�W�F�N�g

COeditApp theApp;

/////////////////////////////////////////////////////////////////////////////
// COeditApp �N���X�̏�����

BOOL COeditApp::InitInstance()
{
	setlocale(LC_ALL, "Japanese");

	// �W���I�ȏ���������
	// ���������̋@�\���g�p�����A���s�t�@�C���̃T�C�Y��������
	// ��������Έȉ��̓���̏��������[�`���̒�����s�K�v�Ȃ���
	// ���폜���Ă��������B
	if(!AfxOleInit()) {
		MessageBox(NULL, _T("OLE�̏������Ɏ��s���܂����B"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}

	// ���L��������������
	g_share_data.Init();

#ifdef _AFXDLL
	Enable3dControls();		// ���L DLL �̒��� MFC ���g�p����ꍇ�ɂ͂������Ăяo���Ă��������B
#else
	// �Â�MFC�p�̏����B���݂͕s�v
	//Enable3dControlsStatic();	// MFC �ƐÓI�Ƀ����N���Ă���ꍇ�ɂ͂������Ăяo���Ă��������B
#endif

	// scheme�}�N��������������
	if(init_scm_macro(&g_sc, _T("oedit.scm")) == FALSE) return FALSE;

	// 
	TCHAR ini_file_name[MAX_PATH];
	TCHAR local_ini_file_name[MAX_PATH];
	_stprintf(ini_file_name, _T("%soedit.ini"), GetAppPath().GetBuffer(0));
	_stprintf(local_ini_file_name, _T("%soedit_local.ini"), GetAppPath().GetBuffer(0));

	if(!is_file_exist(ini_file_name)) {
		if(!RegistryToIniFile(_T("OGAWA"), _T("oedit"), ini_file_name, local_ini_file_name)) {
			MessageBox(NULL, _T("���W�X�g������INI�t�@�C���ւ̈ڍs�Ɏ��s���܂���"), 
				_T("Error"), MB_ICONEXCLAMATION | MB_OK);
		}
	}
	if(is_file_exist(ini_file_name)) {
		SetIniFileName(ini_file_name);
		if(!LoadIniFile()) {
			MessageBox(NULL, _T("INI�t�@�C������ݒ���擾�ł��܂���ł����B\n�f�t�H���g�̐ݒ�ŋN�����܂��B"), 
				_T("Error"), MB_ICONEXCLAMATION | MB_OK);
		}
	}

	// MFC���g���ݒ��local_ini_file_name�ɕۑ�����
	SetLocalIniFileName(local_ini_file_name);

	{
		CString base_dir;
		base_dir.Format(_T("%sdata\\"), GetAppPath().GetBuffer(0));
		TCHAR msg_buf[256];
		if(!g_lang_setting_list.InitList(base_dir, ID_EDIT_MODE_1, MAX_LANGUAGE_CNT, msg_buf)) {
			MessageBox(NULL, msg_buf, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
			return FALSE;
		}
	}

	// �I�v�V�������̃��[�h
	LoadOption();
	LoadEditMode();
	LoadFontOption();

	LoadStdProfileSettings(g_option.max_mru);  // �W���� INI �t�@�C���̃I�v�V���������[�ނ��܂� (MRU ���܂�)

	// �t�H���g�쐬
	CreateFont();

	// �A�v���P�[�V�����p�̃h�L�������g �e���v���[�g��o�^���܂��B�h�L�������g �e���v���[�g
	//  �̓h�L�������g�A�t���[�� �E�B���h�E�ƃr���[���������邽�߂ɋ@�\���܂��B

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(COeditDoc),
		RUNTIME_CLASS(CMainFrame),       // ���C�� SDI �t���[�� �E�B���h�E
		RUNTIME_CLASS(COeditView));
	AddDocTemplate(pDocTemplate);

	// DDE�Afile open �ȂǕW���̃V�F�� �R�}���h�̃R�}���h���C������͂��܂��B
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	CBootParameter file_names;
	if(OpenFiles(file_names, cmdInfo, m_lpCmdLine) == FALSE) return FALSE;

	ProcessBootOption(file_names);

	// �R�}���h���C���Ńf�B�X�p�b�` �R�}���h���w�肵�܂��B
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// �t�@�C���̃h���b�v���󂯎��
	m_pMainWnd->DragAcceptFiles();

	// ���C�� �E�B���h�E�����������ꂽ�̂ŁA�\���ƍX�V���s���܂��B
	if(GetIniFileInt(_T("WINDOW"), _T("ZOOMED"), FALSE) != FALSE) {
		m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
	} else {
		m_pMainWnd->ShowWindow(SW_SHOW);
	}
	m_pMainWnd->UpdateWindow();

	// �A�N�Z�����[�^������������
	((CMainFrame *)m_pMainWnd)->InitAccelerator();

	if(g_option.boot_on_ime) {
		HIMC hIMC = ::ImmGetContext(m_pMainWnd->GetSafeHwnd());
		::ImmSetOpenStatus(hIMC, TRUE); // ON
		::ImmReleaseContext(m_pMainWnd->GetSafeHwnd(), hIMC);
	}

	return TRUE;
}

void COeditApp::LoadEditMode()
{
	g_option.text_editor.line_mode = GetIniFileInt(_T("TEXT_EDITOR"), _T("LINE_MODE"), EC_LINE_MODE_NORMAL);
	CString edit_mode_str = GetIniFileString(_T("APPLICATION"), _T("EDIT_MODE_STR"), _T("TEXT"));
	int edit_mode = g_lang_setting_list.GetSettingIdx(edit_mode_str);
	if(edit_mode == -1) edit_mode = 0;
	g_option.edit_mode = edit_mode;
}

void COeditApp::LoadFontOption()
{
	g_option.font.face_name = GetIniFileString(_T("FONT"), _T("FACENAME"), _T("Terminal"));
	g_option.font.point = GetIniFileInt(_T("FONT"), _T("POINT"), 140);
	g_option.font.weight = GetIniFileInt(_T("FONT"), _T("WEIGHT"), FW_NORMAL);
	g_option.font.italic = GetIniFileInt(_T("FONT"), _T("ITALIC"), 0);
}

void COeditApp::SaveEditMode()
{
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("LINE_MODE"), g_option.text_editor.line_mode);
	CLanguageSetting *s = g_lang_setting_list.GetSetting(g_option.edit_mode);
	if(s == NULL) return;
	WriteIniFileString(_T("APPLICATION"), _T("EDIT_MODE_STR"), s->GetName());
}

void COeditApp::SaveFontOption()
{
	WriteIniFileString(_T("FONT"), _T("FACENAME"), g_option.font.face_name);
	WriteIniFileInt(_T("FONT"), _T("POINT"), g_option.font.point);
	WriteIniFileInt(_T("FONT"), _T("WEIGHT"), g_option.font.weight);
	WriteIniFileInt(_T("FONT"), _T("ITALIC"), g_option.font.italic);
	SaveIniFile();
}

void COeditApp::SaveOtherSettings()
{
	WriteIniFileString(_T("APPLICATION"), _T("KB_MACRO_DIR"), g_option.kb_macro_dir);
}

/////////////////////////////////////////////////////////////////////////////
// �A�v���P�[�V�����̃o�[�W�������Ŏg���� CAboutDlg �_�C�A���O

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �_�C�A���O �f�[�^
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CStatic	m_static_url;
	CString	m_static_version;
	//}}AFX_DATA

	// ClassWizard ���z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �̃T�|�[�g
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	BOOL LinkHitTest(CPoint point);
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
	DDX_Control(pDX, IDC_STATIC_URL, m_static_url);
	DDX_Text(pDX, IDC_STATIC_VERSION, m_static_version);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_WM_CTLCOLOR()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// �_�C�A���O�����s���邽�߂̃A�v���P�[�V���� �R�}���h
void COeditApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// COeditApp ���b�Z�[�W �n���h��

int COeditApp::ExitInstance() 
{
	SaveEditMode();
	SaveOtherSettings();
	SaveIniFile();
	clear_scm_macro(&g_sc);

	return COWinApp::ExitInstance();
}

BOOL COeditApp::CreateFont()
{
	// �t�H���g���쐬

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
	lf.lfHeight = g_option.font.point;
	lf.lfWeight = g_option.font.weight;
	lf.lfItalic = g_option.font.italic;
	lf.lfCharSet = DEFAULT_CHARSET;
	_tcscpy(lf.lfFaceName, g_option.font.face_name);

	return g_font.CreatePointFontIndirect(&lf);
}

void COeditApp::OnSetFont() 
{
	CFontDialog		fontdlg;

	// �_�C�A���O������
	fontdlg.m_cf.Flags &= ~CF_EFFECTS;
	fontdlg.m_cf.Flags |= CF_INITTOLOGFONTSTRUCT;
	g_font.GetLogFont(fontdlg.m_cf.lpLogFont);

	// �_�C�A���O�\��
	if(fontdlg.DoModal() != IDOK) return;

	g_option.font.face_name = fontdlg.GetFaceName();
	g_option.font.point = fontdlg.GetSize();
	g_option.font.weight = fontdlg.GetWeight();
	g_option.font.italic = fontdlg.IsItalic();

	g_font.DeleteObject();
	CreateFont();

	UpdateAllDocViews(NULL, UPD_FONT, &g_font);
	SaveFontOption();
}

void COeditApp::ChangeFontSize(BOOL b_large)
{
	int new_size;
	CDC *pdc = AfxGetMainWnd()->GetDC();
	int cur_size = abs(g_option.font.point) / 10;
	if(b_large) {
		new_size = GetLargeFontSize(pdc->GetSafeHdc(), g_option.font.face_name, cur_size);
	} else {
		new_size = GetSmallFontSize(pdc->GetSafeHdc(), g_option.font.face_name, cur_size);
	}
	AfxGetMainWnd()->ReleaseDC(pdc);

	if(g_option.font.point == new_size * 10) return;

	TRACE(_T("ChangeFontSize: %d -> %d\n"), cur_size, new_size); 

	g_option.font.point = new_size * 10;
	g_font.DeleteObject();
	CreateFont();
	UpdateAllDocViews(NULL, UPD_FONT, &g_font);
	SaveFontOption();
}

void COeditApp::OnFontSizeLarge() 
{
	ChangeFontSize(TRUE);
}

void COeditApp::OnFontSizeSmall() 
{
	ChangeFontSize(FALSE);
}

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	TCHAR	filename[MAX_PATH];
	CString	file_version;

	GetModuleFileName(AfxGetInstanceHandle(), filename, MAX_PATH);
	GetFileVersion(filename, &file_version);
	
	m_static_version = _T("OEdit Version ") + file_version;
	UpdateData(FALSE);
	
	return TRUE;  // �R���g���[���Ƀt�H�[�J�X��ݒ肵�Ȃ��Ƃ��A�߂�l�� TRUE �ƂȂ�܂�
	              // ��O: OCX �v���p�e�B �y�[�W�̖߂�l�� FALSE �ƂȂ�܂�
}

void COeditApp::OnFileOpen() 
{
	CString newName;
	int kanji_code = g_open_kanji_code;
	if (!DoPromptFileName(newName, AFX_IDS_OPENFILE, OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, TRUE, NULL,
		&kanji_code, NULL)) {
		return; // open cancelled
	}

	// ���Ƀt�@�C�����J���Ă���ꍇ�C�ǂݒ���
	{
		TCHAR file_name[MAX_PATH];
		if(GetLongPath(newName.GetBuffer(0), file_name) == FALSE) return;
		TCHAR doc_file_name[MAX_PATH];

		POSITION	doc_tmpl_pos = GetFirstDocTemplatePosition();
		for(; doc_tmpl_pos != NULL;) {
			CDocTemplate	*doc_tmpl = GetNextDocTemplate(doc_tmpl_pos);
			POSITION doc_pos = doc_tmpl->GetFirstDocPosition();
			for(; doc_pos != NULL; ) {
				CDocument *pDoc = doc_tmpl->GetNextDoc(doc_pos);
				CString str_doc_file_name = pDoc->GetPathName();
				if(GetLongPath(str_doc_file_name.GetBuffer(0), doc_file_name) == FALSE) continue;

				if(_tcscmp(doc_file_name, file_name) == 0) {
					((COeditDoc *)pDoc)->ReloadFile(kanji_code);
					return;
				}
			}
		}
	}

	int back_kanji_code = g_open_kanji_code;
	g_open_kanji_code = kanji_code;
	OpenDocumentFile(newName);
	g_open_kanji_code = back_kanji_code;
}

static void AppendFilterSuffix2(CString &filter, OPENFILENAME& ofn,
	CString fileter_name, CString ext_name)
{
	// add to filter
	filter += fileter_name;
	ASSERT(!filter.IsEmpty());  // must have a file type name
	filter += (TCHAR)'\0';  // next string please
	filter += (TCHAR)'*';
	filter += ext_name;
	filter += (TCHAR)'\0';  // next string please
	ofn.nMaxCustFilter++;
}

BOOL COeditApp::DoPromptFileName(CString& fileName, UINT nIDSTitle,
			DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate,
			int *kanji_code, int *line_type)
{
	TCHAR	cur_dir[_MAX_PATH];
	GetCurrentDirectory(_MAX_PATH, cur_dir);

	CExtFileDialog dlgFile(bOpenFileDialog);

	CString title;
	VERIFY(title.LoadString(nIDSTitle));

	dlgFile.m_b_open_mode = bOpenFileDialog;
	if(bOpenFileDialog) {
		dlgFile.m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_EXT_FILEDLG_OPEN);
		if(kanji_code != NULL) dlgFile.m_kanji_code = *kanji_code;
	} else {
		dlgFile.m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_EXT_FILEDLG_SAVE);
		if(kanji_code != NULL) dlgFile.m_kanji_code = *kanji_code;
		if(line_type != NULL) dlgFile.m_line_type = *line_type;
	}
	dlgFile.m_ofn.Flags |= (lFlags | OFN_ENABLETEMPLATE);
	dlgFile.m_ofn.lpstrInitialDir = cur_dir;

	CString strFilter = g_lang_setting_list.GetFilterSuffix(dlgFile.m_ofn, AFX_IDS_ALLFILTER);
	dlgFile.m_ofn.lpstrFilter = strFilter;

	dlgFile.m_ofn.nFilterIndex = 0;
	if(!bOpenFileDialog) {
		// �ۑ��̂Ƃ�
		dlgFile.m_ofn.nFilterIndex = g_option.edit_mode + 2;
	}

	dlgFile.m_ofn.lpstrTitle = title;
	dlgFile.m_ofn.lpstrFile = fileName.GetBuffer(_MAX_PATH);

	dlgFile.m_ofn.lpstrDefExt = _T("");

	BOOL bResult = dlgFile.DoModal() == IDOK ? TRUE : FALSE;
	fileName.ReleaseBuffer();

	if(bResult) {
		if(kanji_code != NULL) *kanji_code = dlgFile.m_kanji_code;
		if(line_type != NULL) *line_type = dlgFile.m_line_type;
	}

	return bResult;
}

HBRUSH CAboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: ���̈ʒu�� DC �̃A�g���r���[�g��ύX���Ă�������
	int id = pWnd->GetDlgCtrlID();
	if(id == IDC_STATIC_URL) {
		pDC->SetTextColor(RGB(0, 0, 255));
	}
	
	// TODO: �f�t�H���g�̃u���V���]�݂̂��̂łȂ��ꍇ�ɂ́A�Ⴄ�u���V��Ԃ��Ă�������
	return hbr;
}

BOOL CAboutDlg::LinkHitTest(CPoint point)
{
	if(m_static_url.IsWindowVisible() == FALSE) return FALSE;

	RECT	rect;
	m_static_url.GetWindowRect(&rect);
	ScreenToClient(&rect);

	if((rect.top <= point.y && rect.bottom >= point.y) &&
		(rect.left <= point.x && rect.right >= point.x)) {
		return TRUE;
	}

	return FALSE;
}

void CAboutDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: ���̈ʒu�Ƀ��b�Z�[�W �n���h���p�̃R�[�h��ǉ����邩�܂��̓f�t�H���g�̏������Ăяo���Ă�������
	if(LinkHitTest(point) == TRUE) {
		if(CEditCtrl::m_link_cursor != NULL) {
			SetCursor(CEditCtrl::m_link_cursor);
		}
	}

	CDialog::OnMouseMove(nFlags, point);
}

void CAboutDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if(LinkHitTest(point) == TRUE) {
		CString url;
		m_static_url.GetWindowText(url);
		ShellExecute(NULL, NULL, url, NULL, NULL, SW_SHOWNORMAL);
	}
	
	CDialog::OnLButtonDown(nFlags, point);
}

void COeditApp::SetEditMode(int edit_mode)
{
	TCHAR		msg_buf[1024];

	if(edit_mode < 0 || edit_mode >= g_lang_setting_list.GetCount()) {
		g_option.edit_mode = 0;
		return;
	}

	g_option.edit_mode = edit_mode;

	if(!g_lang_setting_list.GetSetting(edit_mode)->LoadSetting(&g_str_token, msg_buf)) {
		AfxMessageBox(msg_buf, MB_OK);
	}
}

void COeditApp::OnNewWindow() 
{
	CString					cmd;
	PROCESS_INFORMATION		pi;
	STARTUPINFO				si;

	((CMainFrame *)AfxGetMainWnd())->SetNextWindowPos();

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	TCHAR path_buffer[_MAX_PATH];
	if(GetModuleFileName(NULL, path_buffer, _MAX_PATH) == 0) {
		AfxMessageBox(_T("�V�����E�B���h�E�̍쐬�Ɏ��s���܂���"), MB_OK);
		return;
	}

	cmd = path_buffer;

	if(CreateProcess(NULL, cmd.GetBuffer(0), NULL, NULL, FALSE,
		0, NULL, NULL, &si, &pi) == FALSE) {
		AfxMessageBox(_T("�V�����E�B���h�E�̍쐬�Ɏ��s���܂���"), MB_OK);
		return;
	}

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}

void COeditApp::SetFileBootOption(CBootParameter &file_names, int idx)
{
	int	row;
	if(file_names.GetFileNameOption(idx)->GetOption(_T("r")).IsEmpty()) {
		row = -1;
	} else {
		row = _ttoi(file_names.GetFileNameOption(idx)->GetOption(_T("r")));
	}
	CString mode = file_names.GetFileNameOption(idx)->GetOption(_T("m"));

	SetBootOption(file_names.GetFileName(idx).GetBuffer(0), row, mode);
}

BOOL COeditApp::OpenFiles(CBootParameter &file_names, CCommandLineInfo &cmdInfo, TCHAR *cmd_line)
{
	if(_tcslen(cmd_line) == 0) {
		// �J�����g�f�B���N�g����ݒ�
		SetCurrentDirectory(g_option.initial_dir);
		return TRUE;
	}

	TCHAR msg_buf[1024];
	if(file_names.InitParameter(cmd_line, msg_buf) == FALSE) {
		AfxMessageBox(msg_buf, MB_OK);
		return FALSE;
	}

	if(file_names.GetFileCnt() > 0) {
		// �Q�ȏ�̃t�@�C�����h���b�v���ꂽ�Ƃ��̏���
		int idx;
		for(idx = 1; idx < file_names.GetFileCnt(); idx++) {
			if(file_names.GetFileName(idx) == _T("")) continue;

			HWND	wnd;
			wnd = OeditCheckFileIsOpen(file_names.GetFileName(idx).GetBuffer(0));
			if(wnd == NULL) {
				OeditOpenFile(file_names.GetFileName(idx).GetBuffer(0));
			}
			SetFileBootOption(file_names, idx);
		}

		HWND wnd = OeditCheckFileIsOpen(file_names.GetFileName(0));
		if(wnd != NULL) {
			if(IsIconic(wnd)) {
				ShowWindow(wnd, SW_RESTORE);
			} else {
				SetForegroundWindow(wnd);
			}
			SetFileBootOption(file_names, 0);
			return FALSE;
		}

		{
			CString fname = file_names.GetFileName(0).GetString();
			if(is_file_exist(fname.GetBuffer(0))) {
				if(OpenDocumentFile(fname.GetString()) == NULL) return FALSE;
			} else {
				// ���݂��Ȃ��t�@�C�������w�肳�ꂽ�ꍇ�A�w�肳�ꂽ�t�@�C�����ŕҏW�J�n�ł���悤�ɂ���
				// �t�H���_�����݂��Ȃ��ꍇ�̓G���[�ɂ���
				{
					if(_tcslen(fname.GetString()) > _MAX_PATH - 10) {
						AfxMessageBox(_T("�t�@�C�������������܂�"), MB_OK);
						return FALSE;
					}
					TCHAR path[_MAX_PATH];
					_tcscpy(path, fname.GetString());
					make_parent_dirname(path);
					if(!is_directory_exist(path)) {
						AfxMessageBox(_T("�����ȃp�X���w�肳��܂���"), MB_OK);
						return FALSE;
					}
				}

				//OnFileNew();
				POSITION	doc_tmpl_pos = GetFirstDocTemplatePosition();
				if(doc_tmpl_pos == NULL) return FALSE;

				CDocTemplate* doc_tmpl = GetNextDocTemplate(doc_tmpl_pos);
				if(doc_tmpl == NULL) return FALSE;

				CDocument* doc = doc_tmpl->OpenDocumentFile(NULL, 0);
				if(doc == NULL) return FALSE;

				doc->SetPathName(fname.GetString(), 0);

				doc_tmpl->InitialUpdateFrame((CFrameWnd*)AfxGetMainWnd(), doc, TRUE);

				UpdateAllDocViews(NULL, UPD_SET_FOCUS, 0);
			}
		}

		SetFileBootOption(file_names, 0);

		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;
	} else {
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNew;
	}
	cmdInfo.m_strFileName = "";

	return TRUE;
}

// �N���I�v�V����������
void COeditApp::ProcessBootOption(CBootParameter &file_names)
{
	// �J�����g�f�B���N�g����ݒ�
	if(file_names.GetBootDir() != _T("")) {
		SetCurrentDirectory(file_names.GetBootDir());
	} else if(file_names.GetFileCnt() == 0) {
		SetCurrentDirectory(g_option.initial_dir);
	}

	// �ҏW���[�h��ݒ�
	if(file_names.GetOption(_T("m")) != _T("")) {
		CString mode = file_names.GetOption(_T("m"));
		SetEditMode(g_lang_setting_list.GetSettingIdx(mode));
		UpdateAllDocViews(NULL, UPD_EDIT_MODE, 0);
		RedrawEditCtrl();
	}
}

void COeditApp::OnExtReg() 
{
	CString					cmd;
	PROCESS_INFORMATION		pi;
	STARTUPINFO				si;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	cmd.Format(_T("%soextreg.exe"), GetAppPath().GetBuffer(0));

	if(CreateProcess(NULL, cmd.GetBuffer(0), NULL, NULL, FALSE,
		0, NULL, NULL, &si, &pi) == FALSE) {
		AfxMessageBox(_T("�N���Ɏ��s���܂���"), MB_OK);
		return;
	}

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}

void COeditApp::OnRecKbmacro() 
{
	if(IsRecodingKBMacro()) {
		RecEndKBMacro();
	} else {
		RecStartKBMacro();	
	}
}

void COeditApp::OnUpdateRecKbmacro(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!IsPlaingKBMacro());
	pCmdUI->SetCheck(IsRecodingKBMacro());
}

void COeditApp::OnPlayKbmacro() 
{
	PlayKBMacro();	
}

void COeditApp::OnUpdatePlayKbmacro(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(CanPlayKBMacro());
}

void COeditApp::OnUpdateKbmacroSave(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(CanPlayKBMacro());
}

void COeditApp::OnKbmacroSave() 
{
	TCHAR file_name[_MAX_PATH];

	if(DoFileDlg_SetDir(_T("�L�[�{�[�h�}�N���̕ۑ�"), FALSE, _T("mac"), NULL,
		OFN_NOREADONLYRETURN | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_OVERWRITEPROMPT,
		_T("Keyboard Macro Files (*.mac)|*.mac|All Files (*.*)|*.*||"),
		AfxGetMainWnd(), file_name, g_option.kb_macro_dir) == FALSE) {
		return;
	}
		
	if(SaveKBMacro(file_name) == FALSE) {
		MessageBox(NULL, _T("�L�[�{�[�h�}�N���̕ۑ��Ɏ��s���܂����B"), _T("Error"),
			MB_ICONEXCLAMATION | MB_OK);
	}
}

void COeditApp::OnKbmacroLoad() 
{
	TCHAR file_name[_MAX_PATH];

	if(DoFileDlg_SetDir(_T("�L�[�{�[�h�}�N���̓Ǎ�"), TRUE, _T("mac"), NULL,
		OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER,
		_T("Keyboard Macro Files (*.mac)|*.mac|All Files (*.*)|*.*||"),
		AfxGetMainWnd(), file_name, g_option.kb_macro_dir) == FALSE) return;

	if(LoadKBMacro(file_name) == FALSE) {
		MessageBox(NULL, _T("�L�[�{�[�h�}�N���̓Ǎ��Ɏ��s���܂����B"), _T("Error"),
			MB_ICONEXCLAMATION | MB_OK);
	}
}

void COeditApp::OnKbmacroPlay() 
{
	TCHAR file_name[_MAX_PATH];

	if(DoFileDlg_SetDir(_T("�L�[�{�[�h�}�N���̎��s"), TRUE, _T("mac"), NULL,
		OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER,
		_T("Keyboard Macro Files (*.mac)|*.mac|All Files (*.*)|*.*||"),
		AfxGetMainWnd(), file_name, g_option.kb_macro_dir) == FALSE) return;

	if(PlayKBMacro(file_name) == FALSE) {
		MessageBox(NULL, _T("�L�[�{�[�h�}�N���̎��s�Ɏ��s���܂����B"), _T("Error"),
			MB_ICONEXCLAMATION | MB_OK);
	}
}

void COeditApp::RedrawEditCtrl()
{
	POSITION	doc_tmpl_pos = GetFirstDocTemplatePosition();
	for(; doc_tmpl_pos != NULL;) {
		CDocTemplate	*doc_tmpl = GetNextDocTemplate(doc_tmpl_pos);
		POSITION doc_pos = doc_tmpl->GetFirstDocPosition();
		for(; doc_pos != NULL; ) {
			CDocument *pdoc = doc_tmpl->GetNextDoc(doc_pos);
			if(pdoc != NULL && pdoc->IsKindOf(RUNTIME_CLASS(COeditDoc))) {
				COeditDoc *pd = (COeditDoc *)pdoc;
				pd->RedrawEditCtrl();
			}
		}
	}
}

BOOL COeditApp::OnOpenRecentFile(UINT nID)
{
	ASSERT_VALID(this);
	ASSERT(m_pRecentFileList != NULL);

	ASSERT(nID >= ID_FILE_MRU_FILE1);
	ASSERT(m_pRecentFileList != NULL && nID < ID_FILE_MRU_FILE1 + (UINT)m_pRecentFileList->GetSize());
	int nIndex = nID - ID_FILE_MRU_FILE1;
	ASSERT((*m_pRecentFileList)[nIndex].GetLength() != 0);

	TRACE(_T("MRU: open file (%d) '%s'.\n"), (nIndex) + 1,
			(LPCTSTR)(*m_pRecentFileList)[nIndex]);

//	if (OpenDocumentFile((*m_pRecentFileList)[nIndex]) == NULL)
//		m_pRecentFileList->Remove(nIndex);
	
	// �ۑ��m�F�_�C�A���O�ŃL�����Z�����ꂽ�Ƃ��ɁC�ŋߎg�����t�@�C�����X�g����폜���Ȃ�
	OpenDocumentFile((*m_pRecentFileList)[nIndex]);

	return TRUE;
}

void COeditApp::OnToggleShowRowNum() 
{
	g_option.text_editor.show_row_num = !g_option.text_editor.show_row_num;
	OptionChanged();
}

void COeditApp::OnToggleShowColNum() 
{
	g_option.text_editor.show_col_num = !g_option.text_editor.show_col_num;
	OptionChanged();
}

void COeditApp::OnShowRegexpHelp() 
{
	OpenHelpFile(_T("regexp.txt"));
}

void COeditApp::OnScmLoadFile() 
{
	TCHAR file_name[_MAX_PATH];

	if(DoFileDlg_SetDir(_T("�}�N���̎��s"), TRUE, _T("scm"), NULL,
		OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER,
		_T("Scheme Macro Files (*.scm)|*.scm|All Files (*.*)|*.*||"),
		AfxGetMainWnd(), file_name, g_option.kb_macro_dir) == FALSE) return;

	scm_eval_file(g_sc, file_name);
}


void COeditApp::OnShowSchemeHelp() 
{
	OpenHelpFile(_T("macro_ref.txt"));
}

void COeditApp::OnShowOeditmacroHelp() 
{
	OpenHelpFile(_T("macro_oedit.txt"));
}

CDocument* COeditApp::OpenDocumentFile(LPCTSTR lpszFileName) 
{
	CDocument *doc = COWinApp::OpenDocumentFile(lpszFileName);
	if(doc == NULL) return NULL;

	scm_call_event_handler(g_sc, SCM_EVENT_ON_OPEN_FILE);
	((COeditDoc *)doc)->CheckModified();

	return doc;
}

void COeditApp::OnScmEvalClipboard() 
{
	COleDataObject	obj;

	if(obj.AttachClipboard() == FALSE) return;
	if(obj.IsDataAvailable(CF_TEXT) == FALSE && obj.IsDataAvailable(CF_OEMTEXT) == FALSE ) return;

	HGLOBAL hData = obj.GetGlobalData(CF_UNICODETEXT);
	// CF_UNICODETEXT�����Ȃ��ꍇ�CCF_OEMTEXT, CF_TEXT�̎擾�����݂�
	if(hData == NULL) {
		hData = obj.GetGlobalData(CF_OEMTEXT);
		if(hData == NULL) hData = obj.GetGlobalData(CF_TEXT);
		// FIXME: �����R�[�h�ϊ�
	}
	if(hData == NULL) return;

	TCHAR *pstr = (TCHAR *)GlobalLock(hData);
	if(pstr == NULL) return;
	CString code = pstr;
	GlobalUnlock(hData);

	scm_eval_str(g_sc, code);
}

CString COeditApp::GetCommandString(UINT nID)
{
	CString msg;

	if(nID >= ID_EDIT_MODE_1 && nID <= ID_EDIT_MODE_1 + MAX_LANGUAGE_CNT) {
		msg = g_lang_setting_list.GetCommandString(nID - ID_EDIT_MODE_1);
	} else {
		msg.LoadString(nID);
	}

	return msg;
}

void COeditApp::OnEditMode(UINT nId)
{
	SetEditMode(nId - ID_EDIT_MODE_1);
	UpdateAllDocViews(NULL, UPD_EDIT_MODE, 0);
	RedrawEditCtrl();
}

void COeditApp::OnUpdateEditMode(CCmdUI *pCmdUI)
{
	if(pCmdUI->m_nID - ID_EDIT_MODE_1 == (UINT)g_option.edit_mode) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
	}
}

void COeditApp::OnFileBookmark()
{
	CFileBookMarkDlg dlg;

	dlg.m_app = this;

	if(dlg.DoModal() == IDOK) {
		OpenDocumentFile(dlg.m_open_file_path);
	}
}

void COeditApp::OnShowCredits()
{
	OpenHelpFile(_T("credits.txt"));
}
