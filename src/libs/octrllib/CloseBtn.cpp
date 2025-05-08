/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 // CloseBtn.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "CloseBtn.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCloseBtn

CCloseBtn::CCloseBtn()
{
	CreateXBitmap();
	m_color = ::GetSysColor(COLOR_BTNFACE);
}

CCloseBtn::~CCloseBtn()
{
}


BEGIN_MESSAGE_MAP(CCloseBtn, CButton)
	//{{AFX_MSG_MAP(CCloseBtn)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCloseBtn メッセージ ハンドラ

BOOL CCloseBtn::Create(CWnd *pParentWnd, UINT nID, BOOL border, int btn_size, int bmp_offset)
{
	m_border = border;
	m_btn_size = btn_size;
	m_bmp_offset = bmp_offset;

	CRect btn_rect;
	btn_rect.top = 0;
	btn_rect.bottom = btn_size;
	btn_rect.left = 0;
	btn_rect.right = btn_size;

	return CButton::Create(_T("X"), WS_CHILD | BS_OWNERDRAW, btn_rect, pParentWnd, nID);
}

void CCloseBtn::CreateXBitmap()
{
	unsigned char bmp_data[2*7] = {
		0x3c, 0xff, 
		0x99, 0xff, 
		0xc3, 0xff, 
		0xe7, 0xff, 
		0xc3, 0xff, 
		0x99, 0xff, 
		0x3c, 0xff, 
	};

	BITMAP bmp;
	bmp.bmType = 0;
	bmp.bmWidth = 8;
	bmp.bmHeight = 7;
	bmp.bmWidthBytes = 2;
	bmp.bmPlanes = 1;
	bmp.bmBitsPixel = 1;
	bmp.bmBits = bmp_data;

	m_x_bmp.CreateBitmapIndirect(&bmp);
}

void CCloseBtn::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: 指定されたアイテムを描画するためのコードを追加してください
	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);

	CRect rect(lpDrawItemStruct->rcItem);

	UINT	nOffset = m_bmp_offset;

	dc.FillSolidRect(rect, m_color);

	if (lpDrawItemStruct->itemState & ODS_SELECTED) {
		nOffset += 1;

		if(m_border) {
			dc.DrawFrameControl(rect, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED);
		}
	} else {
		if(m_border) dc.DrawEdge(rect, EDGE_ETCHED, BF_RECT);
	}

	dc.DrawState(CPoint(rect.left + nOffset, rect.top + nOffset), 
		CSize(8, 7), 
		(HBITMAP)m_x_bmp, DST_BITMAP | DSS_NORMAL);

	dc.Detach();
}

BOOL CCloseBtn::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;
}

