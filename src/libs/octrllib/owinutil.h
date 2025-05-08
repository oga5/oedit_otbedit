/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef OWIN_UTIL_H
#define OWIN_UTIL_H

int GetPopupMenuHeight(HMENU hmenu);
BOOL Set256ColorToolBar(CToolBar *tool_bar, UINT bitmap_id);
BOOL SetClipboardText(const TCHAR *txt, TCHAR *msg_buf);

#endif OWIN_UTIL_H
