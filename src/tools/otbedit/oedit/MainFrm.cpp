/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

// MainFrm.cpp : CMainFrame �N���X�̓���̒�`���s���܂��B
//

#include "stdafx.h"
#include "oedit.h"
#include "greputil.h"
#include "oeditdoc.h"
#include "scm_macro.h"
#include "owinutil.h"

#include "MainFrm.h"
#include "AcceleratorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static const char *THIS_FILE = __FILE__;
#endif


// XP�X�^�C����K�p�����Ƃ��A�c�[���o�[��Ń}�E�X�̗��{�^�����������ŁA�n���O�A�b�v���Ă��܂��������
static LRESULT CALLBACK ToolBar_SubclassWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		return 0;
	default:
		break;
	}
	return (CallWindowProc((WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA),
		hwnd, message, wParam, lParam));
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_DROPFILES()
	ON_COMMAND(ID_VIEW_TOOLBAR_EDIT_MODE, OnViewToolbarEditMode)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLBAR_EDIT_MODE, OnUpdateViewToolbarEditMode)
	ON_COMMAND(ID_ACCELERATOR, OnAccelerator)
	ON_WM_INITMENU()
	ON_WM_INITMENUPOPUP()
	ON_UPDATE_COMMAND_UI(ID_SHOW_TOPMOST, OnUpdateShowTopmost)
	ON_COMMAND(ID_SHOW_TOPMOST, OnShowTopmost)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FILE_BAR, OnUpdateViewFileBar)
	ON_COMMAND(ID_VIEW_FILE_BAR, OnViewFileBar)
	ON_WM_MOVE()
	ON_WM_ACTIVATEAPP()
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_CURSOR_POS, OnUpdateCursorPos)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_FILE_TYPE, OnUpdateFileType)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_BOX_SELECT, OnUpdateBoxSelect)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // �X�e�[�^�X ���C�� �C���W�P�[�^
//	ID_INDICATOR_KANA,
	ID_INDICATOR_FILE_TYPE,
	ID_INDICATOR_CURSOR_POS,
	ID_INDICATOR_OVR,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_BOX_SELECT,
//	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame �N���X�̍\�z/����

CMainFrame::CMainFrame()
{
	m_accel = NULL;
}

CMainFrame::~CMainFrame()
{
	DestroyAccelerator();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// �E�B���h�E�T�C�Y��O�񗘗p���̑傫���ɂ���
	int win_left, win_top, win_width, win_height;
	GetNextWindowPos(win_left, win_top, win_width, win_height);
	if(win_width != 0 && win_height != 0) {
		//SetWindowPos(NULL, 0, 0, win_width, win_height, SWP_NOMOVE | SWP_NOZORDER);
		SetWindowPos(NULL, win_left, win_top, win_width, win_height, SWP_NOZORDER);
	}
	SetNextWindowPos();

	if (!m_wndToolBar.CreateEx(this, 0, 
		WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | 
		CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // �쐬�Ɏ��s
	}
	// XP�X�^�C����K�p����Ƃ��Astyle = 0��CreateEx���Ă���AModifyStyle�ŃX�^�C����ݒ肷��
	// �������Ȃ��ƁA���[�̃O���b�v�ʒu���������`�悳��Ȃ�
	m_wndToolBar.ModifyStyle(0, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT);

	// �T�u�N���X��
	// �Â��E�B���h�E�v���V�[�W����ۑ�����
	HWND hwnd = m_wndToolBar.GetSafeHwnd();
	::SetWindowLongPtr(hwnd, GWLP_USERDATA, GetWindowLongPtr(hwnd, GWLP_WNDPROC));
	// �E�B���h�E�v���V�[�W����؂�ւ���
	::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)ToolBar_SubclassWndProc);

	if (!m_wndToolBar2.CreateEx(this, 0, 
		WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | 
		CBRS_FLYBY | CBRS_SIZE_DYNAMIC, CRect(0,0,0,0), AFX_IDW_CONTROLBAR_FIRST + 33) ||
		!m_wndToolBar2.LoadToolBar(IDR_EDIT_MODE))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // �쐬�Ɏ��s
	}
	// XP�X�^�C����K�p����Ƃ��Astyle = 0��CreateEx���Ă���AModifyStyle�ŃX�^�C����ݒ肷��
	// �������Ȃ��ƁA���[�̃O���b�v�ʒu���������`�悳��Ȃ�
	m_wndToolBar2.ModifyStyle(0, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT);

	// �T�u�N���X��
	// �Â��E�B���h�E�v���V�[�W����ۑ�����
	HWND hwnd2 = m_wndToolBar2.GetSafeHwnd();
	::SetWindowLongPtr(hwnd2, GWLP_USERDATA, GetWindowLongPtr(hwnd2, GWLP_WNDPROC));
	// �E�B���h�E�v���V�[�W����؂�ւ���
	::SetWindowLongPtr(hwnd2, GWLP_WNDPROC, (LONG_PTR)ToolBar_SubclassWndProc);

	Set256ColorToolBar(&m_wndToolBar2, IDB_TOOLBAR1_256);

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // �쐬�Ɏ��s
	}

    if (!g_file_list_bar.Create(_T("FileListBar"), this, CSize(200, 200),
        TRUE, AFX_IDW_CONTROLBAR_FIRST + 34, WS_CHILD | CBRS_TOP))
    {
        TRACE0("Failed to create ObjBar\n");
        return -1;      // fail to create
	}

	// TODO: �����c�[�� �`�b�v�X���K�v�Ȃ��ꍇ�A�������폜���Ă��������B
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	g_file_list_bar.SetBarStyle(g_file_list_bar.GetBarStyle() | CBRS_SIZE_DYNAMIC);

	// �C���W�P�[�^�̌����ڂ𒲐�
	m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_FILE_TYPE, SBPS_NORMAL, 120);

	// �C���W�P�[�^�̌����ڂ𒲐�
	m_wndStatusBar.SetPaneInfo(2, ID_INDICATOR_CURSOR_POS, SBPS_NORMAL, 100);

	// �h�b�L���O
	EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);
	m_wndToolBar2.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar2);

    g_file_list_bar.EnableDocking(CBRS_ALIGN_ANY);
    DockControlBar(&g_file_list_bar, AFX_IDW_DOCKBAR_RIGHT);

	CSizingControlBar::GlobalLoadState(this, _T("BarState"));
	LoadBarState(_T("BarState"));

	//g_file_list_bar.Init(g_option.file_list_filter);
	if(g_file_list_bar.GetStyle() & WS_VISIBLE) {
		file_list_bar_init(&g_file_list_bar, g_option.file_list_filter);
	}

	SetLangToolBar();

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: ���̈ʒu�� CREATESTRUCT cs ���C�����āAWindow �N���X��X�^�C����
	//       �C�����Ă��������B

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame �N���X�̐f�f

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame ���b�Z�[�W �n���h��

void CMainFrame::OnUpdateFileType(CCmdUI *pCmdUI)
{
	pCmdUI->SetText(g_file_type_name);
}

void CMainFrame::OnUpdateCursorPos(CCmdUI *pCmdUI)
{
	static POINT cur_pos = {-1, -1};

	if(cur_pos.x == g_cur_pos.x && cur_pos.y == g_cur_pos.y) return;

	cur_pos = g_cur_pos;
	CString str;
	str.Format(_T(" %d �s�C%d ��"), cur_pos.y, cur_pos.x);
	pCmdUI->SetText(str);
}

void CMainFrame::OnDestroy() 
{
	CFrameWnd::OnDestroy();
	
	COWinApp *pApp = GetOWinApp();

	WINDOWPLACEMENT wndPlace;
	wndPlace.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&wndPlace);
	RECT rcWnd = wndPlace.rcNormalPosition;

	pApp->WriteIniFileInt(_T("WINDOW"), _T("LEFT"), rcWnd.left);
	pApp->WriteIniFileInt(_T("WINDOW"), _T("TOP"), rcWnd.top);
	pApp->WriteIniFileInt(_T("WINDOW"), _T("WIDTH"), rcWnd.right - rcWnd.left);
	pApp->WriteIniFileInt(_T("WINDOW"), _T("HEIGHT"), rcWnd.bottom - rcWnd.top);

	if(IsZoomed()) {
		pApp->WriteIniFileInt(_T("WINDOW"), _T("ZOOMED"), TRUE);
	} else {
		pApp->WriteIniFileInt(_T("WINDOW"), _T("ZOOMED"), FALSE);
	}

	pApp->SaveIniFile();
}

void CMainFrame::OnDropFiles(HDROP hDropInfo) 
{
	SetForegroundWindow();

	SetActiveWindow();      // activate us first !
	UINT nFiles = ::DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);
	if(nFiles == 0) return;

	CWinApp* pApp = AfxGetApp();
	ASSERT(pApp != NULL);

	// �ЂƂ߂̃t�@�C��
	TCHAR szFileName[_MAX_PATH];
	::DragQueryFile(hDropInfo, 0, szFileName, _MAX_PATH);
	pApp->OpenDocumentFile(szFileName);

	// ��ڈڍs�̃t�@�C��
	for (UINT iFile = 1; iFile < nFiles; iFile++) {
		::DragQueryFile(hDropInfo, iFile, szFileName, _MAX_PATH);
		pApp->OpenDocumentFile(szFileName);
		//OeditOpenFile(szFileName);
	}
	::DragFinish(hDropInfo);

//	CFrameWnd::OnDropFiles(hDropInfo);
}

BOOL CMainFrame::DestroyWindow() 
{
	CSizingControlBar::GlobalSaveState(this, _T("BarState"));
	SaveBarState(_T("BarState"));
	
	return CFrameWnd::DestroyWindow();
}

void CMainFrame::OnViewToolbarEditMode() 
{
	ShowControlBar(&m_wndToolBar2, (m_wndToolBar2.GetStyle() & WS_VISIBLE) == 0, FALSE);
}

void CMainFrame::OnUpdateViewToolbarEditMode(CCmdUI* pCmdUI) 
{
	if((m_wndToolBar2.GetStyle() & WS_VISIBLE) == 0) {
		pCmdUI->SetCheck(0);
	} else {
		pCmdUI->SetCheck(1);
	}
}

void CMainFrame::SetNextWindowPos()
{
	WINDOWPLACEMENT wndPlace;
	wndPlace.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&wndPlace);
	RECT rcWnd = wndPlace.rcNormalPosition;

	COWinApp *pApp = GetOWinApp();

	pApp->WriteIniFileInt(_T("WINDOW"), _T("TOP"), rcWnd.top + GetSystemMetrics(SM_CYCAPTION));
	pApp->WriteIniFileInt(_T("WINDOW"), _T("LEFT"), rcWnd.left + GetSystemMetrics(SM_CYCAPTION));
	pApp->SaveIniFile();
}

void CMainFrame::GetNextWindowPos(int &left, int &top, int &width, int &height)
{
	COWinApp *pApp = GetOWinApp();
	left = pApp->GetIniFileInt(_T("WINDOW"), _T("LEFT"), 0);
	top = pApp->GetIniFileInt(_T("WINDOW"), _T("TOP"), 0);
	width = pApp->GetIniFileInt(_T("WINDOW"), _T("WIDTH"), 0);
	height = pApp->GetIniFileInt(_T("WINDOW"), _T("HEIGHT"), 0);

	{
		// �}���`���j�^�[�Ή�
		// ���j�^�[�����ς�������ƂŋN�������Ƃ��A���C���E�B���h�E�̈ʒu����ʊO�ɂȂ�Ȃ��悤�ɂ���
		RECT rcWnd;
		rcWnd.left = left;
		rcWnd.top = top;
		rcWnd.right = left + width;
		rcWnd.bottom = top + height;

		HMONITOR hMonitor = MonitorFromRect(&rcWnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi;
		mi.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hMonitor, &mi);

		RECT rcMonitor = mi.rcWork;

		if(rcWnd.left < rcMonitor.left || rcWnd.left > rcMonitor.right || rcWnd.right > rcMonitor.right) {
			rcWnd.left = rcMonitor.left;
			rcWnd.right = min(rcWnd.left + width, rcMonitor.right);
		}
		if(rcWnd.top < rcMonitor.top || rcWnd.top > rcMonitor.bottom || rcWnd.bottom > rcMonitor.bottom) {
			rcWnd.top = rcMonitor.top;
			rcWnd.bottom = min(rcWnd.top + height, rcMonitor.bottom);
		}

		left = rcWnd.left;
		top = rcWnd.top;
		width = rcWnd.right - rcWnd.left;
		height = rcWnd.bottom - rcWnd.top;
	}
}

void CMainFrame::CreateAccelerator()
{
	DestroyAccelerator();

	m_accel = CreateAcceleratorTable(m_accel_list.get_accel_list(), m_accel_list.get_accel_cnt());
	m_hAccelTable = m_accel;
}

void CMainFrame::DestroyAccelerator()
{
	if(m_accel != NULL) {
		DestroyAcceleratorTable(m_accel);
		m_accel = NULL;
		m_hAccelTable = NULL;
	}
}

void CMainFrame::OnAccelerator() 
{
	CAcceleratorDlg	dlg;

	dlg.m_accel_list = m_accel_list;

	if(dlg.DoModal() != IDOK) return;

	m_accel_list = dlg.m_accel_list;
	m_accel_list.save_accel_list();

	CreateAccelerator();
}

void CMainFrame::SetAcceleratorToMenu(CMenu *pMenu)
{
	unsigned int	i;
	CString menu_str;
	CString accel_str;
	CString new_menu_str;
	UINT	menu_state;

	for(i = 0; i < (unsigned int)pMenu->GetMenuItemCount(); i++) {
		if(pMenu->GetMenuItemID(i) == 0) continue;	// �Z�p���[�^
		if(pMenu->GetMenuItemID(i) == -1) {			// �|�b�v�A�b�v
			if(pMenu->GetSubMenu(i) != NULL) {
				//SetAcceleratorToMenu(pMenu->GetSubMenu(i));
				continue;
			}
		}

		pMenu->GetMenuString(i, menu_str, MF_BYPOSITION);

		new_menu_str = menu_str;

		// ���ɃV���[�g�J�b�g�L�[���܂܂�Ă���ꍇ�C�폜����
		int tab_pos = new_menu_str.Find('\t');
		if(tab_pos != -1) new_menu_str.Delete(tab_pos, new_menu_str.GetLength() - tab_pos);

		if(m_accel_list.search_accel_str2(pMenu->GetMenuItemID(i), accel_str) == 0) {
			new_menu_str += "\t";
			new_menu_str += accel_str;
		}

		menu_state = pMenu->GetMenuState(i, MF_BYPOSITION);

		if(menu_str.Compare(new_menu_str) != 0) {
			pMenu->ModifyMenu(i, MF_BYPOSITION | MF_STRING | menu_state, 
				pMenu->GetMenuItemID(i), new_menu_str.GetBuffer(0));
		}
	}
}

void CMainFrame::InitAccelerator()
{
	m_accel_list.load_accel_list();
	CreateAccelerator();
}

void CMainFrame::OnInitMenu(CMenu* pMenu) 
{
	CFrameWnd::OnInitMenu(pMenu);
	
	SetAcceleratorToMenu(pMenu);
}

void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
{
	CFrameWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

	if(pPopupMenu->GetMenuItemID(0) == ID_FILE_NEW) {
		if(g_option.max_mru == 0) {
			if(pPopupMenu->RemoveMenu(ID_FILE_MRU_FILE1, MF_BYCOMMAND)) {
				int i;
				for(i = 0; i < 16; i++) {
					pPopupMenu->RemoveMenu(ID_FILE_MRU_FILE1 + i, MF_BYCOMMAND);
				}

				int cnt = pPopupMenu->GetMenuItemCount();
				if(pPopupMenu->GetMenuItemID(cnt - 2) == 0 &&
				   pPopupMenu->GetMenuItemID(cnt - 3) == 0) {
					pPopupMenu->RemoveMenu(cnt - 2, MF_BYPOSITION);
				}
			}
		}
	}
	if(pPopupMenu->GetMenuItemID(0) == ID_EDIT_MODE_1) {
		g_lang_setting_list.AddEditModeMenu(pPopupMenu, g_option.edit_mode);
	}

	// �^�X�N�o�[�̉E�N���b�N���j���[�̂Ƃ��́ASetAcceleratorToMenu�����s���Ȃ�
	if(pPopupMenu->GetMenuItemID(0) == SC_RESTORE) return;
	SetAcceleratorToMenu(pPopupMenu);
}

void CMainFrame::OnUpdateShowTopmost(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck((GetExStyle() & WS_EX_TOPMOST) != 0);
}

void CMainFrame::OnShowTopmost() 
{
	if(GetExStyle() & WS_EX_TOPMOST) {
		SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	} else {
		SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE); 
	}
}

void CMainFrame::OnUpdateViewFileBar(CCmdUI* pCmdUI) 
{
	if((g_file_list_bar.GetStyle() & WS_VISIBLE) == 0) {
		pCmdUI->SetCheck(0);
	} else {
		pCmdUI->SetCheck(1);
	}
}

void CMainFrame::OnViewFileBar() 
{
	if((g_file_list_bar.GetStyle() & WS_VISIBLE) == 0) {
		file_list_bar_init(&g_file_list_bar, g_option.file_list_filter);
	}
	ShowControlBar(&g_file_list_bar, (g_file_list_bar.GetStyle() & WS_VISIBLE) == 0, FALSE);
}

void CMainFrame::SetLangToolBar()
{
	g_lang_setting_list.AddToolBarBtn(&m_wndToolBar2);
}

void CMainFrame::OnMove(int x, int y) 
{
	CFrameWnd::OnMove(x, y);
	
	if(GetActiveDocument()) {
		GetActiveDocument()->UpdateAllViews(NULL, UPD_WINDOW_MOVED, 0);
	}	
}

void CMainFrame::OnUpdateBoxSelect(CCmdUI *pCmdUI)
{
	COeditDoc *pdoc = (COeditDoc *)GetActiveDocument();

	if(pdoc && 
		(pdoc->GetEditData()->get_disp_data()->GetSelectArea()->next_box_select ||
		 pdoc->GetEditData()->get_disp_data()->GetSelectArea()->box_select)) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

void CMainFrame::GetMessageString(UINT nID, CString& rMessage) const
{
	if(nID >= ID_EDIT_MODE_1 && nID <= ID_EDIT_MODE_1 + MAX_LANGUAGE_CNT) {
		rMessage = g_lang_setting_list.GetMessageString(nID - ID_EDIT_MODE_1);
		return;
	}
	CFrameWnd::GetMessageString(nID, rMessage);
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) 
{
	if(pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) {
		if(scm_apply_keyboard_hook(g_sc, pMsg->wParam)) return TRUE;
	}

	if(g_file_list_bar.IsFileTreeActive()) {
		if(pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN ||
			pMsg->wParam == VK_LEFT || pMsg->wParam == VK_RIGHT) return FALSE;
	}

	return CFrameWnd::PreTranslateMessage(pMsg);
}

#define WM_POST_ACTIVATE_APP	WM_USER + 200

void CMainFrame::OnActivateApp(BOOL bActive, DWORD hTask)
{
	CFrameWnd::OnActivateApp(bActive, hTask);

	if(bActive) {
		// ActivateApp�̏�����ɁA�C�x���g�n���h�������s����
		// �����Ŏ��s���Ă��܂��ƁA�G�f�B�^�����N���b�N���ăA�N�e�B�u�ɂ����Ƃ���
		// �͈͑I���J�n�ƂȂ�A�C�x���g�n���h�����Ń��b�Z�[�W�{�b�N�X��\�������ꍇ�ɁA
		// ���b�Z�[�W�{�b�N�X��������Ƃ��͈͑I�𒆂ɂȂ��Ă��܂�
		PostMessage(WM_POST_ACTIVATE_APP, 0, 0);
	}
}

LRESULT CMainFrame::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message) {
	case WM_POST_ACTIVATE_APP:
		scm_call_event_handler(g_sc, SCM_EVENT_ON_ACTIVATE_APP);
		break;
	}
	
	return CFrameWnd::WindowProc(message, wParam, lParam);
}
