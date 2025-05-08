/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_FILEFORMATDLG_H__649BBE58_716D_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_FILEFORMATDLG_H__649BBE58_716D_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileFormatDlg.h : �w�b�_�[ �t�@�C��
//
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CFileFormatDlg �_�C�A���O

class CFileFormatDlg : public CDialog
{
// �R���X�g���N�V����
public:
	CFileFormatDlg(CWnd* pParent = NULL);   // �W���̃R���X�g���N�^

	void InitData(int kanji_code, int line_type);
	void GetData(int &kanji_code, int &line_type);

// �_�C�A���O �f�[�^
	//{{AFX_DATA(CFileFormatDlg)
	enum { IDD = IDD_FILE_FORMAT_DLG };
	//}}AFX_DATA


// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CFileFormatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
protected:

	// �������ꂽ���b�Z�[�W �}�b�v�֐�
	//{{AFX_MSG(CFileFormatDlg)
		virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	int		m_kanji_code;
	int		m_line_type;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_FILEFORMATDLG_H__649BBE58_716D_11D5_8505_00E018A83B1B__INCLUDED_)
