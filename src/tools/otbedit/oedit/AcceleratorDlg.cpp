/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // AcceleratorDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "resource.h"
#include "AcceleratorDlg.h"

#include "command_list.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static const char *THIS_FILE = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAcceleratorDlg ダイアログ


CAcceleratorDlg::CAcceleratorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAcceleratorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAcceleratorDlg)
	m_new_key = _T("");
	m_current_menu = _T("");
	//}}AFX_DATA_INIT
}

CAcceleratorDlg::~CAcceleratorDlg()
{
}

void CAcceleratorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAcceleratorDlg)
	DDX_Control(pDX, IDC_BTN_DELETE, m_btn_delete);
	DDX_Control(pDX, IDC_BTN_ADD, m_btn_add);
	DDX_Control(pDX, IDC_EDIT_NEW_KEY, m_edit_new_key);
	DDX_Control(pDX, IDC_LIST_COMMAND, m_list_command);
	DDX_Control(pDX, IDC_LIST_CATEGORY, m_list_category);
	DDX_Control(pDX, IDC_LIST_ACCEL, m_list_accel);
	DDX_Text(pDX, IDC_EDIT_NEW_KEY, m_new_key);
	DDX_Text(pDX, IDC_STATIC_CURRENT_MENU, m_current_menu);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAcceleratorDlg, CDialog)
	//{{AFX_MSG_MAP(CAcceleratorDlg)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_CATEGORY, OnItemchangedListCategory)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_COMMAND, OnItemchangedListCommand)
	ON_BN_CLICKED(IDC_BTN_DEFAULT, OnBtnDefault)
	ON_BN_CLICKED(IDC_BTN_ADD, OnBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DELETE, OnBtnDelete)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ACCEL, OnItemchangedListAccel)
	ON_EN_CHANGE(IDC_EDIT_NEW_KEY, OnChangeEditNewKey)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAcceleratorDlg メッセージ ハンドラ

static void SetKeyInfo(HWND hwnd, int key, DWORD keyData)
{
	CString	keyInfo = "";

	keyInfo = CAccelList::get_accel_str(key, (GetAsyncKeyState(VK_MENU) < 0),
		(GetAsyncKeyState(VK_CONTROL) < 0), (GetAsyncKeyState(VK_SHIFT) < 0));

	SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)keyInfo.GetBuffer(0));
}

// 編集用エディットコントロールの，サブクラス後のウィンドウプロシージャ
static LRESULT CALLBACK Edit_SubclassWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		SetKeyInfo(hwnd, (int)wParam, (DWORD)lParam);
		return 0;
	case WM_CHAR:
		return 0;
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;
	default:
		break;
	}
	return (CallWindowProc((WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA),
		hwnd, message, wParam, lParam));
}

BOOL CAcceleratorDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// サブクラス化
	// 古いウィンドウプロシージャを保存する
	HWND hwnd = m_edit_new_key.GetSafeHwnd();
	::SetWindowLongPtr(hwnd, GWLP_USERDATA, GetWindowLongPtr(hwnd, GWLP_WNDPROC));
	// ウィンドウプロシージャを切り替える
	::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)Edit_SubclassWndProc);

	ListView_SetExtendedListViewStyle(m_list_category.GetSafeHwnd(), LVS_EX_FULLROWSELECT);
	m_list_category.InsertColumn(0, _T("分類"), LVCFMT_LEFT, 170);
	SetCategoryList();

	ListView_SetExtendedListViewStyle(m_list_command.GetSafeHwnd(), LVS_EX_FULLROWSELECT);
	m_list_command.InsertColumn(0, _T("機能"), LVCFMT_LEFT, 170);
	m_list_command.InsertColumn(1, _T("説明"), LVCFMT_LEFT, 240);
	m_list_command.InsertColumn(2, _T("キー"), LVCFMT_LEFT, 100);

	ListView_SetExtendedListViewStyle(m_list_accel.GetSafeHwnd(), LVS_EX_FULLROWSELECT);
	m_list_accel.InsertColumn(0, _T("キー"), LVCFMT_LEFT, 170);

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CAcceleratorDlg::SetCategoryList()
{
	int		i;
	LV_ITEM		item;

	m_list_category.DeleteAllItems();
	m_list_command.DeleteAllItems();
	m_list_accel.DeleteAllItems();

	item.mask = LVIF_TEXT | LVIF_PARAM;
	item.iSubItem = 0;
	for(i = 0; i < sizeof(menu_category_list)/sizeof(menu_category_list[0]); i++) {
		item.iItem = i;
		item.pszText = menu_category_list[i].category_name;
		item.lParam = menu_category_list[i].category_id;
		m_list_category.InsertItem(&item);
	}
}

void CAcceleratorDlg::SetCommandList(int category_id)
{
	int			i;
	LV_ITEM		item;
	CString		command_name;
	CString		accel_str;

	m_list_command.DeleteAllItems();
	m_list_accel.DeleteAllItems();

	item.mask = LVIF_TEXT | LVIF_PARAM;
	item.iSubItem = 0;
	for(i = 0; i < sizeof(menu_list)/sizeof(menu_list[0]); i++) {
		if(menu_list[i].category_id != category_id) continue;

		item.iItem = i;
		command_name = m_accel_list.GetCommandName(menu_list[i].menu_id);
		item.pszText = command_name.GetBuffer(0);
		item.lParam = menu_list[i].menu_id;
		item.iItem = m_list_command.InsertItem(&item);
		m_list_command.SetItemText(item.iItem, 1, m_accel_list.GetCommandInfo(menu_list[i].menu_id));

		if(m_accel_list.search_accel_str2(menu_list[i].menu_id, accel_str) == 0) {
			m_list_command.SetItemText(item.iItem, 2, accel_str);
		}
	}

	CheckBtn();
}

void CAcceleratorDlg::SetCommandList_Key()
{
	int			i;
	int			menu_id;
	CString		accel_str;

	for(i = 0; i < m_list_command.GetItemCount(); i++) {
		menu_id = (int)m_list_command.GetItemData(i);

		if(m_accel_list.search_accel_str2(menu_id, accel_str) == 0) {
			m_list_command.SetItemText(i, 2, accel_str);
		} else {
			m_list_command.SetItemText(i, 2, _T(""));
		}
	}

	CheckBtn();
}

void CAcceleratorDlg::OnItemchangedListCategory(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if((pNMListView->uChanged & LVIF_STATE) == 0) return;
	
	if((pNMListView->uNewState & (LVIS_SELECTED | LVNI_ALL)) != 0) {
		SetCommandList((int)pNMListView->lParam);
	}

	*pResult = 0;
}

void CAcceleratorDlg::OnItemchangedListCommand(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if((pNMListView->uChanged & LVIF_STATE) == 0) return;
	
	if((pNMListView->uNewState & (LVIS_SELECTED | LVNI_ALL)) != 0) {
		SetAccelList((int)pNMListView->lParam);
	}
	
	*pResult = 0;
}

void CAcceleratorDlg::SetAccelList(int menu_id)
{
	int			i, pos;
	LV_ITEM		item;
	CString		accel_name;

	m_list_accel.DeleteAllItems();

	item.mask = LVIF_TEXT | LVIF_PARAM;
	item.iSubItem = 0;
	pos = 0;
	for(i = 0;; i++) {
		pos = m_accel_list.search_accel_str(menu_id, pos, accel_name);
		if(pos < 0) break;

		item.iItem = i;
		item.pszText = accel_name.GetBuffer(0);
		item.lParam = menu_list[i].menu_id;
		item.iItem = m_list_accel.InsertItem(&item);
	}

	CheckBtn();
}

void CAcceleratorDlg::CheckBtn()
{
	UpdateData(TRUE);

	int accel_selected_row = m_list_accel.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);

	if(accel_selected_row == -1) {
		m_btn_delete.EnableWindow(FALSE);
	} else {
		m_btn_delete.EnableWindow(TRUE);
	}

	int command_selected_row = m_list_command.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);

	if(command_selected_row == -1 || 
		m_new_key == ""  || m_new_key.GetLength() == 1 ||
		m_new_key.GetAt(m_new_key.GetLength() - 1) == '+') {
		m_btn_add.EnableWindow(FALSE);
	} else {
		m_btn_add.EnableWindow(TRUE);
	}
}

void CAcceleratorDlg::OnBtnDefault() 
{
	if(MessageBox(_T("キー割り当てをデフォルトの設定に戻します。\n")
		_T("よろしいですか？"), _T("確認"), MB_ICONQUESTION | MB_OKCANCEL) == IDCANCEL) return;

	m_accel_list.load_default_accel_list();

	SetCategoryList();
	CheckBtn();
}

void CAcceleratorDlg::OnBtnAdd() 
{
	UpdateData(TRUE);

	int command_selected_row;
	command_selected_row = m_list_command.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(command_selected_row == -1) return;

	m_accel_list.add_accel((WORD)m_list_command.GetItemData(command_selected_row), m_new_key.GetBuffer(0));

	m_new_key = "";
	m_current_menu = "";
	UpdateData(FALSE);

	ResetAccelList();
	SetCommandList_Key();
}

void CAcceleratorDlg::OnBtnDelete() 
{
	int	accel_selected_row;
	accel_selected_row = m_list_accel.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);

	m_accel_list.delete_accel(m_list_accel.GetItemText(accel_selected_row, 0).GetBuffer(0));

	ResetAccelList();
	SetCommandList_Key();
}

void CAcceleratorDlg::ResetAccelList()
{
	int command_selected_row;
	command_selected_row = m_list_command.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(command_selected_row == -1) return;

	SetAccelList((int)m_list_command.GetItemData(command_selected_row));
}

void CAcceleratorDlg::OnItemchangedListAccel(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if((pNMListView->uChanged & LVIF_STATE) == 0) return;

	CheckBtn();
	
	*pResult = 0;
}

void CAcceleratorDlg::OnChangeEditNewKey() 
{
	UpdateData(TRUE);

	CheckBtn();

	int menu_id = m_accel_list.search_cmd(m_new_key.GetBuffer(0));
	if(menu_id > 0) {
		CString current_menu = m_accel_list.GetCommandName(menu_id);
		if(current_menu != "") {
			m_current_menu.Format(_T("現在の割り当て：%s"), current_menu);
		} else {
			m_current_menu = _T("");
		}
	} else {
		m_current_menu = _T("");
	}
	UpdateData(FALSE);
}
