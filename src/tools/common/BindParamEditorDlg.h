#if !defined(AFX_BINDPARAMEDITORDLG_H__88341956_1120_4CA7_90BE_D1E8DB9617A2__INCLUDED_)
#define AFX_BINDPARAMEDITORDLG_H__88341956_1120_4CA7_90BE_D1E8DB9617A2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BindParamEditorDlg.h : �w�b�_�[ �t�@�C��
//

#include "resource.h"
#include "StrGridData.h"
#include "BindParamGridCtrl.h"
#include "sqlstrtoken.h"

/////////////////////////////////////////////////////////////////////////////
// CBindParamEditorDlg �_�C�A���O
class CBindParamEditorDlg : public CDialog
{
// �R���X�g���N�V����
public:
	CBindParamEditorDlg(CWnd* pParent = NULL);   // �W���̃R���X�g���N�^

	CSQLStrToken		*m_str_token;
	const TCHAR			*m_sql;
	CMapStringToString	*m_bind_data_org;
	CMapStringToString	*m_bind_data_tmp;
	int					m_bind_param_editor_grid_cell_width[2];

// �_�C�A���O �f�[�^
	//{{AFX_DATA(CBindParamEditorDlg)
	enum { IDD = IDD_BIND_PARAM_EDITOR_DLG };
		// ����: ClassWizard �͂��̈ʒu�Ƀf�[�^ �����o��ǉ����܂��B
	//}}AFX_DATA


// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CBindParamEditorDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
protected:

	// �������ꂽ���b�Z�[�W �}�b�v�֐�
	//{{AFX_MSG(CBindParamEditorDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnOk();
	virtual void OnOK();
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnEditCut();
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
//	CStrGridData			m_grid_data;
	CEditableStrGridData	m_grid_data;
	CBindParamGridCtrl		m_grid_ctrl;

	void InitData();
	void CreateGridCtrl();
	void MoveCenter();

	void SaveGridCellWidth();

public:
	afx_msg void OnBnClickedCancel();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_BINDPARAMEDITORDLG_H__88341956_1120_4CA7_90BE_D1E8DB9617A2__INCLUDED_)
