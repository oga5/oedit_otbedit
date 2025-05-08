/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#if !defined(AFX_CODEASSISTWND_H__1353F5C6_0C2F_41BB_AC84_853CFB865100__INCLUDED_)
#define AFX_CODEASSISTWND_H__1353F5C6_0C2F_41BB_AC84_853CFB865100__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CodeAssistWnd.h : �w�b�_�[ �t�@�C��
//

#include "GridCtrl.h"
#include "octrl_msg.h"
#include "CodeAssistData.h"

/////////////////////////////////////////////////////////////////////////////
// CCodeAssistWnd �E�B���h�E

enum CodeAssistMode {
	AssistCommit,
	AssistTemp,
	AssistInitial,	// �������p�_�~�[
};

class CCodeAssistWnd : public CGridCtrl
{
	DECLARE_DYNAMIC(CCodeAssistWnd)

// �R���X�g���N�V����
public:
	CCodeAssistWnd();

	void SetAssistMode(CodeAssistMode mode);
	CodeAssistMode GetAssistMode() { return m_mode; }

	int CalcWindowWidth();
	int CalcWindowHeight();

	BOOL GetMatchData(const TCHAR *word, int *match_row, BOOL no_break_cur_grid_data);
	void SelectData(const TCHAR *word);

	// override
	BOOL Create(CWnd *pParentWnd);

	void SetIncrSearch(BOOL b_incr) { m_incremental_search = b_incr; }
	void SetAssistMatchType(int t) { m_assist_match_type = t; }
	void SetGridData(CCodeAssistData *griddata);
	CCodeAssistData *GetCurGridData() { return m_cur_griddata; }

	void UnSelect();

	void SetAssistWindowPos(POINT pt, int row_height);

	void LineUp(BOOL b_loop, CString word = _T(""));
	void LineDown(BOOL b_loop, CString word = _T(""));

	void SetDispCnt(int cnt) { m_disp_cnt = cnt; }
	int GetDispCnt() { return m_disp_cnt; }

	void SetCodeAssistMaxCommentDispWidth(int w) { m_max_comment_disp_width = w; }

// �A�g���r���[�g
private:
	CodeAssistMode	m_mode;
	CWnd			*m_parent_wnd;
	UINT_PTR		m_focus_timer;

	BOOL			m_incremental_search;
	int				m_assist_match_type;

	CCodeAssistData *m_cur_griddata;
	CCodeAssistData *m_orig_griddata;
	CCodeAssistData m_disp_griddata;

	POINT			m_window_pt;
	int				m_row_height;
	int				m_disp_cnt;
	int				m_max_comment_disp_width;

	void RecalcWindowSize();

	void CopyAllDispData();

	BOOL GetMatchDataForwardMatch(const TCHAR* word, int* match_row, BOOL no_break_cur_grid_data);
	BOOL GetMatchDataPartialMatch(const TCHAR* word, int* match_row, BOOL no_break_cur_grid_data);


// �I�y���[�V����
public:

// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B

	//{{AFX_VIRTUAL(CCodeAssistWnd)
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
public:
	virtual ~CCodeAssistWnd();

	// �������ꂽ���b�Z�[�W �}�b�v�֐�
protected:
	//{{AFX_MSG(CCodeAssistWnd)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnActivateApp(BOOL bActive, DWORD hTask);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_CODEASSISTWND_H__1353F5C6_0C2F_41BB_AC84_853CFB865100__INCLUDED_)
