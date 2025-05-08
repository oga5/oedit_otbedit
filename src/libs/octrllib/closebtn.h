/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#if !defined(AFX_CLOSEBTN_H__CC4A629A_AC66_409F_83CD_07F133CD683D__INCLUDED_)
#define AFX_CLOSEBTN_H__CC4A629A_AC66_409F_83CD_07F133CD683D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CloseBtn.h : �w�b�_�[ �t�@�C��
//

/////////////////////////////////////////////////////////////////////////////
// CCloseBtn �E�B���h�E

class CCloseBtn : public CButton
{
// �R���X�g���N�V����
public:
	CCloseBtn();

	BOOL Create(CWnd *pParentWnd, UINT nID, BOOL border = TRUE, int btn_size = 16, int bmp_offset = 4);
	void SetColor(COLORREF color) { m_color = color; }
	void SetParam(int param) { m_param = param; }
	int GetParam() { return m_param; }

// �A�g���r���[�g
public:

// �I�y���[�V����
public:

// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CCloseBtn)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
public:
	virtual ~CCloseBtn();

	// �������ꂽ���b�Z�[�W �}�b�v�֐�
protected:
	//{{AFX_MSG(CCloseBtn)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	CBitmap m_x_bmp;

	void CreateXBitmap();
	BOOL		m_border;
	int			m_btn_size;
	int			m_bmp_offset;
	
	COLORREF	m_color;
	int			m_param;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_CLOSEBTN_H__CC4A629A_AC66_409F_83CD_07F133CD683D__INCLUDED_)
