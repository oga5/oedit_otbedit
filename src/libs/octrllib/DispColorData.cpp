/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#include "stdafx.h"
#include "dispcolordata.h"
#include "editdata.h"

CDispColorData::CDispColorData()
{
	InitMemData();
	InitData();
}

CDispColorData::~CDispColorData()
{
	FreeData();
}

void CDispColorData::InitMemData()
{
	m_disp_color_data = NULL;
	m_disp_color_data_cnt = 0;
}

void CDispColorData::InitData()
{
	m_row = -1;
	m_row_len = -1;
	m_row_data = NULL;
}

void CDispColorData::FreeData()
{
	if(m_disp_color_data != NULL) {
		free(m_disp_color_data);
	}
	InitMemData();
}

void CDispColorData::ClearData()
{
	InitData();
}

void CDispColorData::ClearSearched()
{
	int		i;
	for(i = 0; i < m_row_len; i++) {
		UnsetFlg(i, DISP_COLOR_DATA_SEARCHED);
	}
}

void CDispColorData::ClearSelected()
{
	int		i;
	for(i = 0; i < m_row_len; i++) {
		UnsetFlg(i, DISP_COLOR_DATA_SELECTED);
	}
}

void CDispColorData::InitDispColorData(int row_len)
{
	if(m_disp_color_data_cnt <= row_len) {
		m_disp_color_data_cnt = row_len + 1;
		m_disp_color_data_cnt += 1024 - (m_disp_color_data_cnt % 1024);	// 1k単位で確保する

		m_disp_color_data = (struct _st_disp_color_data *)realloc(m_disp_color_data,
			m_disp_color_data_cnt * sizeof(struct _st_disp_color_data));
	}

	// データを初期化
	memset(m_disp_color_data, 0, sizeof(struct _st_disp_color_data) * row_len);

	m_row = -1;
	m_row_len = row_len;
}

void CDispColorData::SetRowData(CEditRowData *row_data, int row)
{
	m_row_data = row_data;
	m_row = row;
	m_row_data->unset_data_flg(ROW_DATA_UPDATE_FLG);
}

BOOL CDispColorData::CheckReuse(CEditRowData *row_data, int row)
{
	if(m_row_data == row_data && m_row == row && 
		!m_row_data->get_data_flg(ROW_DATA_UPDATE_FLG)) return TRUE;
	return FALSE;
}

CDispColorDataPool::CDispColorDataPool()
{
	m_pool_size = 0;
	m_disp_color_data_list = NULL;
	SetPoolSize(1);
}

CDispColorDataPool::~CDispColorDataPool()
{
	delete[] m_disp_color_data_list;
}

CDispColorData *CDispColorDataPool::GetDispColorData(int row)
{
	if(row >= m_edit_data->get_row_cnt()) return NULL;

	int idx = GetIdx(row);

	if(m_disp_color_data_list[idx].CheckReuse(
			m_edit_data->get_edit_row_data(row), row) == TRUE) {
		return &m_disp_color_data_list[idx];
	}

	return NULL;
}

CDispColorData *CDispColorDataPool::GetNewDispColorData(int row)
{
	int idx = GetIdx(row);

	m_disp_color_data_list[idx].InitDispColorData(m_edit_data->get_row_len(row) + 1);

	return &m_disp_color_data_list[idx];
}

void CDispColorDataPool::ClearAllData()
{
	int		i;

	for(i = 0; i < m_pool_size; i++) {
		m_disp_color_data_list[i].ClearData();		
	}
}

void CDispColorDataPool::ClearAllSearched()
{
	int		i;

	for(i = 0; i < m_pool_size; i++) {
		m_disp_color_data_list[i].ClearSearched();		
	}
}

void CDispColorDataPool::SetPoolSize(int size)
{
	delete[] m_disp_color_data_list;
	m_pool_size = size;
	m_disp_color_data_list = new CDispColorData[size];
}
