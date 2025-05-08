/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef __DLG_SINGLETON_TMPL_INCLUDED__
#define __DLG_SINGLETON_TMPL_INCLUDED__

template <class T, int DLG_ID>
class DlgSingletonTmpl
{
public:
	static T &GetDlg();
	static bool IsVisible();

	static void Show();
	static void Hide();

private:
	DlgSingletonTmpl();
	DlgSingletonTmpl(const DlgSingletonTmpl<T, DLG_ID> &data);
	static void SetWindowPosCenter();

	static bool initialized_;
};

template <class T, int DLG_ID>
bool DlgSingletonTmpl<T, DLG_ID>::initialized_ = false;

template <class T, int DLG_ID>
bool DlgSingletonTmpl<T, DLG_ID>::IsVisible()
{
	return (initialized_ && ::IsWindow(GetDlg().GetSafeHwnd()) && GetDlg().IsWindowVisible());
}

template <class T, int DLG_ID>
void DlgSingletonTmpl<T, DLG_ID>::Show()
{
	if(IsVisible()) Hide();

	GetDlg().ShowWindow(SW_SHOW);
	if(IsVisible())	SetWindowPosCenter();
}

template <class T, int DLG_ID>
void DlgSingletonTmpl<T, DLG_ID>::Hide()
{
	GetDlg().ShowWindow(SW_HIDE);
}

template <class T, int DLG_ID>
void DlgSingletonTmpl<T, DLG_ID>::SetWindowPosCenter()
{
	CRect	main_rect;
	AfxGetMainWnd()->GetWindowRect(&main_rect);
	CRect	win_rect;
	GetDlg().GetWindowRect(&win_rect);

	int		x, y;

	x = main_rect.left + main_rect.Width() / 2 - win_rect.Width() / 2;
	y = main_rect.top + main_rect.Height() / 2 - win_rect.Height() / 2;

	// タスクバーに隠れないようにする
	RECT work_rect;
	if(SystemParametersInfo(SPI_GETWORKAREA, 0, &work_rect, 0)) {
		if(x < work_rect.left) x = work_rect.left;
		if(y < work_rect.top) y = work_rect.top;
	}

	GetDlg().SetWindowPos(NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
}

template <class T, int DLG_ID>
T &DlgSingletonTmpl<T, DLG_ID>::GetDlg()
{
	static T	dlg;

	if(!initialized_ && AfxGetMainWnd()->IsWindowVisible()) {
		dlg.Create(DLG_ID, NULL);
		initialized_ = true;
	} else if(dlg.GetSafeHwnd() == NULL) {
		dlg.Create(DLG_ID, NULL);
	}
	return dlg;
}

#endif __DLG_SINGLETON_TMPL_INCLUDED__

