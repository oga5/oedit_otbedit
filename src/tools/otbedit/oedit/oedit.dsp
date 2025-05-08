# Microsoft Developer Studio Project File - Name="oedit" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=oedit - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "oedit.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "oedit.mak" CFG="oedit - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "oedit - Win32 Release" ("Win32 (x86) Application" 用)
!MESSAGE "oedit - Win32 Debug" ("Win32 (x86) Application" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "oedit - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../../exe/Release/otbedit"
# PROP Intermediate_Dir "../../../../obj/Release/otbedit"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /O2 /I "../../../libs/ostrutil" /I "../../../libs/octrllib" /I "../../../libs/ofileutil" /I "../../common" /I "../../common_editor" /I "./" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 imm32.lib version.lib ostrutil.lib octrllib.lib ofileutil.lib /nologo /subsystem:windows /machine:I386 /out:"../../../../exe/Release/otbedit/otbedit.exe" /libpath:"../../../../lib/release"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=mkdir ..\..\..\..\exe\Release\otbedit\data	copy ..\data\*.txt            ..\..\..\..\exe\Release\otbedit\data	copy ..\data\*.cfg            ..\..\..\..\exe\Release\otbedit\data	mkdir ..\..\..\..\exe\Release\otbedit\doc	copy ..\doc\*.txt           ..\..\..\..\exe\Release\otbedit\doc	copy ..\..\..\libs\common_doc\*.txt ..\..\..\..\exe\Release\otbedit\doc	mkdir ..\..\..\..\exe\Release\otbedit\scmlib	copy ..\scmlib\*.scm ..\..\..\..\exe\Release\otbedit\scmlib	copy ..\..\..\libs\common_scmlib\*.* ..\..\..\..\exe\Release\otbedit\scmlib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "oedit - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../exe/Debug/otbedit"
# PROP Intermediate_Dir "../../../../obj/Debug/otbedit"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /ZI /Od /I "../../../libs/ostrutil" /I "../../../libs/octrllib" /I "../../../libs/ofileutil" /I "../../common" /I "../../common_editor" /I "./" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 imm32.lib version.lib ostrutil.lib octrllib.lib ofileutil.lib /nologo /subsystem:windows /profile /debug /machine:I386 /out:"../../../../exe/Debug/otbedit/otbedit.exe" /libpath:"../../../../lib/debug"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=mkdir ..\..\..\..\exe\Debug\otbedit\data	copy ..\data\*.txt  ..\..\..\..\exe\Debug\otbedit\data	copy ..\data\*.cfg  ..\..\..\..\exe\Debug\otbedit\data	mkdir ..\..\..\..\exe\Debug\otbedit\doc	copy ..\doc\*.txt         ..\..\..\..\exe\Debug\otbedit\doc	mkdir ..\..\..\..\exe\Debug\otbedit\scmlib	copy ..\..\..\libs\common_doc\*.txt ..\..\..\..\exe\Debug\otbedit\doc	copy ..\scmlib\*.scm ..\..\..\..\exe\Debug\otbedit\scmlib	copy ..\..\..\libs\common_scmlib\*.* ..\..\..\..\exe\Debug\otbedit\scmlib
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "oedit - Win32 Release"
# Name "oedit - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AboutDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AcceleratorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AccelList.cpp
# End Source File
# Begin Source File

SOURCE=.\ColorGridWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\CommonUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorOptionPage.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorOptionPage2.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\ExtFileDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\FileFormatDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FileListBar.cpp
# End Source File
# Begin Source File

SOURCE=.\FileTreeCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\FilterDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\greputil.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\kbmacro.cpp
# End Source File
# Begin Source File

SOURCE=.\LineJumpDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\oedit.cpp
# End Source File
# Begin Source File

SOURCE=.\oedit.rc
# End Source File
# Begin Source File

SOURCE=..\..\common_editor\oeditapp2.cpp
# End Source File
# Begin Source File

SOURCE=.\oeditDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\oeditView.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\PrintDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\QueryCloseDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common_editor\ReplaceDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\scm_macro.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\scm_macro_common.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common_editor\SearchDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SelectColorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SetupPage.cpp
# End Source File
# Begin Source File

SOURCE=.\SetupPage2.cpp
# End Source File
# Begin Source File

SOURCE=.\sizecbar.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\..\common\TagJumpCommon.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AboutDlg.h
# End Source File
# Begin Source File

SOURCE=.\AcceleratorDlg.h
# End Source File
# Begin Source File

SOURCE=.\AccelList.h
# End Source File
# Begin Source File

SOURCE=.\appinfo.h
# End Source File
# Begin Source File

SOURCE=.\ColorGridWnd.h
# End Source File
# Begin Source File

SOURCE=.\command_list.h
# End Source File
# Begin Source File

SOURCE=.\CommonUtil.h
# End Source File
# Begin Source File

SOURCE=.\default_accel.h
# End Source File
# Begin Source File

SOURCE=.\editdocdata.h
# End Source File
# Begin Source File

SOURCE=.\EditorOptionPage.h
# End Source File
# Begin Source File

SOURCE=.\EditorOptionPage2.h
# End Source File
# Begin Source File

SOURCE=..\..\common\ExtFileDialog.h
# End Source File
# Begin Source File

SOURCE=.\FileFormatDlg.h
# End Source File
# Begin Source File

SOURCE=.\FileListBar.h
# End Source File
# Begin Source File

SOURCE=.\FileTreeCtrl.h
# End Source File
# Begin Source File

SOURCE=.\FilterDlg.h
# End Source File
# Begin Source File

SOURCE=.\global.h
# End Source File
# Begin Source File

SOURCE=..\..\common\greputil.h
# End Source File
# Begin Source File

SOURCE=..\..\common\kbmacro.h
# End Source File
# Begin Source File

SOURCE=.\LineJumpDlg.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\oedit.h
# End Source File
# Begin Source File

SOURCE=.\oeditDoc.h
# End Source File
# Begin Source File

SOURCE=.\oeditView.h
# End Source File
# Begin Source File

SOURCE=.\OptionSheet.h
# End Source File
# Begin Source File

SOURCE=.\PrintDlg.h
# End Source File
# Begin Source File

SOURCE=.\QueryCloseDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\common_editor\ReplaceDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=..\..\common\scm_macro.h
# End Source File
# Begin Source File

SOURCE=..\..\common\scm_macro_common.h
# End Source File
# Begin Source File

SOURCE=.\ScrollWnd.h
# End Source File
# Begin Source File

SOURCE=..\..\common_editor\SearchDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\common_editor\SearchDlgData.h
# End Source File
# Begin Source File

SOURCE=.\SelectColorDlg.h
# End Source File
# Begin Source File

SOURCE=.\SetupPage.h
# End Source File
# Begin Source File

SOURCE=.\SetupPage2.h
# End Source File
# Begin Source File

SOURCE=.\sizecbar.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\..\common\TagJumpCommon.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\res\close_bt.bmp
# End Source File
# Begin Source File

SOURCE=.\res\close_btn.bmp
# End Source File
# Begin Source File

SOURCE=.\res\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\res\idr_main.ico
# End Source File
# Begin Source File

SOURCE=.\res\oedit.rc2
# End Source File
# Begin Source File

SOURCE=.\res\spoit_cu.cur
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar1_256.bmp
# End Source File
# End Group
# Begin Group "doc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\doc\keyword.txt
# End Source File
# Begin Source File

SOURCE=..\doc\languages.txt
# End Source File
# Begin Source File

SOURCE=..\doc\macro_otbedit.txt
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\common_doc\macro_ref.txt
# End Source File
# Begin Source File

SOURCE=..\doc\option.txt
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\common_doc\regexp.txt
# End Source File
# Begin Source File

SOURCE=..\doc\shortcutkey.txt
# End Source File
# End Group
# Begin Group "data"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\data\cpp.cfg
# End Source File
# Begin Source File

SOURCE=..\data\cpp.txt
# End Source File
# Begin Source File

SOURCE=..\data\html.cfg
# End Source File
# Begin Source File

SOURCE=..\data\html.txt
# End Source File
# Begin Source File

SOURCE=..\data\java.cfg
# End Source File
# Begin Source File

SOURCE=..\data\java.txt
# End Source File
# Begin Source File

SOURCE=..\data\languages_default.txt
# End Source File
# Begin Source File

SOURCE=..\data\perl.cfg
# End Source File
# Begin Source File

SOURCE=..\data\perl.txt
# End Source File
# Begin Source File

SOURCE=..\data\sql.cfg
# End Source File
# Begin Source File

SOURCE=..\data\sql.txt
# End Source File
# Begin Source File

SOURCE=..\data\text.cfg
# End Source File
# Begin Source File

SOURCE=..\data\text.txt
# End Source File
# End Group
# Begin Group "scmlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\libs\common_scmlib\init.scm
# End Source File
# Begin Source File

SOURCE=..\scmlib\otbedit.scm
# End Source File
# Begin Source File

SOURCE="..\..\..\libs\common_scmlib\srfi-11.scm"
# End Source File
# End Group
# Begin Source File

SOURCE=.\ChangeLog.txt
# End Source File
# End Target
# End Project
