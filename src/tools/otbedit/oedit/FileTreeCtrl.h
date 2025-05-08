/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_FILETREECTRL_H__2E626494_9ED1_4710_8C43_849AF52FC246__INCLUDED_)
#define AFX_FILETREECTRL_H__2E626494_9ED1_4710_8C43_849AF52FC246__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileTreeCtrl.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CFileTreeCtrl ウィンドウ
class CSortStringArray : public CStringArray  
{
public:
	void Sort();
private: 
	BOOL CompareAndSwap(int pos);
};

class CFileTreeCtrl : public CTreeCtrl
{
// コンストラクション
public:
	CFileTreeCtrl();

// アトリビュート
private:
	CString		m_strRoot;
	CString		m_strFilter;
	CImageList	m_img_list;
	BOOL		m_bFiles;

public:

// オペレーション
private:
	BOOL GetSysImgList();
	BOOL DisplayDrives();
	BOOL FindSubDir(LPCTSTR strPath);
	HTREEITEM AddItem(HTREEITEM hParent, LPCTSTR strPath);
	void ExpandItem(HTREEITEM hItem, UINT nCode);
	void DisplayPath(HTREEITEM hParent, LPCTSTR strPath);
	HTREEITEM SearchSiblingItem(HTREEITEM hItem, LPCTSTR strText);
	BOOL IsValidPath(LPCTSTR strPath);
	CString GetFullPath(HTREEITEM hItem);

public:
	BOOL DisplayTree(LPCTSTR strRoot, BOOL bFiles);
	void SetFilter(LPCTSTR strFilter);
	BOOL SetSelPath(LPCTSTR strPath);
	CString GetFullPath() { return GetFullPath(GetSelectedItem()); }
	TCHAR *GetSubPath(LPCTSTR strPath);
	
// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CFileTreeCtrl)
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CFileTreeCtrl();

	// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CFileTreeCtrl)
	afx_msg void OnItemexpanded(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_FILETREECTRL_H__2E626494_9ED1_4710_8C43_849AF52FC246__INCLUDED_)
