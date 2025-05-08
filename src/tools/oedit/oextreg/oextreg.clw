; CLW ファイルは MFC ClassWizard の情報を含んでいます。

[General Info]
Version=1
LastClass=COextregDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "oextreg.h"

ClassCount=4
Class1=COextregApp
Class2=COextregDlg
Class3=CAboutDlg

ResourceCount=4
Resource1=IDD_OEXTREG_DIALOG
Resource2=IDR_MAINFRAME
Resource3=IDD_ABOUTBOX
Class4=CAddExtDlg
Resource4=IDD_ADD_EXT

[CLS:COextregApp]
Type=0
HeaderFile=oextreg.h
ImplementationFile=oextreg.cpp
Filter=N
BaseClass=CWinApp
VirtualFilter=AC
LastObject=COextregApp

[CLS:COextregDlg]
Type=0
HeaderFile=oextregDlg.h
ImplementationFile=oextregDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=IDC_IE_SOURCE_VIEWER

[CLS:CAboutDlg]
Type=0
HeaderFile=oextregDlg.h
ImplementationFile=oextregDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=CAboutDlg

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC_VERSION,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_OEXTREG_DIALOG]
Type=1
Class=COextregDlg
ControlCount=9
Control1=IDC_EXT_LIST,SysListView32,1350631453
Control2=IDC_BTN_ADD,button,1342242816
Control3=IDC_BTN_UPDATE,button,1476460544
Control4=IDC_BTN_DELETE,button,1476460544
Control5=IDC_CHECK_TXT,button,1342246915
Control6=IDC_CHECK_JAVA,button,1342246915
Control7=IDCANCEL,button,1342242816
Control8=IDC_STATIC,button,1342177287
Control9=IDC_IE_SOURCE_VIEWER,button,1342242819

[DLG:IDD_ADD_EXT]
Type=1
Class=CAddExtDlg
ControlCount=9
Control1=IDOK,button,1476460545
Control2=IDCANCEL,button,1342242816
Control3=IDC_EDIT_EXT,edit,1350631552
Control4=IDC_EDIT_FILE_TYPE_NAME,edit,1350631552
Control5=IDC_CHECK_EXPLORER_RCLICK,button,1342242819
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308352
Control8=IDC_STATIC,static,1342308352
Control9=IDC_STATIC,static,1342308352

[CLS:CAddExtDlg]
Type=0
HeaderFile=AddExtDlg.h
ImplementationFile=AddExtDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_CHECK_EXPLORER_RCLICK

