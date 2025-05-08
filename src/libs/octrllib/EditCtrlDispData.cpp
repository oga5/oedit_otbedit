
#include "stdafx.h"
#include "EditCtrlDispData.h"
/*
CEditCtrlDispData::CEditCtrlDispData() : 
	m_data_buffer(NULL), m_alloc_cnt(0), m_alloc_hw_mark(0), m_alloced_size(0)
{
}

CEditCtrlDispData::~CEditCtrlDispData()
{
	FreeData();
}

void CEditCtrlDispData::FreeData()
{
	if(m_data_buffer == NULL) return;

	int		i;
	for(i = 0; i < m_alloc_hw_mark; i++) {
		if(m_data_buffer[i] != NULL) {
			free(m_data_buffer[i]);
			m_data_buffer[i] = NULL;
		}
	}

	free(m_data_buffer);
	m_data_buffer = NULL;

	m_alloc_cnt = 0;
	m_alloc_hw_mark = 0;
	m_alloced_size = 0;
}

BOOL CEditCtrlDispData::AllocData(int data_cnt)
{
	int i;
	int need_size = data_cnt / EDIT_CTRL_DISP_DATA_ALLOC_CNT + 1;

	if(need_size > m_alloc_cnt) {
		int alloc_cnt = need_size * 2;
		if(alloc_cnt < 32) alloc_cnt = 32;

		m_data_buffer = (struct edit_ctrl_disp_data_st **)realloc(m_data_buffer,
			sizeof(struct edit_ctrl_disp_data_st *) * alloc_cnt);
		if(m_data_buffer == NULL) return FALSE;

		for(i = m_alloc_cnt; i < alloc_cnt; i++) {
			m_data_buffer[i] = NULL;
		}

		m_alloc_cnt = alloc_cnt;
	}

	for(i = m_alloc_hw_mark; i < need_size; i++) {
		if(m_data_buffer[i] == NULL) {
			m_data_buffer[i] = (struct edit_ctrl_disp_data_st *)malloc(
				sizeof(struct edit_ctrl_disp_data_st) * EDIT_CTRL_DISP_DATA_ALLOC_CNT);
			if(m_data_buffer[i] == NULL) return FALSE;
			m_alloc_hw_mark = i + 1;
		}
	}

	m_alloced_size = m_alloc_hw_mark * EDIT_CTRL_DISP_DATA_ALLOC_CNT;

	return TRUE;
}

*/

CEditCtrlDispData::CEditCtrlDispData() : 
	m_row(NULL), m_offset(NULL), m_alloc_cnt(0)
{
}

CEditCtrlDispData::~CEditCtrlDispData()
{
	FreeData();
}

void CEditCtrlDispData::FreeData()
{
	if(m_row != NULL) {
		free(m_row);
		m_row = NULL;
	}
	if(m_offset != NULL) {
		free(m_offset);
		m_offset = NULL;
	}
	m_alloc_cnt = 0;
}

BOOL CEditCtrlDispData::AllocData(int data_cnt)
{
	if(data_cnt < m_alloc_cnt) return TRUE;

	int alloc_cnt = m_alloc_cnt;
	if(alloc_cnt == 0) alloc_cnt = 1024;
	for(; alloc_cnt <= data_cnt;) alloc_cnt *= 2;

	m_row = (int *)realloc(m_row, sizeof(int) * alloc_cnt);
	if(m_row == NULL) return FALSE;
	m_offset = (int *)realloc(m_offset, sizeof(int) * alloc_cnt);
	if(m_offset == NULL) return FALSE;

	m_alloc_cnt = alloc_cnt;

	return TRUE;
}
