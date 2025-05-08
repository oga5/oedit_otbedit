/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "owinutil.h"

int GetPopupMenuHeight(HMENU hmenu)
{
	int		i;
	int		item_cnt = ::GetMenuItemCount(hmenu);
	int		height = ::GetSystemMetrics(SM_CYEDGE) * 2;

	// FIXME:: アイテムの高さとセパレータの正しい値を取得する方法を見つける
	int		sepa_height = ::GetSystemMetrics(SM_CYMENU) - 6;
	int		item_height = ::GetSystemMetrics(SM_CYMENU) - 2;

	for(i = 0; i < item_cnt; i++) {
		MENUITEMINFO item_info;
		memset(&item_info, 0, sizeof(item_info));
		item_info.cbSize = sizeof(item_info);
		item_info.fMask = MIIM_TYPE | MIIM_STATE;

		::GetMenuItemInfo(hmenu, i, TRUE, &item_info);
		if(item_info.fType & MFT_SEPARATOR) {
			height += sepa_height;
		} else {
			height += item_height;
		}
	}

	return height;
}

BOOL Set256ColorToolBar(CToolBar *tool_bar, UINT bitmap_id)
{
	// toolbarを256色のbitmapにする (firefoxアイコン対応)
	// FIXME: 言語のボタンが正しく表示されない
	// 画面の色数を取得する
	HDC ic = CreateIC(_T("DISPLAY"), NULL, NULL, NULL);
	//16bitカラー以上なら256アイコンを利用する
	if(GetDeviceCaps(ic, BITSPIXEL) <= 8) return FALSE;


	int nBtnCnt = 4; //ボタンの数
	//ツールバーのイメージリストを初期化
	CImageList* pImgList = tool_bar->GetToolBarCtrl().GetImageList();

	//イメージリストを破棄する。
	pImgList->DeleteImageList();

	//イメージリストを16bit(3,2000色)で生成する。
	//256色のアイコンは16bit以上でないと色化けしてしまします。
	pImgList->Create(16,15, ILC_COLOR16 | ILC_MASK,nBtnCnt,nBtnCnt);
	//ビットマップを読み込む
	CBitmap bmp;
	bmp.LoadBitmap(bitmap_id);

	//イメージリストに追加する。
	//透過色を２番目の引数で設定している。
	pImgList->Add(&bmp, RGB(192, 192, 192));

	//256アイコンをToolBarに割り当てる
	tool_bar->GetToolBarCtrl().SetImageList(pImgList);
	DeleteObject(bmp);

	return TRUE;
}

BOOL SetClipboardText(const TCHAR *txt, TCHAR *msg_buf)
{
	size_t len = _tcslen(txt);
	HGLOBAL hData = GlobalAlloc(GHND, (len + 1) * sizeof(TCHAR));
	if(hData == NULL) {
		if(msg_buf != NULL) _stprintf(msg_buf, _T("GlobalAlloc Error"));
		return FALSE;
	}
	TCHAR *pstr = (TCHAR *)GlobalLock(hData);
	if(pstr == NULL) {
		if(msg_buf != NULL) _stprintf(msg_buf, _T("GlobalLock Error"));
		return FALSE;
	}

	_tcscpy(pstr, txt);
	GlobalUnlock(hData);

	if (!OpenClipboard(NULL)) {
		if(msg_buf != NULL) _stprintf(msg_buf, _T("Cannot open the Clipboard"));
		return FALSE;
	}
	// Remove the current Clipboard contents
	if( !EmptyClipboard() ) {
		if(msg_buf != NULL) _stprintf(msg_buf, _T("Cannot empty the Clipboard"));
		return FALSE;
	}
	// ...
	// Get the currently selected data
	// ...
	// For the appropriate data formats...
	if ( ::SetClipboardData( CF_UNICODETEXT, hData ) == NULL ) {
		if(msg_buf != NULL) _stprintf(msg_buf, _T("Unable to set Clipboard data"));
		CloseClipboard();
		return FALSE;
	}
	// ...
	CloseClipboard();
	return TRUE;
}