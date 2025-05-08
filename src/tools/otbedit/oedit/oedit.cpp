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

#define MAIN_PROG
#include "oedit.h"

#include "MainFrm.h"
#include "oeditDoc.h"
#include "oeditView.h"

#include "file_winutil.h"
#include "fileutil.h"

#include "afxpriv.h"

#include "aboutdlg.h"

#include "kbmacro.h"
#include "greputil.h"

#include "filenames.h"
#include "ExtFileDialog.h"
#include "CommonUtil.h"
#include "FileBookMarkDlg.h"

#include "imm.h"
#include "scm_macro.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if defined(UNICODE) || defined(_UNICODE)
#pragma comment(linker, "/entry:\"wWinMainCRTStartup\"")
#endif

/////////////////////////////////////////////////////////////////////////////
// COeditApp

BEGIN_MESSAGE_MAP(COeditApp, CWinApp)
	//{{AFX_MSG_MAP(COeditApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_OPTION, OnOption)
	ON_COMMAND(ID_SET_FONT, OnSetFont)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_NEW_WINDOW, OnNewWindow)
	ON_COMMAND(ID_EXT_REG, OnExtReg)
	ON_COMMAND(ID_PLAY_KBMACRO, OnPlayKbmacro)
	ON_UPDATE_COMMAND_UI(ID_PLAY_KBMACRO, OnUpdatePlayKbmacro)
	ON_COMMAND(ID_REC_KBMACRO, OnRecKbmacro)
	ON_UPDATE_COMMAND_UI(ID_REC_KBMACRO, OnUpdateRecKbmacro)
	ON_UPDATE_COMMAND_UI(ID_KBMACRO_SAVE, OnUpdateKbmacroSave)
	ON_COMMAND(ID_KBMACRO_SAVE, OnKbmacroSave)
	ON_COMMAND(ID_KBMACRO_LOAD, OnKbmacroLoad)
	ON_COMMAND(ID_KBMACRO_PLAY, OnKbmacroPlay)
	ON_COMMAND(ID_TOGGLE_SHOW_COL_NUM, OnToggleShowColNum)
	ON_COMMAND(ID_TOGGLE_SHOW_ROW_NUM, OnToggleShowRowNum)
	ON_COMMAND(ID_SHOW_REGEXP_HELP, OnShowRegexpHelp)
	ON_COMMAND(ID_SET_FONT_TAB_CTRL, OnSetFontTabCtrl)
	ON_COMMAND(ID_SHOW_SCHEME_HELP, OnShowSchemeHelp)
	ON_COMMAND(ID_SHOW_OEDITMACRO_HELP, OnShowOeditmacroHelp)
	ON_COMMAND(ID_SCM_LOAD_FILE, OnScmLoadFile)
	ON_COMMAND(ID_SCM_EVAL_CLIPBOARD, OnScmEvalClipboard)
	ON_COMMAND(ID_FONT_SIZE_LARGE, OnFontSizeLarge)
	ON_COMMAND(ID_FONT_SIZE_SMALL, OnFontSizeSmall)
	//}}AFX_MSG_MAP
	// �W���̃t�@�C����{�h�L�������g �R�}���h
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// �W���̈���Z�b�g�A�b�v �R�}���h
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
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
	m_hMutex = NULL;

	// �O���[�o���ϐ��̏�����
	g_open_kanji_code = UnknownKanjiCode;
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
	// �W���I�ȏ���������
	// ���������̋@�\���g�p�����A���s�t�@�C���̃T�C�Y��������
	// ��������Έȉ��̓���̏��������[�`���̒�����s�K�v�Ȃ���
	// ���폜���Ă��������B
	if(!AfxOleInit()) {
		MessageBox(NULL, _T("OLE�̏������Ɏ��s���܂����B"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}

#ifdef _AFXDLL
	Enable3dControls();		// ���L DLL �̒��� MFC ���g�p����ꍇ�ɂ͂������Ăяo���Ă��������B
#else
//	Enable3dControlsStatic();	// MFC �ƐÓI�Ƀ����N���Ă���ꍇ�ɂ͂������Ăяo���Ă��������B
#endif

	// scheme�}�N��������������
	if(init_scm_macro(&g_sc, _T("otbedit.scm")) == FALSE) return FALSE;

	TCHAR ini_file_name[MAX_PATH];
	TCHAR local_ini_file_name[MAX_PATH];
	_stprintf(ini_file_name, _T("%sotbedit.ini"), GetAppPath().GetBuffer(0));
	_stprintf(local_ini_file_name, _T("%sotbedit_local.ini"), GetAppPath().GetBuffer(0));

	if(!is_file_exist(ini_file_name)) {
		if(!RegistryToIniFile(_T("OGAWA"), _T("otbedit"), ini_file_name, local_ini_file_name)) {
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
		base_dir.Format(_T("%sdata\\"), GetAppPath());
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
	CreateTabFont();

	// �A�v���P�[�V�����p�̃h�L�������g �e���v���[�g��o�^���܂��B�h�L�������g �e���v���[�g
	//  �̓h�L�������g�A�t���[�� �E�B���h�E�ƃr���[���������邽�߂ɋ@�\���܂��B

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(COeditDoc),
		RUNTIME_CLASS(CMainFrame),       // ���C�� SDI �t���[�� �E�B���h�E
		RUNTIME_CLASS(COeditView));
	AddDocTemplate(pDocTemplate);

	// DDE Execute open ���g�p�\�ɂ��܂��B
	EnableShellOpen();

	// DDE�Afile open �ȂǕW���̃V�F�� �R�}���h�̃R�}���h���C������͂��܂��B
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	if(CheckDoubleBoot(cmdInfo) == FALSE) return FALSE;

	if(OpenFiles(cmdInfo, m_lpCmdLine) == FALSE) return FALSE;

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

void COeditApp::SetFileBootOption(CBootParameter &file_names, int idx)
{
	int		row = -1;
	if(!file_names.GetFileNameOption(idx)->GetOption(_T("r")).IsEmpty()) {
		row = _ttoi(file_names.GetFileNameOption(idx)->GetOption(_T("r")));
	}	
	CString mode = file_names.GetFileNameOption(idx)->GetOption(_T("m"));

	SetBootOption(file_names.GetFileName(idx).GetBuffer(0), row, mode);
}

BOOL COeditApp::CheckDoubleBoot(CCommandLineInfo &cmdInfo)
{
	if(g_option.disable_double_boot == FALSE) return TRUE;

	// ��d�N���̃`�F�b�N
	static const TCHAR* STR_MUTEX_NAME = _T("UNIQUE_STRING_OTBEDIT");
	m_hMutex = ::CreateMutex( NULL, FALSE, STR_MUTEX_NAME );
	if ( ::GetLastError() == ERROR_ALREADY_EXISTS ) {
		::CloseHandle(m_hMutex);
		m_hMutex = NULL;

		HWND	hwnd;
		if(cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen) {
			TCHAR msg_buf[1024];
			CBootParameter		file_names;
			if(file_names.InitParameter(m_lpCmdLine, msg_buf) == FALSE) {
				AfxMessageBox(msg_buf, MB_OK);
				return FALSE;
			}

			int idx;
			for(idx = 0; idx < file_names.GetFileCnt(); idx++) {
				if(file_names.GetFileName(idx) == "") continue;

				hwnd = OtbeditCheckFileIsOpen(::GetTopWindow(NULL), file_names.GetFileName(idx).GetBuffer(0));
				if(hwnd == NULL) {
					OtbeditOpenFile(file_names.GetFileName(idx).GetBuffer(0));
					hwnd = OtbeditCheckFileIsOpen(::GetTopWindow(NULL), file_names.GetFileName(idx).GetBuffer(0));
				}

				if(hwnd) SetFileBootOption(file_names, idx);
			}
		} else {
			hwnd = FindWindowLoop(OTBEDIT_VIEW_WINDOWNAME);
		}
		if(hwnd != NULL) {
			if(::GetParent(hwnd) != NULL && IsIconic(::GetParent(hwnd))) {
				ShowWindow(::GetParent(hwnd), SW_RESTORE);
			} else {
				SetForegroundWindow(hwnd);
			}
		}
		return FALSE;
	}
	return TRUE;
}

void COeditApp::LoadEditMode()
{
	g_option.text_editor.line_mode = GetIniFileInt(_T("TEXT_EDITOR"), _T("LINE_MODE"), EC_LINE_MODE_NORMAL);
	g_option.edit_mode = GetIniFileInt(_T("APPLICATION"), _T("EDIT_MODE"), 0);
	SetEditMode(g_option.edit_mode);
}

void COeditApp::LoadFontOption()
{
	g_option.font.face_name = GetIniFileString(_T("FONT"), _T("FACENAME"), _T("Terminal"));
	g_option.font.point = GetIniFileInt(_T("FONT"), _T("POINT"), 140);
	g_option.font.weight = GetIniFileInt(_T("FONT"), _T("WEIGHT"), FW_NORMAL);
	g_option.font.italic = GetIniFileInt(_T("FONT"), _T("ITALIC"), 0);

	g_option.tab_font.face_name = GetIniFileString(_T("TAB_FONT"), _T("FACENAME"), g_option.font.face_name);
	g_option.tab_font.point = GetIniFileInt(_T("TAB_FONT"), _T("POINT"), g_option.font.point);
}

void COeditApp::SaveEditMode()
{
	WriteIniFileInt(_T("TEXT_EDITOR"), _T("LINE_MODE"), g_option.text_editor.line_mode);
	WriteIniFileInt(_T("APPLICATION"), _T("EDIT_MODE"), g_option.edit_mode);
}

void COeditApp::SaveFontOption()
{
	WriteIniFileString(_T("FONT"), _T("FACENAME"), g_option.font.face_name);
	WriteIniFileInt(_T("FONT"), _T("POINT"), g_option.font.point);
	WriteIniFileInt(_T("FONT"), _T("WEIGHT"), g_option.font.weight);
	WriteIniFileInt(_T("FONT"), _T("ITALIC"), g_option.font.italic);

	WriteIniFileString(_T("TAB_FONT"), _T("FACENAME"), g_option.tab_font.face_name);
	WriteIniFileInt(_T("TAB_FONT"), _T("POINT"), g_option.tab_font.point);
	SaveIniFile();
}

void COeditApp::SaveOtherSettings()
{
	WriteIniFileString(_T("APPLICATION"), _T("KB_MACRO_DIR"), g_option.kb_macro_dir);
}

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
	if(m_hMutex != NULL) ::CloseHandle(m_hMutex);

	SaveEditMode();
	SaveOtherSettings();
	SaveIniFile();
	clear_scm_macro(&g_sc);
	
	return CWinApp::ExitInstance();
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

BOOL COeditApp::CreateTabFont()
{
	// �t�H���g���쐬
	return g_tab_font.CreatePointFont(g_option.tab_font.point, g_option.tab_font.face_name);
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

	UpdateAllDocViews(NULL, UPD_FONT, 0);
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

void COeditApp::OnSetFontTabCtrl() 
{
	CFontDialog		fontdlg;

	// �_�C�A���O������
	fontdlg.m_cf.Flags &= ~CF_EFFECTS;
	fontdlg.m_cf.Flags |= CF_INITTOLOGFONTSTRUCT;
	g_tab_font.GetLogFont(fontdlg.m_cf.lpLogFont);

	// �_�C�A���O�\��
	if(fontdlg.DoModal() != IDOK) return;

	g_option.tab_font.face_name = fontdlg.GetFaceName();
	g_option.tab_font.point = fontdlg.GetSize();

	g_tab_font.DeleteObject();
	CreateTabFont();

	UpdateAllDocViews(NULL, UPD_FONT, 0);
	SaveFontOption();
}

CDocument *COeditApp::OpenDocumentFile2(LPCTSTR lpszFileName)
{
	CDocument *pdoc = NULL;

	{
		TCHAR file_name[MAX_PATH];
		if(GetLongPath(lpszFileName, file_name) == FALSE) return NULL;
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
					((COeditDoc *)pDoc)->ReloadFile(g_open_kanji_code);
					return pDoc;
				}
			}
		}
	}

	return OpenDocumentFile(lpszFileName);
}

void COeditApp::OnFileOpen() 
{
	// TODO: ���̈ʒu�ɃR�}���h �n���h���p�̃R�[�h��ǉ����Ă�������
//	CWinApp::OnFileOpen();	
/*
	CString newName;
	if (!DoPromptFileName(newName, AFX_IDS_OPENFILE,
	  OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, TRUE, NULL))
		return; // open cancelled

	OpenDocumentFile(newName);
*/
	// FIXME: DoPromptFileName()���R�[�h�����L����
	CString	fileName;

	TCHAR	cur_dir[_MAX_PATH];
	GetCurrentDirectory(sizeof(cur_dir)/sizeof(cur_dir[0]), cur_dir);

	CExtFileDialog dlgFile(TRUE);

	CString title;
	VERIFY(title.LoadString(AFX_IDS_OPENFILE));

	dlgFile.m_b_open_mode = TRUE;
	dlgFile.m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_EXT_FILEDLG_OPEN);
	dlgFile.m_kanji_code = g_open_kanji_code;
	dlgFile.m_ofn.Flags |= OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_ENABLETEMPLATE;
	dlgFile.m_ofn.lpstrInitialDir = cur_dir;

	CString strFilter = g_lang_setting_list.GetFilterSuffix(dlgFile.m_ofn, AFX_IDS_ALLFILTER);
	dlgFile.m_ofn.lpstrFilter = strFilter;

	dlgFile.m_ofn.lpstrTitle = title;
	dlgFile.m_ofn.nMaxFile = 500;
	dlgFile.m_ofn.lpstrFile = fileName.GetBuffer(_MAX_PATH * 1000);

	BOOL bResult = dlgFile.DoModal() == IDOK ? TRUE : FALSE;
	
	if(bResult == FALSE) {
		fileName.ReleaseBuffer();
		return;
	}

	int back_kanji_code = g_open_kanji_code;
	g_open_kanji_code = dlgFile.m_kanji_code;

	POSITION	pos;
	pos = dlgFile.GetStartPosition();
	for(; pos != NULL;) {
		OpenDocumentFile2(dlgFile.GetNextPathName(pos));
	}

	g_open_kanji_code = back_kanji_code;

	fileName.ReleaseBuffer();
}

BOOL COeditApp::DoPromptFileName(CString& fileName, UINT nIDSTitle,
			DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate,
			int *kanji_code, int *line_type)
{
	TCHAR	cur_dir[_MAX_PATH];
	GetCurrentDirectory(sizeof(cur_dir)/sizeof(cur_dir[0]), cur_dir);

	CExtFileDialog dlgFile(bOpenFileDialog);

	CString title;
	VERIFY(title.LoadString(nIDSTitle));

	dlgFile.m_b_open_mode = bOpenFileDialog;
	if(bOpenFileDialog) {
		dlgFile.m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_EXT_FILEDLG_OPEN);
		if(*kanji_code != NULL) dlgFile.m_kanji_code = *kanji_code;
	} else {
		dlgFile.m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_EXT_FILEDLG_SAVE);
		if(*kanji_code != NULL) dlgFile.m_kanji_code = *kanji_code;
		if(*line_type != NULL) dlgFile.m_line_type = *line_type;
	}
	dlgFile.m_ofn.Flags |= (lFlags | OFN_ENABLETEMPLATE);
	dlgFile.m_ofn.lpstrInitialDir = cur_dir;

	CString strFilter = g_lang_setting_list.GetFilterSuffix(dlgFile.m_ofn, AFX_IDS_ALLFILTER);
	dlgFile.m_ofn.lpstrFilter = strFilter;

	dlgFile.m_ofn.nFilterIndex = 0;
	if(!bOpenFileDialog) {
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

	return;
}

void COeditApp::OnEditMode(UINT nId)
{
	SetEditMode(nId - ID_EDIT_MODE_1);
	UpdateAllDocViews(NULL, UPD_EDIT_MODE, 0);
}

void COeditApp::OnUpdateEditMode(CCmdUI *pCmdUI)
{
	if(pCmdUI->m_nID - ID_EDIT_MODE_1 == (UINT)g_option.edit_mode) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
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
	if(GetModuleFileName(NULL, path_buffer, sizeof(path_buffer)) == 0) {
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

BOOL COeditApp::OpenFiles(CCommandLineInfo &cmdInfo, TCHAR *cmd_line)
{
/* DDE�����܂������Ȃ��P�[�X������̂ŁC�����Ńt�@�C�����J��
	if(strlen(cmd_line) == 0 || cmd_line[0] == '\"') {
		// DDE�ʐM�ɂȂ�̂ŁC�����ł̓t�@�C�����J���Ȃ�
		// FIXME: ���W�X�g�����Q�Ƃ��āCDDE�̐ݒ�ɂȂ��Ă��邩�m�F����
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNew;
		cmdInfo.m_strFileName = "";
		return TRUE;
	}
*/
	// �t�@�C�����h���b�v���ꂽ�Ƃ��̏���
	TCHAR msg_buf[1024];
	CBootParameter		file_names;
	if(file_names.InitParameter(cmd_line, msg_buf) == FALSE) {
		AfxMessageBox(msg_buf, MB_OK);
		return FALSE;
	}

	BOOL ret_v = TRUE;
	if(file_names.GetFileCnt() > 0) {
		ret_v = FALSE;
		HWND fg_wnd = NULL;
		CString fg_file_name;

		int idx;
		for(idx = 0; idx < file_names.GetFileCnt(); idx++) {
			if(file_names.GetFileName(idx) == _T("")) continue;
			
			HWND	wnd;
			wnd = OtbeditCheckFileIsOpen(GetTopWindow(NULL), file_names.GetFileName(idx).GetBuffer(0));
			if(wnd != NULL) {
				fg_file_name = file_names.GetFileName(idx);
				fg_wnd = wnd;
				continue;
			}

			CString fname = file_names.GetFileName(idx).GetString();
			if(is_file_exist(fname.GetBuffer(0))) {
				ret_v = TRUE;
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

				ret_v = TRUE;
				//OnFileNew();
				POSITION	doc_tmpl_pos = GetFirstDocTemplatePosition();
				if(doc_tmpl_pos == NULL) return FALSE;

				CDocTemplate* doc_tmpl = GetNextDocTemplate(doc_tmpl_pos);
				if(doc_tmpl == NULL) return FALSE;

				CDocument* doc = doc_tmpl->OpenDocumentFile(NULL, 0);
				if(doc == NULL) return FALSE;

				doc->SetPathName(fname.GetString(), 0);

				doc_tmpl->InitialUpdateFrame((CFrameWnd *)AfxGetMainWnd(), doc, TRUE);
			}

			SetFileBootOption(file_names, idx);
		}

		if(ret_v == FALSE && fg_wnd != NULL) {
			COPYDATASTRUCT	copy_data;
			TCHAR	file_name2[_MAX_PATH] = _T("");
			_tcscpy(file_name2, fg_file_name.GetBuffer(0));
			copy_data.dwData = WM_COPY_DATA_ACTIVE_FILE;
			copy_data.cbData = sizeof(file_name2);
			copy_data.lpData = &file_name2;

			SendMessage(fg_wnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&copy_data);
		}

		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;
	} else {
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNew;
	}
	cmdInfo.m_strFileName = _T("");

	// �J�����g�f�B���N�g����ݒ�
	if(file_names.GetBootDir() != _T("")) {
		SetCurrentDirectory(file_names.GetBootDir());
	} else if(file_names.GetFileCnt() == 0) {
		SetCurrentDirectory(g_option.initial_dir);
	}

	return ret_v;
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
		AfxGetMainWnd(), file_name, g_option.kb_macro_dir) == FALSE) return;
		
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

void COeditApp::OnToggleShowColNum() 
{
	g_option.text_editor.show_col_num = !g_option.text_editor.show_col_num;
	OptionChanged();
}

void COeditApp::OnToggleShowRowNum() 
{
	g_option.text_editor.show_row_num = !g_option.text_editor.show_row_num;
	OptionChanged();
}

void COeditApp::OnShowRegexpHelp() 
{
	OpenHelpFile(_T("regexp.txt"));
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

void COeditApp::OnShowSchemeHelp() 
{
	OpenHelpFile(_T("macro_ref.txt"));
}

void COeditApp::OnShowOeditmacroHelp() 
{
	OpenHelpFile(_T("macro_otbedit.txt"));
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

CDocument* COeditApp::OpenDocumentFile(LPCTSTR lpszFileName) 
{
	CDocument *doc = COWinApp::OpenDocumentFile(lpszFileName);
	if(doc == NULL) return NULL;

	// on-open-file, on-tab-changed�̏��Ɏ��s����
	scm_call_event_handler(g_sc, SCM_EVENT_ON_OPEN_FILE);
	scm_call_event_handler(g_sc, SCM_EVENT_ON_CHANGE_TAB);
	((COeditDoc *)doc)->CheckModified();

	return doc;
}

void COeditApp::OnFileBookmark()
{
	CFileBookMarkDlg dlg;

	dlg.m_app = this;
	dlg.m_b_multi_sel = TRUE;

	if(dlg.DoModal() == IDOK) {
		int i;
		for(i = 0; i < dlg.m_open_file_path_arr.GetCount(); i++) {
			OpenDocumentFile(dlg.m_open_file_path_arr.GetAt(i));
		}
	}
}

void COeditApp::OnShowCredits()
{
	OpenHelpFile(_T("credits.txt"));
}
