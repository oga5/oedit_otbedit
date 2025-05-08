// ogrep.cpp : �A�v���P�[�V�����p�N���X�̒�`���s���܂��B
//

#include "stdafx.h"
#include "ogrep.h"
#include "ogrepDlg.h"
#include "fileutil.h"
#include "file_winutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static const char *THIS_FILE = __FILE__;
#endif


#if defined(UNICODE) || defined(_UNICODE)
#pragma comment(linker, "/entry:\"wWinMainCRTStartup\"")
#endif

/////////////////////////////////////////////////////////////////////////////
// COgrepApp

BEGIN_MESSAGE_MAP(COgrepApp, CWinApp)
	//{{AFX_MSG_MAP(COgrepApp)
		// ���� - ClassWizard �͂��̈ʒu�Ƀ}�b�s���O�p�̃}�N����ǉ��܂��͍폜���܂��B
		//        ���̈ʒu�ɐ��������R�[�h��ҏW���Ȃ��ł��������B
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COgrepApp �N���X�̍\�z

COgrepApp::COgrepApp()
{
	// TODO: ���̈ʒu�ɍ\�z�p�̃R�[�h��ǉ����Ă��������B
	// ������ InitInstance ���̏d�v�ȏ��������������ׂċL�q���Ă��������B
	m_hMutex = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// �B��� COgrepApp �I�u�W�F�N�g

COgrepApp theApp;

/////////////////////////////////////////////////////////////////////////////
// COgrepApp �N���X�̏�����

BOOL COgrepApp::InitInstance()
{
	// �W���I�ȏ���������
	// ���������̋@�\���g�p�����A���s�t�@�C���̃T�C�Y��������������
	//  ��Έȉ��̓���̏��������[�`���̒�����s�K�v�Ȃ��̂��폜����
	//  ���������B
	if(CheckExecute() == FALSE) return FALSE;

#ifdef _AFXDLL
	Enable3dControls();			// ���L DLL ���� MFC ���g���ꍇ�͂������R�[�����Ă��������B
#else
// �Â�MFC�ŕK�v����������
//	Enable3dControlsStatic();	// MFC �ƐÓI�Ƀ����N����ꍇ�͂������R�[�����Ă��������B
#endif

	// INI�t�@�C���ɕۑ�����
	TCHAR ini_file_name[MAX_PATH];
	_stprintf(ini_file_name, _T("%sogrep.ini"), GetAppPath().GetBuffer(0));
	SetLocalIniFileName(ini_file_name);

	COgrepDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �_�C�A���O�� <OK> �ŏ����ꂽ���̃R�[�h��
		//       �L�q���Ă��������B
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �_�C�A���O�� <��ݾ�> �ŏ����ꂽ���̃R�[�h��
		//       �L�q���Ă��������B
	}

	// �_�C�A���O�������Ă���A�v���P�[�V�����̃��b�Z�[�W �|���v���J�n������́A
	// �A�v���P�[�V�������I�����邽�߂� FALSE ��Ԃ��Ă��������B
	return FALSE;
}

BOOL COgrepApp::CheckExecute()
{
	// ��d�N���̃`�F�b�N
	static const TCHAR* STR_MUTEX_NAME = _T("UNIQUE_STRING_OGAWA_OGREP");
	m_hMutex = ::CreateMutex( NULL, FALSE, STR_MUTEX_NAME );
	if ( ::GetLastError() == ERROR_ALREADY_EXISTS ) {
		if (m_hMutex != NULL) {
			::CloseHandle(m_hMutex);
			m_hMutex = NULL;
		}

		// �A�v���P�[�V�������t�H�A�O���E���h�ɂ���
		HWND	wnd = FindWindowLoop(OGREP_WINDOW_NAME);
		if(wnd != NULL) {
			if(GetParent(wnd) != NULL) {
				SetForegroundWindow(GetParent(wnd));
			}
		}

		return FALSE;
	}

	return TRUE;
}

int COgrepApp::ExitInstance() 
{
	// TODO: ���̈ʒu�ɌŗL�̏�����ǉ����邩�A�܂��͊�{�N���X���Ăяo���Ă�������
	if(m_hMutex != NULL) {
		::CloseHandle(m_hMutex);
		m_hMutex = NULL;
	}
	
	return CWinApp::ExitInstance();
}
