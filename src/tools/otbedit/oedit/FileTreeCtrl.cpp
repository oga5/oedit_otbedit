/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

// FileTreeCtrl.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "fileutil.h"
#include "FileTreeCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSortStringArray
void CSortStringArray::Sort()
{
	BOOL bNotDone = TRUE;   
	while (bNotDone)   
	{      bNotDone = FALSE;
      for(int pos = 0;pos < GetUpperBound();pos++)
        bNotDone |= CompareAndSwap(pos);   
	}
}

BOOL CSortStringArray::CompareAndSwap( int pos )
{
   CString temp;   
   int posFirst = pos;   
   int posNext = pos + 1;
   if (GetAt(posFirst).CompareNoCase(GetAt(posNext)) > 0)   
   {
      temp = GetAt(posFirst);      
	  SetAt(posFirst, GetAt(posNext));
      SetAt(posNext, temp);      
	  return TRUE;   
   }   
   
   return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CFileTreeCtrl

CFileTreeCtrl::CFileTreeCtrl()
{
	m_bFiles = FALSE;
	m_strRoot = "";
	m_strFilter = "*.*";
}

CFileTreeCtrl::~CFileTreeCtrl()
{
	//if(GetImageList(TVSIL_NORMAL)) m_img_list.Detach();
	m_img_list.Detach();
}


BEGIN_MESSAGE_MAP(CFileTreeCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CFileTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnItemexpanded)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileTreeCtrl メッセージ ハンドラ

BOOL CFileTreeCtrl::GetSysImgList()
{
	SHFILEINFO shFinfo;
	HIMAGELIST hImgList = NULL;

	if(GetImageList(TVSIL_NORMAL)) m_img_list.Detach();
	
	hImgList = (HIMAGELIST)SHGetFileInfo( _T(""), 0, &shFinfo, sizeof( shFinfo ), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	if(hImgList == NULL) {
		return FALSE;
	}

	m_img_list.Attach(hImgList);
    
	SetImageList(&m_img_list, TVSIL_NORMAL);

	return TRUE;   // OK
}

BOOL CFileTreeCtrl::DisplayDrives()
{
	DeleteAllItems();
	TCHAR  szDrives[256];
	TCHAR* pDrive;

	// FIXME: フリーズするときがある
	if(!GetLogicalDriveStrings(sizeof(szDrives), szDrives)) {
		return FALSE;
	}

	pDrive = szDrives;
	while(*pDrive) {
		HTREEITEM hParent = AddItem(TVI_ROOT, pDrive);
		if(FindSubDir(pDrive )) {
			InsertItem(_T(""), 0, 0, hParent);
		}
		pDrive += _tcslen(pDrive) + 1;
	}

	return TRUE;
}

HTREEITEM CFileTreeCtrl::AddItem(HTREEITEM hParent, LPCTSTR strPath)
{
	// Adding the Item to the TreeCtrl with the current Icons
	SHFILEINFO shFinfo;
	int iIcon, iIconSel;
    CString    strTemp = strPath;
    
	//if(strTemp.Right(1) != '\\') strTemp += "\\";
	make_dirname(strTemp.GetBuffer(_MAX_PATH));
	strTemp.ReleaseBuffer();
/*
	if(!SHGetFileInfo(strTemp, 0, &shFinfo, sizeof(shFinfo), 
		SHGFI_ICON | SHGFI_SMALLICON)) {
		return NULL;
	}

	iIcon = shFinfo.iIcon;

	// we only need the index from the system image list
	DestroyIcon(shFinfo.hIcon);

	if(!SHGetFileInfo(strTemp, 0, &shFinfo, sizeof(shFinfo),
		SHGFI_ICON | SHGFI_OPENICON | SHGFI_SMALLICON)) {
		return NULL;
	}

	iIconSel = shFinfo.iIcon;

	// we only need the index of the system image list
	DestroyIcon(shFinfo.hIcon);
*/
	if(!SHGetFileInfo(strTemp, 0, &shFinfo, sizeof(shFinfo), 
		SHGFI_SYSICONINDEX | SHGFI_ICON | SHGFI_SMALLICON)) {
		return NULL;
	}
	iIcon = shFinfo.iIcon;

	if(!SHGetFileInfo(strTemp, 0, &shFinfo, sizeof(shFinfo),
		SHGFI_SYSICONINDEX | SHGFI_ICON | SHGFI_OPENICON | SHGFI_SMALLICON)) {
		return NULL;
	}
	iIconSel = shFinfo.iIcon;

	//if(strTemp.Right(1) == "\\") strTemp.SetAt(strTemp.GetLength() - 1, '\0');
	make_dirname2(strTemp.GetBuffer(_MAX_PATH));
	strTemp.ReleaseBuffer();

	if (hParent == TVI_ROOT) return InsertItem(strTemp, iIcon, iIconSel, hParent);
	
	return InsertItem(GetSubPath(strTemp.GetBuffer(0)), iIcon, iIconSel, hParent);
}

TCHAR *CFileTreeCtrl::GetSubPath(LPCTSTR strPath)
{
	//
	// getting the last SubPath from a PathString
	// e.g. C:\temp\readme.txt
	// the result = readme.txt
	static CString strTemp;
	int     iPos;

	strTemp = strPath;
	//if(strTemp.Right(1) == '\\') strTemp.SetAt(strTemp.GetLength() - 1, '\0');
	make_dirname2(strTemp.GetBuffer(_MAX_PATH));
	strTemp.ReleaseBuffer();

	iPos = strTemp.ReverseFind('\\');
	if (iPos != -1) strTemp = strTemp.Mid( iPos + 1);

	return strTemp.GetBuffer(0);
}

BOOL CFileTreeCtrl::FindSubDir(LPCTSTR strPath)
{
	return TRUE;
/* この処理は遅い
	//
	// Are there subDirs ?
	//
	CFileFind find;
	CString   strTemp = strPath;
	BOOL      bFind;

	if(strTemp[strTemp.GetLength()-1] == '\\') {
		strTemp += "*.*";
	} else {
		strTemp += "\\*.*";
	}
		
	char	*filter = m_strFilter.GetBuffer(0);
	bFind = find.FindFile( strTemp );
	
	while(bFind) {
		bFind = find.FindNextFile();

		if(find.IsDirectory() && !find.IsDots()) return TRUE;
		if(!find.IsDirectory() && m_bFiles && !find.IsHidden() &&
			CheckFileType(find.GetFilePath().GetBuffer(0), filter)) return TRUE;
	}

	return FALSE;
*/
}

void CFileTreeCtrl::OnItemexpanded(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	CString strPath;

	if(pNMTreeView->itemNew.state & TVIS_EXPANDED) {
		ExpandItem(pNMTreeView->itemNew.hItem, TVE_EXPAND);
	} else {
		//
		// Delete the Items, but leave one there, for 
		// expanding the item next time
		//
		HTREEITEM hChild = GetChildItem(pNMTreeView->itemNew.hItem);
				
		while(hChild) {
			DeleteItem(hChild);
			hChild = GetChildItem(pNMTreeView->itemNew.hItem);
		}
		InsertItem( _T(""), pNMTreeView->itemNew.hItem );
	}

	*pResult = 0;
}

void CFileTreeCtrl::ExpandItem(HTREEITEM hItem, UINT nCode)
{	
	CString strPath;
	
	if(nCode == TVE_EXPAND) {
		HTREEITEM hChild = GetChildItem(hItem);
		while(hChild) {
			DeleteItem(hChild);
			hChild = GetChildItem(hItem);
		}
        
		strPath = GetFullPath(hItem);
		DisplayPath(hItem, strPath);
	}
}

CString CFileTreeCtrl::GetFullPath(HTREEITEM hItem)
{
	if(hItem == NULL) return "";

	// get the Full Path of the item
	CString strReturn;
	CString strTemp;
	HTREEITEM hParent = hItem;

	strReturn = "";

	while(hParent) {
		strTemp  = GetItemText( hParent );
		strTemp += "\\";
		strReturn = strTemp + strReturn;
		hParent = GetParentItem( hParent );
	}
    
	strReturn.TrimRight('\\');

    return strReturn;
}

void CFileTreeCtrl::DisplayPath(HTREEITEM hParent, LPCTSTR strPath)
{
	//
	// Displaying the Path in the TreeCtrl
	//
	CFileFind find;
	CString   strPathFiles = strPath;
	BOOL      bFind;
	CSortStringArray strDirArray;
	CSortStringArray strFileArray;
	
	//if(strPathFiles.Right(1) != "\\") strPathFiles += "\\";
	make_dirname(strPathFiles.GetBuffer(_MAX_PATH));
	strPathFiles.ReleaseBuffer();

	strPathFiles += "*.*";

	TCHAR	*filter = m_strFilter.GetBuffer(0);

	bFind = find.FindFile(strPathFiles);

	while (bFind) {
		bFind = find.FindNextFile();
		if(find.IsDirectory() && !find.IsDots()) {		
			strDirArray.Add(find.GetFilePath());
		}
		if(!find.IsDirectory() && m_bFiles && CheckFileType(find.GetFilePath().GetBuffer(0), filter)) {
			strFileArray.Add(find.GetFilePath());
		}
	}
    
	strDirArray.Sort();
	SetRedraw(FALSE);
	CWaitCursor wait;

	int i;
	for(i = 0; i < strDirArray.GetSize(); i++) {
		HTREEITEM hItem = AddItem(hParent, strDirArray.GetAt(i));
		if(FindSubDir(strDirArray.GetAt(i))) InsertItem( _T(""), 0, 0, hItem );
	}

	if(m_bFiles) {
		strFileArray.Sort();
		for(i = 0; i < strFileArray.GetSize(); i++) {
			HTREEITEM hItem = AddItem( hParent, strFileArray.GetAt(i));
		}
	}
    
	SetRedraw( TRUE );
}

BOOL CFileTreeCtrl::DisplayTree(LPCTSTR strRoot, BOOL bFiles)
{
	DWORD dwStyle = GetStyle();   // read the windowstyle
	if (dwStyle & TVS_EDITLABELS) {
		// Don't allow the user to edit ItemLabels
		ModifyStyle(TVS_EDITLABELS , 0);
	}
	
	// Display the DirTree with the Rootname e.g. C:\
	// if Rootname == NULL then Display all Drives on this PC
    // First, we need the system-ImageList
	
	DeleteAllItems();

	if(!GetSysImgList()) return FALSE;
    
	m_bFiles = bFiles;  // if TRUE, Display Path- and Filenames 

	if(strRoot == NULL || strRoot[0] == '\0') {
		if(!DisplayDrives()) return FALSE;
		m_strRoot = _T("");
	} else {
		m_strRoot = strRoot;
		//if(m_strRoot.Right(1) != '\\') m_strRoot += "\\";
		make_dirname(m_strRoot.GetBuffer(_MAX_PATH));
		m_strRoot.ReleaseBuffer();
		
		HTREEITEM hParent = AddItem(TVI_ROOT, m_strRoot);
		DisplayPath(hParent, strRoot);
	}
	return TRUE;	
}

BOOL CFileTreeCtrl::SetSelPath(LPCTSTR strPath)
{
	// Setting the Selection in the Tree
	HTREEITEM hParent  = TVI_ROOT;
	size_t     iLen    = _tcslen(strPath) + 2;
	TCHAR*     pszPath = new TCHAR[iLen];
	TCHAR*     pPath   = pszPath;
	BOOL      bRet    = FALSE;
    
	if(!IsValidPath(strPath)) {
		delete [] pszPath; // this must be added 29.03.99
		return FALSE;
	}
		
	_tcscpy(pszPath, strPath);
	_tcsupr(pszPath);
	
	//if(pszPath[strlen(pszPath)-1] != '\\') strcat( pszPath, "\\" );
	make_dirname(pszPath);
    
	size_t iLen2 = _tcslen(pszPath);
	
	for(WORD i = 0; i < iLen2; i++) {
		if(pszPath[i] == '\\') {
			SetRedraw(FALSE);
			pszPath[i] = '\0';
			hParent = SearchSiblingItem( hParent, pPath );
			if (!hParent) {  // Not found!
				break;
			} else {
				// Info:
				// the notification OnItemExpanded 
				// will not called every time 
				// after the call Expand. 
				// You must call Expand with TVE_COLLAPSE | TVE_COLLAPSERESET
				// to Reset the TVIS_EXPANDEDONCE Flag
				
				UINT uState;
				uState = GetItemState(hParent, TVIS_EXPANDEDONCE);
				if(uState) {
					Expand( hParent, TVE_EXPAND );
					Expand( hParent, TVE_COLLAPSE | TVE_COLLAPSERESET );
					InsertItem(_T(""), hParent ); // insert a blank child-item
					Expand( hParent, TVE_EXPAND ); // now, expand send a notification
				} else {
					Expand( hParent, TVE_EXPAND );
				}
			}
			pPath += _tcslen(pPath) + 1;
		}
	}

	delete [] pszPath;
	
	if(hParent) { // Ok the last subpath was found
		SelectItem(hParent); // select the last expanded item
		bRet = TRUE;
	} else {
		bRet = FALSE;
	}
	
	SetRedraw(TRUE);

    return bRet;
}

HTREEITEM CFileTreeCtrl::SearchSiblingItem(HTREEITEM hItem, LPCTSTR strText)
{
	HTREEITEM hFound = GetChildItem(hItem);
	CString   strTemp;
	while (hFound) {
		strTemp = GetItemText(hFound);
        strTemp.MakeUpper();
		if (strTemp == strText) return hFound;
		hFound = GetNextItem(hFound, TVGN_NEXT);
	}

	return NULL;
}

BOOL CFileTreeCtrl::IsValidPath(LPCTSTR strPath)
{
	// This function check the Pathname
	
	HTREEITEM hChild;
	CString   strItem;
	CString   strTempPath = strPath;
	BOOL      bFound = FALSE;
	CFileFind find;

	hChild = GetChildItem(TVI_ROOT);
	strTempPath.MakeUpper();
	//strTempPath.TrimRight('\\');
	make_dirname2(strTempPath.GetBuffer(_MAX_PATH));
	strTempPath.ReleaseBuffer();

	while(hChild) {
		strItem = GetItemText(hChild);
		strItem.MakeUpper();
		if(strItem == strTempPath.Mid( 0, strItem.GetLength())) {
			bFound = TRUE;
			break;
		}
		hChild = GetNextItem(hChild, TVGN_NEXT);
	}
    
	if(!bFound) return FALSE;

	strTempPath += "\\nul";
	if(find.FindFile(strTempPath)) return TRUE;
     
	return FALSE;
}

void CFileTreeCtrl::SetFilter(LPCTSTR strFilter)
{
	m_strFilter = strFilter;
}

BOOL CFileTreeCtrl::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;
}

