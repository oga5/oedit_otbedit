
#ifndef __CEDIT_CTRL_DISP_DATA_H_INCLUDED__
#define __CEDIT_CTRL_DISP_DATA_H_INCLUDED__

#define EDIT_CTRL_DISP_DATA_ALLOC_CNT		1024
/*
struct edit_ctrl_disp_data_st {
	int		row;
	int		offset;
};

class CEditCtrlDispData {
private:
	struct edit_ctrl_disp_data_st **m_data_buffer;
	int		m_alloc_cnt;
	int		m_alloc_hw_mark;
	int		m_alloced_size;

	struct edit_ctrl_disp_data_st *GetDispData(int idx) {
		return m_data_buffer[idx / EDIT_CTRL_DISP_DATA_ALLOC_CNT] + idx % EDIT_CTRL_DISP_DATA_ALLOC_CNT;
	}
	void FreeData();

public:
	CEditCtrlDispData();
	~CEditCtrlDispData();

	int GetAllocedSize() { return m_alloced_size; }
	BOOL AllocData(int data_cnt);

	void SetRow(int idx, int row) { GetDispData(idx)->row = row; }
	void SetOffset(int idx, int offset) { GetDispData(idx)->offset = offset; }

	int GetRow(int idx) { return GetDispData(idx)->row; }
	int GetOffset(int idx) { return GetDispData(idx)->offset; }
};
*/

class CEditCtrlDispData {
private:
	int		*m_row;
	int		*m_offset;
	int		m_alloc_cnt;

	void FreeData();

public:
	CEditCtrlDispData();
	~CEditCtrlDispData();

	int GetAllocedSize() { return m_alloc_cnt; }
	BOOL AllocData(int data_cnt);

	void SetRow(int idx, int row) { m_row[idx] = row; }
	void SetOffset(int idx, int offset) { m_offset[idx] = offset; }

	int GetRow(int idx) { return m_row[idx]; }
	int GetOffset(int idx) { return m_offset[idx]; }
};

#endif __CEDIT_CTRL_DISP_DATA_H_INCLUDED__

