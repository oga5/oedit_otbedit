/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "CodeAssistListMaker.h"


BOOL CCodeAssistListMaker::MakeList(CCodeAssistData *assist_data, CEditData *edit_data)
{
	assist_data->InitData(FALSE);

	assist_data->AddRow();
	assist_data->SetData(0, 0, _T("AAA"));
	assist_data->SetData(0, 1, _T("VARCHAR2(10)"));

	assist_data->AddRow();
	assist_data->SetData(1, 0, _T("BBB"));
	assist_data->SetData(1, 1, _T("VARCHAR2(2)"));

	assist_data->AddRow();
	assist_data->SetData(2, 0, _T("CCC"));
	assist_data->SetData(2, 1, _T("VARCHAR2(9)"));

	return TRUE;
}
