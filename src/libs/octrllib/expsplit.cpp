/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

///////////////////////////////////////////////////////////////////////////// 
// ExpSplitterWnd.cpp
#include "stdafx.h"
//#include "afxpriv.h"

#include "Expsplit.h"

IMPLEMENT_DYNCREATE(CExpSplitterWnd, CWheelSplitterWnd)

BEGIN_MESSAGE_MAP(CExpSplitterWnd, CWheelSplitterWnd)
	//{{AFX_MSG_MAP(CExpSplitterWnd)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// �������̕����o�[�̕���ݒ肷��
// �f�t�H���g��菬�����ق����J�b�R�C�C
#define SPLITTER_CY		5

enum HitTestValue
{
	noHit                   = 0,
	vSplitterBox            = 1,
	hSplitterBox            = 2,
	bothSplitterBox         = 3,        // just for keyboard
	vSplitterBar1           = 101,
	vSplitterBar15          = 115,
	hSplitterBar1           = 201,
	hSplitterBar15          = 215,
	splitterIntersection1   = 301,
	splitterIntersection225 = 525
};


CExpSplitterWnd::CExpSplitterWnd()
{
	m_org_splitter_cy = m_cySplitter;
	m_cySplitter = m_cySplitterGap = SPLITTER_CY;

	m_horizonal_mode = FALSE;
	m_fixed_last_col_width = 0;
}

CExpSplitterWnd::~CExpSplitterWnd()
{
}

void CExpSplitterWnd::SetHorizonalMode(int splitter_cx)
{
	m_horizonal_mode = TRUE;
	m_cySplitter = m_cySplitterGap = m_org_splitter_cy;
	m_cxSplitter = m_cxSplitterGap = splitter_cx;
}

int CExpSplitterWnd::HitTest(CPoint pt) const
{
	if(m_horizonal_mode) {
		return HitTestH(pt);
	}
	return HitTestV(pt);
}

int CExpSplitterWnd::HitTestV(CPoint pt) const
{
	int		v;

	v = CSplitterWnd::HitTest(pt);

	// �������̕����o�[�ɂ͔������Ȃ����Ƃɂ��A�����̃T�C�Y�ύX���ł��Ȃ�����
	// �c�������q�b�g�Ȃ�A�c�̃o�[�̂݃q�b�g������
	if(v == vSplitterBox || (v >= vSplitterBar1 && v <= vSplitterBar15)) v = noHit;
	else if (v == bothSplitterBox) v = hSplitterBox;
	else if (v >= splitterIntersection1 && v <= splitterIntersection225) {
		v = (v - splitterIntersection1) % 15 + hSplitterBar1;
	}

	return v;
}

int CExpSplitterWnd::HitTestH(CPoint pt) const
{
	int		v;

	v = CSplitterWnd::HitTest(pt);

	// �������̕����o�[�ɂ͔������Ȃ����Ƃɂ��A�����̃T�C�Y�ύX���ł��Ȃ�����
	// �c�������q�b�g�Ȃ�A�c�̃o�[�̂݃q�b�g������
	if(v == hSplitterBox || (v >= hSplitterBar1 && v <= hSplitterBar15)) v = noHit;
	else if (v == bothSplitterBox) v = vSplitterBox;
	else if (v >= splitterIntersection1 && v <= splitterIntersection225) {
		v = (v - splitterIntersection1) % 15 + vSplitterBar1;
	}

	return v;
}

void CExpSplitterWnd::OnLButtonDown(UINT /*nFlags*/, CPoint pt)
{
	if (m_bTracking)
		return;

	// �Ǝ���HitTest()���Ă�
	StartTracking(HitTest(pt));
}

void CExpSplitterWnd::OnMouseMove(UINT /*nFlags*/, CPoint pt)
{
	if (GetCapture() != this)
		StopTracking(FALSE);

	if (m_bTracking)
	{
		// �T�C�Y�ύX���̓f�t�H���g�̓���
		CSplitterWnd::OnMouseMove(0, pt);
	}
	else
	{
		// ���ʂɃ}�E�X�𓮂����Ă���Ƃ��́A�Ǝ���HitTest()���Ă�
		int ht = HitTest(pt);
		SetSplitCursor(ht);
	}
}

void CExpSplitterWnd::SetFixLastColWidth(int cx)
{
	m_fixed_last_col_width = cx;
}

void CExpSplitterWnd::OnSize(UINT nType, int cx, int cy)
{
	if(m_fixed_last_col_width > 0 && cx > m_fixed_last_col_width) {
		int cx_0, cx_min_0;
		GetColumnInfo(0, cx_0, cx_min_0);
		SetColumnInfo(0, cx - m_fixed_last_col_width, cx_min_0);
	}

	CSplitterWnd::OnSize(nType, cx, cy);
}
