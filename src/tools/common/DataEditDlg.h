#if !defined(AFX_DATAEDITDLG_H__10BFC25B_FD66_49DF_ADE0_FEF20FAB6173__INCLUDED_)
#define AFX_DATAEDITDLG_H__10BFC25B_FD66_49DF_ADE0_FEF20FAB6173__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DataEditDlg.h : �w�b�_�[ �t�@�C��
//

/////////////////////////////////////////////////////////////////////////////
// CDataEditDlg �_�C�A���O

#include "EditCtrl.h"
#include "GridCtrl.h"

class CDataEditDlg : public CDialog
{
// �R���X�g���N�V����
public:
	CDataEditDlg(CWnd* pParent = NULL);   // �W���̃R���X�g���N�^

	void SetData(CEditData *edit_data, int line_type,
		CFont *font, QWORD edit_style, COLORREF *grid_color);

	void SetWindowTitle(const TCHAR *title) { m_window_title = title; }
	void SetReadOnly(BOOL b) { m_read_only = b; }

	BOOL IsEditData();

// �_�C�A���O �f�[�^
	//{{AFX_DATA(CDataEditDlg)
	enum { IDD = IDD_DATA_EDIT_DLG };
	CStatic	m_static_line_type;
	CButton	m_cancel;
	CComboBox	m_combo_line_type;
	CButton	m_ok;
	BOOL	m_line_mode_right;
	//}}AFX_DATA


// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CDataEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
protected:

	// �������ꂽ���b�Z�[�W �}�b�v�֐�
	//{{AFX_MSG(CDataEditDlg)
	virtual void OnOK();
	afx_msg void OnOk2();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelendokComboLineType();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCheckLineModeRight();
	virtual void OnCancel();
	//}}AFX_MSG
	afx_msg void OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI );
	DECLARE_MESSAGE_MAP()

private:
	CEditCtrl		m_edit_ctrl;
	CEditData		*m_edit_data;
	CFont			*m_font;
	QWORD			m_edit_style;
	COLORREF		*m_grid_color;

	CString			m_window_title;

	int				m_line_type;
	int				m_ini_line_type;

	BOOL			m_read_only;

	void InitLineType();
	int GetLineType();
	void SetLineType(int line_type);
	void CreateEditCtrl();

	void RelayoutControls();
	void SetLineMode();
	void SaveSetting();
public:
	CButton m_check_line_mode_right;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_DATAEDITDLG_H__10BFC25B_FD66_49DF_ADE0_FEF20FAB6173__INCLUDED_)
