/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef __INTARRAY_INCLUDED__
#define __INTARRAY_INCLUDED__

class CIntArray
{
	int		*m_data;
	int		m_alloc_cnt;

	void FreeData();
	int CIntArray::calc_next_row_cnt(int cnt);

public:
	CIntArray();
	~CIntArray();

	int GetAllocedSize() { return m_alloc_cnt; }
	BOOL AllocData(int data_cnt);

	void SetData(int idx, int data) { m_data[idx] = data; }
	int GetData(int idx) { return m_data[idx]; }
};

#endif __INTARRAY_INCLUDED__
