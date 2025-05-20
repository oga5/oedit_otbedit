/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_SELECTCOLORDLG_H__D5DD1881_0E69_11D6_8508_00E018A83B1B__INCLUDED_)
#define AFX_SELECTCOLORDLG_H__D5DD1881_0E69_11D6_8508_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectColorDlg.h : ヘッダー ファイル
//

#define WM_PASTE_COLOR	WM_USER + 150

#include "ColorGridWnd.h"
#include "DlgSingletonTmpl.h"


/////////////////////////////////////////////////////////////////////////////
// CSelectColorDlg ダイアログ

class CSelectColorDlg : public CDialog
{
// コンストラクション
public:
	CSelectColorDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ

	void ShowDialog(CWnd *wnd, int paste_color_message);
	INT_PTR DoModal(CWnd *wnd, int paste_color_message);

	CWnd *GetWnd() { return m_wnd; }

	COLORREF	m_selected_color;

// ダイアログ データ
	//{{AFX_DATA(CSelectColorDlg)
	enum { IDD = IDD_SELECT_COLOR_DLG };
	CString	m_rgb_value;
	int		m_format;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CSelectColorDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	CColorGridWnd	m_color_ctrl;

	BOOL			m_bCapturing;
	COLORREF		m_saved_selected_color;
	HCURSOR			m_spoit_cursor;

	BOOL		m_initialized;
	CWnd		*m_wnd;
	int			m_paste_color_message;

	CString GetRGBValue(int selected_color);
	void CreateColorWnd();
	void GetColorBtnRect(RECT *rect);
	void GetSelectedColorRect(RECT *rect);
	void InvalidateSelectedColor();

	void StartCapture();
	void StopCapture();

	void LoadColorInfo();
	void SaveColorInfo();

	void InitDialog();

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CSelectColorDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnSelColorBtn();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSelchangeComboFormat();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

typedef DlgSingletonTmpl<CSelectColorDlg, IDD_SELECT_COLOR_DLG> CSelectColorDlgSingleton;


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_SELECTCOLORDLG_H__D5DD1881_0E69_11D6_8508_00E018A83B1B__INCLUDED_)
