#if !defined(AFX_SHOWCLOBDLG_H__756751ED_B083_4776_92BF_52A777EC5942__INCLUDED_)
#define AFX_SHOWCLOBDLG_H__756751ED_B083_4776_92BF_52A777EC5942__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ShowCLobDlg.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CShowCLobDlg ダイアログ
#include "resource.h"
#include "EditData.h"

class CShowCLobDlg : public CDialog
{
// コンストラクション
public:
	CShowCLobDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ

	void DoModal2();

	CEditData *GetEditData() { return &m_edit_data; }
	void SetFont(CFont *font) { m_font = font; }
	void ShowDialog();

	void RedrawEditCtrl();

	void SetColName(const TCHAR* col_name);

	CString m_title;

// ダイアログ データ
	//{{AFX_DATA(CShowCLobDlg)
	enum { IDD = IDD_SHOW_CLOB_DLG };
	CButton	m_btn_line_mode_right;
	CButton	m_ok;
	BOOL	m_line_mode_right;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CShowCLobDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CShowCLobDlg)
	virtual void OnOK();
	afx_msg void OnOk2();
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckLineModeRight();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg void OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI );
	DECLARE_MESSAGE_MAP()

private:
	CEditData		m_edit_data;
	CEditCtrl		m_edit_ctrl;
	CFont			*m_font;

	BOOL			m_initialized;
	BOOL			m_b_modal;
	BOOL			m_edit_ctrl_created;

	void InitDialog();
	void CreateEditCtrl();
	void SetLineMode();

	void RelayoutControls();
public:
	afx_msg void OnClose();
	virtual void OnCancel();
};

// DlgSingletonTmplではReleaseビルドでエラーになる
// 2回目以降の表示で、「サポートされていない操作を実行しました」のエラーになる
class CShowCLobDlgHandler
{
private:
	CShowCLobDlg *dlg;
	bool initialized;

	void CreateDlg() {
		dlg = new CShowCLobDlg();
		dlg->Create(IDD_SHOW_CLOB_DLG);
		initialized = true;
	}

public:
	CShowCLobDlgHandler() { 
		initialized = false; 
		dlg = NULL;
	}
	~CShowCLobDlgHandler() {
		Clear();
	}

	void Clear() {
		if(initialized) {
			dlg->DestroyWindow();
			delete dlg;
			dlg = NULL;
		}
		initialized = false;
	}
	
	CShowCLobDlg *GetDlg() {
		if(!AfxGetMainWnd()->IsWindowVisible()) return NULL;

		if(!initialized) {
			// 初回の表示
			CreateDlg();
		} else if(!::IsWindow(dlg->GetSafeHwnd())) {
			// 2回目以降の表示
			// ダイアログを非表示にする処理でDestroyWindowするので、ここでダイアログを再作成する
			Clear();
			CreateDlg();
		}
		return dlg;
	}
	
	bool IsVisible() {
		return (initialized && ::IsWindow(dlg->GetSafeHwnd()) && dlg->IsWindowVisible());
	}
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_SHOWCLOBDLG_H__756751ED_B083_4776_92BF_52A777EC5942__INCLUDED_)
