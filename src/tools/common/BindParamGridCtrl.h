#if !defined(AFX_BINDPARAMGRIDCTRL_H__5326AA1C_CB37_4E16_9E7B_3400CEF4844C__INCLUDED_)
#define AFX_BINDPARAMGRIDCTRL_H__5326AA1C_CB37_4E16_9E7B_3400CEF4844C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BindParamGridCtrl.h : �w�b�_�[ �t�@�C��
//

#include "gridctrl.h"

/////////////////////////////////////////////////////////////////////////////
// CBindParamGridCtrl �E�B���h�E

class CBindParamGridCtrl : public CGridCtrl
{
// �R���X�g���N�V����
public:
	CBindParamGridCtrl();

// �A�g���r���[�g
public:

// �I�y���[�V����
public:

// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B

	//{{AFX_VIRTUAL(CBindParamGridCtrl)
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
public:
	virtual ~CBindParamGridCtrl();

	// �������ꂽ���b�Z�[�W �}�b�v�֐�
protected:
	//{{AFX_MSG(CBindParamGridCtrl)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_BINDPARAMGRIDCTRL_H__5326AA1C_CB37_4E16_9E7B_3400CEF4844C__INCLUDED_)
