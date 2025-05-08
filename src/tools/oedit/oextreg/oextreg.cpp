// oextreg.cpp : �A�v���P�[�V�����p�N���X�̒�`���s���܂��B
//

#include "stdafx.h"
#include "oextreg.h"
#include "oextregDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if defined(UNICODE) || defined(_UNICODE)
#pragma comment(linker, "/entry:\"wWinMainCRTStartup\"")
#endif

/////////////////////////////////////////////////////////////////////////////
// COextregApp

BEGIN_MESSAGE_MAP(COextregApp, CWinApp)
	//{{AFX_MSG_MAP(COextregApp)
		// ���� - ClassWizard �͂��̈ʒu�Ƀ}�b�s���O�p�̃}�N����ǉ��܂��͍폜���܂��B
		//        ���̈ʒu�ɐ��������R�[�h��ҏW���Ȃ��ł��������B
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COextregApp �N���X�̍\�z

COextregApp::COextregApp()
{
	// TODO: ���̈ʒu�ɍ\�z�p�̃R�[�h��ǉ����Ă��������B
	// ������ InitInstance ���̏d�v�ȏ��������������ׂċL�q���Ă��������B
	m_hMutex = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// �B��� COextregApp �I�u�W�F�N�g

COextregApp theApp;

/////////////////////////////////////////////////////////////////////////////
// COextregApp �N���X�̏�����

BOOL COextregApp::InitInstance()
{
	// �W���I�ȏ���������
	// ���������̋@�\���g�p�����A���s�t�@�C���̃T�C�Y��������������
	//  ��Έȉ��̓���̏��������[�`���̒�����s�K�v�Ȃ��̂��폜����
	//  ���������B

	// �Q�d�N���̖h�~
	if(CheckExecute() == FALSE) return FALSE;

#ifdef _AFXDLL
	Enable3dControls();			// ���L DLL ���� MFC ���g���ꍇ�͂������R�[�����Ă��������B
#else
	Enable3dControlsStatic();	// MFC �ƐÓI�Ƀ����N����ꍇ�͂������R�[�����Ă��������B
#endif

	COextregDlg dlg;
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

BOOL COextregApp::CheckExecute()
{
	// ��d�N���̃`�F�b�N
	static const TCHAR* STR_MUTEX_NAME = _T("UNIQUE_STRING_OGAWA_OEXTREG");
	m_hMutex = ::CreateMutex( NULL, FALSE, STR_MUTEX_NAME );
	if ( ::GetLastError() == ERROR_ALREADY_EXISTS ) {
		::CloseHandle(m_hMutex);
		m_hMutex = NULL;

		// �A�v���P�[�V�������t�H�A�O���E���h�ɂ���
		HWND	wnd = GetTopWindow(NULL);
		HWND	find = NULL;
		for(; wnd != NULL; ) {
			find = FindWindowEx(wnd, NULL, NULL, OEXTREG_WINDOW_NAME);
			if(find != NULL) {
				if(GetParent(find) != NULL) {
					SetForegroundWindow(GetParent(find));
				}
				break;
			}
			wnd = GetNextWindow(wnd, GW_HWNDNEXT);
		}

		return FALSE;
	}

	return TRUE;
}

int COextregApp::ExitInstance() 
{
	if(m_hMutex != NULL) {
		::CloseHandle(m_hMutex);
		m_hMutex = NULL;
	}
	
	return CWinApp::ExitInstance();
}
