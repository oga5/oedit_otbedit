/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "IntArray.h"


CIntArray::CIntArray() : m_data(0), m_alloc_cnt(0)
{
}

CIntArray::~CIntArray()
{
	FreeData();
}

void CIntArray::FreeData()
{
	if(m_data != NULL) {
		free(m_data);
		m_data = NULL;
	}
	m_alloc_cnt = 0;
}

BOOL CIntArray::AllocData(int data_cnt)
{
	if(data_cnt < m_alloc_cnt) return TRUE;

	int alloc_cnt = m_alloc_cnt;
	if(alloc_cnt == 0) alloc_cnt = 1024;
	for(; alloc_cnt <= data_cnt;) alloc_cnt = calc_next_row_cnt(alloc_cnt);

	m_data = (int *)realloc(m_data, sizeof(int) * alloc_cnt);
	if(m_data == NULL) return FALSE;

	m_alloc_cnt = alloc_cnt;

	return TRUE;
}

int CIntArray::calc_next_row_cnt(int cnt)
{
	if(cnt < (1024 * 1024)) return cnt * 2;
	return cnt + (1024 * 1024);
}
