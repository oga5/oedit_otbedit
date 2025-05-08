/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef __LIST_CTRL_UTIL_H_INCLUDED__
#define __LIST_CTRL_UTIL_H_INCLUDED__

#include <afxcmn.h>

int ListCtrl_GetColumnCount(CListCtrl *p_list);
CString ListCtrl_GetColumnName(CListCtrl *p_list, int col);

void ListCtrlSwapData(CListCtrl *p_list, int r1, int r2);

void ListCtrl_SelectItem(CListCtrl *p_list, int i);
void ListCtrl_SelectAll(CListCtrl* p_list);

void ListCtrl_EnableMouseHWheel(CListCtrl* p_list);

#endif __LIST_CTRL_UTIL_H_INCLUDED__
