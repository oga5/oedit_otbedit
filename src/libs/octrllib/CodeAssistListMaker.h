/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef _CODE_ASSIST_LIST_MAKER_H_INCLUDE
#define _CODE_ASSIST_LIST_MAKER_H_INCLUDE

#include "CodeAssistData.h"
#include "EditData.h"

class CCodeAssistListMaker
{
private:

public:
	CCodeAssistListMaker() {}
	virtual ~CCodeAssistListMaker() {}

	virtual BOOL MakeList(CCodeAssistData *assist_data, CEditData *edit_data);
};

#endif _CODE_ASSIST_LIST_MAKER_H_INCLUDE
