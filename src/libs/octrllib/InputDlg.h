/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef _INPUT_DLG_H_INCLUDE
#define _INPUT_DLG_H_INCLUDE

class CInputDlg : public CDialog
{
public:
	CInputDlg();
	~CInputDlg();

	BOOL CreateDlg(CWnd *parent, const TCHAR *title, const TCHAR *value);

	virtual BOOL OnInitDialog();
	virtual void OnOK();

	const TCHAR *GetValue() { return m_value; }

private:
	struct
	{
		DLGTEMPLATE dtp;
		WORD        param[ 4 ];
	} m_dtp;

	CString		m_title;
	CString		m_value;
	CEdit		m_edit;
	CButton		m_ok;
	CButton		m_cancel;

	CFont		m_font;
};

#endif _INPUT_DLG_H_INCLUDE
