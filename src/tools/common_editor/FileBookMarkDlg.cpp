/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // FileBookMark.cpp : 実装ファイル
//

#include "stdafx.h"
#include "resource.h"
#include "FileBookMarkDlg.h"
#include "afxdialogex.h"
#include "ListCtrlUtil.h"

// CFileBookMark ダイアログ

IMPLEMENT_DYNAMIC(CFileBookMarkDlg, CDialog)

CFileBookMarkDlg::CFileBookMarkDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_BOOKMARK_DLG, pParent)
{
	m_b_multi_sel = FALSE;
}

CFileBookMarkDlg::~CFileBookMarkDlg()
{
}

void CFileBookMarkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_BOOKMARK, m_list_bookmark);
	DDX_Control(pDX, IDC_BUTTON_DEL, m_btn_del);
	DDX_Control(pDX, IDC_BUTTON_DOWN, m_btn_down);
	DDX_Control(pDX, IDC_BUTTON_UP, m_btn_up);
	DDX_Control(pDX, IDOK, m_ok);
}


BEGIN_MESSAGE_MAP(CFileBookMarkDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CFileBookMarkDlg::OnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_DEL, &CFileBookMarkDlg::OnClickedButtonDel)
	ON_BN_CLICKED(IDC_BUTTON_UP, &CFileBookMarkDlg::OnClickedButtonUp)
	ON_BN_CLICKED(IDC_BUTTON_DOWN, &CFileBookMarkDlg::OnClickedButtonDown)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_BOOKMARK, &CFileBookMarkDlg::OnItemchangedListBookmark)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_BOOKMARK, &CFileBookMarkDlg::OnDblclkListBookmark)
END_MESSAGE_MAP()


// CFileBookMark メッセージ ハンドラー

BOOL CFileBookMarkDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO: ここに初期化を追加してください
	ListView_SetExtendedListViewStyle(m_list_bookmark.GetSafeHwnd(), LVS_EX_FULLROWSELECT);

	m_list_bookmark.InsertColumn(0, _T("File Name"), LVCFMT_LEFT, 200);
	m_list_bookmark.InsertColumn(1, _T("Folder"), LVCFMT_LEFT, 200);

	if(m_b_multi_sel) {
		m_list_bookmark.ModifyStyle(LVS_SINGLESEL, 0);
	}
	
	m_edit_flg = FALSE;

	LoadBookmark();

	CheckBtn();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CFileBookMarkDlg::CheckBtn()
{
	int selected_row = m_list_bookmark.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	int selected_cnt = m_list_bookmark.GetSelectedCount();

	if(selected_cnt == 0) {
		m_ok.EnableWindow(FALSE);
	} else {
		m_ok.EnableWindow(TRUE);
	}

	if(selected_cnt != 1) {
		m_btn_del.EnableWindow(FALSE);
		m_btn_up.EnableWindow(FALSE);
		m_btn_down.EnableWindow(FALSE);
	} else {
		m_btn_del.EnableWindow(TRUE);
		m_btn_up.EnableWindow(TRUE);
		m_btn_down.EnableWindow(TRUE);
	}
}

int CFileBookMarkDlg::AddList(CString file_name, CString folder_name)
{
	LV_ITEM		item;
	memset(&item, 0, sizeof(item));

	item.mask = LVIF_TEXT;
	item.iItem = m_list_bookmark.GetItemCount();
	item.iSubItem = 0;
	item.pszText = file_name.GetBuffer(0);

	item.iItem = m_list_bookmark.InsertItem(&item);

	m_list_bookmark.SetItemText(item.iItem, 1, folder_name);

	return item.iItem;
}

void CFileBookMarkDlg::LoadBookmark()
{
	COWinApp* pApp = GetOWinApp();

	int bookmark_cnt = pApp->GetIniFileInt(_T("BOOKMARK"), _T("BOOKMARK_CNT"), 0);
	if(bookmark_cnt == 0) {
		return;
	}

	CString reg_file_key;
	CString reg_folder_key;
	CString file_name;
	CString folder_name;

	m_list_bookmark.DeleteAllItems();

	for(int i = 0; i < bookmark_cnt; i++) {
		reg_file_key.Format(_T("BOOKMARK_FILE_%d"), i + 1);
		reg_folder_key.Format(_T("BOOKMARK_FOLDER_%d"), i + 1);

		file_name = pApp->GetIniFileString(_T("BOOKMARK"), reg_file_key, _T(""));
		folder_name = pApp->GetIniFileString(_T("BOOKMARK"), reg_folder_key, _T(""));

		if(file_name.IsEmpty() || folder_name.IsEmpty()) continue;

		AddList(file_name, folder_name);
	}
}

void CFileBookMarkDlg::SaveBookmark()
{
	if(!m_edit_flg) return;

	COWinApp* pApp = GetOWinApp();

	int bookmark_cnt = m_list_bookmark.GetItemCount();

	CString reg_file_key;
	CString reg_folder_key;
	CString file_name;
	CString folder_name;

	for(int i = 0; i < bookmark_cnt; i++) {
		reg_file_key.Format(_T("BOOKMARK_FILE_%d"), i + 1);
		reg_folder_key.Format(_T("BOOKMARK_FOLDER_%d"), i + 1);

		file_name = m_list_bookmark.GetItemText(i, 0);
		folder_name = m_list_bookmark.GetItemText(i, 1);

		pApp->WriteIniFileString(_T("BOOKMARK"), reg_file_key, file_name);
		pApp->WriteIniFileString(_T("BOOKMARK"), reg_folder_key, folder_name);
	}

	pApp->WriteIniFileInt(_T("BOOKMARK"), _T("BOOKMARK_CNT"), bookmark_cnt);
}

void CFileBookMarkDlg::OnOK()
{
	int selected_cnt = m_list_bookmark.GetSelectedCount();
	if(selected_cnt == 0) return;

	// TODO: ここに特定なコードを追加するか、もしくは基底クラスを呼び出してください。
	SaveBookmark();

	m_open_file_path.Empty();

	int selected_row = m_list_bookmark.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	for(; selected_row != -1;) {
		CString file_name = m_list_bookmark.GetItemText(selected_row, 0);
		CString folder_name = m_list_bookmark.GetItemText(selected_row, 1);
		CString path;

		path.Format(_T("%s\\%s"), folder_name, file_name);
		if(m_open_file_path.IsEmpty()) {
			m_open_file_path = path;
		}

		m_open_file_path_arr.Add(path);

		selected_row = m_list_bookmark.GetNextItem(selected_row, LVNI_SELECTED | LVNI_ALL);
	}

	CDialog::OnOK();
}


void CFileBookMarkDlg::OnCancel()
{
	// TODO: ここに特定なコードを追加するか、もしくは基底クラスを呼び出してください。
	SaveBookmark();

	CDialog::OnCancel();
}

BOOL CFileBookMarkDlg::IsExists(CString file_name, CString folder_name)
{
	int cnt = m_list_bookmark.GetItemCount();

	for(int i = 0; i < cnt; i++) {
		if(file_name.Compare(m_list_bookmark.GetItemText(i, 0)) == 0 &&
			folder_name.Compare(m_list_bookmark.GetItemText(i, 1)) == 0) {
			return TRUE;
		}
	}

	return FALSE;
}

void CFileBookMarkDlg::OnClickedButtonAdd()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CFileDialog dlg(TRUE, NULL, NULL,
		OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
		NULL, this);

	if(dlg.DoModal() == IDOK) {
		CString file_name = dlg.GetFileName();
		CString folder_name = dlg.GetFolderPath();

		if(IsExists(file_name, folder_name)) {
			MessageBox(_T("このファイルは既に登録されています"), _T("Message"), MB_OK);
		} else {
			AddList(file_name, folder_name);
		}

		m_edit_flg = TRUE;
	}
}

void CFileBookMarkDlg::SwapRow(int i1, int i2)
{
	int cnt = m_list_bookmark.GetItemCount();
	if(i1 < 0 || i2 < 0) return;
	if(i1 >= cnt || i2 >= cnt) return;

	CString file_name1 = m_list_bookmark.GetItemText(i1, 0);
	CString folder_name1 = m_list_bookmark.GetItemText(i1, 1);
	CString file_name2 = m_list_bookmark.GetItemText(i2, 0);
	CString folder_name2 = m_list_bookmark.GetItemText(i2, 1);

	m_list_bookmark.SetItemText(i1, 0, file_name2);
	m_list_bookmark.SetItemText(i1, 1, folder_name2);
	m_list_bookmark.SetItemText(i2, 0, file_name1);
	m_list_bookmark.SetItemText(i2, 1, folder_name1);

	ListCtrl_SelectItem(&m_list_bookmark, i2);
}

void CFileBookMarkDlg::OnClickedButtonDel()
{
	int selected_row = m_list_bookmark.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1) return;

	m_list_bookmark.DeleteItem(selected_row);
	CheckBtn();
}

void CFileBookMarkDlg::OnClickedButtonUp()
{
	int selected_row = m_list_bookmark.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1) return;
	SwapRow(selected_row, selected_row - 1);
}


void CFileBookMarkDlg::OnClickedButtonDown()
{
	int selected_row = m_list_bookmark.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1) return;
	SwapRow(selected_row, selected_row + 1);
}

void CFileBookMarkDlg::OnItemchangedListBookmark(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CheckBtn();
	*pResult = 0;
}

void CFileBookMarkDlg::OnDblclkListBookmark(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	if(m_list_bookmark.GetSelectedCount() == 1) {
		OnOK();
	}

	*pResult = 0;
}
