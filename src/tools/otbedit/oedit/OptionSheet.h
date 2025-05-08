/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_OPTIONSHEET_H__71AC5183_A085_11D4_B06E_00E018A83B1B__INCLUDED_)
#define AFX_OPTIONSHEET_H__71AC5183_A085_11D4_B06E_00E018A83B1B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// OptionSheet.h : �w�b�_�[ �t�@�C��
//
#include "EditorOptionPage.h"
#include "EditorOptionPage2.h"
#include "SetupPage.h"
#include "SetupPage2.h"

/////////////////////////////////////////////////////////////////////////////
// COptionSheet

class COptionSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(COptionSheet)

private:

// �R���X�g���N�V����
public:
	COptionSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	COptionSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// �A�g���r���[�g
public:
	CEditorOptionPage	m_editor_page;
	CEditorOptionPage2	m_editor_page2;
	CSetupPage			m_setup_page;
	CSetupPage2			m_setup_page2;

// �I�y���[�V����
public:

// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(COptionSheet)
	public:
	virtual INT_PTR DoModal();
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
public:
	virtual ~COptionSheet();

	// �������ꂽ���b�Z�[�W �}�b�v�֐�
protected:
	//{{AFX_MSG(COptionSheet)
		// ���� - ClassWizard �͂��̈ʒu�Ƀ����o�֐���ǉ��܂��͍폜���܂��B
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_OPTIONSHEET_H__71AC5183_A085_11D4_B06E_00E018A83B1B__INCLUDED_)
