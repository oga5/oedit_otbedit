// ogrep.h : OGREP �A�v���P�[�V�����̃��C�� �w�b�_�[ �t�@�C���ł��B
//

#if !defined(AFX_OGREP_H__57F719A5_806D_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_OGREP_H__57F719A5_806D_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// ���C�� �V���{��

#define OGREP_WINDOW_NAME	_T("OGREP_WINDOW_NAME")

/////////////////////////////////////////////////////////////////////////////
// COgrepApp:
// ���̃N���X�̓���̒�`�Ɋւ��Ă� ogrep.cpp �t�@�C�����Q�Ƃ��Ă��������B
//
#include "OWinApp.h"

class COgrepApp : public COWinApp
{
public:
	COgrepApp();

// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(COgrepApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����

	//{{AFX_MSG(COgrepApp)
		// ���� - ClassWizard �͂��̈ʒu�Ƀ����o�֐���ǉ��܂��͍폜���܂��B
		//        ���̈ʒu�ɐ��������R�[�h��ҏW���Ȃ��ł��������B
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	HANDLE	m_hMutex;
	BOOL CheckExecute();
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_OGREP_H__57F719A5_806D_11D5_8505_00E018A83B1B__INCLUDED_)
