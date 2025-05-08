/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef COMBOBOX_UTIL_H_INCLUDED
#define COMBOBOX_UTIL_H_INCLUDED

void ComboBoxKeyboardExtend(CComboBox *combo);

int ComboFindString(CComboBox &combo, CString &text);
int ComboSelectString(CComboBox &combo, CString &text);
CString ComboGetCurSelString(CComboBox &combo);

#endif /* COMBOBOX_UTIL_H_INCLUDED */
