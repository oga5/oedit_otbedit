// BindParamGridCtrl.cpp : �C���v�������e�[�V���� �t�@�C��
//

#include "stdafx.h"
#include "BindParamGridCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBindParamGridCtrl

CBindParamGridCtrl::CBindParamGridCtrl()
{
}

CBindParamGridCtrl::~CBindParamGridCtrl()
{
}


BEGIN_MESSAGE_MAP(CBindParamGridCtrl, CGridCtrl)
	//{{AFX_MSG_MAP(CBindParamGridCtrl)
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBindParamGridCtrl ���b�Z�[�W �n���h��

void CBindParamGridCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: ���̈ʒu�Ƀ��b�Z�[�W �n���h���p�̃R�[�h��ǉ����邩�܂��̓f�t�H���g�̏������Ăяo���Ă�������
	BOOL	ctrl = FALSE;
	BOOL	shift = FALSE;
	if(GetKeyState(VK_CONTROL) < 0) ctrl = TRUE;
	if(GetKeyState(VK_SHIFT) < 0) shift = TRUE;

	switch(nChar) {
	case VK_ESCAPE:
		GetParent()->PostMessage(WM_KEYDOWN, nChar, MAKELPARAM(nRepCnt, nFlags));
		break;
	case VK_TAB:
	case VK_RETURN:
		if(shift) {
			if(m_grid_data->get_cur_row() == 0) {
				if(GetParent()->GetNextDlgTabItem(this, TRUE) != NULL) {
					GetParent()->GetNextDlgTabItem(this, TRUE)->SetFocus();
				}
				return;
			}
			if(m_grid_data->get_cur_col() == 0) {
				MoveCell(1, 0);
			}
			MoveCell(0, -1);
			ClearSelected();
		} else {
			if(m_grid_data->get_cur_row() == m_grid_data->Get_RowCnt() - 1) {
				if(GetParent()->GetNextDlgTabItem(this, FALSE) != NULL) {
					GetParent()->GetNextDlgTabItem(this, FALSE)->SetFocus();
				}
				return;
			}
			if(m_grid_data->get_cur_col() == 0) {
				MoveCell(1, 0);
			}
			MoveCell(0, 1);
		}
		return;
	}	

	CGridCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}
