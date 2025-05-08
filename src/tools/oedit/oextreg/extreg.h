
CString MakeFilterExt(TCHAR *ext);
CString GetFileTypeId(TCHAR *ext);
CString GetFileTypeName(TCHAR *ext);
BOOL CheckExplorerRclick(TCHAR *ext);

void UnRegisterShellFileTypes(TCHAR *ext);
void RegisterShellFileTypes(TCHAR *ext, TCHAR *fileTypeId, TCHAR *fileTypeName, BOOL b_explorer_rclick,
	TCHAR *appFileName, TCHAR *iconFileName, int iconIdx, BOOL b_dde);
BOOL CheckRegisterShellFileTypes(TCHAR *ext, TCHAR *fileTypeId);

BOOL UnSetIESourceViewer();
BOOL SetIESourceViewer(TCHAR *path);
BOOL CheckIESourceViewer(TCHAR *path);
