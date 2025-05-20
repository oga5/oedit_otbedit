/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "ListCtrlUtil.h"

int ListCtrl_GetColumnCount(CListCtrl *p_list)
{
	CHeaderCtrl* pHeader = (CHeaderCtrl*)(p_list->GetDlgItem(0));

	//Return the number of items in it (i.e. the number of columns)
	return pHeader->GetItemCount();
}

CString ListCtrl_GetColumnName(CListCtrl *p_list, int col)
{
	LV_COLUMN	lv_column;
	TCHAR		buf[1024];

	lv_column.mask = LVCF_TEXT;
	lv_column.pszText = buf;
	lv_column.cchTextMax = sizeof(buf);

	p_list->GetColumn(col, &lv_column);

	return buf;
}

void ListCtrlSwapData(CListCtrl *p_list, int r1, int r2)
{
	if(r1 < 0 || r2 < 0 ||
		r1 >= p_list->GetItemCount() || r2 >= p_list->GetItemCount()) return;

	CString		tmp;
	int			col_cnt = ListCtrl_GetColumnCount(p_list);

	for(int i = 0; i < col_cnt; i++) {
		tmp = p_list->GetItemText(r1, i);

		p_list->SetItemText(r1, i, p_list->GetItemText(r2, i));
		p_list->SetItemText(r2, i, tmp);
	}

	LVITEM listitem1;
	LVITEM listitem2;
	listitem1.iItem = r1;
	listitem1.mask = LVIF_IMAGE;
	listitem1.iSubItem = 0;
	listitem2.iItem = r2;
	listitem2.mask = LVIF_IMAGE;
	listitem2.iSubItem = 0;
	p_list->GetItem(&listitem1);
	p_list->GetItem(&listitem2);

	int tmpimage = listitem1.iImage;
	listitem1.iImage = listitem2.iImage;
	listitem2.iImage = tmpimage;
	p_list->SetItem(&listitem1);
	p_list->SetItem(&listitem2);

	p_list->SetItemState(r2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	p_list->EnsureVisible(r2, FALSE);
}

void ListCtrl_SelectItem(CListCtrl *p_list, int i)
{
	p_list->SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	p_list->EnsureVisible(i, FALSE);
}

void ListCtrl_SelectAll(CListCtrl* p_list)
{
	if(p_list->GetStyle() & LVS_SINGLESEL) return;

	int		i;
	int		cnt = p_list->GetItemCount();

	for(i = 0; i < cnt; i++) {
		p_list->SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
	}
}

// CListCtrlのサブクラス化
static LRESULT CALLBACK ListCtrl_MouseHWheel_SubclassWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
	case WM_MOUSEHWHEEL:
	{
		// adjust current x position
		int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		SendMessage(hwnd, LVM_SCROLL, zDelta, 0);
		return 0;
	}
	break;
	default:
		break;
	}
	return (CallWindowProc((WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA),
		hwnd, message, wParam, lParam));
}

void ListCtrl_EnableMouseHWheel(CListCtrl* p_list)
{
	// サブクラス化
	// デフォルトのウィンドウプロシージャを保存する
	HWND hwnd = p_list->GetSafeHwnd();
	::SetWindowLongPtr(hwnd, GWLP_USERDATA, GetWindowLongPtr(hwnd, GWLP_WNDPROC));
	// ウィンドウプロシージャを切り替える
	::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)ListCtrl_MouseHWheel_SubclassWndProc);
}
