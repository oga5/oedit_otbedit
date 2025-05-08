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
// FileTreeCtrl.h : �w�b�_�[ �t�@�C��
//

/////////////////////////////////////////////////////////////////////////////
// CFileTreeCtrl �E�B���h�E
class CSortStringArray : public CStringArray  
{
public:
	void Sort();
private: 
	BOOL CompareAndSwap(int pos);
};

class CFileTreeCtrl : public CTreeCtrl
{
// �R���X�g���N�V����
public:
	CFileTreeCtrl();

// �A�g���r���[�g
private:
	CString		m_strRoot;
	CString		m_strFilter;
	CImageList	m_img_list;
	BOOL		m_bFiles;

public:

// �I�y���[�V����
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
	
// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CFileTreeCtrl)
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
public:
	virtual ~CFileTreeCtrl();

	// �������ꂽ���b�Z�[�W �}�b�v�֐�
protected:
	//{{AFX_MSG(CFileTreeCtrl)
	afx_msg void OnItemexpanded(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_FILETREECTRL_H__2E626494_9ED1_4710_8C43_849AF52FC246__INCLUDED_)
