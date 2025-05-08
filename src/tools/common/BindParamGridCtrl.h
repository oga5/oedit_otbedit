#if !defined(AFX_BINDPARAMGRIDCTRL_H__5326AA1C_CB37_4E16_9E7B_3400CEF4844C__INCLUDED_)
#define AFX_BINDPARAMGRIDCTRL_H__5326AA1C_CB37_4E16_9E7B_3400CEF4844C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BindParamGridCtrl.h : ヘッダー ファイル
//

#include "gridctrl.h"

/////////////////////////////////////////////////////////////////////////////
// CBindParamGridCtrl ウィンドウ

class CBindParamGridCtrl : public CGridCtrl
{
// コンストラクション
public:
	CBindParamGridCtrl();

// アトリビュート
public:

// オペレーション
public:

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CBindParamGridCtrl)
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CBindParamGridCtrl();

	// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CBindParamGridCtrl)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_BINDPARAMGRIDCTRL_H__5326AA1C_CB37_4E16_9E7B_3400CEF4844C__INCLUDED_)
